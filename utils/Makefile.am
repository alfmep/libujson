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
AM_CXXFLAGS = -Wall -pipe -O2 -g
AM_LDFLAGS = -L../src -lujson


#noinst_bindir =
bin_PROGRAMS =

bin_PROGRAMS += ujson-verify
ujson_verify_SOURCES = ujson-verify.cpp

endif
