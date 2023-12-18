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


# Project name and info
#
project (libujson
    VERSION 4.2.0
    LANGUAGES CXX
    DESCRIPTION "A JSON C++ library and utility applications for handling JSON documents."
    HOMEPAGE_URL https://github.com/alfmep/libujson
    )

# C++ standard
#
set (CMAKE_CXX_STANDARD 17)


# Default GNU installation directories
#
include (GNUInstallDirs)


# Common compiler flags
#
set (gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set (msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
add_compile_options (
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-pipe;-Wall>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
    )
if (CMAKE_BUILD_TYPE MATCHES "^[Dd]ebug")
    # If build type is debug, add flag -Og for GCC et al.
    add_compile_options (
        "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Og>>"
        )
endif()


# Options
#
option (BUILD_SHARED_LIBS "Build shared library." ON)
option (DISABLE_GMPXX "Don't use libgmpxx to support numbers with arbitrary precision. Default is to use libgmpxx if found." OFF)
option (BUILD_UTILS "Build utility applications." ON)
option (BUILD_EXAMPLES "Build example applications." OFF)
option (BUILD_TESTS "Build test applications." OFF)
option (BUILD_DOC "Generate documentation (if doxygen is found)." ON)


# Support for console color codes
#
if (UNIX)
    set (UJSON_HAS_CONSOLE_COLOR "1")
else()
    set (UJSON_HAS_CONSOLE_COLOR "0")
endif()


# Dependencies
#
if (DISABLE_GMPXX)
    message (STATUS "Don't use libgmpxx to support numbers with arbitrary precision, instead numbers are represented by type double.")
    set (GMPXX_FOUND False)
else()
    include (find-gmpxx.cmake)
endif()

if (GMPXX_FOUND)
    set (UJSON_HAVE_GMPXX "1")
    set (PREDEFINED_IN_DOXYGEN "UJSON_HAVE_GMPXX=1")
else()
    set (UJSON_HAVE_GMPXX "0")
endif()


# libujson
#
add_subdirectory (src)

# Utility applications
#
if (BUILD_UTILS)
    add_subdirectory (utils)
endif()

# Example applications
#
if (BUILD_EXAMPLES)
    add_subdirectory (examples)
endif()

# test applications
#
if (BUILD_TESTS)
    add_subdirectory (test)
endif()

# Doxygen documentation
#
if (BUILD_DOC)
    find_package (Doxygen)
    if (DOXYGEN_FOUND)
        add_subdirectory (doc)
    endif()
endif()


# Add targets for distributable files
#
include (InstallRequiredSystemLibraries)

set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set (CPACK_GENERATOR "TGZ" "TXZ" "ZIP")
set (CPACK_SOURCE_GENERATOR "TGZ" "TXZ" "ZIP")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${libujson_VERSION_MAJOR}.${libujson_VERSION_MINOR}.${libujson_VERSION_PATCH}")
include (CPack)


# Un-installation
#
if (NOT TARGET uninstall)
    configure_file (
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target (uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()


# Print summary
#
message (STATUS "Summary:")
if (CMAKE_BUILD_TYPE MATCHES "^$")
    message (STATUS "    Build type........................... [default]")
else()
    message (STATUS "    Build type........................... ${CMAKE_BUILD_TYPE}")
endif()
message (STATUS "    Installation prefix.................. ${CMAKE_INSTALL_PREFIX}")
if (GMPXX_FOUND)
    message (STATUS "    Using gmpxx.......................... yes - numbers with arbitrary precision supported")
else()
    message (STATUS "    Using gmpxx.......................... no - numbers are represented by type double")
endif()
if (BUILD_UTILS)
    message (STATUS "    Build utilities ..................... yes")
else()
    message (STATUS "    Build utilities ..................... no")
endif()
if (BUILD_EXAMPLES)
    message (STATUS "    Build example applications........... yes (example applications are not installed)")
else()
    message (STATUS "    Build example applications........... no")
endif()
if (BUILD_TESTS)
    message (STATUS "    Build test applications.............. yes (test applications are not installed)")
else()
    message (STATUS "    Build test applications.............. no")
endif()
if (DOXYGEN_FOUND)
    message (STATUS "    Generate API documentation........... yes")
else()
    message (STATUS "    Generate API documentation........... no")
endif()
