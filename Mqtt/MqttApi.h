#ifndef MQTT_API_H
#define MQTT_API_H

#include "cloudEndpoint.h"
#include <sstream>
#include "mqttBusObject.h"
#include "alljoynWrapper.h"
#include "configuration.h"
#include "CommonHeader/debugLog.h"


class mqttBusObject;
struct connectionMap;
struct statusInfo;
struct messagePacket_t;

class MqttApi
{
public:

	explicit MqttApi(mqttBusObject *mqttPtr);

	MqttApi(const MqttApi &mqttApiObj);

	~MqttApi();

	std::string getConnectionList();

	QStatus updateConnectionJson();

	void updatedConnectionList(std::string connectList);

	QStatus registerEndpoint(std::string &id, std::string uniqueName, std::string &payload);

	//bool deregisterEndpoint(std::string &payload, std::string uniqueName);

	uint16_t connect(std::string &payload, std::string uniqueName);

	std::string getSSLPath(std::string path);

	bool checkExistence(const std::string &entityPath);

	QStatus publish(uint16_t ID, std::string &payload);

	QStatus subscribe(uint16_t ID, std::string &payload);

	QStatus unsubscribe(uint16_t ID, std::string &payload);

	QStatus disconnect(uint16_t ID, std::string &payload, std::string uniqueName);

private:

	mqttBusObject *mqttPtr;
	cloudEndpoint* mqttEndpoint;
	cloudWrapper *_cloudWrapper;
	bool certReq = true;

	FileOPS *filePtr;

	std::string connList;
	bool getconn = false;

	static uint16_t endpointID;
};

#endif // MQTT_API_H
