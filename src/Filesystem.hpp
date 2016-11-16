#ifndef __FILESYSTEM_HPP__
#define __FILESYSTEM_HPP__

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unordered_set>

#include "StringHelpers.hpp"
#include "verify.h"

bool CreateDirStruct( const std::string& path );
std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to> ListDirectory( const std::string& path );

#ifdef _MSC_VER
#  define stat64 _stat64
#endif
#ifdef __CYGWIN__
#  define stat64 stat
#endif

#ifndef S_ISREG
#  define S_ISREG(m) ( ( (m) & S_IFMT ) == S_IFREG )
#endif

static inline bool Exists( const std::string& path )
{
    struct stat64 buf;
    return stat64( path.c_str(), &buf ) == 0;
}

static inline bool IsFile( const std::string& path )
{
    struct stat64 buf;
    verify( stat64( path.c_str(), &buf ) == 0 );
    return S_ISREG( buf.st_mode );
}

static inline uint64_t GetFileSize( const char* path )
{
    struct stat64 buf;
    verify( stat64( path, &buf ) == 0 );
    return buf.st_size;
}

#endif
