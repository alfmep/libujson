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


#
# JSON parser test
#
set (PARSE_TEST_SCRIPT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/run-ujson-parse-test.sh")
set (PARSE_TEST_SCRIPT_DST "${CMAKE_CURRENT_BINARY_DIR}/run-ujson-parse-test.sh")

set (PARSE_PATCH_SRC "${CMAKE_CURRENT_SOURCE_DIR}/JSONTestSuite-libujson.patch")
set (PARSE_PATCH_DST "${CMAKE_CURRENT_BINARY_DIR}/JSONTestSuite-libujson.patch")

add_executable (ujson-parse-test ujson-parse-test.cpp)

add_custom_command (OUTPUT ${PARSE_TEST_SCRIPT_DST}
    COMMAND ${CMAKE_COMMAND} -E copy ${PARSE_TEST_SCRIPT_SRC} ${PARSE_TEST_SCRIPT_DST}
    DEPENDS ${PARSE_TEST_SCRIPT_SRC}
    )
add_custom_command (OUTPUT ${PARSE_PATCH_DST}
    COMMAND ${CMAKE_COMMAND} -E copy ${PARSE_PATCH_SRC} ${PARSE_PATCH_DST}
    DEPENDS ${PARSE_PATCH_SRC}
    )
add_custom_target (parse-test-script ALL DEPENDS ${PARSE_TEST_SCRIPT_DST} ${PARSE_PATCH_DST})


#
# JSON patch test
#
set (PATCH_TEST_SCRIPT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/run-ujson-patch-test.sh")
set (PATCH_TEST_SCRIPT_DST "${CMAKE_CURRENT_BINARY_DIR}/run-ujson-patch-test.sh")

add_executable (ujson-patch-test ujson-patch-test.cpp)

add_custom_command (OUTPUT ${PATCH_TEST_SCRIPT_DST}
    COMMAND ${CMAKE_COMMAND} -E copy ${PATCH_TEST_SCRIPT_SRC} ${PATCH_TEST_SCRIPT_DST}
    DEPENDS ${PATCH_TEST_SCRIPT_SRC}
    )
add_custom_target (patch-test-script ALL DEPENDS ${PATCH_TEST_SCRIPT_DST})
