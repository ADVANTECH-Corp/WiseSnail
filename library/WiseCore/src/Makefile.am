ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -fPIC -I$(top_srcdir)/inc
AM_CXXFLAGS = -fPIC -I$(top_srcdir)/inc

lib_LTLIBRARIES =
noinst_HEADERS = ../inc/liteparse.h ../inc/MQTTConnector.h ../inc/susiaccess_def.h ../inc/version.h ../inc/WiseCarrier_MQTT.h ../inc/WISEConnector.h

if ENABLE_CARRIER_PAHO
lib_LTLIBRARIES += libWiseCore_MQTT_Paho.la
libWiseCore_MQTT_Paho_la_SOURCES = liteparse.c WISECore.c MQTT/MQTTConnector.c MQTT/Paho/PahoCarrier.c
libWiseCore_MQTT_Paho_la_LDFLAGS = -lpaho-mqtt3cs -lssl -lcrypto
noinst_HEADERS += ../inc/PahoCarrier.h
endif

if ENABLE_CARRIER_MOSQUITTO
lib_LTLIBRARIES += libWiseCore_MQTT_Mosquitto.la
libWiseCore_MQTT_Mosquitto_la_SOURCES = liteparse.c WISECore.c MQTT/MQTTConnector.c MQTT/Mosquitto/topic.c MQTT/Mosquitto/MosquittoCarrier.c
libWiseCore_MQTT_Mosquitto_la_LDFLAGS = -lmosquitto -lssl -lcrypto
noinst_HEADERS += ../inc/topic.h
endif

noinst_HEADERS += ../inc/WISECore.h
