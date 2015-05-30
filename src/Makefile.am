if ENABLE_MAIN
MAIN_APP = buildz
endif

bin_PROGRAMS=$(MAIN_APP)

MAIN_DEFAULTSRC=signal.c l_error.c memory.c thread.c str.c arg_proc.c arg_opts.c x_f.c gv_off.c sort.c zonebuild.c

buildz_SOURCES=$(MAIN_DEFAULTSRC)

MAIN_DEFAULTLN=

buildz_LDADD=$(GLUTIL_DEFAULTLN)
