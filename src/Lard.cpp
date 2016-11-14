#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "Buffer.hpp"
#include "Debug.hpp"
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
        if( std::find( s2.begin(), s2.end(), v ) == s2.end() )
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
    auto referenced = ReferencedObjects( all );
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
    GetObjectsFromRevs( revs, cb );
    free( revs );

    return ret;
}
