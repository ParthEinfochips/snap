#include <chrono>         // std::chrono::seconds
#include "mqttBusObject.h"
#include <unistd.h>
#include "CommonHeader/debugLog.h"

#define eI_MODULE "MqttBusObject"

using namespace ajn;

uint16_t mqttBusObject::endpointID = 250;


mqttBusObject::mqttBusObject(BusAttachment* bus, const char* objectName, const char* objectPath,
		std::string adverName)
:eIBusObject(bus, objectName, objectPath),
 methodexPtr(new methodExecuter(bus, this, objectName, objectPath)),
 sigexPtr(new signalExecuter(bus, this, objectName)),
 mqttApiPtr(new MqttApi(this)), adverName(adverName)
{
	ajPtr = new ajNameService(bus, this, configurationManage::AppName);
	ajPtr->advertiseNameService(adverName.c_str(), MQTT_ASSIGNED_SESSION_PORT);
	ajPtr->findAdvertisedName(FIND_ADV_NAME_PREFIX);

	this->signalSender();
}

mqttBusObject::~mqttBusObject()
{
	if(ajPtr)
		delete ajPtr;
	if(methodexPtr)
		delete methodexPtr;
	if(sigexPtr)
		delete sigexPtr;
	if(mqttApiPtr)
		delete mqttApiPtr;

	signalThreadRunner = false;
	signalVar.notify_one();

	if(signalThread.joinable())
		signalThread.join();
}

/*
 * @brief : return mqttApiPtr.
 */
//MqttApi* mqttBusObject::GetMqttApiPtr()
//{
//	return mqttApiPtr ? mqttApiPtr : NULL;
//}

/*
 * @brief : for ramlHandler, returning Raml.
 */
QStatus mqttBusObject::getMRAMLHandler(std::string &command, std::string &payload)
{
	LOGDBUG("(mqttBusObject)GetMRAMLHandler");

	return ER_OK;
}

/*
 * @brief : asyncListenerHandler for receiving command and payload.
 */
QStatus mqttBusObject::asyncHandler(std::string &command, std::string &payload)
{
	LOGDBUG("(mqttBusObject)AsyncListenerHandler");

	QStatus status;
	json jsonObj;
	std::string appName;
	try
	{
		jsonObj = json::parse(payload);

		if(command == CONNECT || command == DISCONNECT)
		{

			if(!(jsonChecker::CheckJson(jsonObj, appName, {APPNAME})))
			{
				LOGINFO("(connect) Json Check appName is present. ");
			}
			else
			{
				LOGEROR("(connect) Json Check appName not found.");
				status = ER_FAIL;
				return status;
			}
			appName = jsonObj[APPNAME];
		}
		else
		{
			appName = "";
		}

		try
		{
			status = this->processCommand(command,payload,appName);
		}
		catch(...)
		{
			LOGEROR("(mqttBusObject)In AsyncListenerHandler Exception");
			status = ER_FAIL;
		}
	}
	catch(...)
	{
		status = ER_FAIL;
	}
	command = QCC_StatusText(status);
	return status;
}

/*
 * @brief : SyncListenerHandler for receiving command and payload.
 */
QStatus mqttBusObject::syncHandler(std::string &command, std::string &payload)
{
	LOGDBUG("(mqttBusObject)SyncListenerHandler");

	QStatus status;
	json jsonObj;
	std::string appName;
	try
	{
		jsonObj = json::parse(payload);

		if(command == CONNECT || command == DISCONNECT)
		{

			if(!(jsonChecker::CheckJson(jsonObj, appName, {APPNAME})))
			{
				LOGINFO("(connect) Json Check appName is present. ");
			}
			else
			{
				LOGEROR("(connect) Json Check appName not found.");
				status = ER_FAIL;
				return status;
			}
			appName = jsonObj[APPNAME];
		}
		else
		{
			appName = "";
		}


		try
		{
			status = this->processCommand(command,payload,appName);

			LOGINFO2("(mqttBusObject)In SyncListenerHandler status", QCC_StatusText(status));
		}
		catch(...)
		{
			LOGEROR("(mqttBusObject)In SyncListenerHandler Exception");
			status = ER_FAIL;
		}
	}
	catch(...)
	{
		status = ER_FAIL;
	}
	command = QCC_StatusText(status);
	return status;
}

/*
 * @brief : CallbackRegisterHandler for receiving command and payload.
 */
QStatus mqttBusObject::callbackRegisterHandler(std::string &command, std::string &payload)
{
	LOGDBUG("(mqttBusObject)CallbackRegisterHandler");

	QStatus status;
	json jsonObj;
	std::string appName;

	try
	{
		jsonObj = json::parse(payload);

		if(command == CONNECT || command == DISCONNECT)
		{

			if(!(jsonChecker::CheckJson(jsonObj, appName, {APPNAME})))
			{
				LOGINFO("(connect) Json Check appName is present. ");
			}
			else
			{
				LOGEROR("(connect) Json Check appName not found.");
				status = ER_FAIL;
				return status;
			}
			appName = jsonObj[APPNAME];
		}
		else
		{
			appName = "";
		}


		try
		{
			status = this->processCommand(command,payload,appName);
		}
		catch(...)
		{
			LOGEROR("(mqttBusObject)In CallbackRegisterHandler Exception");
			status = ER_FAIL;
		}
	}
	catch(...)
	{
		status = ER_FAIL;
	}

	return status;
}


/*
 * @brief : CallbackListenerHandler for receiving command and payload.
 */
void mqttBusObject::signalHandler(std::string &command, std::string &payload)
{
	LOGDBUG("(mqttBusObject)CallbackListenerHandler");

	std::cout << "IN CallbackListenerHandler command" << command << std::endl;

	QStatus status;
	json jsonObj;
	std::string appName;
	try
	{
		jsonObj = json::parse(payload);

		if(command == CONNECT || command == DISCONNECT)
		{

			if(!(jsonChecker::CheckJson(jsonObj, appName, {APPNAME})))
			{
				LOGINFO("(connect) Json Check appName is present. ");
			}
			else
			{
				LOGEROR("(connect) Json Check appName not found.");
				status = ER_FAIL;
			}
			appName = jsonObj[APPNAME];
		}
		else
		{
			appName = "";
		}

		try
		{
			status = this->processCommand(command,payload,appName);
		}
		catch(...)
		{
			LOGEROR("(mqttBusObject)In CallbackListenerHandler Exception");
			status = ER_FAIL;
		}
	}
	catch(...)
	{
		status = ER_FAIL;

	}
}

/*
 * @brief : for setting up the appName for sync and Asnc call bifurcation.
 */
void mqttBusObject::setAppName(std::string tempTopic)
{
	std::string appName;

	try
	{
		LOGDBUG("In setAppName");

		for(json::iterator jIter =  configurationManage::configFunJson.begin(); jIter != configurationManage::configFunJson.end(); jIter++)
		{
			std::size_t found = tempTopic.find(jIter.key());

			while(found != std::string::npos)
			{
				std::size_t found = tempTopic.find_last_of(".");

				while(found!=std::string::npos)
				{
					appName = tempTopic.substr(found+4);

					configurationManage::appFunMap[appName] = jIter.value();

					break;
				}

				break;
			}

		}

	}
	catch(...)
	{
		LOGEROR("In setAppName exception");
	}

}

/*
 * @brief : Calling the connect API, sending the signal directly if endpointID found.
 */
QStatus mqttBusObject::establishConnection(std::string &payload, std::string appName)
{
	LOGDBUG("(mqttBusObject)establishConnection");

	QStatus status;
	json jsonObj;
	json jsonData;
	std::stringstream ss;

	std::string strEndpointID;
	credentials cred;

	try
	{
		std::string hostName;
		uint16_t port;
		std::string retStr;

		try
		{
			retStr = mqttApiPtr->getConnectionList();
		}
		catch(...)
		{
			LOGEROR("(mqttBusObject)getConnectionList exception");
		}

		LOGINFO2("(mqttBusObject)connection string", retStr);
		try
		{
			if(retStr != "")
				jsonData = json::parse(retStr);
		}
		catch(...)
		{
			LOGEROR("(mqttBusObject)Error in connection string exception");
		}

		this->setAppName(appName);

		try
		{
			jsonObj = json::parse(payload);
		}
		catch(...)
		{
			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[ENDPOINTID] = -1;
			jsonObj[MSG] = "invalid json";

			payload = jsonObj.dump();

			std::cout << "Json response is" << payload << std::endl;
			return ER_FAIL;
		}

		/** Fetch only host & port for connection **/
		if(!(jsonChecker::CheckJson(jsonObj, hostName, {"hostname"})))
		{
			LOGINFO("(connect) hostname is present. ");
		} else {

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[ENDPOINTID] = -1;
			jsonObj[MSG] = "hostname not present";

			payload = jsonObj.dump();

			LOGEROR("(connect) Json Check hostname not found.");
			return ER_FAIL;
		}

		if(!(jsonChecker::CheckJson(jsonObj, port, {"port"})))
		{
			LOGINFO("(connect) port is present. ");
		} else {

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[ENDPOINTID] = -1;
			jsonObj[MSG] = "port not present";

			payload = jsonObj.dump();

			LOGEROR("(connect) Json Check Port not found.");
			return ER_FAIL;
		}

		hostName = jsonObj[HOSTNAME];
		port = jsonObj[PORT];

		if(hostName == "")
		{
			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[ENDPOINTID] = -1;
			jsonObj[MSG] = "hostname is empty";

			payload = jsonObj.dump();

			LOGEROR("(connect) Json Check hostname not found.");
			std::cout << "Json response is" << payload << std::endl;
			return ER_FAIL;
		}

		LOGINFO2("(mqttBusObject) Hostname", hostName);
		LOGINFO2I("(mqttBusObject) port", port);


		bool isConnectionExits = false;

		for (json::iterator it = jsonData.begin(); it != jsonData.end(); ++it)
		{
			std::cout << it.key() << " : " << it.value() << std::endl;

			std::string strHostName= it.key();
			std::string strMqqtHostName = hostName + "|" + std::to_string(port);
			if(!strHostName.compare(strMqqtHostName))
			{
				isConnectionExits = true;
				int endID = it.value();
				ss << endID;
				strEndpointID = ss.str();
			}
		}
		if(!isConnectionExits)
		{
			LOGINFO("(mqttBusObject) connect");

			uint16_t endID =  mqttApiPtr->connect(payload, appName);
			std::stringstream ss;
			ss << endID;
			strEndpointID = ss.str();
			if(endID != 0)
				status = ER_OK;
			else
				status = ER_FAIL;

		}
		else
		{
			LOGINFO("(mqttBusObject) registerEndpoint");

			status = mqttApiPtr->registerEndpoint(strEndpointID, appName, payload);

			std::cout << "PAYLOAD" << payload << std::endl;

			statusInfo statInfo;

			statInfo.endptID = std::stoi(strEndpointID);
			statInfo.appID = appName;
			statInfo.brokerIp = hostName;

			LOGINFO2I("(connect)statInfo.endptID", statInfo.endptID);

			bool entrybool = false;

			for(std::vector<statusInfo>::iterator it=statusVec.begin();it!= statusVec.end();++it)
			{
				if(it->endptID == statInfo.endptID && it->appID == statInfo.appID)
				{
					entrybool = true;
				}
			}
			if(entrybool == false)
			{
				statInfo.status = SUCCESS;
				statusVec.push_back(statInfo);
			}


			for(std::vector<statusInfo>::iterator it=statusVec.begin();it!= statusVec.end();++it)
			{
				if(it->endptID == statInfo.endptID && it->callback == "true")
				{
					if(status == ER_OK)
					{
						cred.ID = std::stoi(strEndpointID);
						cred.appName = appName;

						credMap.insert(std::make_pair(cred.ID, cred));

						jsonObj[STATUS] = SUCCESS;
						jsonObj[RESPONSECODE] = 200;
						jsonObj[ENDPOINTID] = std::stoi(strEndpointID);
						jsonObj[MSG] = "connection success";

						payload = jsonObj.dump();

						json sigJson;
						sigJson[ENDPOINTID] = std::stoi(strEndpointID);
						sigJson[RESPONSECODE] = 200;
						sigJson[STATUS] = SUCCESS;


						signalPacket_t packet;
						packet.appname = appName;
						packet.endpointid = std::stoi(strEndpointID);
						packet.jsonStr = sigJson.dump();
						packet.callbackstatus = CLOUDCONNECTIONCALLBACK;

						// Secure the access for modification of Queue.
						std::lock_guard<std::mutex> lock(signalMutex);
						// Push the signal packet into a queue for managing connection signal
						signalQ.push(packet);

						if(!isSignalThreadRunning)
						{
							signalVar.notify_one();
						}

						//status = this->sendSignal(std::stoi(strEndpointID), appName, jsonObj.dump(), CLOUDCONNECTIONCALLBACK);
					}
					else
					{
						jsonObj[STATUS] = FAILURE;
						jsonObj[RESPONSECODE] = 400;
						jsonObj[ENDPOINTID] = -1;
						jsonObj[MSG] = "connection failure";

						payload = jsonObj.dump();

						//						json sigJson;
						//						sigJson[ENDPOINTID] = -1;
						//						sigJson[RESPONSECODE] = 400;
						//						sigJson[STATUS] = FAILURE;
						//
						//
						//						signalPacket_t packet;
						//						packet.appname = appName;
						//						packet.endpointid = -1;
						//						packet.jsonStr = sigJson.dump();
						//						packet.callbackstatus = CLOUDDISCONNECTIONCALLBACK;
						//
						//						// Secure the access for modification of Queue.
						//						std::lock_guard<std::mutex> lock(signalMutex);
						//						// Push the signal packet into a queue for managing connection signal
						//						signalQ.push(packet);
						//
						//						if(!isSignalThreadRunning)
						//						{
						//							signalVar.notify_one();
						//						}

						//status = this->sendSignal(std::stoi(strEndpointID),appName, jsonObj.dump(), CLOUDDISCONNECTIONCALLBACK);
					}
				}
				else if(it->endptID == statInfo.endptID && it->callback == "false")
				{
					jsonObj[STATUS] = FAILURE;
					jsonObj[RESPONSECODE] = 400;
					jsonObj[ENDPOINTID] = -1;
					jsonObj[MSG] = "connection failure";

					payload = jsonObj.dump();

					//					json sigJson;
					//					sigJson[ENDPOINTID] = -1;
					//					sigJson[RESPONSECODE] = 400;
					//					sigJson[STATUS] = FAILURE;
					//
					//					signalPacket_t packet;
					//					packet.appname = appName;
					//					packet.endpointid = -1;
					//					packet.jsonStr = sigJson.dump();
					//					packet.callbackstatus = CLOUDDISCONNECTIONCALLBACK;
					//
					//					// Secure the access for modification of Queue.
					//					std::lock_guard<std::mutex> lock(signalMutex);
					//					// Push the signal packet into a queue for managing connection signal
					//					signalQ.push(packet);
					//
					//					if(!isSignalThreadRunning)
					//					{
					//						signalVar.notify_one();
					//					}
					//status = this->sendSignal(std::stoi(strEndpointID),appName, jsonObj.dump(), CLOUDDISCONNECTIONCALLBACK);

				}
			}
		}
	}
	catch(...)
	{
		LOGEROR("(mqttBusObject) establish connection catch");

		jsonObj = json::parse(payload);

		jsonObj[STATUS] = FAILURE;
		jsonObj[RESPONSECODE] = 400;
		jsonObj[ENDPOINTID] = -1;

		payload = jsonObj.dump();

		status = ER_FAIL;
	}

	std::cout << "In function end" << std::endl;
	return status;
}

void mqttBusObject::signalSender()
{
	signalThread = std::thread([this]{
		while(signalThreadRunner)
		{
			// Check if queue has any data
			if(!signalQ.empty() && signalQ.size() > 0)
			{
				isSignalThreadRunning = true;

				signalPacket_t pack = signalQ.front();

				try
				{
					// Wait for some time and then fire the signal
					std::this_thread::sleep_for(std::chrono::milliseconds(400));

					std::cout << "IN signal thread runner" << pack.endpointid << std::endl;
					this->sendSignal(pack.endpointid,pack.appname, pack.jsonStr, pack.callbackstatus);
				}
				catch(...)
				{

				}
				signalQ.pop();
			}
			// Wait till queue contains data
			else
			{
				isSignalThreadRunning = false;

				// Wait for the queue to have some data.
				std::unique_lock<std::mutex> uniLock(signalMutex);
				signalVar.wait(uniLock);
			}
		}
	});
}

/*
 * @brief : calling the publish API.
 */
QStatus mqttBusObject::sendToCloud(std::string &payload, std::string appName)
{
	LOGDBUG("(mqttBusObject) sendToCloud");

	QStatus status;
	json jsonObj;
	std::string jobID;
	std::string topic;
	std::string tempStr;
	json tempJson;
	std::string dataKey;

	try
	{
		jsonObj = json::parse(payload);
	}
	catch(...)
	{
		jsonObj[STATUS] = FAILURE;
		jsonObj[RESPONSECODE] = 400;
		jsonObj[MSG] = "Invalid Json";

		payload = jsonObj.dump();

		std::cout << "Json response is" << payload << std::endl;
		return ER_FAIL;

	}

	std::string tempTopic;

	try
	{
		uint16_t endpointID = 0;

		if(!(jsonChecker::CheckJson(jsonObj, endpointID, {"endpointid"})))
		{
			LOGINFO("(connect) endpointid is present. ");
		} else {

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "endpointID not present";

			payload = jsonObj.dump();

			std::cout << "Json response is" << payload << std::endl;
			return ER_FAIL;
		}

		if(!(jsonChecker::CheckJson(jsonObj, topic, {"topic"})))
		{
			LOGINFO("(connect) topic is present. ");
		} else {

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "topic not present";

			payload = jsonObj.dump();

			std::cout << "Json response is" << payload << std::endl;
			return ER_FAIL;
		}

		if(topic == "")
		{
			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "topic is empty";

			payload = jsonObj.dump();

			std::cout << "Json response is" << payload << std::endl;
			return ER_FAIL;
		}


		topic = jsonObj["topic"];

		tempTopic = topic;

		std::size_t found = tempTopic.find_first_of("/");

		while(found!=std::string::npos)
		{
			tempTopic = tempTopic.substr(found+1);

			std::size_t found1 = tempTopic.find_first_of("/");
			while(found1!=std::string::npos)
			{
				dataKey = tempTopic.substr(0,found1);
				break;
			}
			break;
		}

		if(dataKey == "Data")
		{
			endpointID = jsonObj[ENDPOINTID];

			LOGINFO2I("(mqttBusObject) sendToCloud endpointID", endpointID);

			status = mqttApiPtr->publish(endpointID, payload);
		}
		else
		{
			jsonObj.erase("topic");

			tempStr = jsonObj["msg"];
			tempJson = json::parse(tempStr);

			if(!(jsonChecker::CheckJson(tempJson, jobID, {"JOBID"})))
			{
				LOGINFO("(connect) JOBID is present. ");
			} else {

				jsonObj[STATUS] = FAILURE;
				jsonObj[RESPONSECODE] = 400;
				jsonObj[MSG] = "JOBID not present";

				payload = jsonObj.dump();

				std::cout << "Json response is" << payload << std::endl;
				return ER_FAIL;
			}

			if(jobID == "")
			{
				jsonObj[STATUS] = FAILURE;
				jsonObj[RESPONSECODE] = 400;
				jsonObj[MSG] = "jobID is empty";

				payload = jsonObj.dump();

				std::cout << "Json response is" << payload << std::endl;
				return ER_FAIL;
			}

			jobID = tempJson["JOBID"];

			std::string replyTopic = topic + "/Reply/" + jobID;

			LOGINFO2("(mqttBusObject)sendDataResponse replyTopic", replyTopic);

			jsonObj["topic"] = replyTopic;

			payload = jsonObj.dump();

			endpointID = jsonObj[ENDPOINTID];

			LOGINFO2I("(mqttBusObject) sendToCloud endpointID", endpointID);

			status = mqttApiPtr->publish(endpointID, payload);
		}

		LOGINFO2("(mqttBusObject) publish payload ", payload);

	}
	catch(...)
	{

		jsonObj[STATUS] = FAILURE;
		jsonObj[RESPONSECODE] = 400;
		jsonObj[MSG] = "invalid json";

		payload = jsonObj.dump();

		status = ER_FAIL;
	}

	return status;
}

/*
 * @brief :  Calling the subscribe API.
 */

QStatus mqttBusObject::registerTopic(std::string &payload, std::string appName)
{
	LOGDBUG("(mqttBusObject) registerTopic");

	QStatus status;
	json jsonObj;
	std::string topic;

	try
	{
		jsonObj = json::parse(payload);
	}
	catch(...)
	{
		jsonObj[STATUS] = FAILURE;
		jsonObj[RESPONSECODE] = 400;
		jsonObj[MSG] = "Invalid Json";

		payload = jsonObj.dump();

		std::cout << "Json response is" << payload << std::endl;
		return ER_FAIL;
	}

	try
	{
		uint16_t endpointID = 0;

		if(!(jsonChecker::CheckJson(jsonObj, endpointID, {"endpointid"})))
		{
			LOGINFO("(connect) endpointid is present. ");
		} else {

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "endpointID not present";

			payload = jsonObj.dump();

			std::cout << "Json response is" << payload << std::endl;
			return ER_FAIL;
		}

		if(!(jsonChecker::CheckJson(jsonObj, topic, {"topic"})))
		{
			LOGINFO("(connect) topic is present. ");
		} else {

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "topic not present";

			payload = jsonObj.dump();

			std::cout << "Json response is" << payload << std::endl;
			return ER_FAIL;
		}

		endpointID = jsonObj[ENDPOINTID];

		LOGINFO2I("(mqttBusObject) registerTopic endpointID", endpointID);

		topic = jsonObj[TOPIC];


		if(topic == "")
		{
			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "topic is empty";

			payload = jsonObj.dump();

			std::cout << "Json response is" << payload << std::endl;
			return ER_FAIL;
		}

		appName = this->getAppName(topic);

		LOGINFO2("(mqttBusObject) registerTopic appName", appName);

		queueMgmt *queueObj = new queueMgmt();

		queueObjMap[appName] = queueObj;

		status = mqttApiPtr->subscribe(endpointID, payload);

		LOGINFO2("(mqttBusObject) subscribe payload ", payload);

	}
	catch(...)
	{
		jsonObj[STATUS] = FAILURE;
		jsonObj[RESPONSECODE] = 400;
		jsonObj[MSG] = "invalid json";

		payload = jsonObj.dump();

		status = ER_FAIL;
	}

	return status;
}

/*
 * @brief : Calling the unsubscribe API.
 */
QStatus mqttBusObject::unregisterTopic(std::string &payload, std::string appName)
{
	LOGDBUG("(mqttBusObject) unregisterTopic");

	QStatus status;
	std::string topic;
	json jsonObj;

	try
	{
		jsonObj = json::parse(payload);
	}
	catch(...)
	{
		jsonObj[STATUS] = FAILURE;
		jsonObj[RESPONSECODE] = 400;
		jsonObj[MSG] = "Invalid Json";

		payload = jsonObj.dump();

		std::cout << "Json response is" << payload << std::endl;
		return ER_FAIL;

	}

	try
	{
		uint16_t endpointID = 0;

		if(!(jsonChecker::CheckJson(jsonObj, endpointID, {"endpointid"})))
		{
			LOGINFO("(connect) endpointid is present. ");
		} else {

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "endpointID not present";

			payload = jsonObj.dump();

			std::cout << "Json response is" << payload << std::endl;
			return ER_FAIL;
		}

		if(!(jsonChecker::CheckJson(jsonObj, topic, {"topic"})))
		{
			LOGINFO("(connect) topic is present. ");
		} else {

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "topic not present";

			payload = jsonObj.dump();

			std::cout << "Json response is" << payload << std::endl;
			return ER_FAIL;
		}


		endpointID = jsonObj[ENDPOINTID];
		topic = jsonObj[TOPIC];

		if(topic == "")
		{
			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "topic is empty";

			payload = jsonObj.dump();

			std::cout << "Json response is" << payload << std::endl;
			return ER_FAIL;
		}

		LOGINFO2I("(mqttBusObject) unregisterTopic endpointID", endpointID);

		status = mqttApiPtr->unsubscribe(endpointID, payload);

		LOGINFO2("(mqttBusObject) unsubscribe payload ", payload);
	}
	catch(...)
	{
		jsonObj[STATUS] = FAILURE;
		jsonObj[RESPONSECODE] = 400;
		jsonObj[MSG] = "invalid json";

		payload = jsonObj.dump();

		status = ER_FAIL;
	}

	return status;

}

/*
 * @brief : Calling the disconnect API.
 */
QStatus mqttBusObject::disconnect(std::string &payload, std::string appName)
{
	LOGDBUG("(mqttBusObject) disconnect");

	QStatus status;
	json jsonObj;

	try
	{
		jsonObj = json::parse(payload);
	}
	catch(...)
	{
		jsonObj[STATUS] = FAILURE;
		jsonObj[RESPONSECODE] = 400;
		jsonObj[MSG] = "Invalid Json";

		payload = jsonObj.dump();

		std::cout << "Json response is" << payload << std::endl;
		return ER_FAIL;
	}

	try
	{
		uint16_t endpointID = 0;

		if(!(jsonChecker::CheckJson(jsonObj, endpointID, {"endpointid"})))
		{
			LOGINFO("(connect) endpointid is present. ");
		} else {

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "endpointID not present";

			payload = jsonObj.dump();

			std::cout << "Json response is" << payload << std::endl;
			return ER_FAIL;
		}

		endpointID = jsonObj[ENDPOINTID];

		LOGINFO2I("(mqttBusObject) disconnect endpointID", endpointID);

		status = mqttApiPtr->disconnect(endpointID, payload, appName);

		LOGINFO2("(mqttBusObject) disconnect payload ", payload);

	}
	catch(...)
	{
		jsonObj[STATUS] = FAILURE;
		jsonObj[RESPONSECODE] = 400;
		jsonObj[MSG] = "invalid json";

		payload = jsonObj.dump();

		status = ER_FAIL;
	}
	return status;
}

/*
 * @brief : Processing the request and calling the respective function to call the API.
 */

QStatus mqttBusObject::processCommand(std::string &command, std::string &payload, std::string appName)
{

	LOGDBUG("(mqttBusObject) processCommand");

	LOGINFO2("(mqttBusObject)processCommand command", command);
	LOGINFO2("(mqttBusObject)processCommand appName", appName);


	try
	{
		QStatus status;

		if(command == CONNECT)
		{
			status = this->establishConnection(payload, appName);
			return status;
		}
		else if(command == PUBLISH)
		{
			status = this->sendToCloud(payload, appName);
			return status;
		}
		else if(command == SUBSCRIBE)
		{
			status = this->registerTopic(payload, appName);
			return status;
		}
		else if(command == UNSUBSCRIBE)
		{
			status = this->unregisterTopic(payload, appName);
			return status;
		}
		else if(command == DISCONNECT)
		{
			status = this->disconnect(payload, appName);
			return status;
		}
		else
		{
			LOGDBUG("(mqttBusObject)processCommand command not found");
			return ER_FAIL;
		}
	}

	catch(...)
	{
		LOGEROR("(mqttBusObject) processCommand exception");
		return ER_FAIL;
	}
}


/*
 * Signal functions which sends Signal to Apps
 * These functions will utilize the corresponding queues and send the signal to appropriate.
 */


/*
 * @brief : to get the respective appName info from the arrived topic in the package.
 */
std::string mqttBusObject::getAppName(std::string topic)
{
	LOGDBUG("(mqttBusObject) getAppName");

	LOGINFO2("(mqttBusObject)getAppName topic", topic);

	std::string appName;

	try
	{
		for(std::map<std::string, std::string>::iterator Iter = configurationManage::appFunMap.begin(); Iter!=configurationManage::appFunMap.end(); Iter++)
		{

			std::string keyStr = "/" + Iter->first + "/";

			std::size_t found = topic.find(keyStr);


			if(found != std::string::npos)
			{
				appName = Iter->first;
			}

		}

	}
	catch(...)
	{
		throw std::runtime_error("error on getAppName");
	}
	return appName;
}

/*
 * @brief : for getting the command with respect to topic.
 */
std::string mqttBusObject::getCommand(std::string topic, std::string appName)
{
	std::string command;

	std::string preStr;
	std::string commandStr;

	std::size_t foundApp = topic.find(appName);

	if(foundApp != std::string::npos)
	{
		preStr = topic.substr(0, foundApp-1);
		std::cout << "prestr is" << preStr << std::endl;

		std::size_t found1 = preStr.find_last_of("/");

		if(found1 != std::string::npos)
		{
			gatewayid = preStr.substr(found1+1);
			std::cout << "gatewayID" << gatewayid << std::endl;
		}

		commandStr = topic.substr(foundApp + 1);

		std::size_t found2 = commandStr.find("/");

		if(found2 != std::string::npos)
		{
			command = commandStr.substr(found2+1);
			std::cout << "command is" << command << std::endl;
		}

	}

	return command;
}

/*
 * @brief : To send the response to the cloud with respect to the apps.
 */
void mqttBusObject::sendDataResponse(uint16_t ID, std::string topic, std::string jsonString)
{
	LOGDBUG("(mqttBusObject)sendDataResponse");

	LOGINFO2("(mqttBusObject)sendDataResponse topic", topic);

	QStatus status;
	json jobj;
	std::string jobID;
	std::string tempStr;
	json tempJson;

	try
	{
		jobj = json::parse(jsonString);

		jobj.erase("topic");

		tempStr = jobj["msg"];

		tempJson = json::parse(tempStr);

		jobID = tempJson["JOBID"];

		std::cout << "send response JOBID" << jobID << std::endl;

		std::string replyTopic = topic + "/Reply/" + jobID;

		LOGINFO2("(mqttBusObject)sendDataResponse replyTopic", replyTopic);

		jobj["topic"] = replyTopic;

		jsonString = jobj.dump();

		//need to add the resp topic in json and also in async callback.
		status = mqttApiPtr->publish(ID, jsonString);
	}
	catch(...)
	{
		LOGEROR("(mqttBusObject)sendDataResponse exception");
	}
}

/*
 * @brief : Called for the async callback in order to send the response to the cloud.
 */
void mqttBusObject::methodCallerAsyncCallback(std::string status, std::string payload)
{
	LOGDBUG("(mqttBusObject) MethodCallerAsyncCallback");

	LOGINFO2("(mqttBusObject) MethodCallerAsyncCallback status", status);
	LOGINFO2("(mqttBusObject) MethodCallerAsyncCallback payload", payload);

	std::string topic;
	json jObj;

	try{

		uint16_t ID;
		jObj = json::parse(payload);

		topic = jObj[TOPIC];
		ID = jObj[ENDPOINTID];

		this->sendDataResponse(ID, topic, payload);
	}
	catch(...)
	{
		LOGEROR("(mqttBusObject)sendDataResponse exception");
	}
	return ;
}

/*
 * @brief : For the proxy call to apps upon arrival of their respective message.
 */
QStatus mqttBusObject::proxycall(uint16_t ID, std::string appName, std::string jsonString, std::string command, std::string resTopic)
{
	LOGDBUG("(mqttBusObject) proxycall");

	SessionId sessionID;
	QStatus status;

	std::cout << "(mqttBusObject) proxycall jsonstr" << jsonString << std::endl;

	try
	{
		std::string ifacePath = gatewayid +".APP"+ appName;

		if(ajPtr->findNSByKey(ifacePath, sessionID))
		{
			LOGINFO2I("(mqttBusObject) proxycall sessionID" , sessionID);
			LOGINFO2("(mqttBusObject) proxycall ifacePath", ifacePath);

			std::string _jsonString = jsonString;

			LOGINFO2("APPNAME IS" , appName);

			std::map<std::string, std::string>::iterator Iter = configurationManage::appFunMap.find(appName);

			if(Iter != configurationManage::appFunMap.end())
			{
				if(Iter->second == "sync")
				{
					std::cout << "IN sync" << std::endl;
					status = methodexPtr->methodCaller(ifacePath, sessionID,"SyncListener", command , _jsonString);

					if(status != ER_OK)
					{
						LOGINFO("(mqttBusObject) proxycall MethodCaller Failure");

						json jObj;
						jObj = json::parse(jsonString);
						jObj[RESPONSECODE] = 400;
						jObj[STATUS] = "failure";
						jObj[MESSAGE] = "unable to find the app";
						std::string retStr = jObj.dump();
						this->sendDataResponse(ID, resTopic,  retStr);
					}
					else
					{
						LOGINFO("(mqttBusObject) proxycall MethodCaller Success");
						this->sendDataResponse(ID, resTopic,  _jsonString);
					}
				}
				else if(Iter->second == "async")
				{
					std::cout << "IN async" << std::endl;
					status = methodexPtr->methodCallerAsync(ifacePath, sessionID, "AsyncListener", command, _jsonString);

					if(status != ER_OK)
					{
						LOGINFO("(mqttBusObject) proxycall MethodCaller Failure");
					}
					else
					{
						LOGINFO("(mqttBusObject) proxycall MethodCaller Success");
					}

				}

			}

		}
		else
		{
			LOGEROR("(mqttBusObject) proxycall Unable to find App");

			//			json jObj;
			//			jObj = json::parse(jsonString);
			//
			//			std::string jstr = jObj["msg"];
			//			json tempJson = json::parse(jstr);
			//
			//			tempJson[RESPONSECODE] = 400;
			//			tempJson[STATUS] = "failure";
			//			tempJson[MESSAGE] = "unable to find the app";
			//			std::string retStr = tempJson.dump();
			//			jObj["msg"] = json::parse(retStr);
			//
			//			std::cout << "JOBJ DUMP" << jObj.dump();

			json jObj;
			jObj = json::parse(jsonString);
			jObj[RESPONSECODE] = 400;
			jObj[STATUS] = "failure";
			jObj[MESSAGE] = "unable to find the app";

			std::string retStr = jObj.dump();

			this->sendDataResponse(ID, resTopic,  retStr);

			status = ER_FAIL;
		}
	}
	catch(...)
	{
		LOGEROR("(mqttBusObject) proxycall exception");
		throw std::runtime_error("error on proxycall");
	}

	return status;
}

/*
 * @brief : To send the signal to the apps upon their respective events.
 */
QStatus mqttBusObject::sendSignal(uint16_t ID, std::string appName, std::string jsonString, std::string command)
{

	LOGDBUG("(mqttBusObject) sendSignal ");
	SessionId sessionID;
	QStatus status;

	try
	{
		if(ajPtr->GetNSInfo(appName, sessionID))
		{
			//LOGINFO2("(mqttBusObject) sendSignal sessionID", sessionID);
			std::cout << "(mqttBusObject) sendSignal sessionID" << std::endl;

			status = sigexPtr->signalCaller(appName, sessionID, "CallbackListener", command, jsonString);

			LOGINFO2("(mqttBusObject) sendSignal status", QCC_StatusText(status));

			return status;
		}
		else
		{
			LOGEROR("(mqttBusObject) sendSignal unable to find app");
			status = ER_FAIL;
		}
	}
	catch(...)
	{
		status = ER_FAIL;
		LOGEROR("mqttBusObject) sendSignal exception");
		throw std::runtime_error("Error on send signal");
	}
	return status;

}

/*
 * @brief : To be called by the connection callback function for sending the connection signal to the app's.
 */
void mqttBusObject::connectionSignalFunction()
{
	std::cout << "DBUG : ConnectionSignalFunction .. .. " << std::endl;

	LOGDBUG("(mqttBusObject) connectionSignalFunction");

	QStatus status;
	json jsonObj;

	credentials cred;

	try{

		bool isSignalSend(false);
		while(!connectionQueue.empty())
		{
			uint16_t id = connectionQueue.front();

			// Find the approriate node from the provided endpointID from the list

			for(statusInfo &statInfo : statusVec)
			{
				if(statInfo.endptID == id)
				{
					statInfo.status = SUCCESS;
					statInfo.callback = "true";

					//jsonObj[HOSTNAME] = statInfo.brokerIp;
					jsonObj[ENDPOINTID] = statInfo.endptID;
					jsonObj[STATUS] = SUCCESS;
					jsonObj[RESPONSECODE] = 200;

					//~~~~~~~~~~~~~~~~~~~~~~
					cred.username = statInfo.username;
					cred.password = statInfo.password;

					//credMap[id] = cred;
					//~~~~~~~~~~~~~~~~~~~~~~~~
					for(std::multimap<uint16_t, credentials>::iterator it= credMap.begin(); it!= credMap.end();++it)
					{
						if(it->first == statInfo.endptID && it->second.appName == statInfo.appID)
						{
							std::cout << "SIGNAL SENDED" << std::endl;
							sendSig = true;
							//break;
						}
					}

					//status = this->sendSignal(id, statInfo.appID, jsonObj.dump(), CLOUDCONNECTIONCALLBACK);


					if(sendSig == false)
					{
						std::cout << "sig sended to " << statInfo.appID << std::endl;
						std::this_thread::sleep_for(std::chrono::milliseconds(400));
						status = this->sendSignal(id, statInfo.appID, jsonObj.dump(), CLOUDCONNECTIONCALLBACK);
						cred.ID = id;
						cred.appName = statInfo.appID;
						//credMap[id] = cred;

						credMap.insert(std::make_pair(id,cred));
					}

					sendSig = false;
					isSignalSend = true;
					//break;
				}
			}

			connectionQueue.pop();
		}
		if(!isSignalSend)
		{
			LOGEROR("(mqttBusObject) Failed to Send MQTTConnectionSuccess Signal");
		}
	}
	catch(...)
	{
		LOGEROR("(mqttBusObject) ConnectionSignalFunction exception");
	}
	return ;
}

/*
 * @brief : To be called by the error callback function for providing the error info to the respective app's.
 */
void mqttBusObject::errorSignalFunction()
{
	LOGDBUG("(mqttBusObject) errorSignalFunction");

	QStatus status;
	json jsonObj;

	try
	{
		bool isSignalSend(false);
		while(!errorQueue.empty())
		{
			uint16_t id = errorQueue.front();
			// Find the approriate node from the provided endpointID from the list
			for(statusInfo &statInfo : statusVec)
			{
				if(statInfo.endptID == id && statInfo.status == PROCESS)
				{
					statInfo.status = FAILURE;

					jsonObj[ENDPOINTID] = id;
					jsonObj[STATUS] = FAILURE;
					jsonObj[RESPONSECODE] = 400;

					status = this->sendSignal(id, statInfo.appID, jsonObj.dump(), CLOUDERRORCONNECTCALLBACK);

					//break;
				}
			}

			//________

			std::string connList = mqttApiPtr->getConnectionList();

			json jObj;
			jObj = json::parse(connList);
			std::string jsonKey;

			for (json::iterator iter = jObj.begin(); iter != jObj.end(); ++iter)
			{
				int jsonVal;
				jsonVal = iter.value();
				if(jsonVal == id)
				{
					jsonKey = iter.key();
					jObj.erase(jsonKey);
					connList = jObj.dump();
				}
			}
			mqttApiPtr->updatedConnectionList(connList);


			connectionMap endpointNode;
			for(connectionMap node : activeConnectionList)
			{
				if(node.endpointID == id)
				{
					endpointNode = node;
					break;
				}
			}
			activeConnectionList.remove(endpointNode);
			//___________________

			for(std::vector<statusInfo>::iterator it=statusVec.begin();it!= statusVec.end();++it)
			{
				LOGINFO2I("endpointID In vector are", it->endptID);

				if(it->endptID == id && it->status == PROCESS)
				{
					statusVec.erase(it);
					break;
				}
			}

			errorQueue.pop();
		}
		if(!isSignalSend)
		{
			LOGEROR("Failed to Send errorSignalFunction Signal");
		}
	}
	catch(...)
	{
		LOGEROR("(mqttBusObject) errorSignalFunction exception");
	}
	return;
}

/*
 * @brief : To be called by the disconnection callback for providing the disconnection info to the respective app's.
 */
void mqttBusObject::disconnectSignalFunction()
{
	LOGDBUG("(mqttBusObject) disconnectSignalFunction");

	QStatus status;
	json jsonObj;

	try
	{
		bool isSignalSend(false);

		while(!disconnectQueue.empty())
		{

			uint16_t id = disconnectQueue.front();
			std::cout << "ID" << id << std::endl;

			// Find the approriate node from the provided endpointID from the list
			for(statusInfo &statInfo : statusVec)
			{
				if(statInfo.endptID == id)
				{
					statInfo.status = FAILURE;
					statInfo.callback = "false";

					//jsonObj[HOSTNAME] = statInfo.brokerIp;
					jsonObj[ENDPOINTID] = id;
					jsonObj[STATUS] = FAILURE;
					jsonObj[RESPONSECODE] = "400";

					//std::string command = DEACTIVE_CONNECTION;

					status = this->sendSignal(statInfo.endptID, statInfo.appID, jsonObj.dump(), CLOUDDISCONNECTIONCALLBACK);

					isSignalSend = true;

					//break;
				}
			}

			disconnectQueue.pop();
		}
		if(!isSignalSend)
		{
			LOGEROR("Failed to Send disconnectSignalFunction Signal");
		}
	}
	catch(...)
	{
		LOGEROR("(mqttBusObject) disconnectSignalFunction exception");

	}
	return;
}

/**
 * Callback functions utilized by Threads. Will push the requests to corresponding Queues & fire Signals upon turn.
 * Call these functions from mqttEndpoint objects with the endpointID as the parameter.
 **/
void mqttBusObject::connectionCallback(uint16_t endpointID)
{
	LOGDBUG("(mqttBusObject) connectionCallback");

	LOGINFO2I("(mqttBusObject) connectionCallback endpointID", endpointID);

	try
	{
		connectionMutex.lock();
		connectionQueue.push(endpointID);

		if(!connectionThread.joinable())
		{
			connectionThread = std::thread([this]() { this->connectionSignalFunction();});
		}
		else
		{
			LOGEROR("(mqttBusObject) connectionCallback error in connectionThread");
		}
		connectionThread.join();
		connectionMutex.unlock();
	}
	catch(std::exception& e)
	{
		LOGEROR2("(mqttBusObject) connectionCallback exception", e.what());
	}
}

void mqttBusObject::messageCallback(uint16_t endpointID, std::string jsonString)
{
	LOGDBUG("(mqttBusObject) messageCallback");

	LOGINFO2I("(mqttBusObject) messageCallback endpointID", endpointID);

	LOGINFO2("(mqttBusObject) messageCallback jsonString", jsonString);

	json jsonObj;
	std::string topic;
	std::string command;
	std::string appName;
	std::string lastVal;

	try
	{
		messageMutex.lock();
		messagePacket_t packet;
		packet.endpointID = endpointID;
		packet.msg = jsonString;

		jsonObj = json::parse(jsonString);
		topic = jsonObj[TOPIC];

		appName = this->getAppName(topic);

		LOGINFO2("(mqttBusObject) messageCallback appName", appName);

		std::string tempTopic = topic;

		std::size_t found = tempTopic.find_last_of("/");
		while(found!=std::string::npos)
		{
			tempTopic = tempTopic.substr(0, found);

			std::size_t found1 = tempTopic.find_last_of("/");
			while(found1!=std::string::npos)
			{
				lastVal = tempTopic.substr(found1+1);
				break;
			}
			break;
		}

		if(lastVal != "Reply")
		{
			auto iter = queueObjMap.find(appName);

			if(iter != queueObjMap.end())
			{
				LOGINFO("found in  iterator");

				iter->second->getMqttBusPtr(this);
				iter->second->pushPacket(packet);
			}
		}

		messageMutex.unlock();
		mapQueue = true;
		return;
	}
	catch(std::exception& e)
	{
		LOGEROR2("(mqttBusObject) messageCallback exception", e.what());
	}
}

void mqttBusObject::errorCallback(uint16_t endpointID)
{
	LOGDBUG("(mqttBusObject) errorCallback");

	LOGINFO2I("(mqttBusObject) errorCallback endpointID", endpointID);

	try
	{
		errorMutex.lock();
		errorQueue.push(endpointID);
		if(!errorThread.joinable())
		{
			errorThread = std::thread([this]() { this->errorSignalFunction(); });
		}
		else
		{
			LOGEROR("(mqttBusObject) connectionCallback error in errorThread");
		}

		errorThread.join();
		errorMutex.unlock();
	}
	catch(std::exception& e)
	{
		LOGEROR2("(mqttBusObject) errorCallback exception", e.what());
	}
}

void mqttBusObject::disconnectionCallback(uint16_t endpointID)
{
	LOGDBUG("(mqttBusObject) disconnectionCallback");

	LOGINFO2I("(mqttBusObject) disconnectionCallback endpointID", endpointID);

	try
	{
		disconnectionMutex.lock();
		disconnectQueue.push(endpointID);

		if(!disconnectionThread.joinable())
		{
			disconnectionThread = std::thread([this]() { this->disconnectSignalFunction(); });
		}
		else
		{
			LOGEROR("(mqttBusObject) connectionCallback error in disconnectThread");
		}

		disconnectionThread.join();
		disconnectionMutex.unlock();
	}
	catch(std::exception& e)
	{
		LOGEROR2("(mqttBusObject) disconnectionCallback exception", e.what());
	}
}

/*
 * @brief : get the loss-advertise.
 */
void mqttBusObject::lostAdvertisement(qcc::String busName/*, ajn::SessionId id*/)
{
	LOGDBUG("(alljoynWrapper)lossAdvertise");

	LOGINFO2("(alljoynWrapper)lossAdvertise Busname" , busName);
}

/*
 * brief : find the advertisement.
 */
void mqttBusObject::foundAdvertisement(qcc::String busName)
{
	LOGDBUG("(alljoynWrapper)findAdvertise");

	LOGINFO2("(alljoynWrapper)findAdvertise busName", busName);
}

void mqttBusObject::busDisconnected()
{
}

