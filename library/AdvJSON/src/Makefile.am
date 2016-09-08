ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -I$(top_srcdir)/inc -fPIC
AM_CXXFLAGS = -I$(top_srcdir)/inc -fPIC

lib_LTLIBRARIES = libAdvJSON.la 
#libAdvCJSON.la
libAdvJSON_la_SOURCES = AdvJSON.cpp AdvCollection.cpp AdvVariousType.cpp AdvCJSON.c jsontool.c jsonrule.c jsonparser.c jsonprint.c jsoncollection.c jsoncreator.c
#libAdvCJSON_la_SOURCES = AdvCJSON.c jsontool.c jsonrule.c jsonparser.c jsonprint.c jsoncollection.c jsoncreator.c
#libAdvCJSON_la_CFLAGS = -I../inc -fPIC

include_HEADERS = ../inc/AdvJSON.h ../inc/AdvJSONParallel.h ../inc/AdvJSONCreator.h ../inc/AdvJSONCreatorParallel.h ../inc/AdvVariousType.h ../inc/AdvCollection.h
noinst_HEADERS = ../inc/jsoncollection.h ../inc/jsonparser.h ../inc/jsonrule.h ../inc/jsontype.h ../inc/jsoncreator.h ../inc/jsonprint.h ../inc/jsontool.h ../inc/platform.h
