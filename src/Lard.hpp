#ifndef __LARD_HPP__
#define __LARD_HPP__

#include <string>
#include <unordered_set>

class Lard
{
public:
    Lard();
    ~Lard();

    void Init();
    void Status( int argc, char** argv );

private:
    void Setup();
    bool IsInitDone();

    std::unordered_set<std::string> ReferencedObjects( bool all );

    std::string m_gitdir;
    std::string m_objdir;
};

#endif
