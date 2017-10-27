#include <thread>
#include <chrono>
#include "ServiceProvider.h"
#include "CommonHeader/debugLog.h"

#define eI_MODULE "ServiceProvider"
static bool CLOUD_THREAD_START = 1;


/**
 * Abstract derived class implemented by Device users and called by AllJoyn to inform
 * users of About interface related events as well as handle Interface call.
 */
ServiceProvider::ServiceProvider(TransportMask mask, ajn::BusAttachment *_pBusAttach, const char* _interfaceName, const char* _path)
:eIBusObject(_pBusAttach, _interfaceName, _path),
 methoExePtr(new methodExecuter(_pBusAttach,this,_interfaceName,_path)),
 signalExePtr(new signalExecuter(_pBusAttach,this,_interfaceName)),
 deviceObjPtr(new DeviceObj(mask, this)), pBusAttach(_pBusAttach),
 callbackThread(NULL),
 isDiscovryDone(false), isCloudConnected(false),isLocalConnected(false),isTestConnected(false),
 isCloudThreadStart(false), isNewCloudAppInVicinity(false),
 lossCloudConnection(false), isPTLload(true), transportMast(mask),
 isCloudConnectedSignalGot(false), isLocalConnectedSignalGot(false)
{
	this->init();
	confPtr = new configurationSettings();

	manageConfig = new manageConfiguration(this);

	cloudThreadRunner = true;
	this->startCloudConnection();

	localThreadRunner = true;
	this->startLocalConnection();
}

ServiceProvider::~ServiceProvider()
{
	cloudThreadRunner = false;
	localThreadRunner = false;

	if(methoExePtr)
		delete methoExePtr;
	if(signalExePtr)
		delete signalExePtr;
	if(deviceObjPtr)
		delete deviceObjPtr;

	if(cloudConnectionThread.joinable())
		cloudConnectionThread.join();

	if(localConnectionThread.joinable())
		localConnectionThread.join();


}

void ServiceProvider::init()
{
	persistantFp = "/opt/GA.conf.d/DMA/persistant.conf";
}

QStatus ServiceProvider::loadLIB()
{
	LOGDBUG("In ServiceProvider loadLIB .. .. ");
	LOGDBUG2I("(loadLIB) [TRANSPORTMAST]",transportMast);
	std::string strReturn;
	QStatus status = ER_FAIL;
	if(deviceObjPtr) {
		if(ER_OK != (status = deviceObjPtr->checkAndCreateTransport(transportMast, strReturn))) {
			status = ER_FAIL;
			LOGEROR2("(loadLIB) Fail To load Lib !! [strReturn]",strReturn);
		}
	} else {
		LOGEROR("(loadLIB) Invalid DeviceObj Ptr !!");
		status = ER_FAIL;
	}
	return status;
}

/* RAML Handler will process `GetMRAML` request and returning the string back to the sender.*/
QStatus ServiceProvider::getMRAMLHandler(std::string &command, std::string &payload)
{
	QStatus status = ER_FAIL;
	LOGDBUG3("(ServiceProvider) GetMRAMLHandler [command : payload]",command,payload);
	return status;
}

/* AsyncListener Handler will process `AsyncListener` request and returning the string back to the sender.*/
QStatus ServiceProvider::asyncHandler(std::string &command, std::string &payload)
{
	QStatus status = ER_OK;
	LOGDBUG3("(ServiceProvider) AsyncListenerHandler [command : payload]",command,payload);
	if(command.find(TOPIC_PROCESSFLAG) != std::string::npos){
		status = this->processTopic(command,payload);
	} else {
		status = this->processCommand(command,payload);
	}
	return status;
}

/* SyncListener Handler will process `SyncListener` request and returning the string back to the sender.*/
QStatus ServiceProvider::syncHandler(std::string &command, std::string &payload)
{
	std::lock_guard<std::mutex> lockObj(mLockProcessTopic);

	QStatus status = ER_OK;
	LOGDBUG3("(ServiceProvider) SyncListenerHandler [command : payload]",command,payload);
	if(command.find(TOPIC_PROCESSFLAG) != std::string::npos){
		status = this->processTopic(command,payload);
	} else {
		if(!command.compare(DOC_DEVICEEXISTS)) {
			this->isDeviceExists(command,payload,MC_SYNCLISTENER);
		} else {
			status = this->processCommand(command,payload);
		}
	}

	if(status == ER_OK)
	{
		//this->updatePersistantFile(command, payload);
	}

	return status;
}

/* CallbackRegister Handler will process `CallbackRegister` request and returning the string back to the sender.*/
QStatus ServiceProvider::callbackRegisterHandler(std::string &command, std::string &payload)
{
	QStatus status = ER_FAIL;
	LOGDBUG3("(ServiceProvider) CallbackRegisterHandler [command : payload] ====== ",command,payload);
	if(!command.compare(DOC_REGISTERDEVICECALLBACK)) {
		status = this->addRegisterCallbackApp(payload);
	} else if (!command.compare(DOC_DEREGISTERDEVICECALLBACK)) {
		status = this->removeRegisterCallbackApp(payload);
	} else if (!command.compare(DOC_CALLBACKFORDEVICEREGISTER)) {
		status = this->CallbackRegDev(payload);
	} else if (!command.compare(DOC_DECALLBACKFORDEVICEREGISTER)) {
		status = this->removeCallbackRegDev(payload);
	}
	return status;
}

/* CallbackListener Handler will process `CallbackListener` request.*/
void ServiceProvider::signalHandler(std::string &command, std::string &payload)
{
	LOGDBUG3("(ServiceProvider) CallbackListenerHandler [command : payload]",command,payload);
	//::TODO Added Cloud Command Process Only.
	if((command.find(CKW_CLOUD) != std::string::npos)){
		this->processCloudCallback(command,payload);
	} else if(!command.compare(DOC_DEVICEEXISTS)) {
		this->isDeviceExists(command,payload,MC_CALLBACKLISTENER);
	}
	return;
}


void ServiceProvider::updatePersistantFile(std::string command, std::string payload)
{
	std::cout << "(updatePersistantFile)command" << command << std::endl;
	std::cout << "(updatePersistant File)payload" << payload << std::endl;

}

QStatus ServiceProvider::SendDiscoveredDevices(const qcc::String& dest, const qcc::String& data)
{
	LOGDBUG("In ServiceProvider SendDiscoveredDevices .. .. ");
	LOGINFO3("(SendDiscoveredDevices) Got Destination and data [Destination : Data]",dest,data);
	LOGDBUG2("(SendDiscoveredDevices)[cloudAppName]",cloudEpInfo.cloudAppName);
	LOGDBUG2("(SendDiscoveredDevices)[topic]",cloudEpInfo.topic);
	LOGDBUG2I("(SendDiscoveredDevices)[iCloudEndpoint]",cloudEpInfo.iCloudEndpoint);
	LOGDBUG2I("(SendDiscoveredDevices)[ConnectionStatus]",cloudEpInfo.ConnectionStatus);
	std::string busName = cloudEpInfo.cloudAppName;
	SessionId sessionID;
	std::string payload;
	json jsonData;
	try  {
		jsonData[PP_MSG] = data;
		jsonData[PP_RESPONSECODE] = RC_SUCCESS;
		jsonData[PP_APPNAME] = ajNameSerPtr->getAdvertisedName();
		jsonData[PP_ENDPOINTID] = cloudEpInfo.iCloudEndpoint;
		jsonData[PP_TOPIC] = discovryTopic;
		payload = jsonData.dump();
		LOGDBUG2("(SendDiscoveredDevices) Send To Cloud Data [PAYLOAD]",payload);
	} catch (...) {
		LOGEROR("(SendDiscoveredDevices) Failed to process response Json !!");
	}

	if(isCloudConnected && ajNameSerPtr->GetNSInfo(busName,sessionID)) {
		int tryCout = 0;
		std::string command = CC_SENDTOCLOUD;
		while(tryCout < COULD_CONNECTION_TRY) {
			tryCout++;
			if(methoExePtr) {
				if(ER_OK == methoExePtr->methodCaller(busName, sessionID, "SyncListener", command, payload))
					//this->update
					break ;
			} else {
				LOGEROR("(SendDiscoveredDevices) Invalid Method Executer Ptr !!");
				break ;
			}
			usleep(500);
		}
	} else {
		LOGEROR("(SendDiscoveredDevices) Unable to Send Cloud !!");
	}
	isDiscovryDone = true;
	return ER_OK;
}

/* Get notification after new advertise find.*/
void ServiceProvider::foundAdvertisement(qcc::String busName)
{
	LOGDBUG2("(findAdvertise) Bus Name Find advertised",busName);
	bool sameGateway = false;

	if(busName.find(CLOUD_APP_NAME_PREFIX) != std::string::npos || busName.find(AZUREAPP_APP_NAME_PREFIX) != std::string::npos)
	{
		if(busName.find(configurationSettings::gatewayId) != std::string::npos)
			sameGateway = true;
		{
			std::lock_guard<std::mutex> cloudLock(cloudMutex);

			if (sameGateway)
			{
				personalApp.push_back(busName);
				cloudAppInVicinityVector.push_back(busName);
			}
			else
			{
				vicinityApp.push_back(busName);
				cloudAppInVicinityVector.push_back(busName);
			}
		}


		if(waitingForCloudApp)
		{

			std::unique_lock<std::mutex> uniLock(cloudInitMutex);
			cloudInitVar.notify_one();

			std::unique_lock<std::mutex> uniLocalLock(localInitMutex);
			localInitVar.notify_one();

			//std::unique_lock<std::mutex> unilocalLock(localInitMutex);
			//localInitVar.notify_one();
		}
	}
}

/* Get notification after loss advertise.*/
void ServiceProvider::lostAdvertisement(qcc::String busName)
{
	LOGWARN2("(lossAdvertise) Bus Name lost advertised",busName);
	//	if(isCloudConnected) {
	//		if(!busName.compare(cloudEpInfo.cloudAppName))
	//		{
	//			LOGWARN("(lossAdvertise) Loss Cloud Connection !!");
	//			//cloudEpInfo.cloudAppName = false;
	//			lossCloudConnection = true;
	//		}
	//	}

	if(busName == cloudEpInfo.cloudAppName)
	{
		bool sameGateway = false;

		if(busName.find(CLOUD_APP_NAME_PREFIX) != std::string::npos || busName.find(AZUREAPP_APP_NAME_PREFIX) != std::string::npos)
		{
			if(busName.find(configurationSettings::gatewayId) != std::string::npos)
				sameGateway = true;
			{
				std::lock_guard<std::mutex> cloudLock(cloudMutex);

				if (sameGateway)
				{
					personalApp.erase(std::remove(personalApp.begin(), personalApp.end(), std::string(busName)), personalApp.end());
					cloudAppInVicinityVector.erase(std::remove(cloudAppInVicinityVector.begin(), cloudAppInVicinityVector.end(), std::string(busName)), cloudAppInVicinityVector.end());
				}
				else
				{
					vicinityApp.erase(std::remove(vicinityApp.begin(), vicinityApp.end(), std::string(busName)), vicinityApp.end());
					cloudAppInVicinityVector.erase(std::remove(cloudAppInVicinityVector.begin(), cloudAppInVicinityVector.end(), std::string(busName)), cloudAppInVicinityVector.end());
				}
			}

			lossCloudConnection = true;
		}
	}


}

void ServiceProvider::busDisconnected()
{
}

ServiceProvider* ServiceProvider::getSrvPtr(){

	return this;
}

/*Get notification from Asyn Method Call reply.*/
void ServiceProvider::methodCallerAsyncCallback(std::string retStatus, std::string payload)
{
	LOGDBUG3("(MethodCallerAsyncCallback) CallbackListenerHandler [retStatus : payload]",retStatus, payload);
}

QStatus ServiceProvider::getTopicPrefix(std::string &prefix, std::string appID, json &jsonData)
{
	std::string str1;
	std::string str2;

	keyVec keyVectObj;

	std::string tempPrefix = prefix;

	//manageConfig = new manageConfiguration(this, jsonData);

	try
	{
		std::size_t found = tempPrefix.find_first_of("/");
		while(found!=std::string::npos)
		{
			tempPrefix = tempPrefix.substr(found+1);
			break;
		}
		std::size_t found1 = tempPrefix.find_first_of("$");
		while(found1!=std::string::npos)
		{
			tempPrefix = tempPrefix.substr(found1+1);
			std::size_t found1 = tempPrefix.find_first_of("$");
			str1 = tempPrefix.substr(0, found1);

			//			if(str1 == "appid")
			//			{
			//				str2 = configurationSettings::AppTopic;
			//
			//				keyVectObj.key = str1;
			//				keyVectObj.value = str2;
			//				configurationSettings::configKeyVec.push_back(keyVectObj);
			//			}
			//			else
			//			{
			str2 = jsonData[APPCONFIG][str1];

			keyVectObj.key = str1;
			keyVectObj.value = str2;
			configurationSettings::configKeyVec.push_back(keyVectObj);

			//			}


			std::string replaceStr = "$" + str1 + "$";
			std::size_t repStr = prefix.find(replaceStr);

			prefix.replace(repStr,replaceStr.size(),str2);

			std::size_t found2 = tempPrefix.find_first_of("/");
			if(found2!=std::string::npos)
			{
				tempPrefix = tempPrefix.substr(found1+2);
			}
			else
			{
				break;
			}
		}

		return ER_OK;
	}
	catch(...){
		std::cout << "In exception" << std::endl;
	}
}


QStatus ServiceProvider::processCloudCallback(std::string &command, std::string &payload)
{
	std::cout << "Processcommand" << std::endl;

	json jsonData = json::parse(payload);

	uint16_t endpointID = jsonData[PP_ENDPOINTID];

	QStatus status = ER_FAIL;

	//If got callback signal, check for whether it is, connection callback, disconnection  callback or error callback.
	if(!command.compare(CCR_CONNECTION)){

		for(std::vector<manageConnections>::iterator it = connManage.begin(); it!= connManage.end(); it++)
		{
			if(it->endpointid == endpointID)
			{
				if(it->type == CLOUDCONNECTION)
				{
					isCloudConnectedSignalGot = true;
					status = ER_OK;
				}
				else if(it->type == LOCALCONNECTION)
				{
					isLocalConnectedSignalGot = true;
					status = ER_OK;
				}
				else if(it->type == TESTCONNECTION)
				{
					//notify the test connection CV.
					isTestConnected = true;
					std::unique_lock<std::mutex> uniLock(testMutex);
					testInitVar.notify_one();
					status = ER_OK;
				}
			}
		}



	} else if(!command.compare(CCR_DISCONNECT)) {

		std::cout << "IN DISCOMMECY" << std::endl;

		for(std::vector<manageConnections>::iterator it = connManage.begin(); it!= connManage.end(); it++)
		{
			if(it->endpointid == endpointID)
			{
				if(it->type == CLOUDCONNECTION)
				{
					std::map<std::string, brokerDetails>::iterator iter;
					iter = configurationSettings::brokerMap.find(connectBrokerNum);

					if (iter != configurationSettings::brokerMap.end())
					{
						iter->second.type = SECONDARY;

						if(iter->second.brokerNum < iter->second.totalBroker)
						{

							int newBrokerNum = iter->second.brokerNum + 1;
							std::cout << "newbrokernum is" << newBrokerNum << std::endl;
							for(std::map<std::string, brokerDetails>::iterator tempIt=configurationSettings::brokerMap.begin();tempIt!=configurationSettings::brokerMap.end();++tempIt)
							{
								if(tempIt->second.brokerNum == newBrokerNum)
								{
									tempIt->second.type = PRIMARY;
									connectBrokerNum = tempIt->first;
								}
							}
						}
						else
						{


							for(std::map<std::string, brokerDetails>::iterator tempIt=configurationSettings::brokerMap.begin();tempIt!=configurationSettings::brokerMap.end();++tempIt)
							{
								if(tempIt->second.brokerNum == 1)
								{
									tempIt->second.type = PRIMARY;
									connectBrokerNum = tempIt->first;
								}
							}
						}
					}

					isCloudConnected = false;
					isCloudConnectedSignalGot = false;
					status = ER_OK;
				}
				else if(it->type == LOCALCONNECTION)
				{
					std::map<std::string, brokerDetails>::iterator iter;
					iter = configurationSettings::localBrokerMap.find(localBrokerNum);

					if (iter != configurationSettings::localBrokerMap.end())
					{
						iter->second.type = SECONDARY;

						if(iter->second.brokerNum < iter->second.totalBroker)
						{

							int newBrokerNum = iter->second.brokerNum + 1;

							for(std::map<std::string, brokerDetails>::iterator tempIt=configurationSettings::localBrokerMap.begin();tempIt!=configurationSettings::localBrokerMap.end();++tempIt)
							{
								if(tempIt->second.brokerNum == newBrokerNum)
								{
									tempIt->second.type = PRIMARY;
									localBrokerNum = tempIt->first;
								}
							}
						}
						else
						{
							for(std::map<std::string, brokerDetails>::iterator tempIt=configurationSettings::localBrokerMap.begin();tempIt!=configurationSettings::localBrokerMap.end();++tempIt)
							{
								if(tempIt->second.brokerNum == 1)
								{
									tempIt->second.type = PRIMARY;
									localBrokerNum = tempIt->first;
								}
							}
						}
					}

					isLocalConnected = false;
					isLocalConnectedSignalGot = false;
					status = ER_OK;
				}
				else if(it->type == TESTCONNECTION)
				{

					//notify the the test connection CV.
					isTestConnected = false;
					std::unique_lock<std::mutex> uniLock(testMutex);
					testInitVar.notify_one();
					status = ER_OK;
				}
			}
		}

		for(std::vector<manageConnections>::iterator it = connManage.begin(); it!= connManage.end(); it++)
		{
			if(it->endpointid == endpointID)
			{
				connManage.erase(it);
				break;
			}
		}

	} else if(!command.compare(CCR_ERRCONNECTION)) {


		for(std::vector<manageConnections>::iterator it = connManage.begin(); it!= connManage.end(); it++)
		{
			if(it->endpointid == endpointID)
			{
				if(it->type == CLOUDCONNECTION)
				{
					std::map<std::string, brokerDetails>::iterator iter;
					iter = configurationSettings::brokerMap.find(connectBrokerNum);

					if (iter != configurationSettings::brokerMap.end())
					{
						iter->second.type = SECONDARY;

						if(iter->second.brokerNum < iter->second.totalBroker)
						{
							int newBrokerNum = iter->second.brokerNum + 1;

							for(std::map<std::string, brokerDetails>::iterator tempIt=configurationSettings::brokerMap.begin();tempIt!=configurationSettings::brokerMap.end();++tempIt)
							{
								if(tempIt->second.brokerNum == newBrokerNum)
								{
									tempIt->second.type = PRIMARY;
									connectBrokerNum = tempIt->first;
								}
							}
						}
						else
						{


							for(std::map<std::string, brokerDetails>::iterator tempIt=configurationSettings::brokerMap.begin();tempIt!=configurationSettings::brokerMap.end();++tempIt)
							{
								if(tempIt->second.brokerNum == 1)
								{
									tempIt->second.type = PRIMARY;
									connectBrokerNum = tempIt->first;
								}
							}
						}
					}

					isCloudConnected = false;
					isCloudConnectedSignalGot = false;
					status = ER_OK;
				}
				else if(it->type == LOCALCONNECTION)
				{
					std::map<std::string, brokerDetails>::iterator iter;
					iter = configurationSettings::localBrokerMap.find(localBrokerNum);

					if (iter != configurationSettings::localBrokerMap.end())
					{
						iter->second.type = SECONDARY;

						if(iter->second.brokerNum < iter->second.totalBroker)
						{
							int newBrokerNum = iter->second.brokerNum + 1;

							for(std::map<std::string, brokerDetails>::iterator tempIt=configurationSettings::localBrokerMap.begin();tempIt!=configurationSettings::localBrokerMap.end();++tempIt)
							{
								if(tempIt->second.brokerNum == newBrokerNum)
								{
									tempIt->second.type = PRIMARY;
									localBrokerNum = tempIt->first;
								}
							}
						}
						else
						{
							for(std::map<std::string, brokerDetails>::iterator tempIt=configurationSettings::localBrokerMap.begin();tempIt!=configurationSettings::localBrokerMap.end();++tempIt)
							{
								if(tempIt->second.brokerNum == 1)
								{
									tempIt->second.type = PRIMARY;
									localBrokerNum = tempIt->first;
								}
							}
						}
					}

					isLocalConnected = false;
					isLocalConnectedSignalGot = false;
					status = ER_OK;
				}
				else if(it->type == TESTCONNECTION)
				{
					//notify the test connection CV.
					isTestConnected = false;
					std::unique_lock<std::mutex> uniLock(testMutex);
					testInitVar.notify_one();
					status = ER_OK;
				}
			}
		}

		for(std::vector<manageConnections>::iterator it = connManage.begin(); it!= connManage.end(); it++)
		{
			if(it->endpointid == endpointID)
			{
				connManage.erase(it);
				break;
			}
		}
	}
	return status;
}

QStatus ServiceProvider::connectingCloudAndRegisterTopic(std::string busName)
{
	LOGDBUG("In ServiceProvider connectingCloudAndRegisterTopic .. .. ");

	QStatus status = ER_OK;
	int iCount = 0;
	SessionId sessionID;
	checkBool = false;

	for(std::map<std::string, brokerDetails>::iterator it=configurationSettings::brokerMap.begin();it!=configurationSettings::brokerMap.end();++it)
	{
		if(it->second.type == PRIMARY)
		{
			connectBrokerNum = it->first;

			//connectMap[it->second.mqqthostname] = CLOUDCONNECTION;

			if(ajNameSerPtr->findNSByKey(busName,sessionID))
			{
				//for(iCount = 0; iCount < COULD_CONNECTION_TRY; iCount++)
				{
					LOGINFO2("(CloudConnectionThread) process to cloud connection [BusName]",busName);
					std::string strCommand = CC_CONNECTION;
					json jsonData;
					jsonData[PP_HOSTNAME] = it->second.mqqthostname;
					jsonData[PP_USERNAME] = it->second.mqqtusername;
					jsonData[PP_PASSWORD] = it->second.mqqtpassword;
					jsonData[PP_PORT] = it->second.mqqtport;

					jsonData[PP_APPNAME]  = ajNameSerPtr->getAdvertisedName();
					jsonData[PP_CLIENTID] = ajNameSerPtr->getAdvertisedName();

					if(certRequired == true && it->second.mqqtport != 1883)
					{
						jsonData[PP_CAFILE] = it->second.cafile;
						jsonData[PP_CERTFILE] = it->second.certfile;
						jsonData[PP_KEYFILE] = it->second.keyfile;
						jsonData[PP_VERSION] = it->second.version;
						jsonData[PP_CIPHER] = it->second.cipher;

					}

					std::string strPayload =  jsonData.dump();

					//Store the present sessionID.
					presentSessionID = sessionID;

					LOGDBUG2("(CloudConnectionThread) Connecting Cloud With Payload [Json]", strPayload);

					if((ER_OK == (status = methoExePtr->methodCaller(busName, sessionID, MC_SYNCLISTENER, strCommand, strPayload))))
					{
						LOGINFO2("(CloudConnectionThread) return Data [Commond]",strCommand);
						LOGINFO2("(CloudConnectionThread) return payload [payload]",strPayload);

						//-------------------------------------

						json returnJson = json::parse(strPayload);

						int endpointID = returnJson["endpointid"];

						manageConnections confObj;
						confObj.brokerID = jsonData[PP_HOSTNAME];
						confObj.endpointid = endpointID;
						confObj.type = CLOUDCONNECTION;

						connManage.push_back(confObj);

						//-------------------------------------

						/*Wait For Cloud Connection Success Response. */
						bool loop(true);
						int iCount = 15;
						do {
							LOGWARN2I("(CloudConnectionThread) Waiting for cloud connection signal ",iCount);
							if(isCloudConnectedSignalGot) {
								loop = false;
								LOGINFO("(CloudConnectionThread) Got Connection Signal Move To Register Topic .. .. ");
							}
							iCount--;
							std::this_thread::sleep_for (std::chrono::seconds(THREAD_SLEEP_1));
						}while(loop && iCount);

						//If got the connection signal subscribe the topic.
						if(isCloudConnectedSignalGot) {
							checkBool = true;
							isCloudConnectedSignalGot = false;
							try  {
								jsonData.clear();
								jsonData = json::parse(strPayload);
								if(jsonChecker::checkJson(jsonData,{PP_RESPONSECODE})) {
									if(RC_SUCCESS == jsonData[PP_RESPONSECODE]) {
										if(jsonChecker::checkJson(jsonData,{PP_ENDPOINTID})) {

											cloudEpInfo.iCloudEndpoint = jsonData[PP_ENDPOINTID];
											cloudEpInfo.ConnectionStatus = CS_SUCCESS;
											cloudEpInfo.topic = configurationSettings::subTopicPrefix;
											cloudEpInfo.cloudAppName = busName;

											strCommand = CC_REGISTERTOPIC;

											jsonData.clear();
											jsonData[PP_APPNAME] = ajNameSerPtr->getAdvertisedName();
											jsonData[PP_TOPIC] = configurationSettings::subTopicPrefix;
											jsonData[PP_ENDPOINTID] = cloudEpInfo.iCloudEndpoint;
											strPayload = jsonData.dump();

											//method call for registeration of topic.
											status = methoExePtr->methodCaller(busName, sessionID, MC_SYNCLISTENER, strCommand, strPayload);

											jsonData.clear();
											jsonData = json::parse(strPayload);
											int i = jsonData[PP_RESPONSECODE];
											LOGDBUG2I("(connectingCloudAndRegisterTopic) payload response code[PP_RESPONSECODE]",i);
											if(RC_FAIL == jsonData[PP_RESPONSECODE]) {
												status = ER_FAIL;
												LOGEROR("(connectingCloudAndRegisterTopic) Unable to Register Topic !!");
											}
										} else {
											status = ER_FAIL;
											cloudEpInfo = {};
											LOGEROR("(connectingCloudAndRegisterTopic) Unable to Get Endpoint ID !!");
										}
									} else {
										status = ER_FAIL;
										cloudEpInfo = {};
										LOGEROR("(connectingCloudAndRegisterTopic) Unable to Connect Cloud !!");
									}
								}
							} catch (...) {
								status = ER_FAIL;
								cloudEpInfo = {};
								LOGEROR("(connectingCloudAndRegisterTopic) Unable to Process Json !!");
							}
						} else {

							//If connection is unsuccessful with primary, make the another broker secondary.
							for(std::vector<manageConnections>::iterator iter = connManage.begin(); iter!= connManage.end(); iter++)
							{
								if(iter->endpointid == endpointID)
								{
									connManage.erase(iter);
									break;
								}
							}

							it->second.type = SECONDARY;
							if(it->second.brokerNum < it->second.totalBroker)
							{

								int newBrokerNum = it->second.brokerNum + 1;

								for(std::map<std::string, brokerDetails>::iterator tempIt=configurationSettings::brokerMap.begin();tempIt!=configurationSettings::brokerMap.end();++tempIt)
								{
									if(tempIt->second.brokerNum == newBrokerNum)
									{
										tempIt->second.type = PRIMARY;
										connectBrokerNum = tempIt->first;
									}
								}
							}
							else
							{

								for(std::map<std::string, brokerDetails>::iterator tempIt=configurationSettings::brokerMap.begin();tempIt!=configurationSettings::brokerMap.end();++tempIt)
								{
									if(tempIt->second.brokerNum == 1)
									{
										tempIt->second.type = PRIMARY;
										connectBrokerNum = tempIt->first;
									}
								}
							}

							status = ER_FAIL;
							cloudEpInfo = {};
							LOGWARN("(connectingCloudAndRegisterTopic) Unable to Got Cloud Connection Signal !! ");
						}
					} else {
						status = ER_FAIL;
						cloudEpInfo = {};
						LOGEROR2("(connectingCloudAndRegisterTopic) Unable to Call MethodCaller .. .. ",QCC_StatusText(status));
					}
					if(ER_OK == status) {
						//break;
					}
					else {
						cloudEpInfo = {};
					}
				}
			} else {
				LOGEROR2("(connectingCloudAndRegisterTopic) Unable to find Cloud Bus Name [BusName]",busName);
				status = ER_FAIL;
			}

		}

		if(checkBool == true)
		{
			break;
		}
	}
	return status;
}


QStatus ServiceProvider::connectingLocalAndRegisterTopic(std::string busName)
{
	LOGDBUG("In ServiceProvider connectingLocalAndRegisterTopic .. .. ");
	QStatus status = ER_OK;
	int iCount = 0;
	SessionId sessionID;
	checkBool = false;

	for(std::map<std::string, brokerDetails>::iterator it=configurationSettings::localBrokerMap.begin();it!=configurationSettings::localBrokerMap.end();++it)
	{
		//For connecting DO app to local broker
		if(it->second.type == PRIMARY)
		{

			localBrokerNum = it->first;

			//connectMap[it->second.mqqthostname] = LOCALCONNECTION;

			if(ajNameSerPtr->findNSByKey(busName,sessionID))
			{
				//for(iCount = 0; iCount < COULD_CONNECTION_TRY; iCount++)
				{
					LOGINFO2("(CloudConnectionThread) process to local connection [BusName]",busName);
					std::string strCommand = CC_CONNECTION;
					json jsonData;
					jsonData[PP_HOSTNAME] = it->second.mqqthostname;
					jsonData[PP_USERNAME] = it->second.mqqtusername;
					jsonData[PP_PASSWORD] = it->second.mqqtpassword;
					jsonData[PP_PORT] = it->second.mqqtport;
					jsonData[PP_APPNAME]  = ajNameSerPtr->getAdvertisedName();
					jsonData[PP_CLIENTID] = ajNameSerPtr->getAdvertisedName();

					if(certRequired == true && it->second.mqqtport != 1883)
					{
						jsonData[PP_CAFILE] = it->second.cafile;
						jsonData[PP_CERTFILE] = it->second.certfile;
						jsonData[PP_KEYFILE] = it->second.keyfile;
						jsonData[PP_VERSION] = it->second.version;
						jsonData[PP_CIPHER] = it->second.cipher;

					}

					std::string strPayload =  jsonData.dump();

					LOGDBUG2("(localConnectionThread) Connecting local With Payload [Json]", strPayload);

					if((ER_OK == (status = methoExePtr->methodCaller(busName, sessionID, MC_SYNCLISTENER, strCommand, strPayload))))
					{
						LOGINFO2("(LocalConnectionThread) return Data [Commond]",strCommand);
						LOGINFO2("(LocalConnectionThread) return payload [payload]",strPayload);

						//-------------------------------------

						json returnJson = json::parse(strPayload);

						int endpointID = returnJson["endpointid"];

						manageConnections confObj;
						confObj.brokerID = jsonData[PP_HOSTNAME];
						confObj.endpointid = endpointID;
						confObj.type = LOCALCONNECTION;

						connManage.push_back(confObj);

						//-------------------------------------


						/*Wait For Cloud Connection Success Response. */
						bool loop(true);
						int iCount = 15;
						do {
							LOGWARN2I("(LocalConnectionThread) Waiting for local connection signal ",iCount);
							if(isLocalConnectedSignalGot) {
								loop = false;
								LOGINFO("(CloudConnectionThread) Got Connection Signal Move To Register Topic .. .. ");
							}
							iCount--;
							std::this_thread::sleep_for (std::chrono::seconds(THREAD_SLEEP_1));
						}while(loop && iCount);

						//if got the connection callback, subscribe the topic.
						if(isLocalConnectedSignalGot) {
							checkBool = true;
							isLocalConnectedSignalGot = false;

							std::cout << "LOCAL LOCAL LOACL" << std::endl;
							try  {
								jsonData.clear();
								jsonData = json::parse(strPayload);
								if(jsonChecker::checkJson(jsonData,{PP_RESPONSECODE})) {
									if(RC_SUCCESS == jsonData[PP_RESPONSECODE]) {
										if(jsonChecker::checkJson(jsonData,{PP_ENDPOINTID})) {

											localEpInfo.ilocalEndpoint = jsonData[PP_ENDPOINTID];
											localEpInfo.ConnectionStatus = CS_SUCCESS;
											localEpInfo.topic = configurationSettings::subTopicPrefix;
											localEpInfo.cloudAppName = busName;

											strCommand = CC_REGISTERTOPIC;
											jsonData.clear();

											jsonData[PP_APPNAME] = ajNameSerPtr->getAdvertisedName();
											jsonData[PP_TOPIC] = configurationSettings::subTopicPrefix;
											jsonData[PP_ENDPOINTID] = localEpInfo.ilocalEndpoint;
											strPayload = jsonData.dump();

											//method call for registeration of topic.
											status = methoExePtr->methodCaller(busName, sessionID, MC_SYNCLISTENER, strCommand, strPayload);

											jsonData.clear();
											jsonData = json::parse(strPayload);
											int i = jsonData[PP_RESPONSECODE];

											LOGDBUG2I("(connectinglocalAndRegisterTopic) payload response code[PP_RESPONSECODE]",i);
											if(RC_FAIL == jsonData[PP_RESPONSECODE]) {
												status = ER_FAIL;
												LOGEROR("(connectingLocalAndRegisterTopic) Unable to Register Topic !!");
											}
										} else {
											status = ER_FAIL;
											localEpInfo = {};
											LOGEROR("(connectingLocalAndRegisterTopic) Unable to Get Endpoint ID !!");
										}
									} else {
										status = ER_FAIL;
										localEpInfo = {};
										LOGEROR("(connectingLocalAndRegisterTopic) Unable to Connect Cloud !!");
									}
								}
							} catch (...) {
								status = ER_FAIL;
								localEpInfo = {};
								LOGEROR("(connectingCloudAndRegisterTopic) Unable to Process Json !!");
							}
						} else {

							//If connection is unsuccessful with primary, ,make the another broker primary.
							for(std::vector<manageConnections>::iterator iter = connManage.begin(); iter!= connManage.end(); iter++)
							{
								if(iter->endpointid == endpointID)
								{
									connManage.erase(iter);
									break;
								}
							}

							it->second.type = SECONDARY;
							if(it->second.brokerNum < it->second.totalBroker)
							{

								int newBrokerNum = it->second.brokerNum + 1;

								for(std::map<std::string, brokerDetails>::iterator tempIt=configurationSettings::localBrokerMap.begin();tempIt!=configurationSettings::localBrokerMap.end();++tempIt)
								{
									if(tempIt->second.brokerNum == newBrokerNum)
									{
										tempIt->second.type = PRIMARY;
										localBrokerNum = tempIt->first;
									}
								}
							}
							else
							{
								for(std::map<std::string, brokerDetails>::iterator tempIt=configurationSettings::localBrokerMap.begin();tempIt!=configurationSettings::localBrokerMap.end();++tempIt)
								{
									if(tempIt->second.brokerNum == 1)
									{
										std::cout << "resent" << std::endl;
										tempIt->second.type = PRIMARY;
										localBrokerNum = tempIt->first;
									}
								}
							}

							status = ER_FAIL;
							localEpInfo = {};
							LOGWARN("(connectingLocalAndRegisterTopic) Unable to Got Cloud Connection Signal !! ");
						}

					} else {
						status = ER_FAIL;
						localEpInfo = {};
						LOGEROR2("(connectingLocalAndRegisterTopic) Unable to Call MethodCaller .. .. ",QCC_StatusText(status));
					}
					if(ER_OK == status) {
						//break;
					}
					else {
						localEpInfo = {};
					}
				}
			} else {
				LOGEROR2("(connectingLocalAndRegisterTopic) Unable to find Cloud Bus Name [BusName]",busName);
				status = ER_FAIL;
			}

		}

		if(checkBool == true)
		{
			break;
		}
	}
	return status;
}

QStatus ServiceProvider::testConnection(json &jsonObj)
{
	LOGINFO("(ServiceProvider) Test-connection");

	std::cout << "testconn" << jsonObj.dump() << std::endl;
	QStatus status;
	json jsonData;
	std::string strPayload;

	std::string strCommand = CC_CONNECTION;

	try{
		jsonData[PP_HOSTNAME] = jsonObj[DATA][BROKER][HOST];
		jsonData[PP_USERNAME] = jsonObj[DATA][BROKER][USER];
		jsonData[PP_PASSWORD] = jsonObj[DATA][BROKER][PASS];
		jsonData[PP_PORT] = jsonObj[DATA][BROKER][PORT];

		if(jsonData[PP_PORT] != 1883)
		{
			jsonData[PP_CAFILE] = jsonObj[DATA][BROKER][JKF_SSLCONFIGURATION][JKF_MQTTCAFILE];
			jsonData[PP_CERTFILE] = jsonObj[DATA][BROKER][JKF_SSLCONFIGURATION][JKF_MQTTCERTFILE];
			jsonData[PP_KEYFILE] = jsonObj[DATA][BROKER][JKF_SSLCONFIGURATION][JKF_MQTTKEYFILE];

			jsonData[PP_VERSION] = "tlsv1.2";
			jsonData[PP_CIPHER] = "AES256-SHA";
		}

		jsonData[PP_APPNAME]  = ajNameSerPtr->getAdvertisedName();
		jsonData[PP_CLIENTID] = ajNameSerPtr->getAdvertisedName();

		strPayload = jsonData.dump();

		//Check if the provided broker details is already connected then directly send the response.
		for(std::vector<manageConnections>::iterator it = connManage.begin(); it!= connManage.end(); it++)
		{
			if(jsonData[PP_HOSTNAME] == it->brokerID)
			{
				std::cout << "connection already exist" << std::endl;
				jsonObj[PP_STATUS] = "success";
				jsonObj[PP_RESPONSECODE] = RC_SUCCESS;
				jsonObj[PP_MSG] = "connection successful";
				std::cout << "JSONDATA" << jsonObj.dump() << std::endl;
				return ER_OK;
			}
		}

		LOGDBUG2("(ServiceProvider) Test connection With Payload [Json]", jsonData.dump());

		//If new broker connection then call the method of MqttApp.
		status = methoExePtr->methodCaller(cloudEpInfo.cloudAppName, presentSessionID, MC_SYNCLISTENER, strCommand, strPayload);

		jsonData = json::parse(strPayload);

		//Check response code, if failure , send the response without waiting for the signal.
		if(jsonData[PP_RESPONSECODE] == RC_FAIL)
		{
			jsonObj[PP_STATUS] = "failure";
			jsonObj[PP_RESPONSECODE] = 400;
			jsonObj[PP_MSG] = jsonData[PP_MSG];

			status = ER_FAIL;
		}
		else
		{

			json returnJson = json::parse(strPayload);

			uint16_t endpointID = returnJson["endpointid"];

			manageConnections confObj;
			confObj.brokerID = jsonData[PP_HOSTNAME];
			confObj.endpointid = endpointID;
			confObj.type = TESTCONNECTION;

			connManage.push_back(confObj);


			//Wait until signal is received related to TestConnection Broker.
			std::unique_lock<std::mutex> uniLock(testMutex);
			testInitVar.wait(uniLock);

			if(isTestConnected)
			{
				jsonObj[PP_STATUS] = "success";
				jsonObj[PP_RESPONSECODE] = RC_SUCCESS;
				jsonObj[PP_MSG] = "connection successful";

				status = ER_OK;
			}
			else
			{
				jsonObj[PP_STATUS] = "failure";
				jsonObj[PP_RESPONSECODE] = RC_FAIL;
				jsonObj[PP_MSG] = "connection failure";

				status = ER_FAIL;
			}

			//Erase from map once completed as this is just for test connection.

			for(std::vector<manageConnections>::iterator it = connManage.begin(); it!= connManage.end(); it++)
			{
				if(it->brokerID == confObj.brokerID && it->type == TESTCONNECTION)
				{
					connManage.erase(it);
					break;
				}
			}
		}
	}
	catch(...)
	{
		jsonObj[PP_STATUS] = "failure";
		jsonObj[PP_RESPONSECODE] = RC_SUCCESS;
		jsonObj[PP_MSG] = "connection failure";

		status = ER_FAIL;

	}
	return status;
}


QStatus ServiceProvider::subscribeConfTopic(std::string topicPrefix)
{

	LOGDBUG("In ServiceProvider unsubscribeTOPIC .. .. ");

	QStatus status = ER_FAIL;
	std::string strCommand = CC_UN_REGISTERENDPOINT;
	SessionId sessionID;
	json jsonData;

	try
	{
		jsonData[PP_APPNAME]  = ajNameSerPtr->getAdvertisedName();
		jsonData[PP_TOPIC] = configurationSettings::subTopicPrefix;

		jsonData[PP_ENDPOINTID] = cloudEpInfo.iCloudEndpoint;
		std::string strPayload =  jsonData.dump();

		LOGDBUG2("(unsubscribe) unsubscribe With Payload [Json]", strPayload);
		if((ER_OK == (status = methoExePtr->methodCaller(cloudEpInfo.cloudAppName, presentSessionID, MC_SYNCLISTENER, strCommand, strPayload))))
		{
			try  {
				LOGINFO("(unsubscribe) Successfully unregister topic .. ..");

				jsonData.clear();
				jsonData = json::parse(strPayload);

				//If successfully unsubscribed the old topic, then move onto subscribe the new topic.
				if(RC_SUCCESS == jsonData[PP_RESPONSECODE]) {

					//If old topic unsubscribed subscribe new topic.
					configurationSettings::subTopicPrefix = topicPrefix;
					configurationSettings::pubTelemetryTopic = configurationSettings::appBaseUrl + "/Data/" +  configurationSettings::orgId + "/" + configurationSettings::gatewayId + "/" + configurationSettings::AppTopic + "/Devices/";

					std::cout << "new telemetry topic is" << configurationSettings::pubTelemetryTopic << std::endl;

					cloudEpInfo.topic = configurationSettings::subTopicPrefix;
					localEpInfo.topic = configurationSettings::subTopicPrefix;

					json subJson;
					subJson[PP_APPNAME]  = ajNameSerPtr->getAdvertisedName();
					subJson[PP_TOPIC] = configurationSettings::subTopicPrefix;
					subJson[PP_ENDPOINTID] = cloudEpInfo.iCloudEndpoint;
					std::string subPayload =  subJson.dump();

					strCommand = CC_REGISTERTOPIC;
					status = methoExePtr->methodCaller(cloudEpInfo.cloudAppName, presentSessionID, MC_SYNCLISTENER, strCommand, subPayload);

				} else {
					std::string  strMsg = jsonData[PP_MSG];
					LOGEROR2("(unsubscribing topic) Unable to unsubscribe [msg]",strMsg);
					status = ER_FAIL;
				}
			} catch (...) {
				status = ER_FAIL;
				LOGEROR("(disConnectingCloud) Unable to Process Json !!");
			}
		}
	}
	catch(...)
	{
		status = ER_FAIL;
	}

	return status;
}

QStatus ServiceProvider::disConnectingCloud(std::string busName)
{
	LOGDBUG("In ServiceProvider disConnectingCloud .. .. ");
	QStatus status = ER_OK;
	std::string strCommand = CC_DISCONNECT;
	SessionId sessionID;
	json jsonData;
	jsonData[PP_APPNAME]  = ajNameSerPtr->getAdvertisedName();
	jsonData[PP_ENDPOINTID] = cloudEpInfo.iCloudEndpoint;
	std::string strPayload =  jsonData.dump();

	if(ajNameSerPtr->findNSByKey(cloudEpInfo.cloudAppName,sessionID))
	{
		LOGDBUG2("(disConnectingCloud) DisConnecting Cloud With Payload [Json]", strPayload);
		if((ER_OK == (status = methoExePtr->methodCaller(busName, sessionID, MC_SYNCLISTENER, strCommand, strPayload))))
		{
			try  {
				LOGINFO("(disConnectingCloud) Successfully DisConnecting with Cloud .. ..");
				jsonData.clear();
				jsonData = json::parse(strPayload);
				if(RC_SUCCESS == jsonData[PP_RESPONSECODE]) {
					status = ER_OK;
					cloudEpInfo = {};
				} else {
					std::string  strMsg = jsonData[PP_MSG];
					LOGEROR2("(disConnectingCloud) Unable to Disconnect [msg]",strMsg);
					cloudEpInfo.ConnectionStatus = false;
					status = ER_FAIL;
				}
			} catch (...) {
				status = ER_FAIL;
				cloudEpInfo.ConnectionStatus = false;
				LOGEROR("(disConnectingCloud) Unable to Process Json !!");
			}
		}
	}
	return status;
}



void ServiceProvider::SendTelemetryData(const std::string &senID, std::string data)
{
	LOGDBUG("In ServiceProvider SendTelemetryData .. .. ");

	std::string busName = cloudEpInfo.cloudAppName;
	//EX. Topic - EI/Data/OrgID/GwId/AppName/Devices/DevId
	//std::string strTeleTopic = this->GetTopicID(eITOPIC::EI_TOPIC_DATA);
	std::string strTeleTopic = configurationSettings::pubTelemetryTopic;
	strTeleTopic += senID;
	LOGDBUG2("(SendTelemetryData) Telemetry Topic [strTeleTopic]",strTeleTopic);
	//this->SendToCloudData(strTeleTopic,data);
	this->SendToAppData(senID,data);
	return;
}

/*arg1 is Topic, arg2 is payload */
void ServiceProvider::SendToCloudData(const std::string &senID, std::string data)
{
	LOGDBUG("In ServiceProvider SendCloudData .. .. ");

	std::lock_guard<std::mutex> lockObj(mLockSendToCloud);

	std::string busName = cloudEpInfo.cloudAppName;
	std::string strTeleTopic = senID;
	LOGDBUG2("(SendTelemetryData) Telemetry Topic [strTeleTopic]",strTeleTopic);
	SessionId sessionID;
	std::string payload;
	json jsonData;
	try  {
		jsonData[PP_MSG] = data;
		jsonData[PP_RESPONSECODE] = RC_SUCCESS;
		jsonData[PP_APPNAME] = ajNameSerPtr->getAdvertisedName();
		jsonData[PP_ENDPOINTID] = cloudEpInfo.iCloudEndpoint;
		jsonData[PP_TOPIC] = strTeleTopic;
		payload = jsonData.dump();
		//		LOGDBUG2("(SendTelemetryData) Send To Cloud Data [PAYLOAD]",payload);
	} catch (...) {
		LOGEROR("(SendTelemetryData) Failed to process response Json !!");
	}

	if(isCloudConnected && ajNameSerPtr->GetNSInfo(busName,sessionID)) {
		int tryCout = 0;
		std::string command = CC_SENDTOCLOUD;
		while(tryCout < COULD_CONNECTION_TRY) {
			tryCout++;
			if(methoExePtr) {
				if(ER_OK == signalExePtr->signalCaller(busName, sessionID, MC_CALLBACKLISTENER, command, payload))
					break ;
			} else {
				LOGEROR("(SendDiscoveredDevices) Invalid Signal Executer Ptr !!");
				break ;
			}
			usleep(500);
		}
	} else {
		LOGEROR("(SendDiscoveredDevices) Unable to Send Cloud !!");
	}
	return;
}

QStatus ServiceProvider::CallbackRegDev(std::string& payload)
{
	/* CallbackRegDev -
	 * Command - "CallbackRegDev"
	 * payload - {"appname":"string"}
	 * reply payload - {"appname":"string","responsecode":200/400, ..}
	 * */
	QStatus status = ER_FAIL;

	std::cout << "CallbackRegDev " << payload << std::endl;

	json jsonData;
	try {
		jsonData = json::parse(payload);

		std::string val = jsonData[PP_APPNAME];
		appRegDevVec.emplace_back(val);

		jsonData[PP_RESPONSECODE] = RC_SUCCESS;
		payload = jsonData.dump();
	}
	catch(std::exception &e) {
		std::string strMeg = "Unable to Register CallBack !!";
		try {
			jsonData[PP_RESPONSECODE] = RC_FAIL;
			jsonData[PP_MSG] = strMeg;
			payload = jsonData.dump();
		} catch(...){
			LOGINFO("(addRegisterCallbackApp) Unable to Process Response Json !!");
		}
		LOGWARN2("(addRegisterCallbackApp) Unable to Register CallBack !!",e.what());
	}
	return status;
}

QStatus ServiceProvider::removeCallbackRegDev(std::string& payload)
{
	QStatus status = ER_FAIL;
	json jsonData;
	try {
		jsonData = json::parse(payload);

		std::string appName = jsonData[PP_APPNAME];

		/* Remove App-Info for Vector. */

		appRegDevVec.erase(std::remove_if(appRegDevVec.begin(),
				appRegDevVec.end(),
				[&](std::string  const& rItr ){ return (!rItr.compare(appName));}),
				appRegDevVec.end());

		jsonData[PP_RESPONSECODE] = RC_SUCCESS;
		payload = jsonData.dump();
	} catch(std::exception &e) {
		std::string strMeg = "Unable to unregister CallBack !!";
		try {
			jsonData[PP_RESPONSECODE] = RC_FAIL;
			jsonData[PP_MSG] = strMeg;
			payload = jsonData.dump();
		} catch(...){
			LOGINFO("(addRegisterCallbackApp) Unable to Process Response Json !!");
		}
		LOGWARN2("(addRegisterCallbackApp) Unable to Register CallBack !!",e.what());
	}
	return status;
}

QStatus ServiceProvider::addRegisterCallbackApp(std::string& payload)
{
	/* RegisterDeviceCallback -
	 * Command - "RegisterDeviceCallback"
	 * payload - {"appname":"string", "deviceid":"string", ..}
	 * reply payload - {"appname":"string", "deviceid":"string","responsecode":200/400, ..}
	 * */
	QStatus status = ER_FAIL;
	json jsonData;
	try {
		std::cout << "In addRegister callback" << payload << std::endl;
		jsonData = json::parse(payload);
		appEPInfo.appName = jsonData[PP_APPNAME];
		appEPInfo.strDevId = jsonData[PP_DEVICEID];
		appEPVec.push_back(appEPInfo);
		std::cout << "Pushed" << std::endl;
		appEPInfo = {};
		jsonData[PP_RESPONSECODE] = RC_SUCCESS;
		payload = jsonData.dump();
	} catch(std::exception &e) {
		std::string strMeg = "Unable to Register CallBack !!";
		try {
			jsonData[PP_RESPONSECODE] = RC_FAIL;
			jsonData[PP_MSG] = strMeg;
			payload = jsonData.dump();
		} catch(...){
			LOGINFO("(addRegisterCallbackApp) Unable to Process Response Json !!");
		}
		LOGWARN2("(addRegisterCallbackApp) Unable to Register CallBack !!",e.what());
	}
	return status;
}

QStatus ServiceProvider::removeRegisterCallbackApp(std::string& payload)
{
	QStatus status = ER_FAIL;
	json jsonData;
	try {
		jsonData = json::parse(payload);

		std::string strDevId = jsonData[PP_DEVICEID];

		/* Remove App-Info for Vector. */

		LOGDBUG("(LostAdvertisedName) Remove Device if added.");

		appEPVec.erase(
				remove_if(appEPVec.begin(),
						appEPVec.end(),
						[&](appEndPoint  const& rItr ){ return (!rItr.strDevId.compare(strDevId));}),
						appEPVec.end()
		);

		jsonData[PP_RESPONSECODE] = RC_SUCCESS;
		payload = jsonData.dump();
	} catch(std::exception &e) {
		std::string strMeg = "Unable to unregister CallBack !!";
		try {
			jsonData[PP_RESPONSECODE] = RC_FAIL;
			jsonData[PP_MSG] = strMeg;
			payload = jsonData.dump();
		} catch(...){
			LOGINFO("(addRegisterCallbackApp) Unable to Process Response Json !!");
		}
		LOGWARN2("(addRegisterCallbackApp) Unable to Register CallBack !!",e.what());
	}
	return status;
}

void ServiceProvider::setAdvName(const std::string &advAppName)
{
	advName = advAppName;
}

void ServiceProvider::SendRegDevNotification(const std::string &devID, std::string &status)
{
	json jsonData;
	std::string payload;
	try {
		jsonData[PP_DEVICEID] = devID;
		jsonData[PP_STATUS] = status;
		jsonData[PP_APPNAME] = advName;

		payload = jsonData.dump();
	}
	catch(std::exception &e)
	{
		LOGWARN2("Unable to add Dev ID in payload !! [EXCEPTION]",e.what());
	}

	for(auto &appInfo : appRegDevVec) {

		LOGINFO(appInfo);

		std::cout << "======================= "  << appRegDevVec.size();
		SessionId sessionID;

		if(ajNameSerPtr->GetNSInfo(appInfo,sessionID)) {

			if(methoExePtr)
			{
				signalExePtr->signalCaller(appInfo, sessionID, MC_CALLBACKLISTENER, const_cast<std::string &>(REGISTERED_NS::CB_REGISTERED), payload);
			}
			else
			{
				LOGEROR("(SendToAppData) Invalid Signal Executer Ptr !!");
			}
		} else {
			LOGWARN("(SendToAppData) Unable to Send App !!");
		}
	}
	return ;
}

void ServiceProvider::SendToAppData(const std::string &devID, std::string payload)
{
	LOGDBUG("In ServiceProvider SendToAppData .. .. ");
	LOGDBUG3("(SendToAppData) Telemetry Data [DEVID PAYLOAD]",devID,payload);
	json jsonData;
	try {
		jsonData = json::parse(payload);
		jsonData[PP_DEVICEID] = devID;
		payload = jsonData.dump();
	}catch(std::exception &e){
		LOGWARN2("Unable to add Dev ID in payload !! [EXCEPTION]",e.what());
	}


	for(appEndPoint &appInfo : appEPVec) {
		std::cout << "Here" << std::endl;
		LOGDBUG3("(SendToAppData) App Name [APPNAME DEVID]",appInfo.appName,appInfo.strDevId);
		if(!appInfo.strDevId.compare(devID)) {
			SessionId sessionID;
			std::string strAppname = appInfo.appName;
			if(ajNameSerPtr->GetNSInfo(strAppname,sessionID)) {
				std::string command = CC_TELEMETRYDATA;
				if(methoExePtr) {
					LOGDBUG2("(SendToAppData) Sending Telemetry Data to App [APPNAME]",appInfo.appName);
					signalExePtr->signalCaller(strAppname, sessionID, MC_CALLBACKLISTENER, command, payload);
				} else {
					LOGEROR("(SendToAppData) Invalid Signal Executer Ptr !!");
				}
			} else {
				LOGWARN("(SendToAppData) Unable to Send App !!");
			}
		}
	}
	return ;
}

/*Get MethodExecuter ptr.*/
methodExecuter* ServiceProvider::GetMethodExecuterPtr()
{
	return methoExePtr ? methoExePtr : NULL;
}

/*Get SignalExecuter ptr.*/
signalExecuter* ServiceProvider::GetSignalExecuterPtr()
{
	return signalExePtr ? signalExePtr : NULL;
}

/*Get DeviceObj ptr.*/
DeviceObj* ServiceProvider::GetDeviceObjPtr()
{
	return deviceObjPtr ? deviceObjPtr : NULL;
}

/*Will Update ajNameService ptr. by DMAController*/
void ServiceProvider::updateAJNameServicePtr(ajNameService* _ajNameSerPtr)
{
	if(_ajNameSerPtr)
		ajNameSerPtr = _ajNameSerPtr;
	else
		LOGEROR("(updateAJNameServicePtr) Unable to update ajNameService ptr !!");
}

void ServiceProvider::CloudConnectionStatus(bool Status)
{
	isCloudConnected = Status;
}

std::string ServiceProvider::GetTopicID(eITOPIC eItopic)
{
	std::string strTopic = "EI/";
	strTopic += configurationSettings::orgId;
	strTopic += "/";
	strTopic += configurationSettings::gatewayId;
	strTopic +=	"/";
	strTopic += configurationSettings::AppTopic;
	if(eItopic == eITOPIC::EI_TOPIC_DEV){
		strTopic += "/Devices/";
	} else if(eItopic == eITOPIC::EI_TOPIC_HASH) {
		strTopic += "/#";
	} else if(eItopic == eITOPIC::EI_TOPIC_DATA) {
		strTopic  = "EI/Data/";
		strTopic += configurationSettings::orgId;
		strTopic += "/";
		strTopic += configurationSettings::gatewayId;
		strTopic +=	"/";
		strTopic += configurationSettings::AppTopic;
		strTopic += "/Devices/";
	}
	LOGINFO2("(GetTopicID) Topic ID ",strTopic);
	return strTopic;
}

std::string ServiceProvider::unescape(const std::string& s)
{
	std::string res;
	std::string::const_iterator it = s.begin();
	while (it != s.end())
	{
		char c = *it++;
		if (c == '\\' && it != s.end())
		{
			switch (*it++) {
			case '\\': c = '\\'; break;       // Back Slash
			case 'n': c = '\n'; break;        // New Line
			case 't': c = '\t'; break;        // Tab
			case '"': c = '\"'; break;        // Double Quotes
			case 'b': c = '\b'; break;        // Backspace
			case 'r': c = '\r'; break;        // Carriage Return
			// all other escapes
			default:
				// invalid escape sequence - skip it. alternatively you can copy it as is, throw an exception...
				continue;
			}
		} else if (c == ' '){
			continue;
		}
		res += c;
	}
	return res;
}

void ServiceProvider::isDeviceExists(std::string command, std::string& payload, std::string mcCommand)
{
	LOGDBUG("In ServiceProvider isDeviceExists .. .. ");
	if(!command.compare(DOC_DEVICEEXISTS)) {
		QStatus status = ER_FAIL;
		std::string strEndpointId;
		LOGDBUG2("(isDeviceExists) Payload [PAYLOAD]",payload);
		status = deviceObjPtr->DeviceExists(payload,strEndpointId);
		if(status == ER_OK) {
			LOGINFO2("(isDeviceExists) Reply Payload [PAYLOAD]",payload);
			if(!command.compare(MC_CALLBACKLISTENER)) {
				LOGDBUG("(isDeviceExists) Sending Signal .. .. ");
				SessionId sessionID;
				if(ajNameSerPtr->GetNSInfo(strEndpointId,sessionID)) {
					if(methoExePtr) {
						status = signalExePtr->signalCaller(strEndpointId, sessionID, MC_CALLBACKLISTENER, command, payload);
					} else {
						LOGEROR("(isDeviceExists) Invalid Signal Executer Ptr !!");
					}
				} else {
					LOGEROR("(isDeviceExists) Invalid AjName Service Ptr !!");
				}
			}
		} else {
			LOGINFO("(isDeviceExists) Device Not Exits.. ..");
		}
	}
	return ;
}

/*Will send Data to Register App.*/
std::string  ServiceProvider::processTopicString(std::string Command)
{
	std::string::size_type strFirstPosition  = 0;
	std::string::size_type strSecondPosition = 0;
	std::string strFirstProStr = Command;
	std::string strSecondProStr;
	strFirstPosition = Command.find("/");
	strSecondProStr = Command.substr(strFirstPosition + 1);
	strFirstProStr  = strSecondProStr;
	strSecondPosition = strFirstProStr.find("/");
	strFirstProStr = strFirstProStr.substr(strSecondPosition + 1);
	strFirstProStr = strSecondProStr.erase(strSecondPosition);
	return strFirstProStr;
}


QStatus ServiceProvider::getAllRaml(std::string payload, std::string commandPayload)
{
	QStatus status = ER_OK;
	json jsonRaml = json::parse(payload);
	std::string jobID = jsonRaml[PP_JOBID];
	std::string resCloud;
	json jsonData;

	try{

		for (auto It : jsonRaml[PP_DEVICES])
		{
			json devJson = It;
			std::string deviceID = devJson[PP_DEVID];
			devJson[PP_JOBID] = jobID;

			std::string devString = devJson.dump();
			deviceObjPtr->GetAllRaml(deviceID, devString);
			jsonData = json::parse(commandPayload);
			jsonData[PP_MSG] =  devString;
			jsonData[PP_APPNAME] = ajNameSerPtr->getAdvertisedName();
			jsonData[PP_RESPONSECODE] = RC_SUCCESS;

			std::string topic = jsonData[PP_TOPIC];

			std::string replaceStr = DTP_GETALLRAML;
			std::size_t repStr = topic.find(replaceStr);

			std::string responseTopicPrefix = "Devices/" + deviceID + DTP_RAML_GET1;

			topic.replace(repStr,replaceStr.size(),responseTopicPrefix);

			jsonData[PP_TOPIC] = topic;
			resCloud = jsonData.dump();

			std::string busName = cloudEpInfo.cloudAppName;
			SessionId sessionID;
			if(isCloudConnected && ajNameSerPtr->GetNSInfo(busName,sessionID)) {

				std::string command = CC_SENDTOCLOUD;

				if(methoExePtr) {
					if(ER_OK == methoExePtr->methodCaller(busName, sessionID, "SyncListener", command, resCloud))
						std::cout << "successfully sended to mqttApp" << std::endl ;
				} else {
					LOGEROR("(ServiceProvider) Invalid Method Executer Ptr !!");
					//break ;
				}
				usleep(500);
			} else {
				LOGEROR("(ServiceProvider) Unable to Send Cloud !!");
			}

		}
	}
	catch(...)
	{
		status = ER_FAIL;
	}

	return status;
}

QStatus ServiceProvider::processTopic(std::string &command, std::string &payload)
{
	LOGDBUG("In ServiceProvider processTopic .. .. ");
	LOGDBUG2("(processTopic) Got Command ",command);
	QStatus status = ER_FAIL;
	std::string commandBackup  = command;
	std::string commandPayload = payload;
	std::string strDevId;
	json jsonData;
	try  {
		jsonData = json::parse(payload);
		if(jsonChecker::checkJson(jsonData,{PP_MSG}))
		{
			std::string strTemp = jsonData[PP_MSG];
			json jsonData_ = json::parse(strTemp);
			if(jsonChecker::checkJson(jsonData,{PP_TOPIC}))
			{
				jsonData_[PP_TOPIC] = jsonData[PP_TOPIC];
				strTemp = jsonData_.dump();
			} else {
				LOGWARN("(processTopic) Unable to Find Topic !!");
			}
			payload = strTemp;
			status = ER_OK;
			LOGDBUG2("(processTopic) Extract PayLoad [payload]", payload);
		} else {
			json jsonTem;
			jsonTem[PP_RESPONSECODE] = RC_FAIL;
			jsonTem[PP_MESSAGE] = "MSG Field Not Found !!";
			std::string strTemp = jsonTem.dump();
			jsonData[PP_MSG] = strTemp;
			LOGEROR("(processTopic) Failed To Get MSG Field in Payload.. ..");
			status = ER_FAIL;
		}
	} catch (std::exception &e) {
		LOGEROR2("(processTopic) Failed to process response Json !!!", e.what());
		status = ER_FAIL;
	}
	if(ER_OK == status) {
		try {
			//::TODO - Need to change std::find logic.
			if((!command.compare(DTP_DEV_GETDEVICELIST))) {
				command  = DOC_GETDEVICELIST;
				status = ER_OK;
				LOGDBUG2("(processTopic) Got Devices/GetDeviceList Command [strDevId]", strDevId);
			} else if((command.find(DTP_DEV_GET) != std::string::npos)) {
				if(!this->processTopicString(command).compare(configurationSettings::transportmask))
				{
					if(jsonChecker::checkJson(jsonData,{PP_TOPIC}))
					{
						command = DOC_STARTDISCOVERY;
						discovryTopic = jsonData[PP_TOPIC];
						status = ER_OK;
					} else {
						LOGWARN("(processTopic) Unable to Find Topic !!");
						status = ER_OK;
					}
				} else {
					status = ER_FAIL;
				}
			} else if((command.find(DTP_DEV_PUT) != std::string::npos)) {
				if(isPTLload) {
					if(!this->processTopicString(command).compare(configurationSettings::transportmask)) {
						json jsonData;
						try  {
							jsonData = json::parse(payload);//::TODO
							std::string strDevID = jsonData[PP_DEVICES][0][PP_DEVID];
							std::string strMacId = jsonData[PP_DEVICES][0][PP_MACID];
							jsonData["macid"] = strMacId;
							strDevId = strDevID;
							payload = jsonData.dump();
							command = DOC_ADDDEVICE;
							status = ER_OK;
						} catch (...) {
							LOGEROR("(processTopic) Failed to process response Json !!!");
							status = ER_FAIL;
						}
					} else {
						status = ER_FAIL;
						LOGEROR("(processTopic) Transport Mask Not Match !!");
					}
				} else {
					LOGEROR("(processTopic) Protocol Not Loaded !!");
					status = ER_FAIL;
				}
			} else if((command.find(DTP_STATUS_GET) != std::string::npos)) {
				strDevId = this->processTopicString(command);
				command = DOC_GETDEVICESTATUS;
				status = ER_OK;
				LOGDBUG2("(processTopic) Got Status/Get Command [strDevId]", strDevId);
			} else if((command.find(DTP_STATUS_PUT) != std::string::npos)) {
				strDevId = this->processTopicString(command);
				command = DOC_SETDEVICESTATUS;
				LOGDBUG2("(processTopic) Got Status/Put Command [strDevId]", strDevId);
				status = ER_OK;
			} else if((command.find(DTP_STATE_GET) != std::string::npos)) {
				strDevId = this->processTopicString(command);
				command = DOC_GETDEVICESTATE;
				status = ER_OK;
				LOGDBUG2("(processTopic) Got State/Get Command [strDevId]", strDevId);
			} else if((command.find(DTP_STATE_PUT) != std::string::npos)) {
				strDevId = this->processTopicString(command);
				command = DOC_SETDEVICESTATE;
				status = ER_OK;
				LOGDBUG2("(processTopic) Got State/Put Command [strDevId]", strDevId);
			} else if((command.find(DTP_DEV_DEL) != std::string::npos)) {
				json jsonData;
				try  {
					jsonData = json::parse(payload);
					std::string strDevID = jsonData[PP_DEVICES][0][PP_DEVID]; //::TODO need to remove.
					std::string strMacId = jsonData[PP_DEVICES][0][PP_MACID]; //::TODO need to remove.
					jsonData[PP_MACID] = strMacId;							  //::TODO need to remove.
					strDevId = strDevID;
					payload = jsonData.dump();
					command = DOC_DELETEDEVICE;
					status = ER_OK;
				} catch (...) {
					LOGEROR("(processTopic) Failed to process response Json !!");
					status = ER_FAIL;
				}
			} else if((command.find(DTP_RAML_GET1) != std::string::npos)) {
				std::cout << "FOUNFFNFNFNFNF" << std::endl;
				strDevId  = this->processTopicString(command);
				command  = DOC_GETDEVICERAML;
				status = ER_OK;
				LOGDBUG2("(processTopic) Got RAML/Get Command [strDevId]", strDevId);
			} else if((command.find(DTP_RAML_PUT) != std::string::npos)) {
				strDevId  = this->processTopicString(command);
				command  = DOC_SETDEVICERAML;
				status = ER_OK;
				LOGDBUG2("(processTopic) Got RAML/Put Command [strDevId]", strDevId);
			} else if((command.find(DTP_XML_GET) != std::string::npos)) {
				strDevId  = this->processTopicString(command);
				command  = DOC_GETDEVICEXML;
				status = ER_OK;
				LOGDBUG2("(processTopic) Got XML/Get Command [strDevId]", strDevId);
			} else if((command.find(DTP_XML_PUT) != std::string::npos)) {
				strDevId  = this->processTopicString(command);
				command  = DOC_SETDEVICEXML;
				status = ER_OK;
				LOGDBUG2("(processTopic) Got XML/Put Command [strDevId]", strDevId);
			} else if((command.find(DTP_SCCONFIG) != std::string::npos)) {
				//EI/<orgid>/<gwid>/<appid>/Devices/<deviceid>/SCconfig
				strDevId  = this->processTopicString(command);
				command  = DOC_SCCONFIG;
				status = ER_OK;
				LOGDBUG2("(processTopic) Got SCconfig Command [strDevId]", strDevId);
			}else if((command.find(DTP_BASEURL_UPDATE) != std::string::npos)){

				strDevId = "";
				command = DOC_UPDATEBASEURL;
				status = ER_OK;
			}else if((command.find(DTP_BASEURL_TEST) != std::string::npos)){

				strDevId = "";
				command = DOC_TESTBASEURL;
				status = ER_OK;
			}else if((command.find(DTP_ADD_CC) != std::string::npos)){

				strDevId = "";
				command = DOC_ADDCC;
				status = ER_OK;
			}else if((command.find(DTP_UPDATE_CC) != std::string::npos)){

				strDevId = "";
				command = DOC_UPDATECC;
				status = ER_OK;
			}else if((command.find(DTP_DELETE_CC) != std::string::npos)){

				strDevId = "";
				command = DOC_DELETECC;
				status = ER_OK;
			}


			else if((command.find(DTP_ADD_LC) != std::string::npos)){

				strDevId = "";
				command = DOC_ADDLC;
				status = ER_OK;
			}else if((command.find(DTP_UPDATE_LC) != std::string::npos)){

				strDevId = "";
				command = DOC_UPDATELC;
				status = ER_OK;
			}else if((command.find(DTP_DELETE_LC) != std::string::npos)){

				strDevId = "";
				command = DOC_DELETELC;
				status = ER_OK;
			}

			else if((command.find(DTP_TEST_CC) != std::string::npos)){

				strDevId = "";
				command = DOC_TESTCC;
				status = ER_OK;
			}
			else if((command.find(DTP_STATUSCHANGE_CC) != std::string::npos)){

				strDevId = "";
				command = DOC_STATUSCHANGE;
				status = ER_OK;
			}
			else if((command.find(DTP_GETALLRAML) != std::string::npos)){

				command = DOC_GETALLRAML;

				status = this->getAllRaml(payload, commandPayload);
				status = ER_OK;
				payload = "";
				return status;
			}
			else
			{
				status = ER_FAIL;
			}

		} catch(std::exception &e){
			LOGEROR("(processTopic) Failed to process topic !!");
			status = ER_FAIL;
		}
	}

	if(status == ER_OK)
	{
		//::TODO need to Call Serve_request()
		this->serve_request(strDevId,command,payload);
		LOGDBUG3("(processTopic) Return of serve_request [COMMAND PAYLOAD]",command,payload);
		//json jsonData;
		try  {
			jsonData = json::parse(commandPayload);
			jsonData[PP_MSG] =  payload;
			jsonData[PP_APPNAME] = ajNameSerPtr->getAdvertisedName();
			jsonData[PP_RESPONSECODE] = RC_SUCCESS;
			payload = jsonData.dump();
		} catch (...) {
			LOGEROR("(processTopic) Failed to process response Json !!!");
			return status;
		}

	} else {
		try  {
			json tempJson = json::parse(payload);
			tempJson[PP_MSG] = "Unable To Process Command !!";
			tempJson[PP_RESPONSECODE] = 400;
			payload = tempJson.dump();

			jsonData = json::parse(commandPayload);
			jsonData[PP_MSG] =  payload;
			jsonData[PP_APPNAME] = ajNameSerPtr->getAdvertisedName();
			jsonData[PP_RESPONSECODE] = RC_FAIL;
			payload = jsonData.dump();
		} catch (...) {
			LOGEROR("(processTopic) Failed to process response Json !!!");
			return status;
		}
	}

	command = commandBackup;
	LOGDBUG3("(processTopic) reply [COMMAND PAYLOAD]",command, payload);
	LOGDBUG2("(processTopic) reply [DEVICEID]",strDevId);
	return status;
}

QStatus ServiceProvider::GetDevIDFromJson(std::string &devId, std::string payload)
{
	QStatus status = ER_FAIL;
	json jsonData;
	try {
		jsonData = json::parse(payload);
		if(jsonChecker::checkJson(jsonData,{PP_DEVICEID})){
			devId = jsonData[PP_DEVICEID];
			status = ER_OK;
		} else {
			status = ER_FAIL;
		}
	} catch(std::exception &e) {
		status = ER_FAIL;
		LOGEROR2("(processTopic) Failed to Find DevID in Json !!", e.what());
	}
	return status;
}

QStatus ServiceProvider::processCommand(std::string &command, std::string &payload)
{
	QStatus status = ER_FAIL;
	LOGDBUG("In ServiceProvider processCommand .. .. ");
	LOGDBUG2("(processCommand) Got Command ",command);
	std::string commandBackup  = command;
	std::string commandPayload = payload;
	std::string strDevId;

	json jsonData;

	try  {
		jsonData = json::parse(payload);
		std::cout << "INFO : jsonData " << jsonData.dump() << std::endl;
		if(jsonChecker::checkJson(jsonData,{PP_APPNAME}))
		{
			status = ER_OK;
			LOGDBUG2("(processCommand) Extract PayLoad [payload]", payload);
		} else {
			json jsonTem;
			jsonTem[PP_RESPONSECODE] = RC_FAIL;
			jsonTem[PP_MESSAGE] = "App Name Field Not Found !!";
			std::string strTemp = jsonTem.dump();
			jsonData[PP_MSG] = strTemp;
			LOGEROR("(processCommand) Failed To Get App Name Field in Payload.. ..");
			status = ER_FAIL;
		}
	} catch (std::exception &e) {
		LOGEROR2("(processTopic) Failed to process response Json !!!", e.what());
		status = ER_FAIL;
	}

	if(ER_OK == status) {
		status = ER_FAIL;
		if(!(command.compare(DOC_GETDEVICESTATE))) {
			status = ER_OK;
			command = DOC_GETDEVICESTATE;
			LOGDBUG("(processCommand) Got GetDeviceState Command.");
		} else if(!(command.compare(DOC_SETDEVICESTATE))) {
			status = ER_OK;
			command = DOC_SETDEVICESTATE;
			LOGDBUG("(processCommand) Got SetDeviceState Command.");
		} else if(!(command.compare(DOC_GETDEVICELIST))) {
			status = ER_OK;
			command  = DOC_GETDEVICELIST;
			LOGDBUG("(processCommand) Got GetDeviceList Command.");
		} else {
			LOGEROR("(processCommand) Failed to process topic !!");
			json jsonData;
			try  {
				jsonData = json::parse(payload);
				jsonData[PP_RESPONSECODE] = RC_FAIL;
				jsonData[PP_MSG] = "Failed to process Topic/Command !!";
				std::string payloadData = jsonData.dump();
				payload = payloadData;
			} catch (...) {
				LOGEROR("(processCommand) Failed to process response Json !!");
			}
			command = commandBackup;
			status = ER_FAIL;
		}
	}

	/*If Set/GetState need to fetch device ID.*/
	if((command.compare(DOC_GETDEVICELIST))) {
		if(ER_OK == GetDevIDFromJson(strDevId,payload)) {
			status = ER_OK;
		} else {
			status = ER_FAIL;
		}
	}

	if(status == ER_OK)
	{
		this->serve_request(strDevId,command,payload);
		LOGDBUG3("(processCommand) Return of serve_request [COMMAND PAYLOAD]",command,payload);
		//json jsonData;
		try  {
			jsonData = json::parse(commandPayload);
			jsonData[PP_MSG] =  payload;
			jsonData[PP_APPNAME] = ajNameSerPtr->getAdvertisedName();
			jsonData[PP_RESPONSECODE] = RC_SUCCESS;
			payload = jsonData.dump();
		} catch (...) {
			LOGEROR("(processCommand) Failed to process response Json !!!");
			return status;
		}

	} else {
		try  {
			jsonData = json::parse(commandPayload);
			jsonData[PP_MSG] =  "Unable To Process Command !!";
			jsonData[PP_APPNAME] = ajNameSerPtr->getAdvertisedName();
			jsonData[PP_RESPONSECODE] = RC_FAIL;
			payload = jsonData.dump();
		} catch (...) {
			LOGEROR("(processCommand) Failed to process response Json !!!");
			return status;
		}
	}

	command = commandBackup;
	LOGDBUG3("(processCommand) reply [COMMAND PAYLOAD]",command, payload);
	LOGDBUG2("(processCommand) reply [DEVICEID]",strDevId);
	return status;
}

QStatus ServiceProvider::serve_request(std::string DevId, std::string &command, std::string &payload)
{
	LOGDBUG("In ServiceProvider serve_request .. .. ");
	LOGDBUG3("(serve_request) Got Command [COMMAND PAYLOAD]",command,payload);
	QStatus status = ER_OK;
	if((!command.compare(DOC_STARTDISCOVERY)) || (command.find(DOC_STARTDISCOVERY) != std::string::npos)) {
		deviceObjPtr->StartDiscovery(DevId,payload);
		isPTLload = true;
		json jsonData;
		try  {
			jsonData = json::parse(payload);
			jsonData[PP_RESPONSECODE] = RC_SUCCESS;
			jsonData[PP_STATUS] = "Discovery Inprogress.";
			payload = jsonData.dump();
			LOGDBUG2("(serve_request) DISCOVRY IN PROGRESS [PAYLOAD]",payload);
		} catch (...) {
			LOGEROR("(serve_request) Failed to process response Json !!");
		}
	}
	else if((!command.compare(DOC_STOPDISCOVERY)) || (command.find(DOC_STOPDISCOVERY) != std::string::npos)) {
		deviceObjPtr->StopDiscovery(DevId,payload);
	}
	else if((!command.compare(DOC_ADDDEVICE)) || (command.find(DOC_ADDDEVICE) != std::string::npos)) {
		deviceObjPtr->AddDevice(DevId,payload);
		std::string dummyDevID = DevId;
		std::string dummyPayload = payload;
		deviceObjPtr->RegisterDeviceCallback(dummyDevID,dummyPayload);
		//		LOGINFO2("(serve_request) RegisterDeviceCallback [dummyPayload]",dummyPayload);
		//		json jsonData;
		//		try  {
		//			jsonData = json::parse(payload);
		//			if(jsonChecker::checkJson(jsonData,{PP_RESPONSECODE})) {
		//				if(jsonData[PP_RESPONSECODE] == RC_SUCCESS)	{
		//					LOGINFO("(serve_request) Successfully Added Device Moving To Get RAML .. ..");
		//					if(callbackThread)
		//						delete callbackThread;
		//					callbackThread = new AsynTaskThread(*this, *deviceObjPtr);
		//					callbackThread->setDevID(DevId);
		//					callbackThread->setPayload(payload);
		//					if(ER_OK == callbackThread->Start())
		//					{
		//						LOGINFO("(findAdvertise) Set RAML Thread Started .. .. ");
		//					} else {
		//						LOGEROR("(findAdvertise) Unable to Create Set RAML !!");
		//					}
		//				}
		//			}
		//		} catch (...) {
		//			LOGEROR("(serve_request) Failed to process response Json !!");
		//		}
	}
	else if((!command.compare(DOC_DELETEDEVICE)) || (command.find(DOC_DELETEDEVICE) != std::string::npos)) {
		deviceObjPtr->DeleteDevice(DevId,payload);
	}
	else if((!command.compare(DOC_GETDEVICERAML)) || (command.find(DOC_GETDEVICERAML) != std::string::npos)) {
		deviceObjPtr->GetDeviceRAML(DevId,payload);

		//NEED TO BE CHANGED....
		//~~~~~~~~~~~~~~~~~~~~~~~~`
		json jsonData;
		try  {
			jsonData = json::parse(payload);
			if(jsonChecker::checkJson(jsonData,{PP_RESPONSECODE}))
			{
				if(jsonData[PP_RESPONSECODE] == RC_FAIL)	{ //::TODO
					LOGEROR("(serve_request) Failed To Get RAML, Moving To Get XML .. ..");
					deviceObjPtr->GetDeviceXML(DevId,payload);

					jsonData = json::parse(payload);
					std::cout << "payload response is " << payload << std::endl;
					std::string topic = jsonData[PP_TOPIC];

					std::string replaceStr = DTP_RAML_GET;
					std::size_t repStr = topic.find(replaceStr);

					std::string responseTopicPrefix = DTP_XML_GET;

					topic.replace(repStr,replaceStr.size(),responseTopicPrefix);

					jsonData[PP_TOPIC] = topic;

					payload = jsonData.dump();

					std::cout << "payload response is " << payload << std::endl;

					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

					json jsonData_;
					jsonData_[PP_MSG] = payload;
					jsonData_[PP_APPNAME] = ajNameSerPtr->getAdvertisedName();
					jsonData_[PP_RESPONSECODE] = RC_SUCCESS;
					jsonData_[PP_ENDPOINTID] = cloudEpInfo.iCloudEndpoint;
					jsonData_[PP_TOPIC] = topic;
					payload = jsonData_.dump();
					LOGDBUG2("(serve_request) Send To Cloud Data [PAYLOAD]",payload);

					std::string busName = cloudEpInfo.cloudAppName;
					SessionId sessionID;
					if(isCloudConnected && ajNameSerPtr->GetNSInfo(busName,sessionID)) {
						int tryCout = 0;
						std::string command = CC_SENDTOCLOUD;
						while(tryCout < COULD_CONNECTION_TRY) {
							tryCout++;
							if(methoExePtr) {
								if(ER_OK == methoExePtr->methodCaller(busName, sessionID, "SyncListener", command, payload))
									break ;
							} else {
								LOGEROR("(ServiceProvider) Invalid Method Executer Ptr !!");
								break ;
							}
							usleep(500);
						}
					}
					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

				}
			} else {
				LOGEROR("(serve_request) Unable to Get Response Code !!");
			}
		} catch (...) {
			LOGEROR("(serve_request) Failed to process response Json !!");
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	}
	else if((!command.compare(DOC_SETDEVICERAML)) || (command.find(DOC_SETDEVICERAML) != std::string::npos)) {
		deviceObjPtr->SetDeviceRAML(DevId,payload);
	}
	else if((!command.compare(DOC_GETDEVICEXML)) || (command.find(DOC_GETDEVICEXML) != std::string::npos)) {
		deviceObjPtr->GetDeviceXML(DevId,payload);
	}
	else if((!command.compare(DOC_SETDEVICEXML)) || (command.find(DOC_SETDEVICEXML) != std::string::npos)) {
		deviceObjPtr->SetDeviceXML(DevId,payload);
	}
	else if((!command.compare(DOC_GETDEVICESTATE)) || (command.find(DOC_GETDEVICESTATE) != std::string::npos)) {
		deviceObjPtr->GetDeviceState(DevId,payload);
	}
	else if((!command.compare(DOC_SETDEVICESTATE)) || (command.find(DOC_SETDEVICESTATE) != std::string::npos)) {
		deviceObjPtr->SetDeviceState(DevId,payload);
	}
	else if((!command.compare(DOC_GETDEVICESTATUS)) || (command.find(DOC_GETDEVICESTATUS) != std::string::npos)) {
		deviceObjPtr->GetDeviceStatus(DevId,payload);
	}
	else if((!command.compare(DOC_SETDEVICESTATUS)) || (command.find(DOC_SETDEVICESTATUS) != std::string::npos)) {
		deviceObjPtr->SetDeviceStatus(DevId,payload);
	}
	else if((!command.compare(DOC_REGISTERDEVICEPROPERTIES)) || (command.find(DOC_REGISTERDEVICEPROPERTIES) != std::string::npos)) {
		deviceObjPtr->RegisterDeviceProperties(DevId,payload);
	}
	else if((!command.compare(DOC_DEREGISTERDEVICEPROPERTIES)) || (command.find(DOC_DEREGISTERDEVICEPROPERTIES) != std::string::npos)) {
		deviceObjPtr->DeregisterDeviceProperties(DevId,payload);
	}
	else if((!command.compare(DOC_GETDEVICELIST)) || (command.find(DOC_GETDEVICELIST) != std::string::npos)) {
		deviceObjPtr->GetDeviceList(DevId,payload);
	}
	else if((!command.compare(DOC_REGISTERDEVICECALLBACK)) || (command.find(DOC_REGISTERDEVICECALLBACK) != std::string::npos)) {
		deviceObjPtr->RegisterDeviceCallback(DevId,payload);
	}
	else if((!command.compare(DOC_DEREGISTERDEVICECALLBACK)) || (command.find(DOC_DEREGISTERDEVICECALLBACK) != std::string::npos)) {
		deviceObjPtr->DeregisterDeviceCallback(DevId,payload);
	}
	else if((!command.compare(DOC_SCCONFIG)) || (command.find(DOC_SCCONFIG) != std::string::npos)) {
		deviceObjPtr->SCconfig(command,payload);
	}
	else if((!command.compare(DOC_STATUSCHANGE)) || (command.find(DOC_STATUSCHANGE) != std::string::npos)) {
		deviceObjPtr->StatusChange(command,payload);
	}
	else if((!command.compare(DOC_UPDATEBASEURL)) || (command.find(DOC_UPDATEBASEURL) != std::string::npos)) {
		manageConfig->updateAppbaseUrl(payload);
	}
	else if((!command.compare(DOC_TESTBASEURL)) || (command.find(DOC_TESTBASEURL) != std::string::npos)) {
		manageConfig->testAppbaseUrl(payload);
	}
	else if((!command.compare(DOC_ADDCC)) || (command.find(DOC_ADDCC) != std::string::npos)) {
		manageConfig->addCloudConnection(payload);
	}
	else if((!command.compare(DOC_UPDATECC)) || (command.find(DOC_UPDATECC) != std::string::npos)) {
		manageConfig->updateCloudConnection(payload);
	}
	else if((!command.compare(DOC_DELETECC)) || (command.find(DOC_DELETECC) != std::string::npos)) {
		manageConfig->deleteCloudConnection(payload);
	}


	else if((!command.compare(DOC_ADDLC)) || (command.find(DOC_ADDLC) != std::string::npos)) {
		manageConfig->addLocalConnection(payload);
	}
	else if((!command.compare(DOC_UPDATELC)) || (command.find(DOC_UPDATELC) != std::string::npos)) {
		manageConfig->updateLocalConnection(payload);
	}
	else if((!command.compare(DOC_DELETELC)) || (command.find(DOC_DELETELC) != std::string::npos)) {
		manageConfig->deleteLocalConnection(payload);
	}

	else if((!command.compare(DOC_TESTCC)) || (command.find(DOC_TESTCC) != std::string::npos)) {
		manageConfig->testCloudConnection(payload);
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//else if((!command.compare(DTP_GETALLRAML)) || (command.find(DTP_GETALLRAML) != std::string::npos)) {


	//	deviceObjPtr->SCconfig(command,payload);
	//}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//::TODO Need to Handle Error Case. Check Response Code 400/200.
	return status;
}

void ServiceProvider::FunctionTest()
{
	TransportMask transportMast = TRANSPORT_EI_ZWAVE;
	std::string strDevId = "1234567";
	std::string strPayload = R"({"JOBID":"1234","CLIENTID":"1234"})";;
	LOGDBUG2I("-----------------------------------------------------StartDiscovery - START",transportMast);
	deviceObjPtr->StartDiscovery(strDevId,strPayload);
	LOGDBUG("-------------------------------------------------------StartDiscovery - END");

	sleep(2);

	LOGDBUG2I("-----------------------------------------------------RegisterDeviceCallback - START",transportMast);
	strPayload = R"({"status":"active","JOBID":"1234","CLIENTID":"1234"})";
	deviceObjPtr->RegisterDeviceCallback(strDevId,strPayload);
	LOGINFO3("----------[strDevId : strPayload]",strDevId,strPayload);
	LOGDBUG("-------------------------------------------------------RegisterDeviceCallback - END");

	while(!isDiscovryDone){
		LOGDBUG("-------------------------------------------------------IN LOOP");
		sleep(2);
	}

	LOGDBUG2I("-----------------------------------------------------AddDevice - START",transportMast);
	json relayswitch = {
			{"JOBID","1234"},
			{"CLIENTID","1234"},
			{"macid","010f0801100112"}
	};
	strPayload = relayswitch.dump();
	deviceObjPtr->AddDevice(strDevId,strPayload);
	LOGINFO3("----------[strDevId : strPayload]",strDevId,strPayload);
	LOGDBUG("-------------------------------------------------------AddDevice - END");

	sleep(2);

	LOGDBUG2I("-----------------------------------------------------SetDeviceStatus - START",transportMast);
	strPayload = R"({"status":"active","JOBID":"1234","CLIENTID":"1234"})";
	deviceObjPtr->SetDeviceStatus(strDevId,strPayload);
	LOGINFO3("----------[strDevId : strPayload]",strDevId,strPayload);
	LOGDBUG("-------------------------------------------------------SetDeviceStatus - END");

	sleep(2);

	LOGDBUG2I("-----------------------------------------------------GetDeviceStatus - START",transportMast);
	strPayload = R"({"status":"active","JOBID":"1234","CLIENTID":"1234"})";
	deviceObjPtr->GetDeviceStatus(strDevId,strPayload);
	LOGINFO3("----------[strDevId : strPayload]",strDevId,strPayload);
	LOGDBUG("-------------------------------------------------------GetDeviceStatus - END");

	sleep(2);

	LOGDBUG2I("-----------------------------------------------------GetDeviceRAML - START",transportMast);
	strPayload = R"({"status":"active","JOBID":"1234","CLIENTID":"1234"})";
	deviceObjPtr->GetDeviceRAML(strDevId,strPayload);
	LOGINFO3("----------[strDevId : strPayload]",strDevId,strPayload);
	LOGDBUG("-------------------------------------------------------GetDeviceRAML - END");

	sleep(2);

	LOGDBUG2I("-----------------------------------------------------SetDeviceRAML - START",transportMast);
	//strPayload = R"({"status":"active","JOBID":"1234","CLIENTID":"1234"})";
	deviceObjPtr->SetDeviceRAML(strDevId,strPayload);
	LOGINFO3("----------[strDevId : strPayload]",strDevId,strPayload);
	LOGDBUG("-------------------------------------------------------SetDeviceRAML - END");

	sleep(2);

	LOGDBUG2I("-----------------------------------------------------GetDeviceState - START",transportMast);
	strPayload = R"({"CLIENTID":"1234","JOBID":"1234","properties":[{"property":"Temperature"},{"property":"Luminance"}]})";
	deviceObjPtr->GetDeviceState(strDevId,strPayload);
	LOGINFO3("----------[strDevId : strPayload]",strDevId,strPayload);
	LOGDBUG("-------------------------------------------------------GetDeviceState - END");

	sleep(2);

	LOGDBUG2I("-----------------------------------------------------SetDeviceState - START",transportMast);
	strPayload = R"({"CLIENTID":"1234","JOBID":"1234","properties":[{"Temperature":27.8}],"subproperties":{"config":[{"Temperature reports interval":10},{"Tamper operating modes":"Tamper"},{"LED signaling mode":"Long blink White."}]}})";
	deviceObjPtr->SetDeviceState(strDevId,strPayload);
	LOGINFO3("----------[strDevId : strPayload]",strDevId,strPayload);
	LOGDBUG("-------------------------------------------------------SetDeviceState - END");

	sleep(2);

	LOGDBUG2I("-----------------------------------------------------GetDeviceXML - START",transportMast);
	strPayload = R"({"CLIENTID":"1234","JOBID":"1234"})";
	deviceObjPtr->GetDeviceXML(strDevId,strPayload);
	LOGINFO3("----------[strDevId : strPayload]",strDevId,strPayload);
	LOGDBUG("-------------------------------------------------------GetDeviceXML - END");

	sleep(2);

	LOGDBUG2I("-----------------------------------------------------SetDeviceXML - START",transportMast);
	//strPayload = R"({"CLIENTID":"1234","JOBID":"1234","properties":[{"Temperature":27.8}],"subproperties":{"config":[{"Temperature reports interval":10},{"Tamper operating modes":"Tamper"},{"LED signaling mode":"Long blink White."}]}})";
	deviceObjPtr->SetDeviceXML(strDevId,strPayload);
	LOGINFO3("----------[strDevId : strPayload]",strDevId,strPayload);
	LOGDBUG("-------------------------------------------------------SetDeviceXML - END");
}

qcc::ThreadReturn /*STDCALL*/ ServiceProvider::AsynTaskThread::Run(void* arg)
{
	QCC_UNUSED(arg);
	LOGINFO("(ServiceProvider::processAsynTaskThread) Thread - Start.");
	std::string strTopic = sproPtr.GetTopicID(eITOPIC::EI_TOPIC_DEV);
	bool isRAML = true;
	deviceObjPtr.GetDeviceRAML(devId,payload);
	json jsonData;
	try  {
		jsonData = json::parse(payload);
		if(jsonChecker::checkJson(jsonData,{PP_RESPONSECODE}))
		{
			if(jsonData[PP_RESPONSECODE] == RC_FAIL)	{ //::TODO
				LOGEROR("(serve_request) Failed To Get RAML, Moving To Get XML .. ..");
				deviceObjPtr.GetDeviceXML(devId,payload);
				strTopic +=	devId;
				strTopic += DTP_XML_GET1;
				isRAML = false;
			} else {
				strTopic +=	devId;
				strTopic += DTP_RAML_GET1;
			}
		} else {
			LOGEROR("(serve_request) Unable to Get Response Code !!");
		}
	} catch (...) {
		LOGEROR("(serve_request) Failed to process response Json !!");
	}
	LOGINFO2("(GetTopicID) Topic ID ",strTopic);
	try  {
		json jsonData_;
		jsonData_[PP_MSG] = payload;
		jsonData_[PP_APPNAME] = sproPtr.ajNameSerPtr->getAdvertisedName();
		jsonData_[PP_RESPONSECODE] = RC_SUCCESS;
		jsonData_[PP_ENDPOINTID] = sproPtr.cloudEpInfo.iCloudEndpoint;
		jsonData_[PP_TOPIC] = strTopic;
		payload = jsonData_.dump();
		LOGDBUG2("(serve_request) Send To Cloud Data [PAYLOAD]",payload);
	} catch (...) {
		LOGEROR("(serve_request) Failed to process response Json !!");
	}
	std::string busName = sproPtr.cloudEpInfo.cloudAppName;
	SessionId sessionID;
	if(sproPtr.isCloudConnected && sproPtr.ajNameSerPtr->GetNSInfo(busName,sessionID)) {
		int tryCout = 0;
		std::string command = CC_SENDTOCLOUD;
		while(tryCout < COULD_CONNECTION_TRY) {
			tryCout++;
			if(sproPtr.methoExePtr) {
				if(ER_OK == sproPtr.methoExePtr->methodCaller(busName, sessionID, "SyncListener", command, payload))
					break ;
			} else {
				LOGEROR("(ServiceProvider) Invalid Method Executer Ptr !!");
				break ;
			}
			usleep(500);
		}
	} else {
		LOGEROR("(ServiceProvider) Unable to Send Cloud !!");
	}
	LOGINFO("(ServiceProvider::processAsynTaskThread) Thread - End.");
}

void ServiceProvider::startCloudConnection()
{

	LOGINFO("(ServiceProvider::processCloudConnectionThread) Thread - Start.");

	cloudConnectionThread = std::thread([this]
										 {
		QStatus status;

		int waitCount = 0;

		bool isCloudApp = true;
		bool isMyCloudAPP = false;


		while(cloudThreadRunner)
		{
			bool noCloudApp = false;
			std::string cloudAppName = "";
			waitingForCloudApp = false;

			{
				//Search for the cloud app in same gateway.
				if(!personalApp.empty())
				{
					cloudAppName = personalApp.front();
				}

				//Else check for cloud app in different gateways.
				else if(!vicinityApp.empty())
				{
					cloudAppName = vicinityApp.front();
				}

				// Else wait for cloud apps
				else
				{
					noCloudApp = true;
				}
			}

			if(noCloudApp)
			{
				waitingForCloudApp = true;

				//Wait for the cloud app to be found..
				std::unique_lock<std::mutex> uniLock(cloudInitMutex);
				cloudInitVar.wait(uniLock);

				continue;
			}

			//If success then try to make cloud connection.
			if(!cloudAppName.empty() && isCloudApp) {
				isCloudApp = false;

				waitCount = 0;
				while(waitCount < COULD_CONNECTION_TRY) {
					if(!isCloudConnected) {
						status = connectingCloudAndRegisterTopic(cloudAppName);
						if(ER_OK == status) {
							isMyCloudAPP = true;
							isCloudConnected = true;
							break;
						} else {
							isCloudConnected = false;
							isMyCloudAPP = false;
							waitCount++;
						}
					}
				}

			}


			//If unable to establish connection , then go to near by gateway cloud app for cloud connection.
			if(isMyCloudAPP == false)
			{
				for(std::string &idata : cloudAppInVicinityVector) {
					isMyCloudAPP = false;
					cloudAppName = idata;

					waitCount = 0;
					while(waitCount < COULD_CONNECTION_TRY) {
						if(!isCloudConnected) {
							status = connectingCloudAndRegisterTopic(cloudAppName);
							if(ER_OK == status) {
								isCloudConnected = true;
								break;
							} else {
								isCloudConnected = false;
								waitCount++;
							}
						}
					}
					if(true == isCloudConnected)
						break;
				}
			}

			//If got lost advertisement for connected cloud app, make connection with another near by cloud app.
			if(lossCloudConnection == CS_SUCCESS) {

				isCloudConnected = false;
				isCloudApp = true;
				isMyCloudAPP = false;
				lossCloudConnection = CS_FAIL;

				LOGWARN("(ServiceProvider::processCloudConnectionThread) Loss Cloud Connectivity, Move Next EndPoint !!");
			}

			if(isCloudConnected) {
				//			LOGDBUG2I("(ServiceProvider::processCloudConnectionThread)[isCloudConnected]",sproPtr.isCloudConnected);
			} else {
				isCloudApp = true;
				//LOGWARN2I("(ServiceProvider::processCloudConnectionThread)[isCloudConnected]",isCloudConnected);
				continue;
			}
		}
										 });
	LOGINFO("(ServiceProvider::processCloudConnectionThread) Thread - End.");
}


void ServiceProvider::startLocalConnection()
{

	LOGINFO("(ServiceProvider::processLocalConnectionThread) Thread - Start.");

	localConnectionThread = std::thread([this]
										 {
		QStatus status;

		int waitCount = 0;

		bool isLocalApp = true;
		bool isMyLocalAPP = false;

		while(localThreadRunner)
		{
			bool noCloudApp = false;
			std::string cloudAppName = "";

			waitingForCloudApp = false;

			{
				//Search for the cloud app in same gateway.
				if(!personalApp.empty())
				{
					cloudAppName = personalApp.front();
				}

				//Else check for cloud app in different gateways.
				else if(!vicinityApp.empty())
				{
					cloudAppName = vicinityApp.front();
				}

				// Else wait for cloud apps
				else
					noCloudApp = true;
			}

			if(noCloudApp)
			{
				waitingForCloudApp = true;

				//Wait for the cloud app to be found..
				std::unique_lock<std::mutex> uniLocalLock(localInitMutex);
				localInitVar.wait(uniLocalLock);
				continue;
			}

			//If success then try to make local connection.
			if(!cloudAppName.empty() && isLocalApp)
			{
				isLocalApp = false;
				while(waitCount < COULD_CONNECTION_TRY){
					if(!isLocalConnected) {
						status = connectingLocalAndRegisterTopic(cloudAppName);
						if(ER_OK == status) {
							isMyLocalAPP = true;
							isLocalConnected = true;
							break;
						} else {
							isLocalConnected = false;
							isMyLocalAPP = false;
							waitCount++;
						}
					}
				}

			}


			//If unable to establish connection. then go to near by gateway cloud apps for local connection.
			if(isMyLocalAPP == false)
			{
				for(std::string &idata : cloudAppInVicinityVector) {
					isMyLocalAPP = false;
					cloudAppName = idata;

					waitCount = 0;
					while(waitCount < COULD_CONNECTION_TRY) {
						if(!isLocalConnected) {
							status = connectingLocalAndRegisterTopic(cloudAppName);
							if(ER_OK == status) {
								isLocalConnected = true;
								break;
							} else {
								isLocalConnected = false;
								waitCount++;
							}
						}
					}
					if(true == isLocalConnected)
						break;
				}
			}

			//If got lost advertisement for connected cloud app, make connection with another near by cloud app.
			if(lossCloudConnection == CS_SUCCESS) {

				isLocalConnected = false;
				isLocalApp = true;
				isMyLocalAPP = false;
				lossLocalConnection = CS_FAIL;

				LOGWARN("(ServiceProvider::processCloudConnectionThread) Loss Cloud Connectivity, Move Next EndPoint !!");
			}

			if(isLocalConnected) {
				//			LOGDBUG2I("(ServiceProvider::processCloudConnectionThread)[isCloudConnected]",sproPtr.isCloudConnected);
			} else {
				isLocalApp = true;
				//LOGWARN2I("(ServiceProvider::processCloudConnectionThread)[isCloudConnected]",isLocalConnected);
				continue;

			}
		}
										 });
	LOGINFO("(ServiceProvider::processCloudConnectionThread) Thread - End.");
}
