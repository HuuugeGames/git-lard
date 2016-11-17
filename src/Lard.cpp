#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#include "Buffer.hpp"
#include "Debug.hpp"
#include "FileMap.hpp"
#include "Filesystem.hpp"
#include "glue.h"
#include "Lard.hpp"

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
    SetupGitDirectory();
    m_gitdir = GetGitDir();
    DBGPRINT( "Git dir: " << m_gitdir );
    m_objdir = m_gitdir + "/fat/objects";
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

static std::vector<const char*> RelativeComplement( const std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to>& s1, const std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to>& s2 )
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
    const auto referenced = ReferencedObjects( all );
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
    const auto referenced = ReferencedObjects( false );
    const auto garbage = RelativeComplement( catalog, referenced );
    printf( "Unreferenced objects to remove: %d\n", garbage.size() );
    for( auto& v : garbage )
    {
        char fn[1024];
        sprintf( fn, "%s/%s", m_objdir.c_str(), v );
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
        char fn[1024];
        sprintf( fn, "%s/%s", m_objdir.c_str(), v );
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

void Lard::Find( int argc, char** argv )
{
    assert( argc > 0 );
    const auto maxsize = atoi( argv[0] );
    const auto blobsizes = GenLargeBlobs( maxsize );
    const auto time0 = std::chrono::high_resolution_clock::now();
}

void Lard::Setup()
{
    CreateDirStruct( m_objdir );
}

bool Lard::IsInitDone()
{
    return CheckIfConfigKeyExists( "filter.fat.clean" ) == 0 || CheckIfConfigKeyExists( "filter.fat.smudge" ) == 0;
}

static std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to>* s_susssret;

std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to> Lard::ReferencedObjects( bool all )
{
    std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to> ret;
    s_susssret = &ret;

    auto cb = []( char* ptr ) {
        s_susssret->emplace( Buffer::Store( ptr + 12, 40 ) );
    };

    rev_info* revs = NewRevInfo();
    if( all )
    {
        AddRevAll( revs );
    }
    else
    {
        AddRevHead( revs );
    }
    PrepareRevWalk( revs );
    GetFatObjectsFromRevs( revs, cb );
    free( revs );

    return ret;
}

static std::unordered_map<const char*, size_t, StringHelpers::hash, StringHelpers::equal_to>* s_sumglbret;
static size_t s_glb_numblobs;
static size_t s_glb_numlarge;
static size_t s_glb_threshold;

std::unordered_map<const char*, size_t, StringHelpers::hash, StringHelpers::equal_to> Lard::GenLargeBlobs( int threshold )
{
    std::unordered_map<const char*, size_t, StringHelpers::hash, StringHelpers::equal_to> ret;
    s_sumglbret = &ret;
    s_glb_numblobs = 0;
    s_glb_numlarge = 1;     // ??? Shouldn't this be 0?
    s_glb_threshold = threshold;

    auto cb = []( char* ptr, size_t size ) {
        s_glb_numblobs++;
        if( size > s_glb_threshold )    // ??? This contradicts message below!
        {
            s_glb_numlarge++;
            assert( s_sumglbret->find( ptr ) == s_sumglbret->end() );
            s_sumglbret->emplace( Buffer::Store( ptr ), size );
        }
    };

    const auto time0 = std::chrono::high_resolution_clock::now();
    rev_info* revs = NewRevInfo();
    AddRevAll( revs );
    PrepareRevWalk( revs );
    GetObjectsFromRevs( revs, cb );
    free( revs );
    const auto time1 = std::chrono::high_resolution_clock::now();

    DBGPRINT( s_glb_numlarge << " of " << s_glb_numblobs << " blobs are >= " << threshold << " bytes [elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>( time1 - time0 ).count() << "ms]" );

    return ret;
}
