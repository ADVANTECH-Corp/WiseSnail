ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -fPIC -I$(top_srcdir)/inc -I$(top_srcdir)/../../inc
AM_CXXFLAGS = -fPIC -I$(top_srcdir)/inc -I$(top_srcdir)/../../inc

lib_LTLIBRARIES =
noinst_HEADERS = 

if ENABLE_CARRIER_PAHO
lib_LTLIBRARIES += libWiseCarrier_MQTT_Paho.la
libWiseCarrier_MQTT_Paho_la_SOURCES = Paho/PahoCarrier.c
libWiseCarrier_MQTT_Paho_la_LDFLAGS = -lpaho-mqtt3cs -lssl -lcrypto
noinst_HEADERS += ../inc/PahoCarrier.h
endif

if ENABLE_CARRIER_MOSQUITTO
lib_LTLIBRARIES += libWiseCarrier_MQTT_Mosquitto.la
libWiseCarrier_MQTT_Mosquitto_la_SOURCES = Mosquitto/topic.c Mosquitto/MosquittoCarrier.c
libWiseCarrier_MQTT_Mosquitto_la_LDFLAGS = -lmosquitto -lssl -lcrypto
noinst_HEADERS += ../inc/topic.h ../inc/susiaccess_def.h
endif
