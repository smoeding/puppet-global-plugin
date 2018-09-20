# Makefile.am

check_PROGRAMS  = checkit
checkit_SOURCES = checkit.c
checkit_CFLAGS  = $(AM_CFLAGS)
checkit_LDADD   = $(top_builddir)/plugin/libpuppet.la

TESTS           = t-class.pp t-define.pp t-type.pp
TEST_EXTENSIONS = .pp
PP_LOG_COMPILER = ./testrunner.sh

maintainer-clean-local:
	-rm -f Makefile.in
