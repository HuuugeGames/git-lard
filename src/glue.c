#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "git/cache.h"
#include "git/git-compat-util.h"

#include "glue.h"

static char s_tmpbuf[4096];

void SetupGitDirectory()
{
    setup_git_directory();
}

const char* GetGitDir()
{
    const char* gitdir = getenv( GIT_DIR_ENVIRONMENT );
    if( gitdir ) return gitdir;
    char* cwd = xgetcwd();
    int len = strlen( cwd );
    sprintf( s_tmpbuf, "%s%s.git", cwd, len && cwd[len-1] != '/' ? "/" : "" );
    return s_tmpbuf;
}
