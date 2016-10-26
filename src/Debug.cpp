#include <algorithm>
#include <chrono>
#include <mutex>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "Debug.hpp"

static std::vector<DebugLog::Callback*> s_callbacks;
static std::mutex lock;

static std::chrono::high_resolution_clock::time_point s_start = std::chrono::high_resolution_clock::now();

void DebugLog::Message( const char* msg )
{
    std::lock_guard<std::mutex> lg( lock );

    const auto now = std::chrono::high_resolution_clock::now();
    const auto diff = std::chrono::duration_cast<std::chrono::microseconds>( now - s_start );

    char* tmp = (char*)alloca( strlen(msg)+32 );
    sprintf( tmp, "[%12.5g] %s", diff.count() / 1000000.0, msg );

    for( auto& callback : s_callbacks )
    {
        callback->OnDebugMessage( tmp );
    }
}

void DebugLog::AddCallback( Callback* c )
{
    std::lock_guard<std::mutex> lg( lock );

    const auto it = std::find( s_callbacks.begin(), s_callbacks.end(), c );
    if( it == s_callbacks.end() )
    {
        s_callbacks.push_back( c );
    }
}

void DebugLog::RemoveCallback( Callback* c )
{
    std::lock_guard<std::mutex> lg( lock );

    const auto it = std::find( s_callbacks.begin(), s_callbacks.end(), c );
    if( it != s_callbacks.end() )
    {
        s_callbacks.erase( it );
    }
}
