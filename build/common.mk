SRCPATH = ../src
GITDIR = ../git
GITLIB = $(GITDIR)/libgit.a

INCLUDES = -I$(SRCPATH)

LIBS :=
CFLAGS := $(OPTFLAGS)
CXXFLAGS = $(CFLAGS) -std=c++14

TARGET = git-lard

BUILDDIR = $(BUILD)$(POSTFIX)/.build/build

all: $(TARGET)

TMPOBJS = $(SOURCES:.cpp=.o)
TMPOBJS2 = $(TMPOBJS:.cc=.o)
OBJS = $(TMPOBJS2:.c=.o)

OBJS := $(patsubst %,$(BUILDDIR)/%,$(OBJS))

$(TARGET): $(OBJS) $(GITLIB)
	$(CXX) $(OBJS) $(CXXFLAGS) $(LIBS) -o $(TARGET)

$(GITLIB):
	+make -C $(GITDIR) libgit.a

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) -c $(INCLUDES) $(CXXFLAGS) $(DEFINES) $< -o $@

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $(INCLUDES) $(CFLAGS) $(DEFINES) $< -o $@

$(BUILDDIR)/%.d: %.cpp
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CXX) -MM $(INCLUDES) $(CXXFLAGS) $(DEFINES) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(BUILDDIR)/$(<:.cpp=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(BUILDDIR)/%.d: %.c
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CC) -MM $(INCLUDES) $(CFLAGS) $(DEFINES) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(BUILDDIR)/$(<:.c=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJS:.o=.d)
endif

clean:
	rm -rf $(BUILD)$(POSTFIX)
	rm -f $(TARGET)
	make -C $(GITDIR) clean

.PHONY: clean all help
.SUFFIXES:
