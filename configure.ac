echo Configuring evilnet development's mod.chanfix...
echo

AC_REVISION($Revision$)
AC_INIT
AC_CONFIG_SRCDIR([chanfix.cc])
AM_INIT_AUTOMAKE(chanfix,1.0)
AC_CONFIG_HEADERS(defs_chanfix.h)

AC_DISABLE_STATIC
AC_ENABLE_SHARED

AC_PROG_INSTALL

dnl Substitute INCLTDL and LIBLTDL in the Makefiles
dnl For some reason, INCLTDL and LIBLTDL aren't set
dnl properly on OpenBSD
LIBLTDL="$srcdir/../libltdl/libltdlc.la"

AM_MAINTAINER_MODE
AC_ISC_POSIX
AC_PREFIX_DEFAULT(`pwd`/..)

dnl Check for dlopen support
AC_LIBTOOL_DLOPEN

dnl Configure libtool
AM_PROG_LIBTOOL

CXXFLAGS="-W -Wall -pipe -I$srcdir/../libltdl -I$srcdir/../include"
CXXFLAGS="$CXXFLAGS -I$srcdir/../libgnuworld"
CFLAGS="$CXXFLAGS"

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_LANG([C++])

AC_PROG_MAKE_SET(gmake)

dnl ----------------------------------------------------------
dnl
dnl Checking for headers, functions, and a type declarations
dnl

AC_CHECK_HEADERS(sys/file.h unistd.h sys/stat.h sys/ioctl.h sys/resource.h )
AC_CHECK_HEADERS(sys/time.h asm/ioctls.h xti_inet.h sys/filio.h getopt.h)
AC_CHECK_HEADERS(sys/types.h sys/socket.h netinet/in.h sys/un.h pthread.h)

AC_HEADER_STDC
AC_HEADER_TIME
AC_TYPE_SIGNAL
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T

AC_ARG_WITH(debug,
	AC_HELP_STRING([--without-debug],
		       [Disable debugging in the executables]),
	[dummy=true],
	[CXXFLAGS="$CXXFLAGS -g"]
)

AC_ARG_WITH(pedantic,
	AC_HELP_STRING([--with-pedantic],
			[Add -pedantic to compile options]),
	[CXXFLAGS="$CXXFLAGS -pedantic"]
)

AC_ARG_WITH(optimization,
	AC_HELP_STRING([--with-optimization],
			[Optimize the executables]),
	[CXXFLAGS="$CXXFLAGS -O2"],
	[CXXFLAGS="$CXXFLAGS -O0"]
)

AC_ARG_WITH(prof,
	AC_HELP_STRING([--with-prof],
			[Generate profiling information]),
	[CXXFLAGS="$CXXFLAGS -pg"]
)

check_pgsql=yes
AC_ARG_WITH(pgsql,
	AC_HELP_STRING([--without-pgsql],
			[Do not check for pgsql.  This will cause \
several modules to be uncompilable]),
	[check_pgsql=$withval]
)

PGSQL_LIB=/usr/local/pgsql/lib
AC_ARG_WITH(pgsql-lib,
	AC_HELP_STRING([--with-pgsql-lib=PGSQLLIBDIR],
			[Specify location to find libpq++.so, default is \
/usr/local/pgsql/lib]),
	[PGSQL_LIB=$withval]
)
dnl echo PGSQL_LIB: $PGSQL_LIB

PGSQL_INCLUDE=/usr/local/pgsql/include
AC_ARG_WITH(pgsql-include,
	AC_HELP_STRING([--with-pgsql-include=PGSQLINCLUDEDIR],
			[Specify location to find libpq++.h, default is \
/usr/local/pgsql/include]),
	[PGSQL_INCLUDE=$withval]
)
dnl echo PGSQL_INCLUDE: $PGSQL_INCLUDE

if [test $check_pgsql = "yes" ]; then
	AC_MSG_CHECKING(for libpq++.h)
	if [test -f "$PGSQL_INCLUDE/libpq++.h"]; then
		true
	elif [test -f "/usr/local/include/libpq++.h"]; then
		PGSQL_INCLUDE=/usr/local/include
	elif [test -f "/usr/include/postgresql/libpq++.h"]; then
		PGSQL_INCLUDE=/usr/include/postgresql
	elif [test -f "/usr/include/pgsql/libpq++.h"]; then
		PGSQL_INCLUDE=/usr/include/pgsql
	elif [test -f "/usr/include/libpq++.h"]; then
		PGSQL_INCLUDE=/usr/include
	else
	        AC_MSG_ERROR( Unable to find libpq++.h )
	fi
	AC_MSG_RESULT(yes)
fi

if [test $check_pgsql = "yes" ]; then
	AC_MSG_CHECKING(for libpq++.so)
	if [test -f "$PGSQL_LIB/libpq++.so"]; then
		true
	elif [test -f "/usr/local/lib/libpq++.so"]; then
		PGSQL_LIB=/usr/local/lib
	elif [test -f "/usr/lib/libpq++.so"]; then
		PGSQL_LIB=/usr/lib
	elif [test -f "/usr/lib/postgresql/libpq++.so"]; then
		PGSQL_LIB=/usr/lib/postgresql
	elif [test -f "/usr/lib/pgsql/libpq++.so"]; then
		PGSQL_LIB=/usr/lib/pgsql
	elif [test -f "/usr/local/pgsql/lib/libpq++.so"]; then
		PGSQL_LIB=/usr/local/pgsql/lib
	elif [test -f "/usr/local/postgresql/lib/libpq++.so"]; then
		PGSQL_LIB=/usr/local/postgresql/lib
	else
	        AC_MSG_ERROR( Unable to find libpq++.so)
	fi
	AC_MSG_RESULT(yes)
fi

dnl echo "PGSQL_INCLUDE: $PGSQL_INCLUDE"
dnl echo "PGSQL_LIB: $PGSQL_LIB"
dnl exit

LIBGNUWORLD="../libgnuworld/libgnuworld.la"
AC_SUBST(LIBGNUWORLD)

AC_SUBST(CXXFLAGS)
AC_SUBST(INCLTDL)
AC_SUBST(LIBLTDL)
AC_SUBST(PGSQL_INCLUDE)
AC_SUBST(PGSQL_LIB)

AC_CONFIG_FILES([Makefile \
chanfix.example.conf])

AC_OUTPUT
