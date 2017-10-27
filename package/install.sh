#!/bin/sh
SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

cd $SCRIPTPATH

mkdir /opt/GA.conf.d
mkdir  /opt/GA.conf.d/MQTTApp
cp -r MQTT.conf /opt/GA.conf.d/MQTTApp/MQTTAppMQTT.conf 
cp -r gatewayDtl.conf /opt/GA.conf.d/gatewayDtl.conf
