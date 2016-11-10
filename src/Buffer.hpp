#ifndef __BUFFER_HPP__
#define __BUFFER_HPP__

#include <stdlib.h>
#include <vector>

class Buffer
{
public:
    Buffer( const Buffer& ) = delete;
    Buffer( Buffer&& ) = delete;
    ~Buffer();

    Buffer& operator=( const Buffer& ) = delete;
    Buffer& operator=( Buffer&& ) = delete;

    static const char* Store( const char* str );
    static const char* Store( const char* str, size_t len );

private:
    Buffer();
    static Buffer& GetInstance();

    void CheckSize( size_t size );

    std::vector<char*> m_buffers;
    char* m_current;
    size_t m_left;
};

#endif
