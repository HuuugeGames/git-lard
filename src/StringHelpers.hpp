#ifndef __STRING_HELPERS__
#define __STRING_HELPERS__

#include <algorithm>
#include <ctype.h>
#include <string.h>
#include <string>
#include <unordered_set>
#include "../xxHash/xxhash.h"

namespace StringHelpers
{
    struct hash { size_t operator()( const char* v ) const { return XXH32( v, strlen( v ), 0 ); } };
    struct equal_to { bool operator()( const char* l, const char* r ) const { return strcmp( l, r ) == 0; } };
    struct less { bool operator()( const char* l, const char* r ) const { return strcmp( l, r ) < 0; } };

    namespace
    {
        static inline bool _isspace( char c )
        {
            return c == ' ';
        }
    }

    template <class T>
    static inline void split( const char* s, T o )
    {
        typedef const char* iter;

        iter i = s, j;
        iter e = s;
        while( *e ) e++;

        while( i != e )
        {
            i = std::find_if( i, e, []( char c ){ return !_isspace( c ); } );
            j = std::find_if( i, e, []( char c ){ return _isspace( c ); } );

            if( i != e )
            {
                *o++ = strdup( std::string( i, j ).c_str() );
            }

            i = j;
        }
    }
}

using stringset = std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to>;

#endif
