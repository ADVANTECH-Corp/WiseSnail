#!/bin/bash
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
make clean
make distclean
make
make install
