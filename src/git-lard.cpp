#include <memory>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Debug.hpp"
#include "Lard.hpp"

struct StdErrDebugCallback : public DebugLog::Callback
{
    void OnDebugMessage( const char* msg ) override
    {
        fprintf( stderr, "%s\n", msg );
    }
};

void Usage()
{
    printf( "Usage: git lard [init|status|push|pull|gc|verify|checkout|find|index-filtered|submodule]\n" );
    exit( 1 );
}

int main( int argc, char** argv )
{
    if( argc == 1 )
    {
        Usage();
    }

    auto err = std::make_unique<StdErrDebugCallback>();
    DebugLog::AddCallback( err.get() );

#ifdef _DEBUG
    std::ostringstream s;
    for( int i=0; i<argc; i++ )
    {
        s << argv[i] << " ";
    }
    DBGPRINT( "Command line: " << s.str() );
#endif

    Lard lard( argv[0] );

#define CSTR(x) strcmp( argv[1], x ) == 0
    if( CSTR( "filter-clean" ) )
    {
        lard.Clean();
    }
    else if( CSTR( "filter-smudge" ) )
    {
        lard.Smudge();
    }
    else if( CSTR( "init" ) )
    {
        lard.Init( argc-2, argv+2 );
    }
    else if( CSTR( "status" ) )
    {
        lard.Status( argc-2, argv+2 );
    }
    else if( CSTR( "push" ) )
    {
        lard.Push( argc-2, argv+2 );
    }
    else if( CSTR( "pull" ) )
    {
        lard.Pull( argc-2, argv+2 );
    }
    else if( CSTR( "gc" ) )
    {
        lard.GC();
    }
    else if( CSTR( "verify" ) )
    {
        lard.Verify();
    }
    else if( CSTR( "checkout" ) )
    {
        lard.Checkout();
    }
    else if( CSTR( "find" ) )
    {
        lard.Find( argc-2, argv+2 );
    }
    else if( CSTR( "index-filter" ) )
    {
        printf( "TODO\n" );
        exit( 1 );
    }
    else if( CSTR( "submodule" ) )
    {
        lard.Submodule( argc-2, argv+2 );
    }
    else
    {
        Usage();
    }
#undef CSTR

    return 0;
}
