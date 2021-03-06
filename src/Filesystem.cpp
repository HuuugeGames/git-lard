#include "Buffer.hpp"
#include "Filesystem.hpp"

#ifdef _MSC_VER
#  include <direct.h>
#  include <windows.h>
#else
#  include <dirent.h>
#endif

#include <errno.h>
#include <string.h>

#ifndef DT_DIR
#  define DT_DIR 4
#endif

bool CreateDirStruct( const std::string& path )
{
    if( Exists( path ) ) return true;

    if( errno != ENOENT )
    {
        fprintf( stderr, "%s\n", strerror( errno ) );
        return false;
    }

    size_t pos = 0;
    do
    {
        pos = path.find( '/', pos+1 );
#ifdef _WIN32
        if( pos == 2 ) continue;    // Don't create drive name.
#endif
        auto p = path.substr( 0, pos );
        if( Exists( p ) ) continue;
#ifdef _WIN32
        if( _mkdir( p.c_str() ) != 0 )
#else
        if( mkdir( p.c_str(), S_IRWXU ) != 0 )
#endif
        {
            if( errno != EEXIST )
            {
                fprintf( stderr, "Creating failed on %s (%s)\n", path.substr( 0, pos ).c_str(), strerror( errno ) );
                return false;
            }
        }
    }
    while( pos != std::string::npos );

    return true;
}

std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to> ListDirectory( const std::string& path )
{
    std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to> ret;

#ifdef _MSC_VER
    WIN32_FIND_DATA ffd;
    HANDLE h;

    std::string p = path + "/*";
    for( unsigned int i=0; i<p.size(); i++ )
    {
        if( p[i] == '/' )
        {
            p[i] = '\\';
        }
    }

    h = FindFirstFile( ( p ).c_str(), &ffd );
    if( h == INVALID_HANDLE_VALUE )
    {
        return ret;
    }

    do
    {
        std::string s = ffd.cFileName;
        if( s != "." && s != ".." )
        {
            if( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            {
                s += "/";
            }
            ret.emplace( Buffer::Store( s.c_str() ) );
        }
    }
    while( FindNextFile( h, &ffd ) );

    FindClose( h );
#else
    DIR* dir = opendir( path.c_str() );
    if( dir == nullptr )
    {
        return ret;
    }

    struct dirent* ent;
    while( ( ent = readdir( dir ) ) != nullptr )
    {
        std::string s = ent->d_name;
        if( s != "." && s != ".." )
        {
            if( ent->d_type == DT_DIR )
            {
                s += "/";
            }
            ret.emplace( Buffer::Store( s.c_str() ) );
        }
    }
    closedir( dir );
#endif

    return ret;
}
