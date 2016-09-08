ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -fPIC -I$(top_srcdir)/inc -I$(top_srcdir)/library/WiseCore/inc -I$(top_srcdir)/library/AdvLog/inc
AM_CXXFLAGS = -fPIC -I$(top_srcdir)/inc -I$(top_srcdir)/library/WiseCore/inc -I$(top_srcdir)/library/AdvLog/inc

lib_LTLIBRARIES = libWiseSnail.la
libWiseSnail_la_SOURCES = wiseaccess.c wiseagentlite.c wisestorage.c wiseutility.c WiseSnail.c
#libWiseSnail_la_LDFLAGS = -Wl,--whole-archive ../library/WiseCore/src/.libs/libWiseCore.so -Wl,--no-whole-archive
libWiseSnail_la_LIBADD =
if ENABLE_ADVLOG
libWiseSnail_la_LIBADD += $(top_builddir)/library/AdvJSON/src/libAdvJSON.la $(top_builddir)/library/AdvLog/src/libAdvLog.la 
else
AM_CFLAGS+=-DNATIVE_PRINTF
AM_CXXFLAGS+=-DNATIVE_PRINTF
endif
#$(top_builddir)/library/WiseCore/src/libWiseCore.la

include_HEADERS = ../inc/WiseSnail.h
noinst_HEADERS = ../inc/wiseaccess.h ../inc/wiseagentlite.h ../inc/wisestorage.h ../inc/wiseutility.h ../inc/snail_version.h

../inc/snail_version.h: snail_version.in

snail_version.in:
	`./$(top_srcdir)/script/gen_version.sh $(top_srcdir)/script/version.def ../inc/snail_version.h`
