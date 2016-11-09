#ifdef WIN32

#include <new>
#include <stdlib.h>
#include <windows.h>

extern "C" {

void* malloc( size_t size )
{
    return HeapAlloc( GetProcessHeap(), 0, size );
}

void free( void* ptr )
{
    HeapFree( GetProcessHeap(), 0, ptr );
}

void* calloc( size_t num, size_t size )
{
    return HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, num*size );
}

void* realloc( void* ptr, size_t size )
{
    if( ptr == NULL ) return malloc( size );
    if( size == 0 ) return NULL;
    return HeapReAlloc( GetProcessHeap(), 0, ptr, size );
}

}

void* operator new( size_t size ) throw( std::bad_alloc )
{
    void* ptr = malloc( size );
    if( !ptr ) throw std::bad_alloc();
    return ptr;
}

void* operator new( size_t size, const std::nothrow_t& ) throw()
{
    return malloc( size );
}

void* operator new[]( size_t size ) throw( std::bad_alloc )
{
    void* ptr = malloc( size );
    if( !ptr ) throw std::bad_alloc();
    return ptr;
}

void* operator new[]( size_t size, const std::nothrow_t& ) throw()
{
    return malloc( size );
}

void operator delete( void* ptr ) throw()
{
    free( ptr );
}

void operator delete( void* ptr, const std::nothrow_t& ) throw()
{
    free( ptr );
}

void operator delete[]( void* ptr ) throw()
{
    free( ptr );
}

void operator delete[]( void* ptr, const std::nothrow_t& ) throw()
{
    free( ptr );
}

#endif
