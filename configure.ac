#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#

AC_INIT(global-puppet-plugin, 1.0.0, stm@kill-9.net)
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_SUBST([AM_CPPFLAGS], ['-I$(top_srcdir)/plugin'])

# Checks for programs.
AC_PROG_CC
AM_PROG_AR
AC_PROG_INSTALL
AC_PROG_MAKE_SET

LT_INIT([dlopen])
LT_CONFIG_LTDL_DIR([libltdl])
LTDL_INIT([recursive])

# Checks for header files.
AC_CHECK_HEADERS([argz.h fcntl.h stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset mmap munmap strchr strrchr])

AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
  Makefile
  libltdl/Makefile
  plugin/Makefile
  tests/Makefile
])

AC_OUTPUT
