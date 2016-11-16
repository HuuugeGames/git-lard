#ifndef __LARD_HPP__
#define __LARD_HPP__

#include <string>
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

private:
    void Setup();
    bool IsInitDone();

    std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to> ReferencedObjects( bool all );

    std::string m_gitdir;
    std::string m_objdir;
};

#endif
