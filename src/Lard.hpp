#ifndef __LARD_HPP__
#define __LARD_HPP__

#include <string>

class Lard
{
public:
    Lard();
    ~Lard();

private:
    std::string m_gitroot;
    std::string m_gitdir;
};

#endif
