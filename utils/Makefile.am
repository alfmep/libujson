#
# Copyright (C) 2021 Dan Arrhenius <dan@ultramarin.se>
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

if ENABLE_UTILS_SET

AM_CPPFLAGS = -I$(srcdir)/../src
AM_CXXFLAGS = -Wall -pipe -O2 -g -std=c++17
AM_LDFLAGS = -L../src -lujson


bin_PROGRAMS =

bin_PROGRAMS += ujson-verify
ujson_verify_SOURCES = ujson-verify.cpp

bin_PROGRAMS += ujson-print
ujson_print_SOURCES = ujson-print.cpp

bin_PROGRAMS += ujson-cmp
ujson_cmp_SOURCES = ujson-cmp.cpp

bin_PROGRAMS += ujson-get
ujson_get_SOURCES = ujson-get.cpp

bin_PROGRAMS += ujson-patch
ujson_patch_SOURCES = ujson-patch.cpp

dist_man1_MANS =
dist_man1_MANS += ujson-verify.man
dist_man1_MANS += ujson-print.man
dist_man1_MANS += ujson-cmp.man
dist_man1_MANS += ujson-get.man
dist_man1_MANS += ujson-patch.man

endif
