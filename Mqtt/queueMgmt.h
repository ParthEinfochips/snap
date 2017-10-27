#ifndef QUEUE_MGMT_H
#define QUEUE_MGMT_H


#include <stdlib.h>
#include <sstream>
#include <future>
#include <unordered_map>
#include "CommonHeader/nsprefix.h"
#include "CommonHeader/CPPHeaders.h"
#include <queue>
#include <list>
#include <mutex>
#include <thread>         // std::thread, std::this_thread::sleep_for
#include "directoryOPS.h"

class mqttBusObject;
struct messagePacket_t;
struct connectionMap;


/* class for managing multiple queues and running the messagesigfunction asynchronously*/

class queueMgmt
{
public:
	queueMgmt();

	queueMgmt(const queueMgmt &queueObj);

	~queueMgmt();

	void pushPacket(messagePacket_t packet);

	void getMqttBusPtr(mqttBusObject *mqttBusPtr);

	void messageSignalFunction();

private:

	std::queue<messagePacket_t> messageQueue;

	bool tempBool = false;

	mqttBusObject *mqttPtr;

	FileOPS *filePtr;

	/*Std vector of futures*/
	std::vector<std::future<void>> vectorObj;

};

#endif
