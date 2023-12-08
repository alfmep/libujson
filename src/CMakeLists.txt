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


# Add library libujson
#
add_library (ujson)


# Set library version properties
#
set_target_properties (ujson PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    )


# Set include path
#
if (GMPXX_FOUND)
    target_include_directories (ujson
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        INTERFACE
        $<INSTALL_INTERFACE:include>
        ${GMPXX_INCLUDE_DIRS}
        ${GMP_INCLUDE_DIRS}
        )
else()
    target_include_directories (ujson
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        INTERFACE
        $<INSTALL_INTERFACE:include>
        )
endif()


if (GMPXX_FOUND)
    set (REQUIRE_IN_PC_FILE "gmpxx")
else()
    set (REQUIRE_IN_PC_FILE "")
endif()


configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/ujson/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/ujson/config.hpp
    )


BISON_TARGET(Analyzer
             ujson/Analyzer.ypp
             ${CMAKE_CURRENT_BINARY_DIR}/Analyzer.cpp)
FLEX_TARGET(Lexer
            ujson/Lexer.lpp
            ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp
            DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/Lexer.hpp")
ADD_FLEX_BISON_DEPENDENCY(Lexer Analyzer)




# Link flags (libraries to link with)
#
if (GMPXX_FOUND)
    target_link_libraries (ujson
        PRIVATE
        ${GMPXX_LIBRARIES}
        ${GMP_LIBRARIES}
        INTERFACE
        ujson
        ${GMPXX_LIBRARIES}
        ${GMP_LIBRARIES}
        )
endif()



#
# Source files
#
target_sources (ujson PRIVATE
    ujson/Parser.cpp
    ujson/jvalue.cpp
    ujson/jpointer.cpp
    ujson/utils.cpp
    ujson/jparser.cpp
    ujson/jschema.cpp
    ujson/invalid_schema.cpp
    ujson/schema/validation_context.cpp
    ujson/schema/jvocabulary.cpp
    ujson/schema/jvocabulary_core.cpp
    ujson/schema/jvocabulary_applicator.cpp
    ujson/schema/jvocabulary_validation.cpp
    ujson/schema/jvocabulary_unevaluated.cpp
    ${FLEX_Lexer_OUTPUTS}
    ${BISON_Analyzer_OUTPUTS}
    )


#
# Header files
#
set (MAIN_PUBLIC_HEADER_FILE
    ujson.hpp
    )
set (PUBLIC_HEADER_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ujson/config.hpp
    ujson/multimap_list.hpp
    ujson/json_type_error.hpp
    ujson/jvalue.hpp
    ujson/jpointer.hpp
    ujson/utils.hpp
    ujson/jparser.hpp
    ujson/jschema.hpp
    ujson/invalid_schema.hpp
    )
set (PUBLIC_HEADER_FILES_SCHEMA
    ujson/schema/validation_context.hpp
    ujson/schema/jvocabulary.hpp
    ujson/schema/jvocabulary_core.hpp
    ujson/schema/jvocabulary_applicator.hpp
    ujson/schema/jvocabulary_validation.hpp
    ujson/schema/jvocabulary_unevaluated.hpp
    )


if (WIN32)
    configure_file (
        ${CMAKE_CURRENT_SOURCE_DIR}/unistd.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/unistd.h
        )
    set (PRIVATE_HEADER_FILES
        ujson/internal.hpp
        ujson/Parser.hpp
        ujson/unistd.h
        )
else()
    set (PRIVATE_HEADER_FILES
        ujson/internal.hpp
        ujson/Parser.hpp
        )
endif()


#
# Installation
#

# Library files
#
install (
    TARGETS ujson
    EXPORT ujsonTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Header files
#
install (
    FILES ${MAIN_PUBLIC_HEADER_FILE}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
install (
    FILES ${PUBLIC_HEADER_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ujson
    )
install (
    FILES ${PUBLIC_HEADER_FILES_SCHEMA}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ujson/schema
    )


# pc file
#
if (UNIX)
    configure_file (ujson.pc.in ujson.pc @ONLY)
    install (
        FILES ${CMAKE_CURRENT_BINARY_DIR}/ujson.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        )
endif()


# Install CMake project files
#
install (
    EXPORT ujsonTargets
    FILE ujsonTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ujson
    )

include (CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file (${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/ujsonConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ujson"
    )
write_basic_package_version_file (
    "${CMAKE_CURRENT_BINARY_DIR}/ujsonConfigVersion.cmake"
    VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    COMPATIBILITY AnyNewerVersion
    )
install (FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ujsonConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ujsonConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ujson
    )

export (
    EXPORT ujsonTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/ujsonTargets.cmake"
    )
