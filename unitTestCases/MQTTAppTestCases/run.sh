g++ unsubscribe.cc -std=c++0x -lgtest -lpthread ../../Mqtt/Status.h ../../Mqtt/DebugLog.h ../../Mqtt/mqttBusObject.cc  ../../Mqtt/MqttApi.cc ../../Mqtt/cloudEndpoint.cc ../../Mqtt/cloudWrapper.cc ../../Mqtt/queueMgmt.cc -ldl -o unsubscribeMqttTestCase

