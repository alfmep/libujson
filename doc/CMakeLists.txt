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


if (GMPXX_FOUND)
    set (PREDEFINED_IN_DOXYGEN "UJSON_HAVE_GMPXX=1")
else()
    set (PREDEFINED_IN_DOXYGEN "UJSON_HAVE_GMPXX=0")
endif()


configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/doxygen.cfg.in
    ${CMAKE_CURRENT_BINARY_DIR}/doxygen.cfg
    @ONLY
    )


# Generate doxygen documentation
#
add_custom_command (OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.cfg
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/doxygen.cfg.in
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.hpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../src/ujson/*.hpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation"
    VERBATIM
    )
add_custom_target (doxygen ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
    )

# Remove html output on "make clean"
set_directory_properties (PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/html)


# Installation
#
if (UNIX)
    include (GNUInstallDirs)
    set (HTML_INSTALL_DESTINATION ${CMAKE_INSTALL_DOCDIR})
else()
    set (HTML_INSTALL_DESTINATION ./share/doc/${PROJECT_NAME})
endif()

install (
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION ${HTML_INSTALL_DESTINATION}
    )
