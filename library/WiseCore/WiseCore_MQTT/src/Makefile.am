ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -fPIC -I$(top_srcdir)/inc -I$(top_srcdir)/../inc -I$(top_srcdir)/../../WiseCarrier/inc
AM_CXXFLAGS = -fPIC -I$(top_srcdir)/inc -I$(top_srcdir)/../inc -I$(top_srcdir)/../../WiseCarrier/inc

lib_LTLIBRARIES =
noinst_HEADERS = ../inc/liteparse.h ../inc/MQTTConnector.h ../inc/susiaccess_def.h ../inc/WISEConnector.h

lib_LTLIBRARIES += libWiseCore_MQTT.la
libWiseCore_MQTT_la_SOURCES = liteparse.c WISECore.c MQTT/MQTTConnector.c
libWiseCore_MQTT_la_LDFLAGS = 
