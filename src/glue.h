#ifndef __GLUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct rev_info;
struct commit;

void SetupGitDirectory();
const char* GetGitDir();
const char* GetGitWorkTree();
int CheckIfConfigKeyExists( const char* key );
void SetConfigKey( const char* key, const char* val );

struct rev_info* NewRevInfo();
void AddRevHead( struct rev_info* revs );
void AddRevAll( struct rev_info* revs );
void PrepareRevWalk( struct rev_info* revs );
struct commit* GetRevision( struct rev_info* revs );
void GetFatObjectsFromRevs( struct rev_info* revs, void(*cb)( char* ) );
void GetObjectsFromRevs( struct rev_info* revs, void(*cb)( char*, size_t ) );

const char* CalcSha1( const char* ptr, size_t size );

#ifdef __cplusplus
}
#endif

#endif
