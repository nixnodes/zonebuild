if ENABLE_MAIN
MAIN_APP = zoneb
endif

bin_PROGRAMS=$(MAIN_APP)

MAIN_DEFAULTSRC=signal.c l_error.c memory.c thread.c str.c arg_proc.c arg_opts.c x_f.c zonebuild.c

zoneb_SOURCES=$(MAIN_DEFAULTSRC)

MAIN_DEFAULTLN=

zoneb_LDADD=$(GLUTIL_DEFAULTLN)
