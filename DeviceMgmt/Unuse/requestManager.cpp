#include "requestManager.h"
#include "requestTaskHandler.h"
#include "CommonHeader/DebugLog.h"
#include "CommonHeader/json.hpp"
#include <sstream>

#define QCC_MODULE "DMA_APP"
#define eI_MODULE "REQUESTMANAGER"

using json = nlohmann::json;

RequestManager::RequestManager(std::shared_ptr<DMAController> &dmaCtrlTmpPtr,
                            std::shared_ptr<AsyncTaskQueue> &tempPool) :
                            dmaCtrlObjShrdPtr(dmaCtrlTmpPtr),
                            pool(tempPool)
{
    startRequestTaskHandler();

    std::cout << "Hashed topic " << HASHED_TOPIC << std::endl;

    readConfiguration();

    connectWithCloudObj();

    obtainDevObject();

    prepareFixTopics();
}

RequestManager::~RequestManager()
{

}

void RequestManager::prepareFixTopics()
{
    std::string devBasedTopic = configurationSettings::dmTopic + DEVICES_ENTITY_FIRST;

    QCC_LogMsg(("In prepareFixTopics ::: %s",devBasedTopic.c_str()));

    //Add device
    std::string addDevice = devBasedTopic + COMMANDS_PUT;
    std::unique_ptr<topicCMDnDevID> AddDevPtr = std::make_unique<topicCMDnDevID>(addDevice, DEVICE_ADD);
    mapTopic[addDevice] = std::move(AddDevPtr);
    //reqMgrCloudProxyShrdPtr->registerTopicInCloud(cloudUniqueName, addDevice, std::bind(&RequestManager::IncomingRequestReceiverCB, this, std::placeholders::_1, std::placeholders::_2));

    //Get device list
    std::string deviceList = devBasedTopic + COMMANDS_GET;
    std::unique_ptr<topicCMDnDevID> DevListPtr = std::make_unique<topicCMDnDevID>(deviceList, DEVICE_LIST);
    mapTopic[deviceList] = std::move(DevListPtr);
    //reqMgrCloudProxyShrdPtr->registerTopicInCloud(cloudUniqueName, deviceList, std::bind(&RequestManager::IncomingRequestReceiverCB, this, std::placeholders::_1, std::placeholders::_2));

    //Get APP RAML
    std::string appRAMLGet = configurationSettings::dmTopic + COMMANDS_RAML_AFTER + COMMANDS_GET ;
    std::unique_ptr<topicCMDnDevID> appGetRAMLPtr = std::make_unique<topicCMDnDevID>(appRAMLGet, GET_APP_RAML);
    mapTopic[appRAMLGet] = std::move(appGetRAMLPtr);
    //reqMgrCloudProxyShrdPtr->registerTopicInCloud(cloudUniqueName, appRAMLGet, std::bind(&RequestManager::IncomingRequestReceiverCB, this, std::placeholders::_1, std::placeholders::_2));

    std::string protocolEntity = configurationSettings::dmTopic + PROTOCOLS_ENTITY_FIRST;
    std::string devicePut = DEVICES_ENTITY_FIRST;
    devicePut += COMMANDS_PUT;

    // second add BLE
    addDevice = protocolEntity + SLASH + T_BLE + devicePut;
    registerCommand(DEVICE_ADD,addDevice,T_BLE);

    // second add ZWave
    addDevice = protocolEntity + SLASH + T_ZWAVE + devicePut;
    registerCommand(DEVICE_ADD,addDevice,T_ZWAVE);

    // second add ZigBee
    addDevice = protocolEntity + SLASH + T_ZIGBEE + devicePut;
    registerCommand(DEVICE_ADD,addDevice,T_ZIGBEE);

    // second add Alljoyn
    addDevice = protocolEntity + SLASH + T_ALLJOYN + devicePut;
    registerCommand(DEVICE_ADD,addDevice,T_ALLJOYN);

    // second add OPC
    addDevice = protocolEntity + SLASH + T_OPC + devicePut;
    registerCommand(DEVICE_ADD,addDevice,T_OPC);

    std::string deviceGut = DEVICES_ENTITY_FIRST;
    deviceGut += COMMANDS_GET;

    //BLE Discover
    std::string BLEDiscover = protocolEntity + SLASH + T_BLE + COMMANDS_GET;
    registerCommand(DISCOVER_CMD,BLEDiscover,T_BLE);

    std::string BLEDiscoverDev = protocolEntity + SLASH + T_BLE + deviceGut;
    registerCommand(DISCOVER_CMD,BLEDiscoverDev,T_BLE);

    //Zigbee Discover
    std::string ZIGBEEDiscover = protocolEntity + SLASH + T_ZIGBEE + COMMANDS_GET;
    registerCommand(DISCOVER_CMD,ZIGBEEDiscover,T_ZIGBEE);

    std::string ZIGBEEDiscoverDev = protocolEntity + SLASH + T_ZIGBEE + deviceGut;
    registerCommand(DISCOVER_CMD,ZIGBEEDiscoverDev,T_ZIGBEE);

    //OPC Discover
    std::string OPCDiscover = protocolEntity + SLASH + T_OPC + COMMANDS_GET;
    registerCommand(DISCOVER_CMD,OPCDiscover,T_OPC);

    std::string OPCDiscoverDev = protocolEntity + SLASH + T_OPC + deviceGut;
    registerCommand(DISCOVER_CMD,OPCDiscoverDev,T_OPC);

    //ZWave discover
    std::string ZWAVEDiscover = protocolEntity + SLASH + T_ZWAVE + COMMANDS_GET;
    registerCommand(DISCOVER_CMD,ZWAVEDiscover,T_ZWAVE);

    std::string ZWAVEDiscoverDev = protocolEntity + SLASH + T_ZWAVE + deviceGut;
    registerCommand(DISCOVER_CMD,ZWAVEDiscoverDev,T_ZWAVE);

    //AllJoyn Discover
    std::string AlljoynDiscover = protocolEntity + SLASH + T_ALLJOYN + COMMANDS_GET;
    registerCommand(DISCOVER_CMD,AlljoynDiscover,T_ALLJOYN);

    std::string AlljoynDiscoverDev = protocolEntity + SLASH + T_ALLJOYN + deviceGut;
    registerCommand(DISCOVER_CMD,AlljoynDiscoverDev,T_ALLJOYN);

    //Transfer Ownership
    std::string TransferOwnership = configurationSettings::dmTopic + TRANSFER_OWNERSHIP;
    registerCommand(TRANSFER_OWNERSHIP_DEVICE,TransferOwnership,"");

    //Hard Transfer Devices
    std::string HardTransfer = configurationSettings::dmTopic + TRANSFER_DEVICES;
    registerCommand(TRANSFER_DEVICES_DEVICE,HardTransfer,"");

    //Delete device
    std::string deleteDevIDtopic = configurationSettings::dmTopic + DEVICES_ENTITY_FIRST + COMMANDS_DEL;
    std::unique_ptr<topicCMDnDevID> delPtr = std::make_unique<topicCMDnDevID>(deleteDevIDtopic, DEVICE_DEL);
    mapTopic[deleteDevIDtopic] = std::move(delPtr);
    //reqMgrCloudProxyShrdPtr->registerTopicInCloud(cloudUniqueName, deleteDevIDtopic, std::bind(&RequestManager::IncomingRequestReceiverCB, this, std::placeholders::_1, std::placeholders::_2));
}

void RequestManager::registerCommand(COMMAND_TYPE_ENUM commandType, string topic, string protocol)
{
    auto topicPtr = std::make_unique<topicCMDnDevID>(topic,protocol ,commandType);
    mapTopic[topic] = std::move(topicPtr);
    //reqMgrCloudProxyShrdPtr->registerTopicInCloud(cloudUniqueName, topic, std::bind(&RequestManager::IncomingRequestReceiverCB, this, std::placeholders::_1, std::placeholders::_2));
}

void RequestManager::registerDeviceCommand(COMMAND_TYPE_ENUM commandType, std::string topic,const std::string &devID)
{
    auto topicPtr = std::make_unique<topicCMDnDevID>(devID, commandType);
    mapTopic[topic] = std::move(topicPtr);
    //reqMgrCloudProxyShrdPtr->registerTopicInCloud(cloudUniqueName, topic, std::bind(&RequestManager::IncomingRequestReceiverCB, this, std::placeholders::_1, std::placeholders::_2));
}

void RequestManager::prepareDevIDTopics(const std::string &devID)
{
    std::string devBasedTopic = configurationSettings::dmTopic + DEVICES_ENTITY_FIRST + SLASH + devID;

    QCC_LogMsg(("In PrepareDevIDTopics ::: %s",devBasedTopic.c_str()));

    std::cout << "\nIn PrepareDevIDTopics ::: " << devBasedTopic << std::endl;

    //Delete Device
    registerDeviceCommand(DEVICE_DEL,COMMANDS_DEL,devID);

    // Set Device State
    std::string tpc = devBasedTopic + COMMANDS_STATE_AFTER ;
    tpc += COMMANDS_PUT;
    registerDeviceCommand(SET_DEVICE_STATE_,tpc,devID);

    // Get Device State
    tpc = devBasedTopic + COMMANDS_STATE_AFTER ;
    tpc += COMMANDS_GET;
    registerDeviceCommand(GET_DEVICE_STATE_,tpc,devID);

    // RegisterProperties
    tpc = COMMANDS_PROPERTIES_AFTER ;
    tpc += COMMANDS_PUT;
    registerDeviceCommand(PUT_DEVICE_PROPERTIES,tpc,devID);

    // De-RegisterProperties
    tpc = devBasedTopic + COMMANDS_PROPERTIES_AFTER ;
    tpc += COMMANDS_DEL;
    registerDeviceCommand(DEL_DEVICE_PROPERTIES,tpc,devID);

    // GetRAML
    tpc = devBasedTopic + COMMANDS_RAML_AFTER ;
    tpc += COMMANDS_GET;
    registerDeviceCommand(GET_DEVICE_RAML_,tpc,devID);

    // SetRAML
    tpc = devBasedTopic + COMMANDS_RAML_AFTER ;
    tpc += COMMANDS_PUT;
    registerDeviceCommand(SET_DEVICE_RAML_,tpc,devID);

    // GetDeviceStatus
    tpc = devBasedTopic + COMMANDS_DEVICESTATUS_AFTER ;
    tpc += COMMANDS_GET;
    registerDeviceCommand(GET_DEVICE_STATUS_,tpc,devID);

    // SetDeviceStatus
    tpc = devBasedTopic + COMMANDS_DEVICESTATUS_AFTER ;
    tpc += COMMANDS_PUT;
    registerDeviceCommand(PUT_DEVICE_STATUS,tpc,devID);
}

void RequestManager::readConfiguration()
{
    configurationSettings::dmTopic = "EI/" + configurationSettings::orgId +  SLASH + configurationSettings::gatewayId +  SLASH + configurationSettings::AppTopic;

    std::cout << "DMA topic " << configurationSettings::dmTopic << std::endl;
}

void RequestManager::obtainDevObject()
{
    std::cout << "Reached in obtainDevObject 1" << std::endl;
    //reqMgrDevProxyShrdPtr = dmaCtrlObjShrdPtr->getDeviceProxyPtr();
    std::cout << "Reached in obtainDevObject 2" << std::endl;
}

void RequestManager::connectWithCloudObj()
{
	json jsonData;
    std::string busName = MQTT_INTERFACE_NAME;
    //busName += ".GW123456";
    busName += ".GW" + configurationSettings::gatewayId;
    reqMgrCloudProxyShrdPtr = dmaCtrlObjShrdPtr->getCloudProxyPtr(busName);
    std::string connectId = reqMgrCloudProxyShrdPtr->cloudActiveConnection();
    std::string strLog  = "Cloud Active Connection List - " + connectId;
    LOGINFO(strLog);
    try
    {
    	jsonData = json::parse(connectId);
    } catch(...) {
    	LOGEROR("Unable to parse connection list Json !!");
    }

    uint16_t uniqueId;
    bool isConnectionExits(false);
    std::string strEndpointID;
    for (json::iterator it = jsonData.begin(); it != jsonData.end(); ++it)
    {
      //std::cout << it.key() << " : " << it.value() << std::endl;
      LOGDBUG("Check whether End Point is Connected "+ it.key() + " @ " + configurationSettings::mqqthostname + "|1883");
      std::string strHostName= it.key();
      std::string strMqqtHostName = configurationSettings::mqqthostname + "|1883";
      if(!strHostName.compare(strMqqtHostName))
      {
    	  isConnectionExits = true;
    	  strEndpointID     = it.value();
      }
    }

    if(isConnectionExits)
    {
    	LOGINFO("Got Endpoint Id - "+ strEndpointID);
    	uniqueId = std::stoi(strEndpointID);
    	cloudUniqueName = strEndpointID;
    	if(reqMgrCloudProxyShrdPtr->callback)
    	{
    		LOGINFO("Setting Request Manager Connection Flag.");
    		reqMgrCloudProxyShrdPtr->callback->isConnected = true;
    	} else {
    		LOGEROR("Unable to Set Connection flag (Request Manager)!!");
    	}
    } else {
    	LOGINFO("End Point Connecting .. .. ");
    	if(-1 != (uniqueId = reqMgrCloudProxyShrdPtr->connectWithCLDProxyObj
    			(configurationSettings::mqqthostname,
    					configurationSettings::mqqtusername,
						configurationSettings::mqqtpassword)))
    	{
    		std::stringstream ss;
    		ss << uniqueId;
    		cloudUniqueName = ss.str();
    		std::cout << "INFO : Got End Point ID - " << cloudUniqueName << std::endl;
    		LOGINFO("Got Endpoint Id - "+ cloudUniqueName);
    	} else {
    		std::cout << "DBUG : --- -- "<<  uniqueId << std::endl;
    		LOGEROR("Unable to Connect With MQTT Broker !!");
    	}
    }
    reqMgrCloudProxyShrdPtr->registerTopicInCloud(uniqueId,
    		configurationSettings::dmTopic,
			std::bind(&RequestManager::IncomingRequestReceiverCB, this,
			std::placeholders::_1, std::placeholders::_2));

    if(cloudUniqueName.empty())
    {
    	LOGEROR("Unable to Connect With MQTT Broker. cloud Unique Name is Empty !!");
        return;
    }
    LOGINFO("Registered Topic In Cloud .. .. ");
}

topicCMDnDevID *RequestManager::getCommandInfo(const std::string &topic)
{
    auto iterat = mapTopic.find(topic);
    topicCMDnDevID *topicPtr(NULL);

    if(iterat != mapTopic.end())
    {
        topicPtr = iterat->second.get();
    }

    return topicPtr;
}

std::string RequestManager::convertSpecialCharacters(const std::string& str)
{
  std::string res;
  std::string::const_iterator it = str.begin()+1;
  while (it != str.end()-1) {
    char ch = *it++;
    if (ch == '\\' && it != str.end()) {
      switch (*it++)
      {
          case '\\': ch = '\\'; break;        // Back Slash
          case 'n':  ch = '\n'; break;        // New Line
          case 't':  ch = '\t'; break;        // Tab
          case '"':  ch = '\"'; break;        // Double Quotes
          case 'b':  ch = '\b'; break;        // Backspace
          case 'r':  ch = '\r'; break;        // Carriage Return
          // all oth er escapes
          default:
          // invalid escape sequence - skip it. alternatively you can copy it as is, throw an exception.
            continue;
      }
    }
    res += ch;
  }
  return res;
}

void RequestManager::setCommandParameters(std::string topic, std::string msg, topicCMDnDevID *ptrTopicCMDnDevID, std::unique_ptr<command> ptr)
{
    try
    {
        json jsonData = json::parse(msg);
        std::string jobid = jsonData[JOBID];
        ptr->setCloudPtr(cloudUniqueName, reqMgrCloudProxyShrdPtr);
        //ptr->setDevicePtr(reqMgrDevProxyShrdPtr);
        ptr->commandDataObj.deviceID = ptrTopicCMDnDevID->devID;
        ptr->commandDataObj.jsonPaylod = msg;
        ptr->commandDataObj.destinationMQttURL = topic + std::string(REPLY_TOPIC) + std::string(SLASH) + jobid;
        functPtr(std::move(ptr));
    }
    catch(std::exception& e)
    {
        std::cout << "Exception caught in setCommandParameters :: " <<  e.what() << std::endl;
    }
}

void RequestManager::discoverCommandParameters(std::string topic, topicCMDnDevID *ptrTopicCMDnDevID, std::string msg)
{
    try
    {
        json jsonData = json::parse(msg);
        std::string jobid = jsonData[JOBID];
        std::unique_ptr<command> ptr(new discoverCommand);
        ptr->setCloudPtr(cloudUniqueName, reqMgrCloudProxyShrdPtr);
        //ptr->setDevicePtr(reqMgrDevProxyShrdPtr);
        ptr->commandDataObj.jsonPaylod = msg;
        ptr->commandDataObj.destinationMQttURL = topic + std::string(REPLY_TOPIC) + std::string(SLASH) + jobid;
        ptr->commandDataObj.protocolID = ptrTopicCMDnDevID->protocolIDD;
        functPtr(std::move(ptr));
    }
    catch(std::exception &e)
    {
        std::cout << "Exception caught in discoverCommandParameters :: " <<  e.what() << std::endl;
    }
}

void RequestManager::addDeviceCommandParameters(std::string msg, topicCMDnDevID *ptrTopicCMDnDevID, std::string topic)
{
    try
    {
        json jsonData = json::parse(msg);
        std::string jobid = jsonData[JOBID];
        std::unique_ptr<command> ptr(new addDeviceCommand);
        ptr->setCloudPtr(cloudUniqueName, reqMgrCloudProxyShrdPtr);
        //ptr->setDevicePtr(reqMgrDevProxyShrdPtr);
        ptr->commandDataObj.jsonPaylod = msg;
        ptr->setRequestManager(this);
        ptr->commandDataObj.destinationMQttURL = topic + std::string(REPLY_TOPIC) + std::string(SLASH) + jobid;
        std::cout << "Protocol id in device add " << ptrTopicCMDnDevID->protocolIDD << std::endl;
        ptr->commandDataObj.protocolID = ptrTopicCMDnDevID->protocolIDD;
        functPtr(std::move(ptr));
    }
    catch(std::exception &e)
    {
        std::cout << "Exception caught in addDeviceCommandParameters :: " <<  e.what() << std::endl;
    }
}

void RequestManager::setCommonCommandParameters(std::string topic, std::string msg,std::unique_ptr<command> ptr)
{
    try
    {
        json jsonData = json::parse(msg);
        std::string jobid = jsonData[JOBID];
        ptr->setCloudPtr(cloudUniqueName, reqMgrCloudProxyShrdPtr);
        //ptr->setDevicePtr(reqMgrDevProxyShrdPtr);
        ptr->commandDataObj.jsonPaylod = msg;
        ptr->commandDataObj.destinationMQttURL = topic + std::string(REPLY_TOPIC) + std::string(SLASH) + jobid;
        functPtr(std::move(ptr));
    }
    catch(std::exception &e)
    {
        std::cout << "Exception caught in deleteDeviceCommandParameters :: " <<  e.what() << std::endl;
    }
}

void RequestManager::IncomingRequestReceiverCB(std::string topic, std::string msg)
{
    std::cout << "TempCall Back " << topic << " " << msg << std::endl;

    std::cout << topic << std::endl;

    topicCMDnDevID *ptrTopicCMDnDevID = getCommandInfo(topic);

    if(!ptrTopicCMDnDevID)
    {
        std::cout << "topic found is null" << std::endl;
        return;
    }

    try
    {
        std::cout << "command type in IncomingRequestRecievedCB " << ptrTopicCMDnDevID->cmdType << std::endl;

        switch(ptrTopicCMDnDevID->cmdType)
        {
            case DISCOVER_CMD:
            {
                discoverCommandParameters(topic, ptrTopicCMDnDevID, msg);
            }
            break;
            case DEVICE_LIST:
            {
                std::unique_ptr<command> ptr(new deviceListCommand);
                setCommonCommandParameters(topic, msg,std::move(ptr));
            }
            break;
            case DEVICE_ADD:
            {
                addDeviceCommandParameters(msg, ptrTopicCMDnDevID, topic);
            }
            break;
            case DEVICE_DEL:
            {
                std::unique_ptr<command> ptr(new deleteDeviceCommand);
                setCommonCommandParameters(topic,msg,std::move(ptr));
            }
            break;

            case GET_APP_RAML:
            {
                std::unique_ptr<command> ptr(new getAppRAMLCommand);
                setCommonCommandParameters(topic,msg,std::move(ptr));
            }
            break;
            case GET_DEVICE_RAML_:
            {
                std::unique_ptr<command> ptr(new getDeviceRAMLCommand);
                setCommandParameters(topic, msg, ptrTopicCMDnDevID, std::move(ptr));
            }
            break;

            case SET_DEVICE_RAML_:
            {
                std::cout << "reached in setdeviceraml "<< std::endl;
                std::unique_ptr<command> ptr(new putDeviceRAMLCommand);
                setCommandParameters(topic, msg, ptrTopicCMDnDevID, std::move(ptr));
            }
            break;
            case GET_DEVICE_STATUS_:
            {
                std::unique_ptr<command> ptr(new getDeviceStatusCommand);
                setCommandParameters(topic, msg, ptrTopicCMDnDevID, std::move(ptr));
            }
            break;
            case PUT_DEVICE_STATUS:
            {
                std::unique_ptr<command> ptr(new putDeviceStatusCommand);
                setCommandParameters(topic, msg, ptrTopicCMDnDevID, std::move(ptr));
            }
            break;
            case GET_DEVICE_STATE_:
            {
                std::unique_ptr<command> ptr(new getDeviceStateCommand);
                setCommandParameters(topic, msg, ptrTopicCMDnDevID, std::move(ptr));
            }
            break;
            case SET_DEVICE_STATE_:
            {
                std::unique_ptr<command> ptr(new setDeviceStateCommand);
                setCommandParameters(topic, msg, ptrTopicCMDnDevID, std::move(ptr));
            }
            break;
            case DEL_DEVICE_PROPERTIES:
            {
                std::unique_ptr<command> ptr(new deleteDevicePropertiesCommand);
                setCommandParameters(topic, msg, ptrTopicCMDnDevID, std::move(ptr));
            }
            break;
            case PUT_DEVICE_PROPERTIES:
            {
                std::unique_ptr<command> ptr(new setDevicePropertiesCommand);
                setCommandParameters(topic, msg, ptrTopicCMDnDevID, std::move(ptr));
            }
            break;
            case GET_GATEWAY_STATUS:
            {

            }
            break;
            case PUT_GATEWAY_STATUS:
            {

            }
            break;
            case TRANSFER_OWNERSHIP_DEVICE:
            {
                std::unique_ptr<command> ptr(new TransferOwnershipCommand);
                setCommonCommandParameters(topic, msg,std::move(ptr));
            }
            break;
            case TRANSFER_DEVICES_DEVICE:
            {
                std::unique_ptr<command> ptr(new TransferDevicesCommand);
                setCommonCommandParameters(topic, msg, std::move(ptr));
            }
            break;

        default:
            std::cout << "Command not found in IncomingRequestReceiverCB:: " << ptrTopicCMDnDevID->cmdType << std::endl;
        }
    }
    catch(std::exception &e)
    {
        std::cout << "Exception caught in IncomingRequestReceiverCB" << e.what() << std::endl;
        QCC_LogMsg(("\nExceptions: Invalid json :: %s",e.what()));
    }
}

void RequestManager::decisionOnForwarding(std::unique_ptr<command> uniqCommandPtr)
{
    pool->Enqueue(std::move(uniqCommandPtr));
}

void RequestManager::startRequestTaskHandler()
{
    RequestTaskHandler *handler = new RequestTaskHandler();

    pool->Start(handler);
}

void RequestManager::stopRequestTaskHandler()
{
    pool->Stop();
}

void RequestManager::registerConfigMgrCallBack(std::function<void(std::unique_ptr<command>)> functTempPtr)
{
    functPtr = functTempPtr;
}
