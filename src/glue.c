#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "git/cache.h"
#include "git/dir.h"
#include "git/git-compat-util.h"
#include "git/pathspec.h"
#include "git/revision.h"
#include "git/list-objects.h"
#include "git/submodule.h"
#include "git/lockfile.h"

#include "glue.h"
#include "verify.h"

static char s_tmpbuf[4096];
static struct pathspec pathspec;
static int prefixlen;

const char* SetupGitDirectory()
{
    const char* prefix = setup_git_directory();
    prefixlen = prefix ? strlen( prefix ) : 0;
    return prefix;
}

const char* GetGitDir()
{
    const char* gitdir = getenv( GIT_DIR_ENVIRONMENT );
    if( gitdir ) return gitdir;
    char* cwd = xgetcwd();
    int len = strlen( cwd );
    sprintf( s_tmpbuf, "%s%s.git", cwd, len && cwd[len-1] != '/' ? "/" : "" );
    free( cwd );
    return s_tmpbuf;
}

const char* GetGitWorkTree()
{
    return get_git_work_tree();
}

void ParsePathspec( const char* prefix )
{
    parse_pathspec( &pathspec, 0, PATHSPEC_PREFER_CWD, prefix, NULL );
}

int CheckIfConfigKeyExists( const char* key )
{
    const char* tmp;
    return !git_config_get_value( key, &tmp );
}

void SetConfigKey( const char* key, const char* val )
{
    git_config_set( key, val );
}

int GetConfigKey( const char* key, const char** val )
{
    return !git_config_get_value( key, val );
}

int GetConfigSetKey( const char* key, const char** val, struct config_set* cs )
{
    int ret = git_configset_get_value( cs, key, val );
    if( ret )
    {
        ret = git_config_get_value( key, val );
    }
    return !ret;
}

struct config_set* NewConfigSet()
{
    struct config_set* cs = (struct config_set*)malloc( sizeof( struct config_set ) );
    git_configset_init( cs );
    return cs;
}

void ConfigSetAddFile( struct config_set* cs, const char* file )
{
    git_configset_add_file( cs, file );
}

void FreeConfigSet( struct config_set* cs )
{
    git_configset_clear( cs );
}

struct rev_info* NewRevInfo()
{
    struct rev_info* revs = (struct rev_info*)malloc( sizeof( struct rev_info ) );
    init_revisions( revs, NULL );
    return revs;
}

void AddRevHead( struct rev_info* revs )
{
    add_head_to_pending( revs );
}

void AddRevAll( struct rev_info* revs )
{
    const char* argv[2] = { "", "--all" };
    verify( setup_revisions( 2, argv, revs, NULL ) == 1 );
}

int AddRev( struct rev_info* revs, const char* rev )
{
    struct object_id oid;
    struct object* obj;
    if( get_oid( rev, &oid ) )
    {
        return 1;
    }
    obj = parse_object( &oid );
    if( !obj )
    {
        return 1;
    }
    add_pending_object( revs, obj, rev );
    return 0;
}

void PrepareRevWalk( struct rev_info* revs )
{
    verify( prepare_revision_walk( revs ) == 0 );
}

void FreeRevs( struct rev_info* revs )
{
    free( revs );
    reset_revision_walk();
}

struct commit* GetRevision( struct rev_info* revs )
{
    return get_revision( revs );
}

static void null_show_commit( struct commit* a, void* b ) {}
static void null_show_object( struct object* a, const char* b, void* c ) {}

static void show_fat_object( struct object* obj, const char* name, void* data )
{
    if( obj->type == OBJ_BLOB )
    {
        unsigned long size;
        struct object_info oi = { NULL };
        oi.sizep = &size;
        sha1_object_info_extended( obj->oid.hash, &oi, LOOKUP_REPLACE_OBJECT );
        if( size == GitFatMagic )
        {
            enum object_type type;
            void* ptr = read_sha1_file( obj->oid.hash, &type, &size );
            if( memcmp( ptr, "#$# git-fat ", 12 ) == 0 )
            {
                ((void(*)(char*))data)( ptr );
            }
            free( ptr );
        }
    }
}

void GetFatObjectsFromRevs( struct rev_info* revs, int nowalk, void(*cb)( char* ) )
{
    revs->blob_objects = 1;
    revs->tree_objects = 1;
    revs->no_walk = nowalk;
    traverse_commit_list( revs, null_show_commit, show_fat_object, cb );
}

static void show_object( struct object* obj, const char* name, void* data )
{
    if( obj->type == OBJ_BLOB )
    {
        unsigned long size;
        struct object_info oi = { NULL };
        oi.sizep = &size;
        sha1_object_info_extended( obj->oid.hash, &oi, LOOKUP_REPLACE_OBJECT );
        ((void(*)(char*,size_t))data)( sha1_to_hex( obj->oid.hash ), size );
    }
}

void GetObjectsFromRevs( struct rev_info* revs, void(*cb)( char*, size_t ) )
{
    revs->blob_objects = 1;
    revs->tree_objects = 1;
    traverse_commit_list( revs, null_show_commit, show_object, cb );
}

void GetCommitList( struct rev_info* revs, void(*cb)( char* ) )
{
    struct commit* commit;
    while( ( commit = get_revision( revs ) ) != NULL )
    {
        ((void(*)(char*))cb)( sha1_to_hex( commit->object.oid.hash ) );
    }
}

struct TreeCallbacks
{
    int(*find)( const char* );
};

static struct commit* tree_commit;

static int ReadTree( const unsigned char* sha1, struct strbuf* base, const char* pathname, unsigned int mode, int stage, void* _cb )
{
    struct TreeCallbacks* cb = (struct TreeCallbacks*)_cb;

    if( S_ISGITLINK( mode ) )
    {
        return 0;
    }
    if( S_ISDIR( mode ) )
    {
        return READ_TREE_RECURSIVE;
    }

    int found = cb->find( sha1 );
    if( found )
    {
        printf( "Found: %s\n", sha1_to_hex( tree_commit->object.oid.hash ) );
    }

    return 0;
}

void GetCommitsForBlobs( struct rev_info* revs, int(*find)( const char* ) )
{
    struct TreeCallbacks cb;
    cb.find = find;
    while( ( tree_commit = get_revision( revs ) ) != NULL )
    {
        read_tree_recursive( tree_commit->tree, "", 0, 0, &pathspec, ReadTree, &cb );
    }
}

int ReadCache()
{
    return read_cache();
}

void ListFiles( void(*cb)( const char*, const char*, const char* ) )
{
    const char* super_prefix = get_super_prefix();

    for( int i=0; i<active_nr; i++ )
    {
        const struct cache_entry* ce = active_cache[i];
        if( ce->ce_flags & CE_UPDATE ) continue;
        char buf[1024];
        char* ptr = buf;
        if( super_prefix )
        {
            size_t len = strlen( super_prefix );
            memcpy( ptr, super_prefix, len );
            ptr += len;
        }

        size_t len = strlen( ce->name );
        memcpy( ptr, ce->name, len );
        ptr += len;

        *ptr = '\0';

        static char* ps_matched;
        if( match_pathspec( &pathspec, buf, ptr-buf, 0, ps_matched, S_ISDIR( ce->ce_mode ) || S_ISGITLINK( ce->ce_mode ) ) )
        {
            cb( buf, buf + prefixlen, (const char*)ce->oid.hash );
        }
    }
}

static struct lock_file lock_file;

void CheckoutFiles( struct CheckoutData(*cb)() )
{
    struct CheckoutData data = cb();
    if( !data.from ) return;

    assert( &the_index );
    int newfd = hold_locked_index( &lock_file, LOCK_DIE_ON_ERROR );

    do
    {
        int namelen = strlen( data.to );
        int pos = cache_name_pos( data.to, namelen );

        if( pos < 0 ) pos = -pos - 1;

        while( pos < active_nr )
        {
            struct cache_entry* ce = active_cache[pos];
            if( ce_namelen( ce ) != namelen || memcmp( ce->name, data.to, namelen ) ) break;
            pos++;

            FILE* in = fopen( data.from, "rb" );
            FILE* out = fopen( data.to, "wb" );

            enum { BufSize = 64 * 1024 };
            char buf[BufSize];
            int size;
            do
            {
                size = fread( buf, 1, BufSize, in );
                fwrite( buf, 1, size, out );
            }
            while( size == BufSize );

            fclose( in );
            fclose( out );

            struct stat st;
            lstat( ce->name, &st );
            fill_stat_cache_info( ce, &st );
            ce->ce_flags |= CE_UPDATE_IN_BASE;

            break;
        }
        data = cb();
    }
    while( data.from );

    the_index.cache_changed |= CE_ENTRY_CHANGED;

    if( 0 <= newfd && write_locked_index( &the_index, &lock_file, COMMIT_LOCK ) )
    {
        fprintf( stderr, "Unable to write new index file\n" );
        exit( 1 );
    }
}

const char* GetSha1( const char* name )
{
    unsigned char sha1[20];
    if( !get_sha1( name, sha1 ) )
    {
        return sha1_to_hex( sha1 );
    }
    else
    {
        return NULL;
    }
}
