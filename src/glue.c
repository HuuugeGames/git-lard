#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "git/cache.h"
#include "git/git-compat-util.h"
#include "git/revision.h"
#include "git/list-objects.h"

#include "glue.h"
#include "verify.h"

enum { GitFatMagic = 74 };

static char s_tmpbuf[4096];

void SetupGitDirectory()
{
    setup_git_directory();
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

int CheckIfConfigKeyExists( const char* key )
{
    const char* tmp;
    return git_config_get_value( key, &tmp );
}

void SetConfigKey( const char* key, const char* val )
{
    git_config_set( key, val );
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

void PrepareRevWalk( struct rev_info* revs )
{
    verify( prepare_revision_walk( revs ) == 0 );
}

struct commit* GetRevision( struct rev_info* revs )
{
    return get_revision( revs );
}

static void null_show_commit( struct commit* a, void* b ) {}

static void show_object( struct object* obj, const char* name, void* data )
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

void GetObjectsFromRevs( struct rev_info* revs, void(*cb)( char* ) )
{
    revs->blob_objects = 1;
    revs->tree_objects = 1;
    traverse_commit_list( revs, null_show_commit, show_object, cb );
}

const char* CalcSha1( const char* ptr, size_t size )
{
    git_SHA_CTX ctx;
    unsigned char sha1[20];
    git_SHA1_Init( &ctx );
    git_SHA1_Update( &ctx, ptr, size );
    git_SHA1_Final( sha1, &ctx );
    return sha1_to_hex( sha1 );
}
