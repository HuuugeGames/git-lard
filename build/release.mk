OPTFLAGS = -O3 -s
DEFINES = -D__UNIX__ -DNDEBUG
BUILD = release

include objs.mk
include common.mk
