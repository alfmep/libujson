#!/bin/sh

if [ -f Makefile ]; then
    make distclean
fi

rm -rf \
   Makefile.in \
   aclocal.m4 \
   autom4te.cache \
   config.guess \
   config.log \
   config.sub \
   configure \
   configure~ \
   depcomp \
   install-sh \
   ltmain.sh \
   m4 \
   missing \
   src/Makefile.in \
   src/ujson/.deps \
   src/ujson/config.hpp.in \
   src/ujson/config.hpp \
   src/ujson/stamp-h1 \
   utils/Makefile.in \
   utils/ujson-cmp.1 \
   utils/ujson-get.1 \
   utils/ujson-patch.1 \
   utils/ujson-print.1 \
   utils/ujson-verify.1 \
   examples/Makefile.in \
   test/Makefile.in \
   doc/Makefile.in \
   ar-lib \
   compile \
   doc/doxygen_sqlite3.db \
   ylwrap
