# puppet-global-plugin

A GNU global plugin to parse Puppet manifests

## Building

Note: This has only be tested on Debian 9 (Stretch)!

You will need autoconf/automake/libtool to build the plugin. You can start by running the `autogen.sh` script in the top level directory:

``` shellsession
./autogen.sh
```

This will build the plugin. Then run `make install` as `root` to install the plugin:

``` shellsession
make install
```

The `autogen.sh` script uses `--prefix=/usr` and therefore the plugin will be installed in `/usr/lib/gtags` where GNU global expects the files on Debian.

The installation also creates `/usr/lib/gtags/puppet.la`. This file is not used on Debian and can be deleted if you care.

## Setup

Create the file `~/.globalrc` with the following content:

``` text
# Configuration file for GNU GLOBAL source code tag system.
#
# Please refer to gtags.conf(5) for details.
#
default:\
    :tc=native:tc=puppet:
native:\
    :tc=gtags:tc=htags:
ctags:\
    :tc=htags:
#---------------------------------------------------------------------
# Configuration for gtags(1)
# See gtags(1).
#---------------------------------------------------------------------
common:\
    :skip=tags,TAGS,gtags.files,*.orig,*.rej,*.bak,*~,#*#,*.swp,*.tmp,*.zip,*.gz,*.bz2,*.xz,*.lzh,*.Z,*.tgz:
#
# Built-in parsers.
#
gtags:\
    :tc=common:\
    :tc=builtin-parser:
#
builtin-parser:\
    :langmap=c\:.c.h,yacc\:.y,asm\:.s.S,java\:.java,cpp\:.c++.cc.hh.cpp.cxx.hxx.hpp.C.H,php\:.php.php3.phtml:
#
# Puppet
#
puppet|Puppet plugin parser:\
    :tc=common:\
    :langmap=puppet\:.pp:\
    :gtags_parser=puppet\:$libdir/gtags/puppet.so:
#---------------------------------------------------------------------
# Configuration for htags(1)
#---------------------------------------------------------------------
htags:\
    ::
```

This instructs GNU global to use the plugin for Puppet manifests.
