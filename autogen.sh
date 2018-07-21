#!/bin/sh

PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

mkdir -p m4
aclocal --install
autoheader
libtoolize --copy --force --ltdl
automake --copy --add-missing --force-missing --foreign
autoconf
./configure
make
