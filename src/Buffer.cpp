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

Buffer& Buffer::GetInstance()
{
    static Buffer buf;
    return buf;
}

const char* Buffer::Store( const char* str )
{
    auto& buf = GetInstance();
    const auto size = strlen( str ) + 1;
    buf.CheckSize( size );
    memcpy( buf.m_current, str, size );
    const auto ret = buf.m_current;
    buf.m_current += size;
    buf.m_left -= size;
    return ret;
}

const char* Buffer::Store( const char* str, size_t size )
{
    auto& buf = GetInstance();
    buf.CheckSize( size+1 );
    memcpy( buf.m_current, str, size );
    buf.m_current[size++] = '\0';
    const auto ret = buf.m_current;
    buf.m_current += size;
    buf.m_left -= size;
    return ret;
}

void Buffer::CheckSize( size_t size )
{
    if( size > m_left )
    {
        assert( size <= BufSize );
        m_current = new char[BufSize];
        m_left = BufSize;
        m_buffers.emplace_back( m_current );
    }
}
