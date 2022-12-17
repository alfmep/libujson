#
# Copyright (C) 2021,2022 Dan Arrhenius <dan@ultramarin.se>
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

if ENABLE_EXAMPLES_SET

AM_CPPFLAGS = -I$(srcdir)/../src
AM_CXXFLAGS = -Wall -pipe -O2 -g -std=c++17
AM_LDFLAGS = -L../src -lujson

if HAVE_GMPXX
  AM_CXXFLAGS += $(gmpxx_CFLAGS)
  AM_LDFLAGS += $(gmpxx_LIBS)
endif


noinst_bindir =
noinst_bin_PROGRAMS =

noinst_bin_PROGRAMS += find_jvalue
find_jvalue_SOURCES  = find_jvalue.cpp

noinst_bin_PROGRAMS += initialize-jvalue
initialize_jvalue_SOURCES  = initialize-jvalue.cpp

noinst_bin_PROGRAMS += json-object
json_object_SOURCES  = json-object.cpp

noinst_bin_PROGRAMS += json-array
json_array_SOURCES  = json-array.cpp

noinst_bin_PROGRAMS += json-number
json_number_SOURCES  = json-number.cpp

endif
