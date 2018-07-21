# Makefile.am

plugindir			 = ${libdir}/gtags

# Rules to build the shared object puppet.so
plugin_LTLIBRARIES	 = puppet.la
puppet_la_SOURCES	 = puppet.c parser.h
puppet_la_LDFLAGS	 = -module -avoid-version -shared

# Rules to build the static library
check_LTLIBRARIES	 = libpuppet.la
libpuppet_la_SOURCES = puppet.c parser.h

maintainer-clean-local:
	-rm -f Makefile.in
