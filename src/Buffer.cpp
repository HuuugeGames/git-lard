#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "Buffer.hpp"

enum { BufSize = 1024*1024 };

Buffer::Buffer()
    : m_current( nullptr )
    , m_left( 0 )
{
}

Buffer::~Buffer()
{
    for( auto& v : m_buffers )
    {
        delete[] v;
    }
}

const char* Buffer::Store( const char* str )
{
    static Buffer buf;
    const auto size = strlen( str ) + 1;
    if( size > buf.m_left )
    {
        assert( size <= BufSize );
        buf.m_current = new char[BufSize];
        buf.m_left = BufSize;
        buf.m_buffers.emplace_back( buf.m_current );
    }
    memcpy( buf.m_current, str, size );
    const auto ret = buf.m_current;
    buf.m_current += size;
    buf.m_left -= size;
    return ret;
}
