# Makefile.am

plugindir			 = ${libdir}/gtags

AM_YFLAGS			 = -d
SUFFIXES			 = .c .h .y .l

COMMON_SOURCES       = pp_parser.y pp_scanner.l \
					   symbol.c symbol.h \
					   stack.c stack.h \
					   file.c file.h \
					   errnum.h parser.h
BUILT_SOURCES		 = pp_parser.h pp_parser.c pp_scanner.c

# Rules to build the shared object puppet.so
plugin_LTLIBRARIES	 = puppet.la
puppet_la_SOURCES	 = ${COMMON_SOURCES}
puppet_la_LDFLAGS	 = -module -avoid-version -shared

# Rules to build the static library
check_LTLIBRARIES	 = libpuppet.la
libpuppet_la_SOURCES = ${COMMON_SOURCES}

maintainer-clean-local:
	-rm -f Makefile.in
