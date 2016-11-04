#include <stdio.h>
#include <string.h>

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

std::unordered_set<std::string> Lard::ReferencedObjects( bool all )
{
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
    while( auto c = GetRevision( revs ) )
    {
    }
    free( revs );
    return std::unordered_set<std::string>();
}
