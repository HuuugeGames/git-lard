#include <memory>
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
    printf( "Usage: git lard [init|status|push|pull|gc|verify|checkout|find|index-filtered]\n" );
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

    Lard lard;

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
        lard.Init();
    }
    else if( CSTR( "status" ) )
    {
        lard.Status( argc-2, argv+2 );
    }
    else if( CSTR( "push" ) )
    {
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
    else
    {
        Usage();
    }
#undef CSTR

    return 0;
}
