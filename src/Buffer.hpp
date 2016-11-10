#ifndef __BUFFER_HPP__
#define __BUFFER_HPP__

#include <vector>

class Buffer
{
public:
    ~Buffer();

    static const char* Store( const char* str );

private:
    Buffer();

    std::vector<char*> m_buffers;
    char* m_current;
    size_t m_left;
};

#endif
