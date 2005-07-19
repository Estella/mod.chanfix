## Process this file with automake to produce Makefile.in
# "$Id$"

lib_LTLIBRARIES = libchanfix.la

libchanfix_la_SOURCES = chanfix.cc \
	CHANFIXCommand.cc \
	CHECKCommand.cc \
	INVITECommand.cc \
	OPNICKSCommand.cc \
	QUOTECommand.cc \
	RELOADCommand.cc \
	SCORECommand.cc \
	SHUTDOWNCommand.cc \
	STATUSCommand.cc \
	sqlChannel.cc \
	sqlChanOp.cc 

libchanfix_la_CXXFLAGS = -I$(PGSQL_INCLUDE) -I${top_srcdir}/include \
	-I${top_srcdir}/libgnuworld \
	-DHAVE_NAMESPACE_STD -DHAVE_CXX_STRING_HEADER -DDLLIMPORT=""

libchanfix_la_LDFLAGS = -module -export-dynamic -L@PGSQL_LIB@ -lpq++

EXTRA_DIST = chanfix.h \
	chanfix.example.conf.in \
	chanfixCommands.h \
	chanfix_config.h \
	chanfix_misc.h \
	levels.h \
	sqlChannel.h \
	sqlChanOp.h \
	sqlUser.h \
	Timer.h \
        $(COMMAND_CONF)

confdir=${prefix}/bin
conf_DATA = chanfix.example.conf \
        $(COMMAND_CONF)
