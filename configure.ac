#
# Copyright (C) 2017,2020,2021 Dan Arrhenius <dan@ultramarin.se>
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
AC_INIT([libujson], [2.0.0], [dan@ultramarin.se])
AM_INIT_AUTOMAKE([-Wall -Werror foreign dist-bzip2 subdir-objects])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_MKDIR_P
AC_PROG_INSTALL
AC_PROG_YACC
AM_PROG_AR
AM_PROG_LEX

LT_INIT()
LT_LANG([C++])

AC_CONFIG_MACRO_DIR([m4])
#AC_CONFIG_HEADERS(src/include/ujson/config.h src/include/ujson/ujson_config.hpp)
AM_SILENT_RULES([yes])

#
# Release 2.0.0
#
# Library interface version x:y:z
#                           | | |
#                           | | +- AGE - how many previous versions of CURRENT this interface is compatible with.
#                           | |
#                           | +--- REVISION - Update this number when no interface change has been made.
#                           |
#                           +----- CURRENT - Update this number when an interface change has been made.
#
# Library version (CURRENT:REVISION:AGE)
#
LIBRARY_VERSION=2:0:0
AC_SUBST([LIBRARY_VERSION])

#
# Libraries to be listed in ujson.pc
#
LIBS_IN_PC_FILE=

#
# Required packages to be listed in ujson.pc
#
REQUIRE_IN_PC_FILE=

#
# Defines in the doxygen config file
#
PREDEFINED_IN_DOXYGEN=


#
# Give the user an option to enable parser trace debugging
#
AC_ARG_ENABLE([parser-debugging],
	[AS_HELP_STRING([--enable-parser-debugging],
		[enable API for debugging the libujson parser])],,
	enable_parser_debugging=no)
AM_CONDITIONAL([ENABLE_PARSER_DEBUGGING_SET], [test "x$enable_parser_debugging" != "xno"])
AM_COND_IF([ENABLE_PARSER_DEBUGGING_SET],
	[AC_DEFINE([PARSER_DEBUGGING], [1])
	 PREDEFINED_IN_DOXYGEN+=" PARSER_DEBUGGING=1"
	],
	[AC_DEFINE([PARSER_DEBUGGING], [0])]
)


#
# Give the user an option to not build utility applications
#
AC_ARG_ENABLE([utils],
	[AS_HELP_STRING([--disable-utils],
	[don't build utility applications])])
AM_CONDITIONAL([ENABLE_UTILS_SET], [test "x$enable_utils" != "xno"])


#
# Give the user an option to not build example applications
#
AC_ARG_ENABLE([examples],
	[AS_HELP_STRING([--enable-examples],
	[build examples, but they are not installed])],,
	enable_examples=no)
AM_CONDITIONAL([ENABLE_EXAMPLES_SET], [test "x$enable_examples" != "xno"])


#
# All libraries are added
#
AC_SUBST([LIBS_IN_PC_FILE])
AC_SUBST([REQUIRE_IN_PC_FILE])
AC_SUBST([PREDEFINED_IN_DOXYGEN])


#
# Give the user an option to build doxygen documentation
#
AC_ARG_ENABLE([doxygen],
	[AS_HELP_STRING([--enable-doxygen],
	[build doxygen documentation, if possible])],,
	enable_doxygen=no)
AM_CONDITIONAL([ENABLE_DOXYGEN_SET], [test "x$enable_doxygen" != "xno"])


AM_COND_IF([ENABLE_DOXYGEN_SET],
	[
	#
	# Check for doxygen support
	#
	AC_CHECK_PROGS([DOXYGEN], [doxygen])
	if test -z "$DOXYGEN";
	   then AC_MSG_WARN([doxygen not found - continuing without doxygen support])
	fi
	AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
	AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doc/doxygen.cfg])])

	# Disable dot in doxygen
	AM_CONDITIONAL([HAVE_DOT], false)
	AM_COND_IF([HAVE_DOT], [DOXYGEN_HAVE_DOT=YES], [DOXYGEN_HAVE_DOT=NO])
	AM_COND_IF([HAVE_DOT], [DOXYGEN_HAVE_CLASS_GRAPH=NO], [DOXYGEN_HAVE_CLASS_GRAPH=YES])
	AC_SUBST([DOXYGEN_HAVE_DOT])
	AC_SUBST([DOXYGEN_HAVE_CLASS_GRAPH])
	],
	[
	AM_CONDITIONAL([HAVE_DOXYGEN], false)
	AM_CONDITIONAL([HAVE_DOT], false)
	]
)


AC_OUTPUT([
	Makefile
	src/Makefile
	src/ujson.pc
	utils/Makefile
	examples/Makefile
	doc/Makefile
])

AC_MSG_NOTICE([])
AC_MSG_NOTICE([*************************************************************])
AC_MSG_NOTICE([* Installation prefix  : ${prefix}])
AM_COND_IF([ENABLE_UTILS_SET],
	[AC_MSG_NOTICE([* Utilities            : enabled])],
	[AC_MSG_NOTICE([* Utilities            : disabled])]
)
AM_COND_IF([ENABLE_EXAMPLES_SET],
	[AC_MSG_NOTICE([* Examples             : enabled (exmples are not installed)])],
	[AC_MSG_NOTICE([* Examples             : disabled])]
)
AM_COND_IF([HAVE_DOXYGEN],
	[AC_MSG_NOTICE([* Doxygen documentation: enabled])],
	[AC_MSG_NOTICE([* Doxygen documentation: disabled])]
)
AC_MSG_NOTICE([*************************************************************])
AC_MSG_NOTICE([])
