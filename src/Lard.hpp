#ifndef __LARD_HPP__
#define __LARD_HPP__

#include <string>

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

    std::string m_gitdir;
    std::string m_objdir;
};

#endif
