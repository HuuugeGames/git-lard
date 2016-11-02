#ifndef __GLUE_H__

#ifdef __cplusplus
extern "C" {
#endif

void SetupGitDirectory();
const char* GetGitDir();
const char* GetGitWorkTree();
int CheckIfConfigKeyExists( const char* key );

#ifdef __cplusplus
}
#endif

#endif
