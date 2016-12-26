
#CC=arm-openwrt-linux-gcc
#CPP=arm-openwrt-linux-g++
#AR=arm-openwrt-linux-ar

CC=gcc
CPP=g++
AR=ar

INCPATH=-I.
LIBPATH=-L.
LIBS=-lpthread -lrt -lWiseSnail -lWiseCore_MQTT -lWiseCarrier_MQTT_Paho -lpaho-mqtt3cs -lssl -lcrypto
LIBS2=-lpthread -lrt -lWiseSnail -lWiseCore_MQTT -lWiseCarrier_MQTT_Mosquitto -lmosquitto -lssl -lcrypto

CFLAGS=-g
LDFLAGS=

ProgramName=test

DEPEND=test.o


all: $(ProgramName)

$(ProgramName): $(DEPEND)
	$(CC) -o $@ $^ $(CFLAGS) $(INCPATH) $(LIBPATH) $(LIBS) $(LDFLAGS)

.c.o:
	$(CC) -c $(INCPATH) $(CFLAGS) $<

.cpp.o:
	$(CC) -c $(INCPATH) $(CFLAGS) $<

clean:
	rm -rf *.o $(ProgramName)
