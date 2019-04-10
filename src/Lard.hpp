#ifndef __LARD_HPP__
#define __LARD_HPP__

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "StringHelpers.hpp"

using set_str = std::unordered_set<const char*, StringHelpers::hash, StringHelpers::equal_to>;
using map_strsize = std::unordered_map<const char*, size_t, StringHelpers::hash, StringHelpers::equal_to>;

class Lard
{
public:
    Lard( const char* commandName );
    ~Lard();

    void Init( int argc, char** argv );
    void Status( int argc, char** argv );
    void GC();
    void Verify();
    void Find( int argc, char** argv );
    void Clean();
    void Smudge();
    void Checkout();
    void Pull( int argc, char** argv );
    void Push( int argc, char** argv );

    void Submodule( int argc, char** argv );

private:
    void Setup();
    bool IsInitDone();
    void AssertInitDone();

    void FilterClean( FILE* in, FILE* out );
    void SubmoduleUpdate( bool recurse = false );
    void SubmoduleInit( bool recurse = false );
    void ExecuteOnSubmodules( char** args, const char* msg );
    std::vector<std::string> GetSubmodules();

    const char* CalcSha1( const char* ptr, size_t size ) const;
    const char* Sha1ToHex( const unsigned char sha1[20] ) const;

    static bool Decode( const char* data, const char*& sha1, size_t& size, bool errOnFail = false );
    static const char* Encode( const char* sha1, size_t size );
    const char* GetObjectFn( const char* sha1 ) const;

    std::vector<const char*> GetRsyncCommand( bool push ) const;
    bool ExecuteRsync( const std::vector<const char*>& cmd, const std::vector<const char*>& files ) const;

    set_str ReferencedObjects( bool all, bool nowalk, const char* rev );
    map_strsize GenLargeBlobs( int threshold );

    std::string m_prefix;
    std::string m_gitdir;
    std::string m_objdir;

    const char* m_commandName;
};

#endif
