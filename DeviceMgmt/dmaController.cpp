#include "dmaController.h"
#include "CommonHeader/fileoperations.h"
#include "CommonHeader/debugLog.h"

#define eI_MODULE "DmaController"
//#define MQTT_CONFIG_FILE_PATH "/opt/ESIA.conf.d/MQTT.conf"
//#define DMA_CONFIG_FILE_PATH "/opt/ESIA.conf.d/DMA.conf"

std::string configurationSettings::orgId = "58c3ce458441383a938c5c75";
std::string configurationSettings::packageId = "";
std::string configurationSettings::gatewayId = GWid;
std::string configurationSettings::standalone = "1"; //1 - for Stand alone, 0 - for bundle
std::string configurationSettings::original = "1";
std::string configurationSettings::AppTopic = "";
std::string configurationSettings::dmTopic = "";
std::string configurationSettings::mqqthostname = MQTT_CLOUD_HOSTNAME;
std::string configurationSettings::mqqtusername = MQTT_CLOUD_USERNAME;
std::string configurationSettings::mqqtpassword = MQTT_CLOUD_PASWRD;
uint16_t configurationSettings::mqqtport = MQTT_CLOUD_PORT;
std::string configurationSettings::macId = "";
std::string configurationSettings::wcmacId = "";
std::string configurationSettings::transportmask = "";
std::string configurationSettings::dmAppConfDirPath = "";
std::string configurationSettings::appBaseUrl = "";
std::map<std::string, brokerDetails> configurationSettings::brokerMap = {};
std::string configurationSettings::subTopicPrefix = "";
std::string configurationSettings::pubTelemetryTopic = "";
std::string configurationSettings::connectedCloudApp = "";
std::vector<keyVec> configurationSettings::configKeyVec= {};
std::map<std::string, brokerDetails> configurationSettings::localBrokerMap = {};
json configurationSettings::connectivityJson = {};
//std::

DMAController::DMAController()
:transportMast(TRANSPORT_NONE)
{
}

DMAController::~DMAController()
{
	if(nameService) {
		nameService->cancelNameService(appName.c_str());
		delete nameService;
	}
	if(servProPtr)
		delete servProPtr;
	if(initUniqPtr)
		initUniqPtr.release();
	if(ajCommonPtr)
		ajCommonPtr.release();
	if(busAttachPtr)
		delete busAttachPtr;
}

QStatus DMAController::Init()
{
	initUniqPtr = std::make_unique<AJInitializer>();
	initUniqPtr->Initialize();

	ajCommonPtr = std::make_unique<AJCommon>();
	busAttachPtr = ajCommonPtr->prepareBusAttachment();

	QStatus status = this->GetConfigurationFromConfigFile();
	if((busAttachPtr == NULL) || (initUniqPtr == nullptr)
			|| (ajCommonPtr == nullptr) || (status == ER_FAIL))
	{
		LOGEROR("(Init) Bus Pointer is NULL/Unable To Fetch .Config Data  !!");
		return ER_FAIL;
	}

	configurationSettings::subTopicPrefix = prefixTopic + "/" + configurationSettings::orgId + "/" + configurationSettings::gatewayId  + "/" + configurationSettings::AppTopic + "/#";
	configurationSettings::pubTelemetryTopic = prefixTopic + "/Data/" + configurationSettings::orgId + "/" + configurationSettings::gatewayId  + "/" + configurationSettings::AppTopic + "/Devices/";

	servProPtr = new ServiceProvider(transportMast, busAttachPtr,EI_COMMON_INTERFACE_NAME,EI_COMMON_INTERFACE_PATH);
	status = servProPtr->loadLIB();

	//	std::string appID = configurationSettings::AppTopic;

	//	status = servProPtr->getTopicPrefix(prefixTopic, appID, jsonData);
	//	if(status == ER_OK)
	//	{
	//		configurationSettings::subTopicPrefix = prefixTopic +"/" + configurationSettings::AppTopic + "/#";
	//
	//		std::string telemetryTopic;
	//		std::size_t found3 = prefixTopic.find("/");
	//		while(found3!= std::string::npos)
	//		{
	//			telemetryTopic = prefixTopic.substr(found3+1);
	//			break;
	//		}
	//		configurationSettings::pubTelemetryTopic = telemetryPrefix + telemetryTopic + "/Devices/";
	//	}

	//sleep(4);
	status = this->InitNS();
	return status;
}

void DMAController::DInit()
{
	if(nameService) {
		nameService->cancelNameService(nameService->getAdvertisedName().c_str());
		delete nameService;
	}
	if(servProPtr)
		delete servProPtr;
	if(initUniqPtr)
		initUniqPtr.release();
	if(ajCommonPtr)
		ajCommonPtr.release();
	if(busAttachPtr) {
		busAttachPtr->Stop();
		delete busAttachPtr;
	}
}

QStatus DMAController::InitNS()
{
	QStatus status = ER_FAIL;
	if(servProPtr) {
		try{
			status = servProPtr->initeIBusObject(EI_COMMON_INTERFACE_NAME);
			nameService = new ajNameService(busAttachPtr, servProPtr, configurationSettings::AppTopic);
			servProPtr->updateAJNameServicePtr(nameService);
			if(nameService && (ER_OK == status))
			{
				appName = DMA_APP_NAME_PREFIX;
				appName += ".GW";
				appName += configurationSettings::gatewayId;
				appName += ".APP";
				appName += configurationSettings::AppTopic;

				if(ER_OK == (status = nameService->advertiseNameService(appName.c_str(),DMA_ASSIGNED_SESSION_PORT)))
					status = nameService->findAdvertisedName(FIND_ADV_NAME_PREFIX);
			} else {
				LOGEROR("Invalid ajNameService/servProPtr ptr !!");
			}
		} catch(...) {
			LOGEROR("Fail to Initialize BusObject !!");
		}
	} else {
		LOGEROR("Invalid ServiceProvider/MethodExecuter/signalExePtr ptr !!");
	}
	servProPtr->setAdvName(appName);

	return status;
}

void DMAController::FunctionTest()
{
	servProPtr->FunctionTest();
}

QStatus DMAController::GetConfigurationFromConfigFile()
{
	QStatus status = ER_FAIL;
	FileOperation operations;
	json jsonRead;

	configurationSettings::dmAppConfDirPath = DMA_CONFIG_FILE_PATH + configurationSettings::AppTopic + "/" + configurationSettings::AppTopic;

	std::string fileDMAPath = configurationSettings::dmAppConfDirPath + CONFIG_FILE_PRIFIX;

	if(operations.readFromFile(fileDMAPath, jsonRead) == 0)
	{
		try
		{
			configurationSettings::packageId  = jsonRead[JKF_PACKAGEID];
			configurationSettings::gatewayId  = jsonRead[JKF_GATEWAYID];
			configurationSettings::original   = jsonRead[JKF_ORIGINAL];
			configurationSettings::transportmask = jsonRead[JKF_PROTOCOL];//JKF_TRANSPORTMASK
			//configurationSettings::standalone = jsonRead[JKF_STANDALONE];
			//configurationSettings::AppTopic   = jsonRead[JKF_APPTOPIC];
			status = ER_FAIL;
			if(!configurationSettings::transportmask.compare(JKF_ZWAVE))
				transportMast = TRANSPORT_EI_ZWAVE;
			else if(!configurationSettings::transportmask.compare(JKF_OPC))
				transportMast = TRANSPORT_EI_OPC;
			else if(!configurationSettings::transportmask.compare(JKF_ZIGBEE))
				transportMast = TRANSPORT_EI_ZIGBEE;
			else if(!configurationSettings::transportmask.compare(JKF_BLE))
				transportMast = TRANSPORT_EI_BLE;
			else if(!configurationSettings::transportmask.compare(JKF_MODBUS))
				transportMast = TRANSPORT_EI_MODBUS;
			else if(!configurationSettings::transportmask.compare(JKF_THREAD))
				transportMast = TRANSPORT_EI_THREAD;
			else if(!configurationSettings::transportmask.compare(JKF_COAP))
				transportMast = TRANSPORT_EI_COAP;
			else if(!configurationSettings::transportmask.compare(JKF_MQTTSIMULATOR))
				transportMast = TRANSPORT_EI_MQTTSIMULATOR;
			else {
				LOGEROR("(GetConfigurationFromConfigFile) Fail to Get Transport Mast !!");
				status = ER_FAIL;
			}
		}
		catch(std::exception &e)
		{
			status = ER_FAIL;
			LOGWARN2("(GetConfigurationFromConfigFile) Unable to fetch DMA Configurations. [e.WHAT]",e.what());
		}
	}
	else
	{
		status = ER_FAIL;
		LOGEROR2("(GetConfigurationFromConfigFile) Unable to open the file",DMA_CONFIG_FILE_PATH);
	}
	std::string fileMqttPath = MQTT_CONFIG_FILE_PATH + configurationSettings::AppTopic + "/" + configurationSettings::AppTopic + MQTT_CONFIG_FILE_PRIFIX + CONFIG_FILE_PRIFIX;
	if(operations.readFromFile(fileMqttPath, jsonRead) == 0)
	{
		try
		{
			brokerDetails brokInfo;
			//configurationSettings *confStr = new configurationSettings();
			std::cout << "JSON" << jsonRead.dump() << std::endl;

			json cloudConnectivityJson = jsonRead[APPCONFIG][CLOUDBROKERS];

			std::cout << "cloudjson" << cloudConnectivityJson.dump() << std::endl;

			prefixTopic = jsonRead[APPCONFIG][APPBASEURL];

			configurationSettings::orgId = jsonRead[APPCONFIG][ORGID];

			//telemetryPrefix = jsonRead[APPCONFIG]["telemetryprefix"];

			jsonData = jsonRead;
			configurationSettings::connectivityJson = jsonData;

			int size = cloudConnectivityJson.size();
			int x = 0;
			for(int i = 0 ; i < size ; i++)
			{
				std::string connectID = cloudConnectivityJson[i][CONNECTID];
				brokInfo.mqqthostname= cloudConnectivityJson[i][HOST];
				brokInfo.mqqtpassword = cloudConnectivityJson[i][PASS];
				brokInfo.mqqtusername = cloudConnectivityJson[i][USER];
				//brokInfo.orgID = cloudConnectivityJson[i][JKF_ORGID];
				brokInfo.mqqtport = cloudConnectivityJson[i][PORT];
				brokInfo.type = cloudConnectivityJson[i][BROKERTYPE];

				brokInfo.brokerNum = ++x;
				brokInfo.totalBroker = size;

				if(certRequired && brokInfo.mqqtport != 1883)
				{
					std::cout << "innncafile" << brokInfo.cafile << std::endl;
					brokInfo.cafile = cloudConnectivityJson[i][JKF_SSLCONFIGURATION][JKF_MQTTCAFILE];
					brokInfo.certfile = cloudConnectivityJson[i][JKF_SSLCONFIGURATION][JKF_MQTTCERTFILE];
					brokInfo.keyfile = cloudConnectivityJson[i][JKF_SSLCONFIGURATION][JKF_MQTTKEYFILE];
					brokInfo.version = "tlsv1.2";
					brokInfo.cipher = "AES256-SHA";
				}

				configurationSettings::brokerMap[connectID] = brokInfo;
				LOGDBUG("(alljoynWrapper...!!) ConnectMqtt");
			}

			for(std::map<std::string, brokerDetails>::iterator it=configurationSettings::brokerMap.begin();it!=configurationSettings::brokerMap.end();++it){

				std::cout << "IT->SECONDaaa" << it->second.type << std::endl;
			}

			//			configurationSettings::orgId = jsonRead[JKF_ORGID];
			//			configurationSettings::mqqtpassword = jsonRead[JKF_MQTTPASSWORD];
			//			configurationSettings::mqqthostname = jsonRead[JKF_MQTTHOSTNAME];
			//			configurationSettings::mqqtusername = jsonRead[JKF_MQTTUSERNAME];
			//			configurationSettings::mqqtport = jsonRead[JKF_MQTTPORT];

			//----------------------------localbroker-------------------------------------
			std::cout << "JSON READ" << jsonRead.dump() << std::endl;
			json localConnectivityJson = jsonRead[APPCONFIG][LOCALBROKERS];

			std::cout << "localjson" << localConnectivityJson.dump() << std::endl;

			int localsize = localConnectivityJson.size();
			int y = 0;
			for(int i = 0 ; i < localsize ; i++)
			{
				std::string connectID = localConnectivityJson[i][CONNECTID];
				brokInfo.mqqthostname= localConnectivityJson[i][HOST];
				brokInfo.mqqtpassword = localConnectivityJson[i][PASS];
				brokInfo.mqqtusername = localConnectivityJson[i][USER];
				//brokInfo.orgID = localConnectivityJson[i][JKF_ORGID];
				brokInfo.mqqtport = localConnectivityJson[i][PORT];
				brokInfo.type = localConnectivityJson[i][BROKERTYPE];
				brokInfo.brokerNum = ++y;
				brokInfo.totalBroker = localsize;

				if(certRequired && brokInfo.mqqtport != 1883)
				{
					brokInfo.cafile = localConnectivityJson[i][JKF_SSLCONFIGURATION][JKF_MQTTCAFILE];
					std::cout << "innncafile" << brokInfo.cafile << std::endl;
					brokInfo.certfile = localConnectivityJson[i][JKF_SSLCONFIGURATION][JKF_MQTTCERTFILE];
					brokInfo.keyfile = localConnectivityJson[i][JKF_SSLCONFIGURATION][JKF_MQTTKEYFILE];
					brokInfo.version = "tlsv1.2";
					brokInfo.cipher = "AES256-SHA";
				}

				configurationSettings::localBrokerMap[connectID] = brokInfo;
				LOGDBUG("(alljoynWrapper...!!) ConnectMqtt");
			}

			for(std::map<std::string, brokerDetails>::iterator it=configurationSettings::localBrokerMap.begin();it!=configurationSettings::localBrokerMap.end();++it){

							std::cout << "IT->SECONDaaall" << it->second.type << std::endl;
			}

			status = ER_OK;
		}
		catch(std::exception &e)
		{
			status = ER_FAIL;
			LOGWARN2("(GetConfigurationFromConfigFile) Unable to fetch MQTT Configurations.[e.WHAT]",e.what());
		}
	}
	else
	{
		status = ER_FAIL;
		LOGEROR2("(GetConfigurationFromConfigFile) Unable to open the file",MQTT_CONFIG_FILE_PATH);
	}

	if(configurationSettings::transportmask.empty())
	{
		status = ER_FAIL;
		LOGEROR2("(GetConfigurationFromConfigFile) Unable to Get Tansport Mask !! [FILE PATH]",DMA_CONFIG_FILE_PATH);
	}

	return status;
}

ajn::BusAttachment *DMAController::getBusAttachmentPtr() const
{
	return busAttachPtr;
}
