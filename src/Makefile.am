#
# Copyright (C) 2017,2019-2021 Dan Arrhenius <dan@ultramarin.se>
#
# This file is part of ujson.
#
# ujson is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
lib_LTLIBRARIES = libujson.la

AM_CPPFLAGS = -I$(srcdir)
AM_LDFLAGS =

AM_CXXFLAGS = -Wall -pipe -O2 -g
#AM_YFLAGS = -d
#AM_LFLAGS =


# CURRENT : REVISION : AGE
AM_LDFLAGS += -version-info @LIBRARY_VERSION@


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ujson.pc

# Generated source and header files
BUILT_SOURCES =
BUILT_SOURCES += ujson/Analyzer.hpp
BUILT_SOURCES += ujson/Analyzer.cpp
BUILT_SOURCES += ujson/Lexer.cpp
BUILT_SOURCES += Lexer.hh
BUILT_SOURCES += location.hh
BUILT_SOURCES += position.hh
BUILT_SOURCES += stack.hh

CLEANFILES = $(BUILT_SOURCES)


# Source files
libujson_la_SOURCES  =
libujson_la_SOURCES += ujson/Analyzer.ypp
libujson_la_SOURCES += ujson/Lexer.lpp
libujson_la_SOURCES += ujson/Parser.cpp
libujson_la_SOURCES += ujson/jvalue.cpp
libujson_la_SOURCES += ujson/utils.cpp
libujson_la_SOURCES += ujson/Json.cpp

# Header files
libujsondir = $(includedir)
nobase_libujson_HEADERS =
nobase_libujson_HEADERS += ujson.hpp
nobase_libujson_HEADERS += ujson/multimap_list.hpp
nobase_libujson_HEADERS += ujson/jvalue.hpp
nobase_libujson_HEADERS += ujson/utils.hpp
nobase_libujson_HEADERS += ujson/Json.hpp

# Header files that is not to be installed
noinst_HEADERS =
noinst_HEADERS += ujson/Parser.hpp

#
# Generated files
#
ujson/Analyzer.cpp:	ujson/Analyzer.hpp

ujson/Analyzer.hpp:	ujson/Analyzer.ypp

location.hh:	ujson/Analyzer.ypp

position.hh:	ujson/Analyzer.ypp

stack.hh:	ujson/Analyzer.ypp


ujson/Lexer.cpp:	Lexer.hh

Lexer.hh:	ujson/Lexer.lpp
