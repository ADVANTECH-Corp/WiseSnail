ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -O0 -fPIC -I$(top_srcdir)/inc -I$(top_srcdir)/library/WiseCore/inc
AM_CXXFLAGS = -O0 -fPIC -I$(top_srcdir)/inc -I$(top_srcdir)/library/WiseCore/inc

lib_LTLIBRARIES = libWiseSnail.la
libWiseSnail_la_SOURCES = wisememory.c wiseaccess.c wiseagentlite.c wisestorage.c wiseutility.c WiseSnail.c base64.c
libWiseSnail_la_LIBADD =

include_HEADERS = ../inc/WiseSnail.h
noinst_HEADERS = ../inc/wisememory.h ../inc/wiseaccess.h ../inc/wiseagentlite.h ../inc/wisestorage.h ../inc/wiseutility.h ../inc/base64.h ../inc/snail_version.h

../inc/snail_version.h: snail_version.in

snail_version.in:
	`./$(top_srcdir)/script/gen_version.sh $(top_srcdir)/script/version.def ../inc/snail_version.h`
