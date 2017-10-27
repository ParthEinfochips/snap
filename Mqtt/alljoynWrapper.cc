#include "alljoynWrapper.h"
#include "CommonHeader/debugLog.h"
#define eI_MODULE "alljoynWrapper"



std::string configurationManage::gatewayId = "";
std::string configurationManage::appBaseUrl = "";
std::string configurationManage::subTopicPrefix = "";
std::string configurationManage::AppName = "";
json configurationManage::configFunJson = {};
std::map<std::string, std::string>  configurationManage::appFunMap= {};

using namespace ajn;

json brokerInfo;
json metaData;

alljoynWrapper::alljoynWrapper()
{
	filePtr = new FileOPS();

}
alljoynWrapper::alljoynWrapper(alljoynWrapper &wrapPtr)
{
	filePtr = new FileOPS();
}


alljoynWrapper::~alljoynWrapper()
{
	if(ifc)
		delete ifc;
	if(bus)
		delete bus;
	if(busObject)
		delete busObject;
	if(mqttApiPtr)
		delete mqttApiPtr;
	if(filePtr)
		delete filePtr;

#ifdef ROUTER
	AllJoynRouterShutdown();
#endif
	AllJoynShutdown();
}

/*
 * @brief : Creating the required instances, starting the bus and advertising.
 */
QStatus alljoynWrapper::init()
{
	QStatus status;

	try
	{
		try{
			filePtr->readFromFile(metaDetails, metaData);

			configurationManage::gatewayId = metaData[GATEWAYID];
		}
		catch(...)
		{
			LOGINFO("Unable to get Gateway_meta details");
		}

		status = AllJoynInit();
		if (status != ER_OK)
		{
			LOGEROR2("(AJInitializer) Fail to Initialize AJInitializer ::Initialize [Status]",QCC_StatusText(status));
			return status;
		}

		bus = new BusAttachment("MQTTApp", true, 10);
		//bus->EnableConcurrentCallbacks();

		/* Start the BusAttachment */
		status = bus->Start();
		if (status != ER_OK)
		{
			LOGEROR2("(prepareBusAttachment) Failed to Start BusAttachment [Status]", QCC_StatusText(status));
			delete bus;
			return status;
		}

		/* Connect to the daemon using address provided*/
		status = bus->Connect();
		LOGDBUG2("(prepareBusAttachment) BusAttachment Status [Status]", QCC_StatusText(status));
		if (status != ER_OK)
		{
			LOGEROR2("(prepareBusAttachment) Failed to Connect BusAttachment [Status]", QCC_StatusText(status));
			delete bus;
			return status;
		}

		LOGINFO2("(alljoynWrapper) init gatewayID",configurationManage::AppName);

		strInterfaceName = std::string(CLOUD_APP_NAME_PREFIX) + ".GW" + configurationManage::gatewayId + ".APP" + configurationManage::AppName;// Add GateWay ID.

		LOGINFO2("(alljoynWrapper) init strInterfaceName", strInterfaceName);

		busObject = new mqttBusObject(bus, EI_COMMON_INTERFACE_NAME, EI_COMMON_INTERFACE_PATH, strInterfaceName);

		try
		{
			status = busObject->initeIBusObject(EI_COMMON_INTERFACE_NAME);
		}
		catch(...)
		{
			LOGINFO("(alljoynWrapper) init error in initbusobject");
			return ER_FAIL;
		}

		//		std::string brokerDetails = brokerDataPath + configurationManage::AppName + "/" + configurationManage::AppName + "MQTT.conf";
		//
		//		LOGINFO2("brokerdetails" , brokerDetails);
		//		filePtr->readFromFile(brokerDetails, brokerInfo);

		std::string configStr = configDataPath + configurationManage::AppName + "/" + "AccessMethod.conf";

		filePtr->readFromFile(configStr, configurationManage::configFunJson);

	}
	catch(...)
	{
		std::cout << "Init failure" << std::endl;
	}

	return status;
}


void alljoynWrapper::Dinit()
{
	if(ifc)
		delete ifc;
	if(bus)
		delete bus;
	if(busObject)
		delete busObject;
	if(mqttApiPtr)
		delete mqttApiPtr;
}
