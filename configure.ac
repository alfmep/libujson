#
# Copyright (C) 2017,2020-2022 Dan Arrhenius <dan@ultramarin.se>
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
AC_INIT([libujson], [3.0.0], [dan@ultramarin.se])
AM_INIT_AUTOMAKE([-Wall -Werror foreign dist-bzip2 subdir-objects])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_MKDIR_P
AC_PROG_INSTALL
AC_PROG_YACC
AM_PROG_AR
AC_PROG_LEX([noyywrap])

LT_INIT()
LT_LANG([C++])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS(src/ujson/config.hpp)
AM_SILENT_RULES([yes])

#
# Release 3.0.0
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
LIBRARY_VERSION=3:0:0
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
# Give the user an option to build with gmpxx to support numbers with arbitrary precision
# Default is autodetect
#
AC_ARG_ENABLE(
	[gmpxx],
	[AS_HELP_STRING(
		[--disable-gmpxx],
		[don't use libgmpxx to support numbers with arbitrary precision, instead numbers are represented by type double [default=autodetect]]
	)],
	enable_gmpxx=$enableval,
	enable_gmpxx=autodetect
)
AM_CONDITIONAL([ENABLE_GMPXX_AUTODETECT], [test "x$enable_gmpxx" = "xautodetect"])
AM_CONDITIONAL([ENABLE_GMPXX_SET], [test "x$enable_gmpxx" != "xno"])
AM_COND_IF([ENABLE_GMPXX_SET],
	[
		# Check for gmpxx
		PKG_CHECK_MODULES([gmpxx],
			[gmpxx],
			[
				# gmpxx found
				AM_CONDITIONAL([HAVE_GMPXX], true)
				AC_DEFINE([UJSON_HAVE_GMPXX],[1],[Define to 1 if libgmpxx is used])
				PREDEFINED_IN_DOXYGEN+=" UJSON_HAVE_GMPXX=1"
				REQUIRE_IN_PC_FILE+="gmpxx"
			],
			[
				# gmpxx not found
				AM_COND_IF([ENABLE_GMPXX_AUTODETECT],
				[
					# --enable-gmpxx=autodetect - allow building without gmpxx
					AC_MSG_WARN([Could not find gmpxx - numbers will be represented by type double]);
					AM_CONDITIONAL([HAVE_GMPXX], false)
				],
				[
					# --enable-gmpxx=yes - Fail since we can't find gmpxx
					AC_MSG_ERROR([Could not find gmpxx])
				])
			])
	],
	[
		# --disable-gmpxx - Don't check for gmpxx
		AM_CONDITIONAL([HAVE_GMPXX], false)
		AC_MSG_NOTICE([gmpxx disabled - numbers will be represented by type double])
	]
)
AC_SUBST([gmpxx_CFLAGS])
AC_SUBST([gmpxx_LIBS])


#
# Give the user an option to enable parser trace debugging
#
AC_ARG_ENABLE([parser-debugging],
	[AS_HELP_STRING([--enable-parser-debugging],
		[enable API for debugging the libujson parser])],,
	enable_parser_debugging=no)
AM_CONDITIONAL([ENABLE_PARSER_DEBUGGING_SET], [test "x$enable_parser_debugging" != "xno"])
AM_COND_IF([ENABLE_PARSER_DEBUGGING_SET],
	[AC_DEFINE([UJSON_PARSER_DEBUGGING], [1], [Define to 1 if debugging parser])
	 PREDEFINED_IN_DOXYGEN+=" UJSON_PARSER_DEBUGGING=1"
	],
	[AC_DEFINE([UJSON_PARSER_DEBUGGING], [0], [Define to 1 if debugging parser])]
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


AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/ujson.pc
	utils/Makefile
	utils/ujson-cmp.1
	utils/ujson-get.1
	utils/ujson-patch.1
	utils/ujson-print.1
	utils/ujson-verify.1
	examples/Makefile
	doc/Makefile
])

AC_OUTPUT


AC_MSG_NOTICE([])
AC_MSG_NOTICE([*************************************************************])
AC_MSG_NOTICE([* Installation prefix  : ${prefix}])
AM_COND_IF([HAVE_GMPXX],
	[AC_MSG_NOTICE([* Using gmpxx          : yes - numbers with arbitrary precision supported])],
	[AC_MSG_NOTICE([* Using gmpxx          : no - numbers are represented by type double])]
)
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
