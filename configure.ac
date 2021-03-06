AC_PREREQ(2.63)


AC_INIT([zonebuild], [0.4.2], [io@nixnodes.net],,[http://nixnodes.net/]) 
AC_CONFIG_SRCDIR([src/main.c]) 

AM_INIT_AUTOMAKE([subdir-objects silent-rules]) 
AM_MAINTAINER_MODE([disable])
AM_PROG_CC_C_O
AM_SILENT_RULES([yes])

AC_PROG_CC_STDC
AC_USE_SYSTEM_EXTENSIONS

AC_CONFIG_FILES([Makefile src/Makefile]) 
AC_CONFIG_SRCDIR([src])

AC_PREFIX_DEFAULT(["/usr"])


AM_CONDITIONAL(ENABLE_MAIN, test 1 = 1)


AS_VAR_SET(datadir, ["$prefix/scripts"])


CFLAGS="-Wall -O2"
AC_SUBST(CFLAGS)


AC_HEADER_MAJOR

AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_STRUCT_ST_BLOCKS

GLUTIL_MSG_REQH="missing required header file"

AC_CHECK_HEADER([fcntl.h],,[AC_MSG_ERROR([$GLUTIL_MSG_REQH (fnctl.h)])])
AC_CHECK_HEADER([limits.h],,[AC_MSG_ERROR([$GLUTIL_MSG_REQH (limits.h)])])
AC_CHECK_HEADER([utime.h],,[AC_MSG_ERROR([$GLUTIL_MSG_REQH (utime.h)])])
AC_CHECK_HEADER([errno.h],,[AC_MSG_ERROR([$GLUTIL_MSG_REQH (errno.h)])])
AC_CHECK_HEADER([math.h],,[AC_MSG_ERROR([$GLUTIL_MSG_REQH (math.h)])])
AC_CHECK_HEADER([regex.h],,[AC_MSG_ERROR([$GLUTIL_MSG_REQH (regex.h)])])


AC_HEADER_STDC

GLUTIL_MSG_FH="missing procedure"

AC_CHECK_LIB([m], [pow], [], [AC_MSG_FAILURE([could not find math library])])

AC_CHECK_FUNCS([bzero],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (bzero)])])
AC_CHECK_FUNCS([dup2],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (dup2)])])
AC_CHECK_FUNCS([regcomp],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (regcomp)])])
AC_CHECK_FUNCS([regexec],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (regexec)])])
AC_CHECK_FUNCS([strchr],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (strchr)])])
AC_CHECK_FUNCS([strdup],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (strdup)])])
AC_CHECK_FUNCS([strerror],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (strerror)])])
AC_CHECK_FUNCS([strrchr],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (strrchr)])])
AC_CHECK_FUNCS([strstr],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (strstr)])])
AC_CHECK_FUNCS([strtol],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (strtol)])])
AC_CHECK_FUNCS([strtoul],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (strtoul)])])
AC_CHECK_FUNCS([strtoull],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (strtoull)])])
AC_CHECK_FUNCS([utime],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (utime)])])
AC_CHECK_FUNCS([opendir],,[AC_MSG_ERROR([$GLUTIL_MSG_FH (opendir)])])


AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC


AC_TYPE_SIZE_T
AC_TYPE_OFF_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_SIGNAL

AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SSIZE_T

AC_OUTPUT

