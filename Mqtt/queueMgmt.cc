#include "queueMgmt.h"
#include "mqttBusObject.h"
#define eI_MODULE "QUEUEMGMT"

queueMgmt::queueMgmt()
{
	filePtr = new FileOPS();
	mqttPtr = NULL;
}

queueMgmt::queueMgmt(const queueMgmt &queueObj)
{
	filePtr = new FileOPS();
	filePtr = queueObj.filePtr;

	mqttPtr = NULL;
	mqttPtr = queueObj.mqttPtr;

}

queueMgmt::~queueMgmt()
{
	if(mqttPtr)
		delete mqttPtr;
	if(filePtr)
		delete filePtr;

}

/*
 * brief : get the mqttBusObject pointer.
 */
void queueMgmt::getMqttBusPtr(mqttBusObject *mqttBusPtr)
{
	if(tempBool == false)
	{
		tempBool = true;
		mqttPtr = mqttBusPtr;
	}
}

/*
 * @brief: Adds packet into messageQueue with respect to different app's and runs the messagesignalfunction in an async thread.
 */

void queueMgmt::pushPacket(messagePacket_t packet)
{
	LOGINFO("(queueMgmt)pushPacket...");

	try
	{
		messageQueue.push(packet);

		vectorObj.push_back(std::async(std::launch::async, &queueMgmt::messageSignalFunction, this));

		//for (auto it = vectorObj.begin(); it != vectorObj.end(); ++it){
		//	delete *it;
		//}
		vectorObj.erase(vectorObj.begin(),vectorObj.begin()+1);
		//vectorObj.clear();
	}
	catch(...)
	{
		LOGEROR("(queueMgmt) pushPacket catch");
	}
}


/*
 * @brief : To be called by the message callback for sending the event info to the respective app's.
 */
void queueMgmt::messageSignalFunction()
{

	LOGDBUG("(mqttBusObject) messageSignalFunction");


	QStatus status;
	try
	{
		bool isSignalSend(false);

		while(!messageQueue.empty())
		{
			messagePacket_t packet = messageQueue.front();

			uint16_t id = packet.endpointID;
			std::string jsonString = packet.msg;
			json jsonObj;
			std::string topic;
			std::string appName;

			MsgArg args[1];
			args[0].Set("s", jsonString.data());

			// Find the approriate node from the provided endpointID from the list
			for(connectionMap node : mqttPtr->activeConnectionList)
			{
				if(node.endpointID == id)
				{
					jsonObj = json::parse(jsonString);
					topic = jsonObj[TOPIC];
					jsonObj[ENDPOINTID] = id;

					appName = mqttPtr->getAppName(topic);

					LOGINFO2("(mqttBusObject) messageSignalFunction topic", topic);
					LOGINFO2("(mqttBusObject) messageSignalFunction appname", appName);

					std::string command = mqttPtr->getCommand(topic, appName);
					jsonObj["command"] = command;

					status = mqttPtr->proxycall(id, appName, jsonObj.dump(), command, topic);

					isSignalSend = true;
				}
			}

			if(!messageQueue.empty())
				messageQueue.pop();

		}
		if(!isSignalSend)
		{
			LOGWARN("Failed to Send messageSignalFunction Signal");
		}
	}
	catch(...)
	{
		LOGEROR("(mqttBusObject) messageSignalFunction exception");
	}
	return ;

}
