#!/bin/bash

TOPDIR=$(cd $(dirname $0) && pwd)/../..
ROOTFS=${TOPDIR}/release
BIN_INSDIR=${ROOTFS}/
LIB_INSDIR=${ROOTFS}/
INC_INSDIR=${ROOTFS}/
MY_ROOT=${TOPDIR}

export CFLAGS=" -I${INC_INSDIR} -L${LIB_INSDIR}"

function f_clean() {
  if [ -d "$MY_ROOT" ]; then
    cd ${MY_ROOT} || exit 1
    if [ -f ./Makefile ]; then
        make clean
        make distclean
    fi
    cd ${MY_ROOT} || exit 1
    if [ -f ./library/AdvJSON/Makefile ]; then
        cd  ./library/AdvJSON || exit 1
        make clean
        make distclean
    fi
    cd ${MY_ROOT} || exit 1
    if [ -f ./library/AdvLog/Makefile ]; then
        cd ./library/AdvLog || exit 1
        make clean
        make distclean
    fi
  fi
}


function f_build() {
  cd ${MY_ROOT}/library/AdvJSON || exit 1
  autoreconf -f || exit 1
  ./configure --prefix=${ROOTFS} || exit 1
  make || exit 1

  cd ${MY_ROOT}/library/AdvLog || exit 1
  autoreconf -f || exit 1
  ./configure --prefix=${ROOTFS} || exit 1
  make || exit 1

  cd ${MY_ROOT}/library/WiseCore/WiseCore_MQTT || exit 1
  autoreconf -f || exit 1
  ./configure --prefix=${ROOTFS} || exit 1
  make || exit 1

  cd ${MY_ROOT}/library/WiseCarrier/MQTT/WiseCarrier_MQTT_Linux || exit 1
  autoreconf -f || exit 1
  ./configure --prefix=${ROOTFS} || exit 1
  make || exit 1

  cd ${MY_ROOT} || exit 1
  autoreconf -f || exit 1
  chmod +x ./snailconf.sh || exit 1
  chmod +x ./script/gen_version.sh || exit 1
  ./configure --prefix=${ROOTFS} || exit 1
  make || exit 1
}


function f_install() {
  mkdir -p ${BIN_INSDIR} || exit 1
  mkdir -p ${LIB_INSDIR} || exit 1
  mkdir -p ${INC_INSDIR} || exit 1
  cd ${MY_ROOT}/library/AdvJSON || exit 1
  cp -af src/.libs/libAdvJSON.so* ${LIB_INSDIR} || exit 1
  cd ${MY_ROOT}/library/AdvLog || exit 1
  cp -af src/.libs/libAdvLog.so* ${LIB_INSDIR} || exit 1
  cd ${MY_ROOT}/library/WiseCore/WiseCore_MQTT || exit 1
  cp -af ./src/.libs/libWiseCore_MQTT.so* ${LIB_INSDIR} || exit 1
  cd ${MY_ROOT}/library/WiseCarrier/MQTT/WiseCarrier_MQTT_Linux || exit 1
  cp -af ./src/.libs/libWiseCarrier_MQTT_Mosquitto.so* ${LIB_INSDIR} || exit 1
  cp -af ./src/.libs/libWiseCarrier_MQTT_Paho.so* ${LIB_INSDIR} || exit 1
  cd ${MY_ROOT} || exit 1
  cp -af ./src/.libs/libWiseSnail.so* ${LIB_INSDIR} || exit 1
  cp -af inc/WiseSnail.h ${INC_INSDIR} || exit 1
}


function f_install_sample() {
  mkdir -p ${BIN_INSDIR} || exit 1
  cd ${MY_ROOT}/sample || exit 1
  make || exit 1
  cp -af test ${BIN_INSDIR} || exit 1
}

f_clean
f_build
f_install
f_install_sample

echo "build ... done"
