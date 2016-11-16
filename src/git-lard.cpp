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
    }
    else if( CSTR( "filter-smudge" ) )
    {
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
    }
    else if( CSTR( "find" ) )
    {
    }
    else if( CSTR( "index-filter" ) )
    {
    }
    else
    {
        Usage();
    }
#undef CSTR

    return 0;
}
