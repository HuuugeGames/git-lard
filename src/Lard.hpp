#ifndef __LARD_HPP__
#define __LARD_HPP__

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "StringHelpers.hpp"

class Lard
{
public:
    Lard();
    ~Lard();

    void Init();
    void Status( int argc, char** argv );
    void GC();
    void Verify();
    void Find( int argc, char** argv );

private:
    void Setup();
    bool IsInitDone();

    std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to> ReferencedObjects( bool all );
    std::unordered_map<const char*, size_t, StringHelpers::hash, StringHelpers::equal_to> GenLargeBlobs( int threshold );

    std::string m_gitdir;
    std::string m_objdir;
};

#endif
