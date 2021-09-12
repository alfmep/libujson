#!/bin/sh

if [ -f Makefile ]; then
    make distclean
    rm -f Makefile
fi

rm -rf \
   Makefile.in \
   aclocal.m4 \
   autom4te.cache \
   config.guess \
   config.sub \
   configure \
   depcomp \
   install-sh \
   ltmain.sh \
   m4 \
   missing \
   src/Makefile.in \
   src/include/ujson/config.h.in \
   src/include/ujson/config.h.in~ \
   ar-lib \
   compile \
   doc/doxygen_sqlite3.db \
   ylwrap \
   utils/Makefile.in \
   doc/Makefile.in \
   examples/Makefile.in
