OPTFLAGS = -O3 -g3
DEFINES = -D__UNIX__ -DNDEBUG -D_PROFILE
BUILD = profile

include objs.mk
include common.mk
