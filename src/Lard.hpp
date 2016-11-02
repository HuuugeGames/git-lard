#ifndef __LARD_HPP__
#define __LARD_HPP__

#include <string>

class Lard
{
public:
    Lard();
    ~Lard();

private:
    void Setup();

    std::string m_gitdir;
    std::string m_objdir;
};

#endif
