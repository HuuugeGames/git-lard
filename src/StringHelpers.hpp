#ifndef __STRING_HELPERS__
#define __STRING_HELPERS__

#include <string.h>
#include <unordered_set>
#include "../xxHash/xxhash.h"

namespace StringHelpers
{
    struct hash { size_t operator()( const char* v ) const { return XXH32( v, strlen( v ), 0 ); } };
    struct equal_to { bool operator()( const char* l, const char* r ) const { return strcmp( l, r ) == 0; } };
}

using stringset = std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to>;

#endif
