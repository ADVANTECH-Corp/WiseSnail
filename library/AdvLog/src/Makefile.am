ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -fPIC -I$(top_srcdir)/inc -I$(top_srcdir)/inc/internal -I$(top_srcdir)/../AdvJSON/inc
AM_CXXFLAGS = -fPIC -I$(top_srcdir)/inc -I$(top_srcdir)/inc/internal -I$(top_srcdir)/../AdvJSON/inc

lib_LTLIBRARIES = libAdvLog.la
libAdvLog_la_LIBADD = $(top_builddir)/../AdvJSON/src/libAdvJSON.la
libAdvLog_la_SOURCES = AdvLog.cpp configure.cpp rollingfile.cpp advstring.cpp tool.cpp
#libAdvLog_la_LDFLAGS = -Wl,--whole-archive ../../AdvJSON/src/.libs/libAdvJSON.so -Wl,--no-whole-archive
include_HEADERS = ../inc/AdvLog.h
noinst_HEADERS = ../inc/internal/advstring.h ../inc/internal/configure.h ../inc/internal/fplatform.h ../inc/internal/rollingfile.h ../inc/internal/tool.h
