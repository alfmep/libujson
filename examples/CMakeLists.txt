#
# Copyright (C) 2023 Dan Arrhenius <dan@ultramarin.se>
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
cmake_minimum_required (VERSION 3.22)


link_libraries (ujson)


add_executable (find_jvalue find_jvalue.cpp)
add_executable (initialize-jvalue initialize-jvalue.cpp)
add_executable (json-object json-object.cpp)
add_executable (json-array json-array.cpp)
add_executable (json-number json-number.cpp)

add_executable (jtokens jtokens.cpp)
