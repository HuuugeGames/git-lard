#ifndef __GLUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum { GitFatMagic = 74 };

struct rev_info;
struct commit;
struct config_set;

const char* SetupGitDirectory();
const char* GetGitDir();
const char* GetGitWorkTree();
void ParsePathspec( const char* prefix );

int CheckIfConfigKeyExists( const char* key );
void SetConfigKey( const char* key, const char* val );
int GetConfigKey( const char* key, const char** val );
int GetConfigSetKey( const char* key, const char** val, struct config_set* cs );

struct config_set* NewConfigSet();
void ConfigSetAddFile( struct config_set* cs, const char* file );
void FreeConfigSet( struct config_set* cs );

struct rev_info* NewRevInfo();
void AddRevHead( struct rev_info* revs );
void AddRevAll( struct rev_info* revs );
void PrepareRevWalk( struct rev_info* revs );
void FreeRevs( struct rev_info* revs );
struct commit* GetRevision( struct rev_info* revs );
void GetFatObjectsFromRevs( struct rev_info* revs, void(*cb)( char* ) );
void GetObjectsFromRevs( struct rev_info* revs, void(*cb)( char*, size_t ) );
void GetCommitList( struct rev_info* revs, void(*cb)( char* ) );

int ReadCache();
void ListFiles( void(*cb)( const char*, const char* ) );

#ifdef __cplusplus
}
#endif

#endif
