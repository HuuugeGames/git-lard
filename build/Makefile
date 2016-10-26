help:
	@echo Type "make debug" for debug build.
	@echo Type "make release" for release build.
	@echo Type "make profile" for profiling build.

clean:
	@echo Type "make cleandebug" to clean debug build.
	@echo Type "make cleanrelease" to clean release build.
	@echo Type "make cleanprofile" to clean profiling build.

debug:
	@+make -f debug.mk

release:
	@+make -f release.mk

profile:
	@+make -f profile.mk

cleandebug:
	@make -f debug.mk clean

cleanrelease:
	@make -f release.mk clean

cleanprofile:
	@make -f profile.mk clean

.PHONY: help clean debug release profile cleandebug cleanrelease cleanprofile
.SUFFIXES:
