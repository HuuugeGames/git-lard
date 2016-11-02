#include "Debug.hpp"
#include "glue.h"
#include "Lard.hpp"

Lard::Lard()
{
    SetupGitDirectory();
    m_gitdir = GetGitDir();
    DBGPRINT( "Git dir: " << m_gitdir );
}

Lard::~Lard()
{
}
