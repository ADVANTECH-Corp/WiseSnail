#!/bin/bash
apt update
apt-get -y install git
apt-get -y install build-essential
apt-get -y install automake
apt-get -y install autoconf
apt-get -y install libtool
apt-get -y install cmake

#
apt-get -y install libmosquitto-dev

# for paho.mqtt.c
apt-get -y install libssl-dev
apt-get -y install doxygen
apt-get -y install graphviz

