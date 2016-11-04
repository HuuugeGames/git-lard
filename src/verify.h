#ifndef __DARKRL_VERIFY_HPP__
#define __DARKRL_VERIFY_HPP__

#include <assert.h>

#ifdef _DEBUG
#  define verify(x) assert(x)
#else
#  define verify(x) x
#endif

#endif
