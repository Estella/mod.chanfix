## Process this file with automake to produce Makefile.in
# "$Id$"

lib_LTLIBRARIES = libchanfix.la
libchanfix_la_SOURCES = \
	chanfix.cc \
	INVITECommand.cc \
	QUOTECommand.cc \
	RELOADCommand.cc \
	SHUTDOWNCommand.cc \
	sqlChanOp.cc

libchanfix_la_CXXFLAGS = -I${top_srcdir}/include \
	-I${top_srcdir}/libgnuworld
libchanfix_la_LDFLAGS = -module -export-dynamic -L@PGSQL_LIB@ -lpq++

EXTRA_DIST = \
	chanfix.h \
	chanfixCommands.h \
	functor.h \
	levels.h \
	sqlChanOp.h \
	sqlUser.h
