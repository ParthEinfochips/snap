#include "MqttApi.h"
#define eI_MODULE "MQTTAPI"

uint16_t MqttApi::endpointID = 250;

bool setEndpoint = false;

MqttApi::MqttApi( mqttBusObject *_mqttPtr)
: mqttPtr(_mqttPtr), mqttEndpoint(NULL), _cloudWrapper(NULL)
{

	filePtr = new FileOPS();
}

MqttApi::MqttApi(const MqttApi &mqttApiObj)
{
	filePtr = new FileOPS();
	mqttEndpoint = NULL;
	_cloudWrapper = NULL;
	mqttPtr = mqttApiObj.mqttPtr;
}

MqttApi::~MqttApi()
{
	if(mqttEndpoint)
		delete mqttEndpoint;
	if(_cloudWrapper)
		delete _cloudWrapper;
}

/*
 * @brief : Provides the get connectionList of the existing connection.
 */
QStatus MqttApi::updateConnectionJson()
{
	LOGDBUG("(MqttApi)updateConnectionJson...");

	json tempJson;

	if(mqttPtr) {

		try
		{
			std::stringstream ss;
			std::stringstream ss1;

			// Fetch connections from connectionList here.
			for(connectionMap map : mqttPtr->activeConnectionList)
			{
				LOGINFO2I("(MqttApi)updateConnectionJson endpointID", map.endpointID);
				ss << map.endpointID;
				ss << "$";

				LOGINFO2("(MqttApi)updateConnectionJson connectionString", map.connectionString);
				ss1 << map.connectionString;
				ss1 << "$";
			}

			std::string endIdStr = ss.str();
			std::string connectStr = ss1.str();

			bool cond = false;

			while(endIdStr != "")
			{
				if(cond)
					break;

				std::size_t found = endIdStr.find_first_of("$");
				while(found!=std::string::npos && endIdStr != "")
				{
					std::string temp;
					std::string temp1;
					temp = endIdStr.substr(0 , found);
					endIdStr = endIdStr.substr(found+1);

					if(endIdStr == "")
						cond = true;

					std::size_t found1 = connectStr.find_first_of("$");
					if (found1!=std::string::npos)
					{
						temp1 = connectStr.substr(0 , found1);
						connectStr = connectStr.substr(found1+1);

						tempJson[temp1] = std::stoi(temp);

						break;
					}
					break;
				}
			}

			connList = tempJson.dump();

			LOGINFO2("(updateConnectionJson) updated Connection List", connList);

			return ER_OK;
		}
		catch(...)
		{
			throw std::runtime_error("error in updating json");
		}
	} else {
		LOGEROR("(updateConnectionJson) Invalid MqttBusObj Ptr !!");
		return ER_FAIL;
	}

}

/*
 * @brief : returns connection list.
 */
std::string MqttApi::getConnectionList()
{	
	if(getconn)
	{
		sleep(2);
	}

	getconn = true;

	return connList;
}

void MqttApi::updatedConnectionList(std::string connectList)
{
	connList = connectList;
}

/*
 * @brief : Registering the endpoint if connection already found.
 */
QStatus MqttApi::registerEndpoint(std::string &id, std::string uniqueName, std::string &payload)
{
	LOGDBUG("(MqttApi)registerEndpoint");
	QStatus status;

	json jsonObj;
	std::string cafilePath;
	std::string certfilePath;
	std::string keyfilePath;

	std::stringstream tlsStr;
	std::stringstream tlsCertStr;
	std::stringstream tlsKeyStr;
	std::string username;
	std::string password;

	int port;

	try
	{
		jsonObj = json::parse(payload);
		port = jsonObj[PORT];
		username = jsonObj[USERNAME];
		password = jsonObj[PASSWORD];
	}
	catch(...)
	{
		return ER_FAIL;
	}

	try
	{
		if(certReq && port != 1883)
		{
			try
			{

				//jsonObj = json::parse(payload);
				cafilePath = jsonObj["cafile"];
				certfilePath = jsonObj["certfile"];
				keyfilePath = jsonObj["keyfile"];

			}
			catch (...)
			{
				return ER_FAIL;
			}
		}

		int ID = std::stoi(id);

		LOGINFO2I("(registerEndpoint) Fetch Connection ID", ID);

		//TODO : check username passord provided with  cred map username password

		std::multimap<uint16_t, credentials>::iterator it= mqttPtr->credMap.find(ID);
		{
			if (it != mqttPtr->credMap.end())
			{
				if(it->second.username != username)
				{
					jsonObj[STATUS] = FAILURE;
					jsonObj[RESPONSECODE] = 400;
					jsonObj[ENDPOINTID] = -1;
					jsonObj[MSG] = "wrong username";

					payload = jsonObj.dump();
					return ER_FAIL;
				}

				if(it->second.password != password)
				{
					jsonObj[STATUS] = FAILURE;
					jsonObj[RESPONSECODE] = 400;
					jsonObj[ENDPOINTID] = -1;
					jsonObj[MSG] = "wrong password";

					payload = jsonObj.dump();
					return ER_FAIL;
				}

			}

		}
		bool isReplay(false);

		/** Check if the node exists with such an endpoint **/
		for(connectionMap &node : mqttPtr->activeConnectionList)
		{
			if(node.endpointID == ID)
			{
				LOGINFO("(registerEndpoint) Node Exists");

				if(certReq && port != 1883)
				{
					std::ifstream inputStr(cafilePath);
					while(inputStr >> tlsStr.rdbuf());

					if(node.caCrt != tlsStr.str())
						return ER_FAIL;
					else
						std::cout << "(registerEndpoint) CACRT found is" << tlsStr.str() << std::endl;

					std::ifstream inputStr1(certfilePath);
					while(inputStr1 >> tlsCertStr.rdbuf());

					if(node.cert != tlsCertStr.str())
						return ER_FAIL;
					else
						std::cout << "(registerEndpoint) CERT found is" << tlsStr.str() << std::endl;

					std::ifstream inputStr2(keyfilePath);
					while(inputStr2 >> tlsKeyStr.rdbuf());

					if(node.key != tlsKeyStr.str())
						return ER_FAIL;
					else
						std::cout << "(registerEndpoint) KEY found is" << tlsStr.str() << std::endl;


				}

				/** Update the appList of connectionMap **/
				std::string appName = uniqueName;
				node.appList.push_back(appName);

				isReplay = true;

				jsonObj[STATUS] = SUCCESS;
				jsonObj[RESPONSECODE] = 200;
				jsonObj[ENDPOINTID] = ID;
				jsonObj[MSG] = "connection success";

				payload = jsonObj.dump();
				status = ER_OK;
			}
		}

		if(!isReplay)
		{
			LOGEROR("(registerEndpoint) Node Not Exists");
			/** No such node exists with the provided endpoint, send negative response **/
			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[ENDPOINTID] = ID;
			jsonObj[MSG] = "connection failure";

			payload = jsonObj.dump();
			status = ER_FAIL;
		}
	}
	catch(...)
	{
		throw std::runtime_error("error on register-Endpoint");
	}

	return status;
}

//bool MqttApi::checkExistence(const std::string &entityPath)
//{
//	struct stat fb;
//
//	auto retVal = stat(entityPath.c_str(), &fb);
//
//	if(retVal < 0)
//	{
//		return false;
//	}
//	else
//	{
//		return true;
//	}
//}


std::string MqttApi::getSSLPath(std::string path)
{
	std::array<char, 512> buffer;
	std::stringstream strResult;
	std::shared_ptr<FILE> pipe(popen(path.c_str(), "r"), pclose);
	if (!pipe) throw std::runtime_error("popen() failed!");
	while (!feof(pipe.get())) {
		if (fgets(buffer.data(), 128, pipe.get()) != NULL)
			strResult << buffer.data() << "|" << std::endl;
	}

	return strResult.str();
}

/*
 * @brief : Deregistering the endpoint.
 */

//bool MqttApi::deregisterEndpoint(std::string &id, std::string uniqueName)
//{
//	LOGDBUG("(MqttApi)deregisterEndpoint");
//
//	QStatus status;
//
//	// Fetch the connection ID from the message arguments
//	int ID = std::stoi(id);
//
//	LOGINFO2I("(deregisterEndpoint) Fetch Connection ID", ID);
//
//	/* Reply to request */
//	bool isReplay(false);
//
//	/** Check if the node exists with such an endpoint **/
//	for(auto itConnectionNode = mqttPtr->activeConnectionList.begin(); itConnectionNode != mqttPtr->activeConnectionList.end(); itConnectionNode++)
//	{
//		auto &node = *itConnectionNode;
//		uint16_t temp = node.endpointID;
//
//		if(temp == ID)
//		{
//			LOGINFO("(deregisterEndpoint) Node Exists");
//
//			/** Update the appList of connectionMap **/
//			std::string appName = uniqueName;
//			node.appList.remove(appName);
//
//			/** Remove this node if appList is empty now. **/
//			if(node.appList.size() == 0)
//			{
//				node.mqttEndpoint->getPointer()->Disconnect();
//				mqttPtr->activeConnectionList.erase(itConnectionNode);
//			}
//			// Send the response.
//			isReplay = true;
//		}
//
//		if(!isReplay)
//		{
//			LOGEROR("(deregisterEndpoint) Node Not Exists");
//			/** No such node exists with the provided endpoint, send negative response **/
//		}
//		return isReplay;
//
//	}
//}

/*
 * @brief : For establishing the connection.
 */
uint16_t MqttApi::connect(std::string &payload, std::string uniqueName)
{

	LOGDBUG("(MqttApi)Connect");

	QStatus status = ER_FAIL;

	json jsonObj;
	json tlsJson;
	std::stringstream tlsStr;
	std::stringstream tlscertStr;
	std::stringstream tlskeyStr;

	try
	{

		/* Fetch the connection json string */
		LOGINFO2("INFO : (connect) Fetch Connection json - ", payload);

		/* Reply to request */

		try         // Trying to parse the json connection string
		{
			jsonObj = json::parse(payload);
		}
		catch (...)
		{
			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[ENDPOINTID] = -1;
			jsonObj[MSG] = "Invalid json";


			payload = jsonObj.dump();
			LOGEROR("(connect) Unable to parse Json !!");
			return 0;
		}

		// Parameter to check return value received from json Checker
		uint16_t port;
		std::string hostname = "";

		/** Fetch only host & port for connection **/
		if(!(jsonChecker::CheckJson(jsonObj, hostname, {"hostname"})))
		{
			LOGINFO("(connect) hostname is present. ");
		} else {

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[ENDPOINTID] = -1;
			jsonObj[MSG] = "hostname not present";

			payload = jsonObj.dump();

			LOGEROR("(connect) Json Check hostname not found.");
			return 0;
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
			return 0;
		}

		/** Verify the connection is not present already **/
		std::string connectionString = hostname + "|" + std::to_string(port);
		/** Check if the required endpoint is already created, if so return the same */
		connectionMap endpointNode;
		bool nodeFound = false;

		//std::cout << "MQTTPTR" << mqttPtr << std::endl;
		for(connectionMap node : mqttPtr->activeConnectionList)
		{
			if(node.connectionString == connectionString)
			{
				endpointNode = node;
				nodeFound = true;
				break;
			}
		}

		if(nodeFound)  // Found endpoint in map
		{
			/** Send the success response. **/
			LOGINFO("(connect) Found end point. Send SUCCESS.");

			std::string appName = uniqueName;
			endpointNode.appList.push_back(appName);
			//std::thread connectionSignalThread(&mqttBusObject::connectionThreadFunction, this, appName.data());
			//connectionSignalThread.detach();
		} else {

			endpointID++;

			LOGINFO("INFO : (connect) End point not created. Make NEW Connection.");

			mqttEndpoint = new cloudEndpoint(MQTT,endpointID,
					std::bind(&mqttBusObject::connectionCallback, mqttPtr, std::placeholders::_1),
					std::bind(&mqttBusObject::errorCallback, mqttPtr, std::placeholders::_1),
					std::bind(&mqttBusObject::messageCallback, mqttPtr, std::placeholders::_1, std::placeholders::_2),
					std::bind(&mqttBusObject::disconnectionCallback, mqttPtr, std::placeholders::_1));


			if(mqttEndpoint)
			{
				_cloudWrapper = mqttEndpoint->getPointer();
				if(_cloudWrapper)
				{

					//					std::string cafilePath = "/opt/GA.conf.d/SecurityCert/" + hostname  + "/cacert.pem";
					//					std::string certfilePath = "/opt/GA.conf.d/SecurityCert/" + hostname + "/cert.pem";
					//					std::string keyfilePath = "/opt/GA.conf.d/SecurityCert/" + hostname + "/key.pem";

					if(certReq && port != 1883)
					{
						std::string cafilePath;
						std::string cipher;
						std::string version;
						std::string certfilePath;
						std::string keyfilePath;



						if(!(jsonChecker::CheckJson(jsonObj,cafilePath, {"cafile"})) && !(jsonChecker::CheckJson(jsonObj,certfilePath, {"certfile"})) && !(jsonChecker::CheckJson(jsonObj,keyfilePath, {"keyfile"})))
						{
							LOGINFO("(connect) certificates are present. ");
						}
						else
						{
							LOGINFO("(connect) certificates are not present. ");

							jsonObj[STATUS] = FAILURE;
							jsonObj[RESPONSECODE] = 400;
							jsonObj[ENDPOINTID] = -1;
							jsonObj[MSG] = "certificate not present";

							payload = jsonObj.dump();

							return 0;
						}



						//						std::string sslFolder = getSSLPath("ls /opt/GA.conf.d/SecurityCert");
						//						sslFolder.erase(std::remove(sslFolder.begin(),sslFolder.end(),'\n'),sslFolder.end());
						//						std::cout << "ASASSASAS" << sslFolder << std::endl;
						//
						//						bool retVal;
						//						retVal = this->checkExistence(cafilePath);
						//						if (retVal == true)
						//						{
						//							tlsJson[CAFILE] = cafilePath;
						//						}
						//						else
						//						{
						//							std::cout << "cacert doesn't exists" << std::endl;
						//						}
						//
						//						retVal = this->checkExistence(certfilePath);
						//						if(retVal == true)
						//						{
						//							tlsJson[CERTFILE] = certfilePath;
						//						}
						//						else
						//						{
						//							std::cout << "certfile doesn't exists" << std::endl;
						//						}
						//
						//						retVal = this->checkExistence(keyfilePath);
						//						if(retVal == true)
						//						{
						//							tlsJson[KEYFILE] = keyfilePath;
						//						}
						//						else
						//						{
						//							std::cout << "keyfilepath doesn't exists" << std::endl;
						//						}
						//						version = "tlsv1.2";
						//						cipher = "AES256-SHA";

						std::string dirPath = "mkdir -p /opt/secCer/" + hostname;

						this->getSSLPath(dirPath);

						cafilePath = jsonObj[CAFILE];
						std::string caPath = "/opt/secCer/"+ hostname + "/cacert.pem";
						filePtr->writeToFile(caPath, cafilePath);

						certfilePath = jsonObj[CERTFILE];
						std::string certPath = "/opt/secCer/"+ hostname + "/cert.pem";
						filePtr->writeToFile(certPath, certfilePath);

						keyfilePath = jsonObj[KEYFILE];
						std::string keyPath = "/opt/secCer/"+ hostname + "/key.pem";
						filePtr->writeToFile(keyPath, keyfilePath);

						version = "tlsv1.2";
						cipher = "AES256-SHA";

						tlsJson[CAFILE] = caPath;
						tlsJson[CERTFILE] = certPath;
						tlsJson[KEYFILE] = keyPath;
						tlsJson[VERSION] = version;
						tlsJson[CIPHER] = cipher;

						LOGINFO2("(MqttApi)Connect tlsJson is" , tlsJson.dump());

						status = _cloudWrapper->ConfigureTLS(tlsJson.dump());


						if(status == ER_OK)
						{
							std::ifstream inputStr(cafilePath);
							while(inputStr >> tlsStr.rdbuf());

							std::ifstream inputStr1(certfilePath);
							while(inputStr1 >> tlscertStr.rdbuf());

							std::ifstream inputStr2(keyfilePath);
							while(inputStr2 >> tlskeyStr.rdbuf());

						}
						else
						{
							jsonObj[STATUS] = FAILURE;
							jsonObj[RESPONSECODE] = 400;
							jsonObj[ENDPOINTID] = -1;
							jsonObj[MSG] = "error in configTLS";

							payload = jsonObj.dump();

							return status;

						}
					}

					status  = _cloudWrapper->Connect(payload);
					LOGINFO2("(connect) Called cloudWrapper.Connect Function ", QCC_StatusText(status));
				}
				else
					LOGEROR("(connect) Unable to get Cloud Wrapper Pointer !!");
			} else {
				LOGEROR("(connect) Unable to get cloud Endpoint Pointer !!");
			}

			// Forward the request to cloud wrapper
			if(status != ER_OK)
			{
				endpointID--;
				jsonObj[STATUS] = FAILURE;
				jsonObj[RESPONSECODE] = 400;
				jsonObj[ENDPOINTID] = -1;
				jsonObj[MSG] = "unable to connect";

				payload = jsonObj.dump();

				LOGEROR2("(from cloudWrapper.Connect) Bad Packet Format !!", QCC_StatusText(status));
				return status;

			} else {

				/** Connection is complete. Add connectionString, connectionEndpoint, appName in activeConnectionList. **/

				connectionMap node;
				std::string appName = uniqueName;
				node.appList.push_back(appName);
				node.connectionString = connectionString;
				node.caCrt = tlsStr.str();
				node.cert = tlscertStr.str();
				node.key = tlskeyStr.str();
				node.endpointID = endpointID;
				node.mqttEndpoint = mqttEndpoint;

				statusInfo statInfo;
				statInfo.endptID = endpointID;
				statInfo.brokerIp = hostname;
				statInfo.appID = appName;
				statInfo.status = PROCESS;
				statInfo.username = jsonObj[USERNAME];
				statInfo.password = jsonObj[PASSWORD];

				mqttPtr->statusVec.push_back(statInfo);

				mqttPtr->activeConnectionList.push_back(node);

				status = this->updateConnectionJson();

				if(status == ER_OK)
					LOGINFO("(MqttApi)json updated successfully");
				else
					LOGINFO("(MqttApi)error in updating json");


				//mqttPtr->appNameMap[uniqueName] = endpointID;

				//setEndpoint = true;

				// Once Connection Call is complete increment the counter
				LOGINFO("(connect) Send Success Response.");
				/** Send the success response. **/

				jsonObj[STATUS] = SUCCESS;
				jsonObj[RESPONSECODE] = 200;
				jsonObj[ENDPOINTID] = endpointID;
				jsonObj[MSG] = "connection success";

				payload = jsonObj.dump();

				/* Log error if reply could not be sent */
				if (ER_OK != status)
					LOGEROR2("EROR : Failed to Send Method Reply - ", QCC_StatusText(status));
			}
		}
	}
	catch(...)
	{
		throw std::runtime_error("error on connect");
	}
	return endpointID;
}

/*
 * @brief : For publishing the data on the topic, calling the CTL library function.
 */
QStatus MqttApi::publish(uint16_t ID, std::string &payload)
{
	LOGDBUG("(MqttApi)publish");

	json jsonObj;
	QStatus status;

	try
	{
		bool isReply = false;

		try
		{
			jsonObj = json::parse(payload);
		}
		catch(...)
		{
			LOGINFO("(MqttApi)publish Bad Package Format !!");

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "Invalid json";

			payload = jsonObj.dump();

			return ER_BAD_ARG_2;
		}

		LOGINFO2I("(publish) Fetch Connection ID", ID);

		connectionMap endpointNode;
		for(connectionMap node : mqttPtr->activeConnectionList)
		{
			if(node.endpointID == ID)
			{
				isReply = true;
				endpointNode = node;

				status = ER_OK;
				break;
			}
		}

		/* Reply to request */
		if(!isReply)     // Connection ID is wrong!
		{
			LOGEROR("EROR : (publish) broker ID Wrong.");

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "Invalid json - invalid broker ID";

			payload = jsonObj.dump();
			// Send ER_BAD_ARG_1
			return ER_BAD_ARG_1;

		} else {

			// Fetch the publish json string

			LOGINFO2("INFO : Fetch publish json", payload);

			LOGINFO("INFO : (publish) Fetch the pointer and forward the packet.");

			/** Fetch the pointer and forward the packet **/
			cloudEndpoint* mqttEndpoint = endpointNode.mqttEndpoint;

			// Forward the request to cloud wrapper
			status = mqttEndpoint->getPointer()->SendData(payload);

			if(status != ER_OK)
			{
				LOGEROR("(publish) Unable to publish to cloud - broker issue!!");

				jsonObj[STATUS] = FAILURE;
				jsonObj[RESPONSECODE] = 400;
				jsonObj[MSG] = "Unable to publish to cloud";

				payload = jsonObj.dump();

			} else {

				LOGINFO("INFO : (publish) Response Success.");

				jsonObj[STATUS] = SUCCESS;
				jsonObj[RESPONSECODE] = 200;
				jsonObj[MSG] = "Successfully published";

				payload = jsonObj.dump();
				/** Send the success response. **/
			}
		}
	}
	catch(...)
	{
		throw std::runtime_error("error on publish");
	}

	return status;
}

/*
 * @brief : For subscribing to the particular topic, calling the CTL lib function.
 */
QStatus MqttApi::subscribe(uint16_t ID, std::string &payload)
{
	LOGDBUG("(MqttApi)subscribe .. .. ");

	QStatus status;
	json jsonObj;

	try
	{
		bool isReply = false;

		try{

			jsonObj = json::parse(payload);
		}
		catch(...)
		{
			LOGEROR("(subscribe) Bad Package Format !!");

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "Invalid json";

			payload = jsonObj.dump();
			// Send ER_BAD_ARG_2
			return ER_BAD_ARG_2;
		}

		// Fetch the connection ID from the message arguments

		LOGINFO2I("(subscribe) Fetch Connection ID", ID);

		connectionMap endpointNode;
		for(connectionMap node : mqttPtr->activeConnectionList)
		{
			if(node.endpointID == ID)
			{
				isReply = true;
				endpointNode = node;

				status = ER_OK;
				break;
			}
		}

		/* Reply to request */
		if(!isReply)     // Connection ID is wrong!
		{
			LOGEROR("(subscribe) Broker ID is wrong !");

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "Invalid json";

			payload = jsonObj.dump();
			return ER_BAD_ARG_1;

		} else {

			// Fetch the Subscribe json string
			LOGINFO2("Fetch Subscribe json - ", payload);

			/** Fetch the pointer and forward the packet **/
			cloudEndpoint* mqttEndpoint = endpointNode.mqttEndpoint;
			// Forward the request to cloud wrapper
			status = mqttEndpoint->getPointer()->RegisterListener(payload);

			if(status != ER_OK)
			{
				LOGEROR("(from subscribe) unable to register- broker issue!!");

				jsonObj[STATUS] = FAILURE;
				jsonObj[RESPONSECODE] = 400;
				jsonObj[MSG] = "registration failure";

				payload = jsonObj.dump();

			} else {

				LOGINFO("(subscribe) Send the success response.");
				jsonObj[STATUS] = SUCCESS;
				jsonObj[RESPONSECODE] = 200;
				jsonObj[MSG] = "successfully registered";

				payload = jsonObj.dump();
				/** Send the success response. **/
			}
		}
	}
	catch(...)
	{
		throw std::runtime_error("Issue in subscribe..");
	}
	return status;
}

/*
 * @brief : For unsubscribing to the specific topic, calling the CTLlib function.
 */
QStatus MqttApi::unsubscribe(uint16_t ID, std::string &payload)
{
	LOGDBUG("(MqttApi)unsubscribe");

	QStatus status;
	json jsonObj;
	try
	{
		bool isReply = false;
		try
		{
			jsonObj = json::parse(payload);
		}
		catch(...)
		{
			LOGEROR("EROR : (subscribe) Bad Package Format !!");

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "Invalid json";

			payload = jsonObj.dump();
			// Send ER_BAD_ARG_2
			return ER_BAD_ARG_2;
		}

		// Fetch the connection ID from the message arguments

		LOGINFO2I("(unsubscribe) Fetch Connection ID - ", ID);

		connectionMap endpointNode;
		for(connectionMap node : mqttPtr->activeConnectionList)
		{
			if(node.endpointID == ID)
			{
				isReply = true;
				endpointNode = node;
				status = ER_OK;
				break;
			}
		}

		/* Reply to request */

		if(!isReply)
		{
			LOGEROR("(unsubscribe) Connection ID Not Match !!");

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "Invalid json";

			payload = jsonObj.dump();

			return ER_BAD_ARG_1;
		}

		// Fetch the unsubscribe json string
		LOGINFO2("(unsubscribe) Fetch unsubscribe Json - ", payload);

		/** Fetch the pointer and forward the packet **/
		cloudEndpoint* mqttEndpoint = endpointNode.mqttEndpoint;
		// Forward the request to cloud wrapper
		status = mqttEndpoint->getPointer()->CancelListener(payload);

		if(status != ER_OK)
		{
			LOGEROR("(unsubscribe) unable to unregister !!");

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "unregistration failure";

			payload = jsonObj.dump();

		} else {

			LOGINFO("INFO : (unsubscribe) Send the success response.");

			jsonObj[STATUS] = SUCCESS;
			jsonObj[RESPONSECODE] = 200;
			jsonObj[MSG] = "successfully unregistered";

			payload = jsonObj.dump();

		}
	}
	catch(...)
	{
		throw std::runtime_error("Error on un-subscribe");
	}
	return status;
}

/*
 * @brief : for disconnecting with the broker, calling the CTL lib function.
 */
QStatus MqttApi::disconnect(uint16_t ID, std::string &payload, std::string uniqueName)
{
	LOGDBUG("(MqttApi)disconnect");

	json jsonObj;

	jsonObj = json::parse(payload);

	QStatus status;
	try
	{
		bool isReply = false;
		json jsonObj;
		try
		{
			jsonObj = json::parse(payload);
		}
		catch(...)
		{
			LOGEROR("Bad packet format");

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "invalid json";

			payload = jsonObj.dump();

			status= ER_BAD_ARG_2;
			return status;
		}

		LOGINFO2I("(Disconnect) Fetch Connection ID - ", ID);

		connectionMap endpointNode;
		for(connectionMap node : mqttPtr->activeConnectionList)
		{
			if(node.endpointID == ID)
			{
				isReply = true;
				endpointNode = node;
				status = ER_OK;
				break;
			}
		}

		/* Reply to request */
		if(!isReply)     // Connection ID is wrong!
		{
			LOGEROR("(Disconnect) Broker ID is wrong !!");

			jsonObj[STATUS] = FAILURE;
			jsonObj[RESPONSECODE] = 400;
			jsonObj[MSG] = "Wrong brokerID";

			payload = jsonObj.dump();

			status = ER_BAD_ARG_2;
			return status;

		} else {

			/** Remove appName from appList of that specific node. **/

			std::string appName = uniqueName;

			LOGINFO2("(MqttApp) Disconnect appName is", appName);

			endpointNode.appList.remove(appName);

			for(std::vector<statusInfo>::iterator it =mqttPtr->statusVec.begin();it!=mqttPtr->statusVec.end();++it)
			{
				LOGINFO2I("endpointID In vector are", it->endptID);

				if(it->endptID == ID && it->appID == appName)
				{
					mqttPtr->statusVec.erase(it);

					break;
				}
			}

			/** Remove this node if appList is empty now. **/

			/**
			 * Perform actual disconnection of MQTT Cloud Endpoint
			 * Fetch the pointer and disconnect
			 **/
			if(endpointNode.appList.size() == 0)
			{
				json jObj;
				jObj = json::parse(connList);
				std::string jsonKey;

				for (json::iterator iter = jObj.begin(); iter != jObj.end(); ++iter)
				{
					int jsonVal;

					jsonVal = iter.value();
					if(jsonVal == ID)
					{
						jsonKey = iter.key();
						jObj.erase(jsonKey);
						connList = jObj.dump();
					}
				}

				LOGINFO("(MqttApi)Disconnect appList is empty");
				status = endpointNode.mqttEndpoint->getPointer()->Disconnect();

				/** Remove node from active connection list **/
				mqttPtr->activeConnectionList.remove(endpointNode);
			}
			else
			{
				LOGINFO("(MqttApi)Disconnect appList is not empty");
				status = ER_OK;
			}
			if(status != ER_OK)
			{
				jsonObj[STATUS] = FAILURE;
				jsonObj[RESPONSECODE] = 400;
				jsonObj[MSG] = "unable to disconnect";

				payload = jsonObj.dump();

				LOGEROR2("Failed to Send Method Reply - ", QCC_StatusText(status));
			}
			else
			{
				jsonObj[STATUS] = SUCCESS;
				jsonObj[RESPONSECODE] = 200;
				jsonObj[MSG] = "successfully disconnected";

				payload = jsonObj.dump();



				/** Send the success response. **/
				LOGINFO("(disconnect) Send the success responses");
			}

		}
	}
	catch(...)
	{
		throw std::runtime_error("Error on disconnect");
	}
	return status;
}



