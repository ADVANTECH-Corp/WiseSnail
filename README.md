# WiseSnail
## Introduction
WiseSnail is the short name of Wise Sensor Network Abstract Interactive Layer. It is a layer to define the topology and behavior of sensor network. A complete device represented in WiseSnail includes one virtual gateway, one sensor connectivity interface and several sensor devices. WiseSnail provides a form-like array which we call it infospec array to define the parameters of sensor device. And a form-like data array updates your real time data. So, you can easy to represent your device by creating several arrays.

## Hardware requirements
  - CPU architecture
    * x86
    * ARM
  - Minimum ram size
    * 256kb and above

## OS
  * Ubuntu ( 14.04, 16.04 )
  * Yocto ( 1.5.3 Dora )
  * OpenWRT
  * Windows ( 7, 10 )

## Build WiseSnail on Ubuntu 16.04

Install development tools
```sh
sudo apt update
sudo apt -y install git build-essential automake autoconf libtool cmake libmosquitto-dev
```

Install Eclipse Paho MQTT C client library
```sh
sudo apt update
sudo apt -y install libssl-dev doxygen graphviz
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
make
make html
sudo make install
```

Build WiseSnail
```sh
$ cd script/Ubuntu-1604
$ ./build_wisesnail.bash
```
Then, WiseSnail library, header file and sample program (test) will be installed at release folder.

Notice:
* Sample program connect to local MQTT Broker (ip: 127.0.0.1), so if you want to run sample program, you need to install a local MQTT Broker. 

Install Mosquitto Broker
```sh
sudo apt-add-repository -y ppa:mosquitto-dev/mosquitto-ppa
sudo apt update
sudo apt -y install mosquitto mosquitto-clients
```

Run sample program
```sh
cd release
./test
```

# Document
For more detail information, please visit our online Wiki:
http://ess-wiki.advantech.com.tw/view/IoTGateway/WiseSnail
