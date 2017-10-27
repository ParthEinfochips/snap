#include "configSensorManager.h"
#include "utility"
#include "devicespecificcommands.h"

#define QCC_MODULE "DMA_APP"

ConfigSensorManager::ConfigSensorManager(std::shared_ptr<DMAController> &dmaCtrlTmpPtr)
{
	std::string filePath = JSON_PATH_RE;

	if(!directoryOPS::checkExistence(filePath))
	{
		directoryOPS::createPath(filePath, 0755);
	}

	deviceMapInfo = new std::map<std::string, std::shared_ptr<deviceInfo> >();
	dmaCtrlPtr = dmaCtrlTmpPtr;
	pool = std::make_shared<AsyncTaskQueue>();
    RmMgr = std::make_shared<RequestManager>(dmaCtrlTmpPtr, pool);

	busAttachPtr = dmaCtrlTmpPtr->getBusAttachmentPtr();

	_alljoynmanager = new AlljoynAppManager(busAttachPtr);

	std::cout << "DBUG : Config Sensor Manager Initializing ** " <<std::endl;
    //busListenerPtr = std::make_shared<CommonBusListener>(busAttachPtr);
    //busListenerPtr->setDmaCtrlPtr(dmaCtrlPtr);
    //busListenerPtr->setSensorManager(this);
	hackRouterHooker = new HackRouterHooker(busAttachPtr, busListenerPtr.get(), HAWKROUTER_INTERFACE_PATH);

	dataTaskHndler = std::make_shared<DataTaskHandler>();
	std::string telemtryTopic = "EI/Data/" + configurationSettings::orgId + "/" + configurationSettings::gatewayId + "/" + configurationSettings::AppTopic +   DEVICES_ENTITY_FIRST  + "/";
	dataTaskHndler->setCloudPtr(RmMgr->getCloudUniqueName(),RmMgr->getCloudProxy(),telemtryTopic);
	dataTaskHndler->setAlljoynManager(_alljoynmanager);
	dataTaskHndler->setConfigSensorManager(this);
	dataTaskHndler->setHackRouterHooker(hackRouterHooker);

	QCC_LogMsg(("Reached in ConfigSensorManager 1"));
	RmMgr->registerConfigMgrCallBack(std::bind(&ConfigSensorManager::handleCommand, this, std::placeholders::_1));
	QCC_LogMsg(("Reached in ConfigSensorManager 2"));
	gatewayStatus = true;

	readFromDir();

    const char* OPC_DISCOVER_PACKET_2 = R"({"address":"opc.tcp://10.99.15.84:4842/freeopcua/server"})";
//    json discoverPkt = json::parse(OPC_DISCOVER_PACKET_2);
    std::this_thread::sleep_for(std::chrono::seconds(50));
    std::unique_ptr<discoverCommand> commandPtr(new discoverCommand);
//    commandPtr->setDevicePtr(getDeviceProxyPtr());
    commandPtr->commandDataObj.jsonPaylod = OPC_DISCOVER_PACKET_2;
    commandPtr->commandDataObj.protocolID = "OPC";
    commandPtr->setConfigManager(this);
    //commandPtr->execute();
}

ConfigSensorManager::~ConfigSensorManager()
{
	pool->Stop();
	if(busAttachPtr && busListenerPtr)
		ajCommonPtr->cancelNameService(busAttachPtr, busListenerPtr.get());

	if(_alljoynmanager)
		delete _alljoynmanager;
	if(hackRouterHooker)
		delete hackRouterHooker;
}


AlljoynAppManager* ConfigSensorManager::getAlljoynmanager() const
{
    return _alljoynmanager;
}

void ConfigSensorManager::setAlljoynmanager(AlljoynAppManager *alljoynmanager)
{
    _alljoynmanager = alljoynmanager;
}

HackRouterHooker* ConfigSensorManager::getHackRouterHooker() const
{
    return hackRouterHooker;
}

void ConfigSensorManager::setHackRouterHooker(HackRouterHooker *value)
{
    hackRouterHooker = value;
}

void ConfigSensorManager::setGatewayMacId(std::string macId)
{
    gatewayMacId = macId;
}

std::string ConfigSensorManager::getGatewayMacId()
{
    return gatewayMacId;
}

bool ConfigSensorManager::getGatewayStatus() const
{
    return gatewayStatus;
}

void ConfigSensorManager::setGatewayStatus(bool value)
{
    gatewayStatus = value;
}

void ConfigSensorManager::handleCommand(std::unique_ptr<command> commandPtr)
{
    auto cmdEnumVal = commandPtr->getCMDType();

    switch(cmdEnumVal)
    {
        case COMMAND_TYPE_ENUM::NONE:
            break;
        case DISCOVER_CMD:
        case DEVICE_ADD:
        case DEVICE_DEL:
        case GET_DEVICE_RAML:
        case SET_DEVICE_RAML:
        case GET_DEVICE_STATUS:
        case PUT_DEVICE_STATUS:
        case GET_DEVICE_STATE:
        case SET_DEVICE_STATE:
        case DEL_DEVICE_PROPERTIES:
        case PUT_DEVICE_PROPERTIES:
        case GET_GATEWAY_STATUS:
        case PUT_GATEWAY_STATUS:
        case TRANSFER_OWNERSHIP_DEVICE:
        case TRANSFER_DEVICES_DEVICE:
        case REGISTER_TOPICS_CLOUD:
        case REGISTER_DEVICE_CALLBACK_:
        case DEVICE_LIST:
            std::cout << "reached in configsensormanger handlecommand "<< cmdEnumVal <<  std::endl;
            commandPtr->setConfigManager(this);
            commandPtr->setAllJoynManager(_alljoynmanager);
            commandPtr->setHackRouterHooker(hackRouterHooker);
            commandPtr->setRequestManager(&*RmMgr);
            pool->Enqueue(std::move(commandPtr));
            break;
    }
}

void ConfigSensorManager::addDeviceInList(std::string deviceID, deviceInfo devInfo)
{
    std::lock_guard<std::mutex> guardObj(mlock);

    std::shared_ptr<deviceInfo> pointer = std::make_shared<deviceInfo>(devInfo);
    deviceMapInfo->insert(std::make_pair(deviceID,pointer));
    QCC_LogMsg(("Device Instered from addDeviceInList"));

    //write the current device list in devicelist.json for rule engine
    writeRuleEngineInfo();
}

void ConfigSensorManager::deleteDeviceFromList(std::string deviceID)
{
    std::lock_guard<std::mutex> guardObj(mlock);

    auto it = deviceMapInfo->find(deviceID);
    if(it !=  deviceMapInfo->end())
    {
        deviceMapInfo->erase(it);
    }

    //write the current device list in devicelist.json for rule engine
    writeRuleEngineInfo();
}

bool ConfigSensorManager::checkDeviceExist(std::string deviceID)
{
    std::lock_guard<std::mutex> guardObj(mlock);

    bool success(false);
    auto it = deviceMapInfo->find(deviceID);
    if(it !=  deviceMapInfo->end())
    {
        QCC_LogMsg(("Device ID found from map:: %d",deviceID.c_str()));
//        std::cout << "device id found from map:::" << std::endl;
        success = true;
    }
    return success;
}

std::shared_ptr<deviceInfo> ConfigSensorManager::getDeviceFromList(std::string deviceID)
{
    std::lock_guard<std::mutex> guardObj(mlock);

    std::shared_ptr<deviceInfo> ptr = nullptr;
    auto it = deviceMapInfo->find(deviceID);
    if(it !=  deviceMapInfo->end())
    {
        QCC_LogMsg(("device id found from map:: %s",deviceID.c_str()));
//        std::cout << "device id found from map:::" << std::endl;
        ptr = it->second;
    }
    return ptr;
}

//This function is implemented specifically to provide devices information to RuleEngine
void ConfigSensorManager::writeRuleEngineInfo()
{
    try
    {
        json ruleEngineData;
        bool deviceNull(false);

        typedef std::map<std::string, std::shared_ptr<deviceInfo>>::iterator it_type;
        for(it_type iterator = deviceMapInfo->begin(); iterator != deviceMapInfo->end(); iterator++)
        {
            auto devInfo = iterator->second;
            json jsonRef = devInfo->getJsonPayload();
            std::string macId = jsonRef["macid"];
            std::string devId = jsonRef["id"];
            ruleEngineData[macId] = devId;
            deviceNull = true;
        }

        std::cout << "Devilist.json info :: " << ruleEngineData.dump() << std::endl;

        FileOPS fileOPSObj;
        std::string filePath = JSON_PATH_RE;
        filePath += RE_FILENAME;

        QCC_LogMsg(("File Path in writeRuleEngineInfo :: %s",filePath.c_str()));
        if(!directoryOPS::checkExistence(filePath) && deviceNull)
        {
            QCC_LogMsg(("File does not exist in writejson :: %s",filePath.c_str()));
            fileOPSObj.writeToFile(filePath, ruleEngineData.dump());
//            filePathsVec.push_back(filePath);
        }
        else
        {
            std::remove(filePath.c_str());
            if(deviceNull)
                fileOPSObj.writeToFile(filePath, ruleEngineData.dump());
        }
    }
    catch(std::exception &e)
    {
        std::cout << "Caught Exception in writeRuleEngileInfo ---- " << e.what() << std::endl;
    }
}

void ConfigSensorManager::readFromDir()
{
    try
    {
        std::unique_ptr<restorePersitentDevices> commandPtr(new restorePersitentDevices);
//        commandPtr->setDevicePtr(getDeviceProxyPtr());
        commandPtr->setConfigManager(this);
        commandPtr->setRequestManager(&*RmMgr);
        commandPtr->execute();
    }
    catch(std::exception &e)
    {
        QCC_LogMsg(("Exceptions: Invalid json:: %s",e.what()));
    }
}

void ConfigSensorManager::registerDeviceWithRouter(std::string busName)
{
    // This command will register the device callbacks in Remote Device Object to receive the telemetry.
    std::unique_ptr<registerDeviceCallbackForRegisterDevice> regCommandDev(new registerDeviceCallbackForRegisterDevice);
    regCommandDev->commandDataObj.jsonPaylod = busName;
    regCommandDev->setConfigManager(this);
    pool->Enqueue(std::move(regCommandDev));
}

void ConfigSensorManager::transferDevicess(std::string jsonPayload)
{
    try
    {
        QCC_LogMsg(("reached in transferDevicess"));
        std::unique_ptr<hardTransferCommand> commandPtr(new hardTransferCommand);
        commandPtr->commandDataObj.jsonPaylod = jsonPayload;

        QCC_LogMsg(("reached in transferDevicess 1"));
//        std::cout <<"\nreached in deviceDataCallback command 1" <<std::endl;
        commandPtr->setAllJoynManager(_alljoynmanager);
        QCC_LogMsg(("reached in transferDevicess 2"));
//        std::cout <<"\nreached in deviceDataCallback command 2" <<std::endl;
        commandPtr->setConfigManager(this);
        commandPtr->setHackRouterHooker(hackRouterHooker);
        commandPtr->setRequestManager(&*RmMgr);

        //commandPtr->setDevicePtr(devShrdPtr);

        std::string uniqueName = RmMgr->getCloudUniqueName();
        /*std::shared_ptr<*/CloudProxy*/*>*/ cloudshrPtr = RmMgr->getCloudProxy();

        commandPtr->setCloudPtr(uniqueName,cloudshrPtr);

        pool->Enqueue(std::move(commandPtr));
    }
    catch(std::exception &e)
    {
        QCC_LogMsg(("Error in the transferDevicess in DMA::  %s",e.what()));
//        std::cout << "Error in the add device in DMA " << e.what() << std::endl;
    }
}

void ConfigSensorManager::sendTelemetry(std::string jsonPayload)
{
    QCC_LogMsg(("reached in sendTelemetry function :: %s",jsonPayload.c_str()));
    try
    {
        json jsonData = json::parse(jsonPayload);
        std::string deviceID = jsonData[DEVICE_ID];
        jsonData.erase(DEVICE_ID);
        jsonPayload = jsonData.dump();

        dataTaskHndler->deviceDataCallback(deviceID,jsonPayload);
    }
    catch (std::exception &e)
    {
        QCC_LogMsg(("Error in the send Telemetry in DMA::  %s",e.what()));
        std::cout << "send telemetry " << e.what() << std::endl;
    }

}

void ConfigSensorManager::setDeviceState(std::string& jsonPayload)
{
    QCC_LogMsg(("Reached in setDeviceState :: %s",jsonPayload.c_str()));
    try
    {
        json jsonData = json::parse(jsonPayload);
        std::string deviceID = jsonData[DEVICE_ID];

        jsonPayload = jsonData.dump();
        auto devInfo = getDeviceFromList(deviceID);
        if(devInfo)
        {
            QCC_LogMsg(("device found  in setDeviceStateCommand :: %s",deviceID.c_str()));
            std::unique_ptr<setDeviceStateCommand> commandPtr(new setDeviceStateCommand);
            commandPtr->commandDataObj.deviceID = deviceID;
            commandPtr->commandDataObj.jsonPaylod = jsonPayload;

            commandPtr->setAllJoynManager(_alljoynmanager);
            commandPtr->setConfigManager(this);
            commandPtr->setHackRouterHooker(hackRouterHooker);
//            commandPtr->setDevicePtr(dmaCtrlPtr->getDeviceProxyPtr());

            pool->Enqueue(std::move(commandPtr));
            QCC_LogMsg(("command enqueued in setDeviceState"));

        }
//        else
//        {
//            json jsonData = json::parse(commandDataObj.jsonPaylod);
//            jsonData[RESPONSE_CODE] = 400;
//            jsonData[MESSAGE] = DEVICE_NOT_REGISTERED;
//            commandDataObj.jsonPaylod = jsonData.dump();
//        }

    }
    catch (std::exception &e)
    {
        QCC_LogMsg(("Error in the setDevicestate in DMA::  %s",e.what()));
        std::cout << "setDevicestate " << e.what() << std::endl;
    }

}


void ConfigSensorManager::sendDeviceStateResponse(std::string& jsonPayload)
{
    QCC_LogMsg(("Reached in sendDeviceStateResponse :: %s",jsonPayload.c_str()));
    try
    {
        json jsonData = json::parse(jsonPayload);
        std::string deviceID = jsonData[DEVICE_ID];

        jsonPayload = jsonData.dump();
        auto devInfo = getDeviceFromList(deviceID);
        if(devInfo)
        {
            QCC_LogMsg(("device found  in sendDeviceStateResponse :: %s",deviceID.c_str()));
            std::unique_ptr<sendResponseToCloud> commandPtr(new sendResponseToCloud);
            commandPtr->commandDataObj.jsonPaylod = jsonPayload;
            commandPtr->commandDataObj.destinationMQttURL = devInfo->getDestMQTTUrl();
            std::string cldUniqueName = RmMgr->getCloudUniqueName();
            /*std::shared_ptr<*/CloudProxy*/*>*/ cldProxy = RmMgr->getCloudProxy();

            commandPtr->setCloudPtr(cldUniqueName, cldProxy);
            pool->Enqueue(std::move(commandPtr));
            QCC_LogMsg(("command enqueued in sendDeviceStateResponse"));

        }
    }
    catch (std::exception &e)
    {
        QCC_LogMsg(("Error in the setDevicestate in DMA::  %s",e.what()));
        std::cout << "setDevicestate " << e.what() << std::endl;
    }

}

void ConfigSensorManager::SendResponseToCloud(std::string& jsonPayload)
{
    QCC_LogMsg(("Reached in sendResponseToCloud :: %s",jsonPayload.c_str()));
    try
    {
        json jsonData = json::parse(jsonPayload);
        std::string destMQTTUrl = jsonData[DEST_CLOUD_URL];

        std::unique_ptr<sendResponseToCloud> commandPtr(new sendResponseToCloud);
        commandPtr->commandDataObj.jsonPaylod = jsonPayload;
        commandPtr->commandDataObj.destinationMQttURL = destMQTTUrl;
        std::string cldUniqueName = RmMgr->getCloudUniqueName();
        /*std::shared_ptr<*/CloudProxy*/*>*/ cldProxy = RmMgr->getCloudProxy();

        commandPtr->setCloudPtr(cldUniqueName, cldProxy);
        pool->Enqueue(std::move(commandPtr));
        QCC_LogMsg(("command enqueued in sendResponseToCloud"));
    }
    catch (std::exception &e)
    {
        QCC_LogMsg(("Error in the setDevicestate in DMA::  %s",e.what()));
        std::cout << "setDevicestate " << e.what() << std::endl;
    }
}

void ConfigSensorManager::getDeviceState(std::string& jsonPayload)
{
    QCC_LogMsg(("reached in getDeviceState ::  %s",jsonPayload.c_str()));
    try
    {
        json jsonData = json::parse(jsonPayload);
        std::string deviceID = jsonData[DEVICE_ID];

        jsonPayload = jsonData.dump();
        auto devInfo = getDeviceFromList(deviceID);
        if(devInfo)
        {
            QCC_LogMsg(("device found in getDeviceState"));
            std::unique_ptr<getDeviceStateCommand> commandPtr(new getDeviceStateCommand);
            commandPtr->commandDataObj.deviceID = deviceID;
            commandPtr->commandDataObj.jsonPaylod = jsonPayload;

            commandPtr->setAllJoynManager(_alljoynmanager);
            commandPtr->setConfigManager(this);
            commandPtr->setHackRouterHooker(hackRouterHooker);
//            commandPtr->setDevicePtr(dmaCtrlPtr->getDeviceProxyPtr());

            pool->Enqueue(std::move(commandPtr));
            QCC_LogMsg(("command enqueued in getDeviceState"));

        }
//        else
//        {
//            json jsonData = json::parse(commandDataObj.jsonPaylod);
//            jsonData[RESPONSE_CODE] = 400;
//            jsonData[MESSAGE] = DEVICE_NOT_REGISTERED;
//            commandDataObj.jsonPaylod = jsonData.dump();
//        }

    }
    catch (std::exception &e)
    {
        QCC_LogMsg(("Error in the getDevicestate in DMA::  %s",e.what()));
        std::cout << "getDevicestate " << e.what() << std::endl;
    }
}

std::string ConfigSensorManager::getBusUniqueName()
{
    std::string uniqueName = busAttachPtr->GetUniqueName();
    return uniqueName;
}

DeviceProxy* ConfigSensorManager::getDeviceProxyPtr(string busName)
{
    return dmaCtrlPtr->getDeviceProxyPtr(busName);
}
