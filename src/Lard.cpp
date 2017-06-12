#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <openssl/ssl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Buffer.hpp"
#include "Debug.hpp"
#include "FileMap.hpp"
#include "Filesystem.hpp"
#include "glue.h"
#include "Lard.hpp"

static set_str* ptr_set_str;
static map_strsize* ptr_map_strsize;

static int checkarg( int argc, char** argv, const char* arg )
{
    for( int i=0; i<argc; i++ )
    {
        if( strcmp( argv[i], arg ) == 0 ) return i;
    }
    return -1;
}

Lard::Lard()
{
    auto prefix = SetupGitDirectory();
    if( prefix ) m_prefix = prefix;
    m_gitdir = GetGitDir();
    m_objdir = m_gitdir + "/fat/objects";

    DBGPRINT( "Prefix: " << m_prefix );
    DBGPRINT( "Git dir: " << m_gitdir );
    DBGPRINT( "Obj dir: " << m_objdir );
}

Lard::~Lard()
{
}

void Lard::Init()
{
    Setup();
    if( IsInitDone() )
    {
        printf( "Git lard already configured, check configuration in .git/config\n" );
        printf( "    Note: migration from git-fat may require changing executable name.\n" );
    }
    else
    {
        SetConfigKey( "filter.fat.clean", "git-fat filter-clean" );
        SetConfigKey( "filter.fat.smudge", "git-fat filter-smudge" );
    }
}

static std::vector<const char*> RelativeComplement( const set_str& s1, const set_str& s2 )
{
    std::vector<const char*> ret;
    for( auto& v : s1 )
    {
        if( s2.find( v ) == s2.end() )
        {
            ret.emplace_back( v );
        }
    }
    return ret;
}

void Lard::Status( int argc, char** argv )
{
    Setup();
    const auto catalog = ListDirectory( m_objdir );
    DBGPRINT( "Fat objects: " << catalog.size() );
    bool all = checkarg( argc, argv, "--all" ) != -1;
    const auto referenced = ReferencedObjects( all, nullptr );
    DBGPRINT( "Referenced objects: " << referenced.size() );

    const auto garbage = RelativeComplement( catalog, referenced );
    const auto orphans = RelativeComplement( referenced, catalog );

    if( all )
    {
        for( auto& v : referenced )
        {
            printf( "%s\n", v );
        }
    }
    if( !orphans.empty() )
    {
        printf( "Orphan objects:\n" );
        for( auto& v : orphans )
        {
            printf( "    %s\n", v );
        }
    }
    if( !garbage.empty() )
    {
        printf( "Garbage objects:\n" );
        for( auto& v : garbage )
        {
            printf( "    %s\n", v );
        }
    }
}

void Lard::GC()
{
    const auto catalog = ListDirectory( m_objdir );
    const auto referenced = ReferencedObjects( false, nullptr );
    const auto garbage = RelativeComplement( catalog, referenced );
    printf( "Unreferenced objects to remove: %d\n", garbage.size() );
    for( auto& v : garbage )
    {
        auto fn = GetObjectFn( v );
        printf( "%10d %s\n", GetFileSize( fn ), v );
        unlink( fn );
    }
}

void Lard::Verify()
{
    std::vector<std::pair<const char*, const char*>> corrupted;
    const auto catalog = ListDirectory( m_objdir );
    for( auto& v : catalog )
    {
        auto fn = GetObjectFn( v );
        FileMap<char> f( fn );
        auto sha1 = CalcSha1( f, f.DataSize() );
        if( strncmp( v, sha1, 40 ) != 0 )
        {
            corrupted.emplace_back( v, Buffer::Store( sha1, 40 ) );
        }
    }
    if( !corrupted.empty() )
    {
        printf( "Corrupted objects: %d\n", corrupted.size() );
        for( auto& v : corrupted )
        {
            printf( "%s data hash is %s\n", v.first, v.second );
        }
        exit( 1 );
    }
}

static std::vector<const char*>* ptr_vec_str;

void Lard::Find( int argc, char** argv )
{
    assert( argc > 0 );
    const auto maxsize = atoi( argv[0] );
    const auto blobsizes = GenLargeBlobs( maxsize );
    const auto time0 = std::chrono::high_resolution_clock::now();

    std::vector<const char*> revlist;
    ptr_vec_str = &revlist;

    auto cb = []( char* ptr ) {
        ptr_vec_str->emplace_back( Buffer::Store( ptr ) );
    };

    rev_info* revs = NewRevInfo();
    AddRevAll( revs );
    PrepareRevWalk( revs );
    GetCommitList( revs, cb );
    FreeRevs( revs );

    DBGPRINT( "Rev walk found " << revlist.size() << " commits" );

    printf( "TODO\n" );
}

// file content -> fat-sha-magic
void Lard::Clean()
{
    Setup();
    FilterClean( stdin, stdout );
}

void Lard::FilterClean( FILE* in, FILE* out )
{
    size_t size = 0;

    enum { ChunkSize = 4 * 1024 * 1024 };
    char* buf = new char[ChunkSize];
    size_t len = fread( buf, 1, ChunkSize, in );
    if( len == GitFatMagic )
    {
        const char* sha1;
        if( Decode( buf, sha1, size ) )
        {
            fwrite( buf, 1, GitFatMagic, out );
            delete[] buf;
            return;
        }
    }

    std::vector<const char*> payload;

    SHA_CTX ctx;
    SHA1_Init( &ctx );

    size += len;
    SHA1_Update( &ctx, buf, len );
    payload.emplace_back( buf );
    while( len == ChunkSize )
    {
        buf = new char[ChunkSize];
        len = fread( buf, 1, ChunkSize, in );
        size += len;
        SHA1_Update( &ctx, buf, len );
        payload.emplace_back( buf );
    }

    unsigned char sha1[20];
    SHA1_Final( sha1, &ctx );

    const char* hex = Sha1ToHex( sha1 );
    const char* encoded = Encode( hex, size );
    fwrite( encoded, 1, GitFatMagic, out );

    auto path = GetObjectFn( hex );
    if( !Exists( path ) )
    {
        DBGPRINT( "Caching file to " << path );
        FILE* cache = fopen( path, "wb" );
        assert( cache );
        for( auto& ptr : payload )
        {
            auto s = std::min<size_t>( size, ChunkSize );
            fwrite( ptr, 1, s, cache );
            size -= s;
        }
        fclose( cache );
        assert( size == 0 );
    }

    for( auto& ptr : payload )
    {
        delete[] ptr;
    }
}

// fat-sha-magic -> file content
void Lard::Smudge()
{
    enum { ChunkSize = 64 * 1024 };

    Setup();
    char buf[ChunkSize];
    auto len = fread( buf, 1, ChunkSize, stdin );

    const char* sha1;
    size_t size;
    if( len == GitFatMagic && Decode( buf, sha1, size ) )
    {
        auto fn = GetObjectFn( sha1 );
        FILE* f = fopen( fn, "rb" );
        if( f )
        {
            size_t read_size = 0;
            do
            {
                len = fread( buf, 1, ChunkSize, f );
                fwrite( buf, 1, len, stdout );
                read_size += len;
            }
            while( len == ChunkSize );

            fclose( f );

            if( size == read_size )
            {
                DBGPRINT( "git-lard filter-smudge: restoring from " << fn );
            }
            else
            {
                DBGPRINT( "git-lard filter-smudge: invalid size of " << fn << " (expected " << size << ", got " << read_size <<")" );
            }
        }
        else
        {
            DBGPRINT( "git-lard filter-smudge: fat object missing " << fn );
        }
    }
    else
    {
        fwrite( buf, 1, len, stdout );
        do
        {
            len = fread( buf, 1, ChunkSize, stdin );
            fwrite( buf, 1, len, stdout );
        }
        while( len == ChunkSize );

        DBGPRINT( "git-lard filter-smudge: not a managed file" );
    }
}

void Lard::Checkout()
{
    Checkout( true );
}

void Lard::Checkout( bool showOrphans )
{
    static char objbuf[1024];
    memcpy( objbuf, m_objdir.c_str(), m_objdir.size() );
    static char* objbufptr = objbuf + m_objdir.size();
    *objbufptr++ = '/';
    objbufptr[40] = '\0';

    AssertInitDone();
    ParsePathspec( m_prefix.c_str() );
    if( ReadCache() < 0 )
    {
        fprintf( stderr, "index file corrupt\n" );
        exit( 1 );
    }

    auto cb = []( const char* fn, const char* localFn ) {
        struct stat sb;
        if( stat( fn, &sb ) != 0 ) return;
        if( sb.st_size != GitFatMagic ) return;
        char buf[GitFatMagic];
        FILE* f = fopen( fn, "rb" );
        verify( fread( buf, 1, GitFatMagic, f ) == GitFatMagic );
        fclose( f );
        size_t size;
        const char* sha1;
        if( !Decode( buf, sha1, size ) ) return;

        memcpy( objbufptr, sha1, 40 );
        f = fopen( objbuf, "rb" );
        if( f )
        {
            FILE* out = fopen( fn, "wb" );
            if( out )
            {
                // TODO: Original git-fat implementation restores file permissions here
                printf( "Restoring %s -> %s\n", sha1, localFn );
                enum { ChunkSize = 64 * 1024 };
                char buf[ChunkSize];
                size_t len;
                do
                {
                    len = fread( buf, 1, ChunkSize, f );
                    fwrite( buf, 1, len, out );
                }
                while( len == ChunkSize );
                fclose( out );
            }
            else
            {
                printf( "Cannot open %s for writing during %s restore\n", localFn, sha1 );
            }
            fclose( f );
        }
        else
        {
            printf( "Data unavailable: %s %s\n", sha1, localFn );
        }
    };

    ListFiles( cb );
}

void Lard::Pull( int argc, char** argv )
{
    Setup();

    bool all = false;
    const char* rev = nullptr;

    int n;
    for( int n=0; n<argc; n++ )
    {
        if( *argv[n] == '-' )
        {
            if( strcmp( argv[n]+1, "-" ) == 0 )
            {
                n++;
                break;
            }
            if( strcmp( argv[n]+1, "-all" ) == 0 )
            {
                all = true;
            }
        }
        else
        {
            auto _rev = GetSha1( argv[n] );
            if( _rev ) rev = _rev;
        }
    }

    DBGPRINT( "Rev: " << ( rev ? rev : "(none)" ) << ", all: " << all );

    const auto catalog = ListDirectory( m_objdir );
    const auto referenced = ReferencedObjects( all, rev );
    const auto orphans = RelativeComplement( referenced, catalog );
    // TODO: match orphans against patterns (in argv, if n<argc)

    const char* cmd = GetRsyncCommand( false );
    DBGPRINT( "Executing: " << cmd );

    // fork

    Checkout();
}

void Lard::Setup()
{
    CreateDirStruct( m_objdir );
}

bool Lard::IsInitDone()
{
    return CheckIfConfigKeyExists( "filter.fat.clean" ) == 0 || CheckIfConfigKeyExists( "filter.fat.smudge" ) == 0;
}

void Lard::AssertInitDone()
{
    if( !IsInitDone() )
    {
        fprintf( stderr, "fatal: git-lard is not yet configured in this repository.\nRun \"git lard init\" to configure.\n" );
        exit( 1 );
    }
}

const char* Lard::CalcSha1( const char* ptr, size_t size ) const
{
    unsigned char sha1[20];
    SHA1( (const unsigned char*)ptr, size, sha1 );
    return Sha1ToHex( sha1 );
}

const char* Lard::Sha1ToHex( const unsigned char sha1[20] ) const
{
    static char ret[41] = {};
    for( int i=0; i<20; i++ )
    {
        sprintf( ret+i*2, "%02x", sha1[i] );
    }
    return ret;
}

bool Lard::Decode( const char* data, const char*& sha1, size_t& size, bool errOnFail )
{
    enum { MagicLen = 12 };
    static const char magic[MagicLen+1] = "#$# git-fat ";
    static char retbuf[41] = {};

    if( memcmp( data, magic, MagicLen ) == 0 )
    {
        memcpy( retbuf, data + MagicLen, 40 );
        sha1 = retbuf;
        size = atoi( data + MagicLen + 41 );
        DBGPRINT( "Decoded git-fat string: hash " << sha1 << ", size: " << size );
        return true;
    }
    else if( errOnFail )
    {
        char buf[128];
        sprintf( buf, "Could not decode %.*s", GitFatMagic, data );
        DBGPRINT( buf );
        exit( 1 );
    }
    else
    {
        return false;
    }
}

const char* Lard::Encode( const char* sha1, size_t size )
{
    static char ret[GitFatMagic+1];
    sprintf( ret, "#$# git-fat %s %20d\n", sha1, size );
    return ret;
}

const char* Lard::GetObjectFn( const char* sha1 ) const
{
    static char fn[1024];
    sprintf( fn, "%s/%s", m_objdir.c_str(), sha1 );
    return fn;
}

const char* Lard::GetRsyncCommand( bool push ) const
{
    static char cmd[2048] = "rsync --progress --ignore-existing --from0 --files-from=-";
    static const auto initLen = strlen( cmd );
    char* ptr = cmd + initLen;

    std::string cfgPath = std::string( GetGitWorkTree() ) + "/.gitfat";

    const char *remote, *sshuser = nullptr, *sshport = nullptr, *options = nullptr;
    auto cs = NewConfigSet();
    ConfigSetAddFile( cs, cfgPath.c_str() );
    if( !GetConfigSetKey( "rsync.remote", &remote, cs ) )
    {
        fprintf( stderr, "No rsync.remote in %s", cfgPath.c_str() );
        exit( 1 );
    }
    GetConfigSetKey( "rsync.sshport", &sshport, cs );
    GetConfigSetKey( "rsync.sshuser", &sshuser, cs );
    GetConfigSetKey( "rsync.options", &options, cs );

    if( sshport || sshuser )
    {
        memcpy( ptr, " --rssh=ssh", 11 );
        ptr += 11;
        if( sshport )
        {
            memcpy( ptr, "\\ -p\\ ", 6 );
            ptr += 6;
            auto len = strlen( sshport );
            memcpy( ptr, sshport, len );
            ptr += len;
        }
        if( sshuser )
        {
            memcpy( ptr, "\\ -l\\ ", 6 );
            ptr += 6;
            auto len = strlen( sshuser );
            memcpy( ptr, sshuser, len );
            ptr += len;
        }
    }

    if( options )
    {
        *ptr++ = ' ';
        auto len = strlen( options );
        memcpy( ptr, options, len );
        ptr += len;
    }

    *ptr++ = ' ';
    if( push )
    {
        auto len = m_objdir.size();
        memcpy( ptr, m_objdir.c_str(), len );
        ptr += len;
        *ptr++ = '/';
        *ptr++ = ' ';
        len = strlen( remote );
        memcpy( ptr, remote, len );
        ptr += len;
        *ptr++ = '/';
    }
    else
    {
        auto len = strlen( remote );
        memcpy( ptr, remote, len );
        ptr += len;
        *ptr++ = '/';
        *ptr++ = ' ';
        len = m_objdir.size();
        memcpy( ptr, m_objdir.c_str(), len );
        ptr += len;
        *ptr++ = '/';
    }

    *ptr = '\0';
    printf( "%s %s\n", push ? "Pushing to" : "Pulling from", remote );
    FreeConfigSet( cs );
    return cmd;
}

set_str Lard::ReferencedObjects( bool all, const char* rev )
{
    set_str ret;
    ptr_set_str = &ret;

    auto cb = []( char* ptr ) {
        ptr_set_str->emplace( Buffer::Store( ptr + 12, 40 ) );
    };

    rev_info* revs = NewRevInfo();
    if( all )
    {
        AddRevAll( revs );
    }
    else if( rev )
    {
        if( AddRev( revs, rev ) != 0 )
        {
            fprintf( stderr, "Cannot resolve %s\n", rev );
            exit( 1 );
        }
    }
    else
    {
        AddRevHead( revs );
    }
    PrepareRevWalk( revs );
    GetFatObjectsFromRevs( revs, cb );
    FreeRevs( revs );

    return ret;
}

static size_t s_glb_numblobs;
static size_t s_glb_numlarge;
static size_t s_glb_threshold;

map_strsize Lard::GenLargeBlobs( int threshold )
{
    map_strsize ret;
    ptr_map_strsize = &ret;
    s_glb_numblobs = 0;
    s_glb_numlarge = 1;     // ??? Shouldn't this be 0?
    s_glb_threshold = threshold;

    auto cb = []( char* ptr, size_t size ) {
        s_glb_numblobs++;
        if( size > s_glb_threshold )    // ??? This contradicts message below!
        {
            s_glb_numlarge++;
            assert( ptr_map_strsize->find( ptr ) == ptr_map_strsize->end() );
            ptr_map_strsize->emplace( Buffer::Store( ptr ), size );
        }
    };

    const auto time0 = std::chrono::high_resolution_clock::now();
    rev_info* revs = NewRevInfo();
    AddRevAll( revs );
    PrepareRevWalk( revs );
    GetObjectsFromRevs( revs, cb );
    FreeRevs( revs );
    const auto time1 = std::chrono::high_resolution_clock::now();

    DBGPRINT( s_glb_numlarge << " of " << s_glb_numblobs << " blobs are >= " << threshold << " bytes [elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>( time1 - time0 ).count() << "ms]" );

    return ret;
}
