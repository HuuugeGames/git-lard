#ifndef __DARKRL__DEBUG_HPP__
#define __DARKRL__DEBUG_HPP__

//#define DEBUG_MESSAGES_IN_RELEASE

#if defined DEBUG || defined DEBUG_MESSAGES_IN_RELEASE
#  include <sstream>
#  define DBGPRINT(msg) { std::ostringstream __buf; __buf << msg; DebugLog::Message( __buf.str().c_str() ); }
#else
#  define DBGPRINT(msg) ((void)0)
#endif

class DebugLog
{
public:
    struct Callback
    {
        virtual ~Callback() {}
        virtual void OnDebugMessage( const char* msg ) = 0;
    };

    DebugLog() = delete;

    static void Message( const char* msg );
    static void AddCallback( Callback* c );
    static void RemoveCallback( Callback* c );
};

#endif
