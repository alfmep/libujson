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


# Utilities
#
add_executable (ujson-print ujson-print.cpp option-parser.cpp)
add_executable (ujson-verify ujson-verify.cpp option-parser.cpp)
add_executable (ujson-get ujson-get.cpp option-parser.cpp)
add_executable (ujson-patch ujson-patch.cpp option-parser.cpp)
#add_executable (ujson-cmp ujson-cmp.cpp option-parser.cpp)
add_executable (ujson-tool ujson-tool.cpp option-parser.cpp)

# Manpages
#
if (UNIX)
    configure_file (ujson-print.1.in ujson-print.1)
    configure_file (ujson-verify.1.in ujson-verify.1)
    configure_file (ujson-get.1.in ujson-get.1)
    configure_file (ujson-patch.1.in ujson-patch.1)
    #configure_file (ujson-cmp.1.in ujson-cmp.1)
    configure_file (ujson-tool.1.in ujson-tool.1)
endif()


# Installation
#
install (TARGETS ujson-print DESTINATION ${CMAKE_INSTALL_BINDIR})
install (TARGETS ujson-verify DESTINATION ${CMAKE_INSTALL_BINDIR})
install (TARGETS ujson-get DESTINATION ${CMAKE_INSTALL_BINDIR})
install (TARGETS ujson-patch DESTINATION ${CMAKE_INSTALL_BINDIR})
#install (TARGETS ujson-cmp DESTINATION ${CMAKE_INSTALL_BINDIR})
install (TARGETS ujson-tool DESTINATION ${CMAKE_INSTALL_BINDIR})
if (UNIX)
    install (
        FILES "${PROJECT_BINARY_DIR}/utils/ujson-print.1"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1")
    install (
        FILES "${PROJECT_BINARY_DIR}/utils/ujson-verify.1"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1")
    install (
        FILES "${PROJECT_BINARY_DIR}/utils/ujson-get.1"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1")
    install (
        FILES "${PROJECT_BINARY_DIR}/utils/ujson-patch.1"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1")
    #install (
    #    FILES "${PROJECT_BINARY_DIR}/utils/ujson-cmp.1"
    #    DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1")
    install (
        FILES "${PROJECT_BINARY_DIR}/utils/ujson-tool.1"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1")
endif()
