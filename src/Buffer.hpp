#ifndef __BUFFER_HPP__
#define __BUFFER_HPP__

#include <stdlib.h>
#include <vector>

class Buffer
{
public:
    ~Buffer();

    static const char* Store( const char* str );

private:
    Buffer();

    void CheckSize( size_t size );

    std::vector<char*> m_buffers;
    char* m_current;
    size_t m_left;
};

#endif
