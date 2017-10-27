#include "commands.h"
#include "configSensorManager.h"
#include "CommonHeader/CPPHeaders.h"

#define QCC_MODULE "DMA_APP"
#define DMA_INTERFACE_NAME ""
DeviceProxy* command::getDeviceProxy(std::string busName)
{
    std::cout << "Bus Name in getDeviceProxy ::" << busName;
    DeviceProxy* proxyDevice(NULL);
    if(devConfigInfo)
    {
        proxyDevice = devConfigInfo->getDeviceProxyPtr(busName);
    }
    return proxyDevice;
}


void sendDataToCloud::execute()
{
    try
    {
        QCC_LogMsg(("reached in senddatatocloud command"));
        bool found = devConfigInfo->checkDeviceExist(deviceId);
        QCC_LogMsg(("reached in senddatatocloud command 1"));
        if(found)
        {
            QCC_LogMsg(("reached in senddatatocloud command 2"));
            std::shared_ptr<deviceInfo> devInfo = devConfigInfo->getDeviceFromList(deviceId);
            QCC_LogMsg(("reached in senddatatocloud command 3"));
            if(devInfo)
            {
                if(devInfo->getState() == deviceState::DEV_TRANSFERRED_OWNERSHIP )
                {
                    std::string deviceID = devInfo->getDeviceDest();
                    json jsonData = json::parse(this->commandDataObj.jsonPaylod);
                    jsonData[DEVICE_ID] = deviceId;
                    std::string data = jsonData.dump();
                    hackRouterHooker->GetRouterDeviceData(deviceID,data);
                    QCC_LogMsg(("Device Ownership transferd %s",deviceId.c_str()));
                }
                else
                {
                    sendToCloud();
                }
            }
        }
    }
    catch(std::exception &e)
    {
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        QCC_LogMsg((EXCEPTION_MSG,e.what()));
        std::cout<<"\nExceptions: Invalid json" << e.what() << std::endl;
    }
}

void registerTopicsCommand::execute()
{
    try
    {
        std::cout << "reached in registerTopicsCommand" << std::endl;
        usleep(300);
        std::cout << "came out of sleep reached in registerTopicsCommand" << std::endl;
        json jsonData = json::parse(commandDataObj.jsonPaylod);
        for (auto &jsonRef : jsonData[DEVICES]) 	// iterate the array
        {
            std::string deviceId = jsonRef[DEVICE_ID];
            requestManager->prepareDevIDTopics(deviceId);
        }
        std::cout << "came out of registerTopicsCommand" << std::endl;
    }
    catch(std::exception &e)
    {
        std::string msg = EXCEPTION_MSG;
        msg += "in registerTopicsCommand";
        QCC_LogMsg((msg.c_str(),e.what()));
        std::cout << "Error in the add device in DMA " << e.what() << std::endl;
    }
}

void registerDeviceCallback::execute()
{
    try
    {
        std::cout << "reached in registerDeviceCallback" << std::endl;
        json jsonData = json::parse(commandDataObj.jsonPaylod);
        for (auto &jsonRef : jsonData[DEVICES]) 	// iterate the array
        {
            std::string deviceId = jsonRef[DEVICE_ID];
            commandDataObj.protocolID = jsonRef[PROTOCOL_ID];
            if(!local)
            {
                std::string busName = jsonRef[BUSNAME];
                DeviceProxy* proxyDevice = getDeviceProxy(busName);
                if(proxyDevice)
                {
                    std::string jsonStr = jsonRef.dump();
                    proxyDevice->registerDeviceCallback(getTransportMask(), deviceId,jsonStr,
                                                        std::bind(&DataTaskHandler::deviceDataCallback, devConfigInfo->dataTaskHndler, std::placeholders::_1, std::placeholders::_2));
                }
            }
            else
            {
                std::string jsonValue = jsonRef.dump();
                auto devInfo = devConfigInfo->getDeviceFromList(deviceId);
                std::string busName = devInfo->getBusName();
                DeviceProxy* proxyDevice = getDeviceProxy(busName);
                if(proxyDevice)
                {
                    bool ret = proxyDevice->registerDeviceCallback(getTransportMask(),deviceId, jsonValue ,
                                                                              std::bind(&DataTaskHandler::deviceDataCallback, devConfigInfo->dataTaskHndler, std::placeholders::_1, std::placeholders::_2));
                    if(ret)
                    {
                        QCC_LogMsg(("successfully registered callback"));
                        std::cout << "successfully registered callback" << std::endl;
                    }
                }
            }
        }
        std::cout << "came out of registerDeviceCallback" << std::endl;
    }
    catch(std::exception &e)
    {
        std::string msg = EXCEPTION_MSG;
        msg += "in registerDeviceCallback";
        QCC_LogMsg((msg.c_str(),e.what()));
        std::cout << "Error in the add device in DMA " << e.what() << std::endl;
    }

}

void registerDeviceCallbackForRegisterDevice::execute()
{
    try
    {
        std::cout << "reached in registerDeviceCallbackForRegisterDevice" << std::endl;

        typedef std::map<std::string, std::shared_ptr<deviceInfo>>::iterator it_type;
        for(it_type iterator = devConfigInfo->deviceMapInfo->begin(); iterator != devConfigInfo->deviceMapInfo->end(); iterator++)
        {
            auto devInfo = iterator->second;
            json jsonRef = devInfo->getJsonPayload();

            std::string deviceId = jsonRef[DEVICE_ID];
            commandDataObj.protocolID = jsonRef[PROTOCOL_ID];
            std::string busName = devInfo->getBusName();
            std::string recBusName = commandDataObj.jsonPaylod;
            if(recBusName  == busName && devInfo->getState() == deviceState::DEV_OWNERSHIP_TAKEN)
            {
                std::cout << "reached in registerDeviceCallbackForRegisterDevice got router info" << std::endl;
                DeviceProxy* proxyDevice = getDeviceProxy(busName);
                if(proxyDevice)
                {
                    if(!devInfo->getRemoteDeviceRegisterd())
                    {
                        std::cout << "reached in registerDeviceCallbackForRegisterDevice Proxy device created" << std::endl;
                        std::string jsonStr = jsonRef.dump();
                        proxyDevice->registerDeviceCallback(getTransportMask(), deviceId,jsonStr,
                                                            std::bind(&DataTaskHandler::deviceDataCallback, devConfigInfo->dataTaskHndler, std::placeholders::_1, std::placeholders::_2));

                        devInfo->setRemoteDeviceRegisterd(true);
                    }
                    else
                    {
                        std::cout << "Device already registerd with Router -- in registerDeviceCallbackFor" << std::endl;
                    }
                }
                else
                {
                    std::cout << "DeviceProxy not found in DMA in  RegisterDevice" << std::endl;
                }
            }
            else
            {
                std::cout << "busname different than received" << std::endl;
            }
        }
        std::cout << "came out of registerDeviceCallback" << std::endl;
    }
    catch(std::exception &e)
    {
        std::string msg = EXCEPTION_MSG;
        msg += "in RegisterDeviceCallback DMA";
        QCC_LogMsg((msg.c_str(),e.what()));
        std::cout << "Error in the add device in DMA " << e.what() << std::endl;
    }

}

void TransferOwnershipCommand::execute()
{
    try
    {
        std::string jsonPayload = commandDataObj.jsonPaylod;
        json jsonData = json::parse(jsonPayload);

        json &deviceJson = jsonData[DEVICES];
        bool deviceFound(false);
        std::string deviceDest = "";
        deviceState state = deviceState::NORMAL;
        std::string busName;

        //Destination DMA ID extracted from json payload
        std::string deviceID ;
        std::string destinationAddr = jsonData["dgwid"];

        deviceID = "org.alljoyn.eIAJ.Router.mac" + destinationAddr + "DMA";
        ajDmaInfo::HawkRouterDeviceDataStruct routerStruct = hackRouterHooker->extractStructData(deviceID);

        if(routerStruct.deviceMacId.empty())
        {
            generateJson(400, DESTINATION_NOT_FOUND);
            sendToCloud();
            return;
        }

        //Check the devices exist in DMA and extract the properties and form the json
        for (json::iterator it = deviceJson.begin(); it != deviceJson.end(); ++it) 	// iterate the array
        {
            json &jsonRef = it.value();

            std::string devId = jsonRef[DEVICE_ID];

            if(devConfigInfo)
            {
                bool found = devConfigInfo->checkDeviceExist(devId);
                if(found)
                {
                    deviceFound = true;
                    std::shared_ptr<deviceInfo> devInfo = devConfigInfo->getDeviceFromList(devId);
                    commandDataObj.protocolID = devInfo->getProtocolID();
                    jsonRef = devInfo->getJsonPayload();
                    deviceDest = devInfo->getDeviceDest();
                    state = devInfo->getState();
                    busName = devInfo->getBusName();
                    QCC_LogMsg(("Device deregisterDeviceCallback success"));
                }
            }
        }

        if(!deviceFound)
        {
            jsonData[RESPONSE_CODE] = 400;
            jsonData[MESSAGE] = DEVICE_NOT_REGISTERED;
        }
        else
        {
            // Source DMA ID
            std::string sDevId = HAWKROUTER + configurationSettings::macId + COLON + configurationSettings::AppTopic;

            // Source Router ID
            std::string sRouterDevId = ROUTER_ADV_NAME_PREFIX + configurationSettings::wcmacId;

            // If the DMA has taken ownership of the device from other DMA and if the user is trying to transfer
            // the device to another DMA( not source)
            if(state == deviceState::DEV_OWNERSHIP_TAKEN && deviceDest != deviceID && !deviceDest.empty())
            {
                std::cout << "\reached in  state == deviceState::DEV_OWNERSHIP_TAKEN && deviceDest != deviceID" << std::endl;
                jsonData[SOURCE_DMA_DID] = deviceDest;
                jsonData[BUSNAME] = busName;
            }
            else
            {
                //First time transfer from original DMA to another DMA
                std::cout << "\reached in  state == NORMAL" << std::endl;
                jsonData[SOURCE_DMA_DID] = sDevId;
                jsonData[BUSNAME] = sRouterDevId;
            }

            std::string devicesList = jsonData.dump();

            //Notify the other DMA about the transfer of the ownership
            QStatus status = hackRouterHooker->signalTransferOwnership(deviceID,devicesList);
            if(status == ER_OK)
            {
                std::cout << "\nsignalTransferOwnership return success" << std::endl;
                //The transfer ownership is success and now update the properties in source DMA
                for (json::iterator it = deviceJson.begin(); it != deviceJson.end(); ++it)
                {
                    json &jsonRef = it.value();

                    std::string devId = jsonRef[DEVICE_ID];

                    if(devConfigInfo)
                    {
                        bool found = devConfigInfo->checkDeviceExist(devId);
                        if(found)
                        {
                            std::shared_ptr<deviceInfo> devInfo = devConfigInfo->getDeviceFromList(devId);
                            commandDataObj.protocolID = devInfo->getProtocolID();
                            std::string busName = devInfo->getBusName();
                            std::string devPayload = devInfo->getJsonPayload().dump();
                            DeviceProxy* proxyDevice = getDeviceProxy(busName);
                            std::cout << "\nBusName in TransferOwnership -- DeregisterDevicecallback" << busName <<  std::endl;
                            if(proxyDevice)
                            {
                                proxyDevice->deregisterDeviceCallback(getTransportMask(), commandDataObj.deviceID,
                                                                      devPayload);
                            }
                            //devInfo->removeFile(devId);
                            //devConfigInfo->deleteDeviceFromList(devId);
                        }
                    }
                }
            }
            jsonData = json::parse(devicesList);
        }
        commandDataObj.jsonPaylod = jsonData.dump();
        //send the response to cloud
        sendToCloud();
    }
    catch(std::exception &e)
    {
        QCC_LogMsg((EXCEPTION_MSG,e.what()));
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        std::cout<<"\nExceptions: Invalid json" << e.what() << std::endl;
    }
}

void TransferDevicesCommand::execute()
{
    try
    {
        std::string jsonPayload = commandDataObj.jsonPaylod;
        json jsonData = json::parse(jsonPayload);

        std::string deviceID ;
//        std::string macId = jsonData[DMACID];
//        std::string appName = jsonData[DAPPNAME];
        std::string destinationAddr = jsonData["dgwid"];
        deviceID = "org.alljoyn.eIAJ.Router.mac" + destinationAddr + "DMA";

        ajDmaInfo::HawkRouterDeviceDataStruct routerStruct = hackRouterHooker->extractStructData(deviceID);
        if(routerStruct.deviceMacId.empty())
        {
            generateJson(400, DESTINATION_NOT_FOUND);
            sendToCloud();
            return;
        }

        json &deviceJson = jsonData[DEVICES];
        bool deviceFound(false);

        for (json::iterator it = deviceJson.begin(); it != deviceJson.end(); ++it) 	// iterate the array
        {
            json &jsonRef = it.value();

            std::string devId = jsonRef[DEVICE_ID];

            if(devConfigInfo)
            {
                bool found = devConfigInfo->checkDeviceExist(devId);
                if(found)
                {
                    auto devInfo = devConfigInfo->getDeviceFromList(devId);
                    commandDataObj.protocolID = devInfo->getProtocolID();
                    jsonRef = devInfo->getJsonPayload();
                    TransportMask tMask(getTransportMask());
                    bool success = false;
                    std::string jsonValue = jsonRef.dump();

                    sendProgressResponse(HARD_TRANSFER_DELETE_INPRG);

                    std::string busName = devInfo->getBusName();
                    DeviceProxy *proxyDevice = getDeviceProxy(busName);
                    if(proxyDevice)
                        success = proxyDevice->deleteDevice(getTransportMask(), devId,jsonValue);
                    jsonRef = json::parse(jsonValue);
                    int responseCode =jsonRef[RESPONSE_CODE];
                    if(responseCode == 200)
                    {
                        std::cout << "\nreached inside delete device from dma"<< std::endl;
                        if(devConfigInfo)
                        {
                            devInfo->removeFile(devId);
                            devConfigInfo->deleteDeviceFromList(devId);
                        }
                        if(tMask != TRANSPORT_EI_ALLJOYN)
                        {
                            QCC_LogMsg(("reached inside delete device from dma 2"));
                            if(proxyDevice)
                            {
                                bool ret = proxyDevice->deregisterDeviceCallback(tMask, devId, jsonValue);
                                if(ret)
                                {
                                    deviceFound = true;
                                    QCC_LogMsg(("device successfully deregistered"));
                                }
                            }
                        }
                    }
                }
            }
        }

        if(!deviceFound)
        {
            generateJson(400, DEVICE_NOT_REGISTERED);
            sendToCloud();
        }
        else
        {
            // Source DMA ID
            std::string sDevId = HAWKROUTER + configurationSettings::macId + COLON + configurationSettings::AppTopic;

            jsonData[SOURCE_DMA_DID] = sDevId;
            jsonData[DEST_CLOUD_URL] = commandDataObj.destinationMQttURL;

            std::string devicesList = jsonData.dump();

            QStatus status = hackRouterHooker->signalTransferDevices(deviceID,devicesList);
            if(status == ER_OK)
            {
                QCC_LogMsg(("signal transferred success"));
                std::cout << "signal send transferred success";
            }
            else
            {
                jsonData = json::parse(devicesList);
                commandDataObj.jsonPaylod = jsonData.dump();
                sendToCloud();
            }
        }
    }
    catch(std::exception &e)
    {
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        QCC_LogMsg(("Exceptions: Invalid json :: %s",e.what()));
        std::cout<<"\nExceptions: Invalid json" << e.what() << std::endl;
    }
}

void TransferDevicesCommand::sendProgressResponse(std::string msg)
{
    std::string destUrl = this->commandDataObj.destinationMQttURL + COMMANDS_DEVICESTATUS_AFTER;
    json payload = json::parse(commandDataObj.jsonPaylod);
    payload[MESSAGE] = msg;
    std::string payloadStr = payload.dump();

    if(this->cmdInfoCloudProxyShrdPtr)
        this->cmdInfoCloudProxyShrdPtr->sendDataToCloud(this->cldUniqueName,destUrl,payloadStr);
}

void getGatewayStatusCommand::execute()
{
    try
    {
        std::string jsonPayload = commandDataObj.jsonPaylod;
        json jsonData = json::parse(jsonPayload);
        jsonData[RESPONSE_CODE] = 200;
        bool gatewayStatus(devConfigInfo->getGatewayStatus());
        if(gatewayStatus)
            jsonData[STATUS] = ACTIVE;
        else
            jsonData[STATUS] = DEACTIVE;
        time_t now;
        time(&now);
        char timebuf[sizeof "2011-10-08T07:07:09Z"];
        strftime(timebuf, sizeof timebuf, "%FT%TZ", gmtime(&now));
        jsonData[TIMESTAMP] = timebuf;
        commandDataObj.jsonPaylod = jsonData.dump();
        QCC_LogMsg(("getGatewayStatusCommand created"));
        sendToCloud();
    }
    catch(std::exception &e)
    {
        QCC_LogMsg(("Exceptions: Invalid json :: %s",e.what()));
        std::cout<<"\nExceptions: Invalid json" << e.what() << std::endl;
    }
}

void putGatewayStatusCommand::execute()
{
    try
    {
        std::string jsonPayload = commandDataObj.jsonPaylod;
        json jsonData = json::parse(jsonPayload);
        std::string status = jsonData[STATUS];
        time_t now;
        time(&now);
        char timebuf[sizeof "2011-10-08T07:07:09Z"];
        strftime(timebuf, sizeof timebuf, "%FT%TZ", gmtime(&now));
        if(status.compare(ACTIVE) == 0)
        {
            jsonData[RESPONSE_CODE] = 200;
            jsonData[TIMESTAMP] = timebuf;
            devConfigInfo->setGatewayStatus(true);
        }
        else if(status.compare(DEACTIVE) == 0)
        {
            jsonData[RESPONSE_CODE] = 200;
            jsonData[TIMESTAMP] = timebuf;
            devConfigInfo->setGatewayStatus(false);
        }
        else
        {
            jsonData[RESPONSE_CODE] = 400;
            jsonData[TIMESTAMP] = timebuf;
            jsonData[MESSAGE] = "invalid status field";
        }
        commandDataObj.jsonPaylod = jsonData.dump();
        QCC_LogMsg(("setGatewayStatusCommand created"));
        sendToCloud();
    }
    catch(std::exception &e)
    {
        QCC_LogMsg((EXCEPTION_MSG,e.what()));
        std::cout<<"\nExceptions: Invalid json" << e.what() << std::endl;
    }
}

void deleteDeviceCommand::execute()
{
    try
    {
        std::cout << "reached in delete device command" << std::endl;
        std::string jsonPayload = commandDataObj.jsonPaylod;
        json jsonData = json::parse(jsonPayload);

        json &deviceJson = jsonData[DEVICES];

        for (json::iterator it = deviceJson.begin(); it != deviceJson.end(); ++it) 	// iterate the array
        {
            json &jsonRef = it.value();

            std::string deviceId = jsonRef[DEVICE_ID];
            std::string jsonValue = jsonRef.dump();

            if(devConfigInfo)
            {
                bool found = devConfigInfo->checkDeviceExist(deviceId);
                if(found)
                {
                    auto devInfo = devConfigInfo->getDeviceFromList(deviceId);
                    commandDataObj.protocolID = devInfo->getProtocolID();
                    TransportMask tMask(getTransportMask());
                    bool success = false;
                    if(tMask == TRANSPORT_EI_ALLJOYN)
                    {
                        alljoynAppManager->DeleteDevice(deviceId,jsonValue);
                        QCC_LogMsg(("INFO : DeleteDevice JSON - ",jsonValue.c_str()));
                        success = true;
                    }
                    else
                    {
                        std::string busName = devInfo->getBusName();
                        DeviceProxy* proxyDevice = getDeviceProxy(busName);
                        if(proxyDevice)
                        {
                            success = proxyDevice->deleteDevice(tMask,deviceId,jsonValue);
                        }
                    }
                    jsonRef = json::parse(jsonValue);
                    int responseCode =jsonRef[RESPONSE_CODE];
                    if(responseCode == 200)
                    {
                        QCC_LogMsg(("reached inside delete device from dma"));
                        if(devConfigInfo)
                        {
                            devInfo->removeFile(deviceId);
                            devConfigInfo->deleteDeviceFromList(deviceId);
                        }
                        if(tMask != TRANSPORT_EI_ALLJOYN)
                        {
                            QCC_LogMsg(("reached inside delete device from dma 2"));
                            std::string busName = devInfo->getBusName();
                            DeviceProxy* proxyDevice = getDeviceProxy(busName);
                            if(proxyDevice)
                            {
                                bool ret = proxyDevice->deregisterDeviceCallback(tMask,deviceId, jsonValue);
                                if(ret)
                                {
                                    QCC_LogMsg(("device successfully deregistered"));
                                }
                            }
                        }
                    }
                }
                else
                {
                    jsonRef[RESPONSE_CODE] = 400;
                    jsonRef[MESSAGE] = DEVICE_NOT_REGISTERED;
                }
            }
        }

        jsonData[RESPONSE_CODE] = 200;
        commandDataObj.jsonPaylod = jsonData.dump();

        QCC_LogMsg(("Json output to cloud ::: %s",commandDataObj.jsonPaylod.c_str()));
        sendToCloud();
    }
    catch(std::exception &e)
    {
        QCC_LogMsg((EXCEPTION_MSG,e.what()));
    }
}



void hardTransferCommand::discoverDevices(std::map<std::string, json> deviceList, std::string destMQTTUrl, json jsonPayload,DeviceProxy* deviceProxy)
{
    flag = false;
    for (auto &jsonRef : jsonPayload[DEVICES]) 	// iterate the array
    {
        std::string protocolID = jsonRef[PROTOCOL_ID];

        auto it = deviceList.find(protocolID);
        if(it ==  deviceList.end())
        {
            //Send the Progress response to cloud for discover started in gateway

            sendProgressResponse(destMQTTUrl,commandDataObj.jsonPaylod, HARD_TRANSFER_INPRG);

            deviceList.insert(std::make_pair(protocolID,jsonRef));
            commandDataObj.protocolID = protocolID;
            TransportMask pType = getTransportMask();
            std::string jsonStr = jsonRef.dump();
            deviceProxy->discover(pType, jsonStr,
                                             std::bind(&hardTransferCommand::DiscoverDeviceCallback, this, std::placeholders::_1, std::placeholders::_2));

            int count(0);

            while(!flag)
            {
                QCC_LogMsg(("discover while loop running"));
                if(count >= DISCOVER_TIME_OUT)
                {
                    break;
                }
                sleep(1);
                count++;
            }
            QCC_LogMsg(("came out of while loop"));
        }
    }
}

void hardTransferCommand::execute()
{
    try
    {
        flag = false;
        json jsonPayload = json::parse(commandDataObj.jsonPaylod);

        std::map<std::string, json> deviceList;
        std::vector<std::string> devicesMacList;

        std::string dmaName = DMA_INTERFACE_NAME;
        dmaName += ".GW";
        dmaName += configurationSettings::gatewayId;
        dmaName += ".APP";
        dmaName += configurationSettings::AppTopic;

        DeviceProxy *devShrdPtr = devConfigInfo->getDeviceProxyPtr(dmaName);

        std::string sourceDMA = jsonPayload[SOURCE_DMA_DID];
        std::string destMQTTUrl = jsonPayload[DEST_CLOUD_URL];

        for(auto &jsonRef : jsonPayload[DEVICES]) 	// iterate the array
        {
            std::string macID = jsonRef[MAC_ID];
            devicesMacList.push_back(macID);
        }

        discoverDevices(deviceList, destMQTTUrl, jsonPayload,devShrdPtr);

        if(!discoveredDevices.empty())
        {
            bool found(false);
            int index(0);
            while(index < 3)
            {
                json jsonDiscovered = json::parse(discoveredDevices);
                int foundDev = 0;
                for(auto &jsonRef : jsonDiscovered[DEVICES]) 	// iterate the array
                {
                    std::string macId = jsonRef[MAC_ID];

                    std::vector<std::string>::iterator it;

                    it = find (devicesMacList.begin(), devicesMacList.end(), macId);
                    if (it != devicesMacList.end())
                    {
                        foundDev++;
                        std::cout << "Element found in devicesMacList: Mac id " << *it << std::endl;
                    }
                    else
                    {
                        std::cout << "Element not found in devicesMacList" << std::endl;
                    }

                    found = true;
                }
                if(found && foundDev == devicesMacList.size())
                {
                    std::cout << "Devices found in HardTransfer command " << std::endl;
                    break;
                }

                std::cout << "Retry in TransferDevices hard for discover " << std::endl;
                discoverDevices(deviceList, destMQTTUrl, jsonPayload,devShrdPtr);
                index++;
            }

            if(!found)
            {
                // Devices not discovered
                //Send the failure response to cloud
                jsonPayload[RESPONSE_CODE] = ERROR_RES;
                jsonPayload[MESSAGE] =  DISCOVERY_ERROR;
                commandDataObj.jsonPaylod = jsonPayload.dump();
                hackRouterHooker->SendResponseToCloud(sourceDMA,commandDataObj.jsonPaylod);
            }
            else
            {
                bool deviceRegistered(false);

                for(auto &jsonRef : jsonPayload[DEVICES]) 	// iterate the array
                {
                    std::string protocolID = jsonRef[PROTOCOL_ID];
                    commandDataObj.protocolID = protocolID;
                    std::string deviceId = jsonRef[DEVICE_ID];
                    TransportMask pType = getTransportMask();
                    std::string jsonStr = jsonRef.dump();
                    auto success = devShrdPtr->addDevice(pType, deviceId, jsonStr);

                    jsonRef = json::parse(jsonStr);
                    if(success)
                    {
                        int responseCode = jsonRef[RESPONSE_CODE];
                        if(responseCode == 200)
                        {
                            QCC_LogMsg(("reached in success adddevice"));
                            deviceInfo devInfo(jsonRef, protocolID);

                            devConfigInfo->addDeviceInList(deviceId,devInfo);

                            if(requestManager)
                                requestManager->prepareDevIDTopics(deviceId);

                            if(pType != TRANSPORT_EI_ALLJOYN)
                            {
                                QCC_LogMsg(("payload empty sent"));
                                std::string payload = "";
                                bool ret = devShrdPtr->registerDeviceCallback(pType,deviceId, payload ,
                                                                                          std::bind(&DataTaskHandler::deviceDataCallback, devConfigInfo->dataTaskHndler, std::placeholders::_1, std::placeholders::_2));
                                if(ret)
                                {
                                    QCC_LogMsg(("successfully registered callback"));
                                    deviceRegistered = true;
                                }
                                devShrdPtr->setDeviceStatus(pType, deviceId,jsonStr);
                            }
                        }
                    }
                }

                if(!deviceRegistered)
                {
                    // device registeration failed in router
                    //Send response to Cloud for Failure Transfer

                    jsonPayload[RESPONSE_CODE] = ERROR_RES;
                    jsonPayload[MESSAGE] =  DEVICE_REGISTRATION_FAILED;
                    commandDataObj.jsonPaylod = jsonPayload.dump();
                    hackRouterHooker->SendResponseToCloud(sourceDMA,commandDataObj.jsonPaylod);
                }
                else
                {
                    // device registeration success in router
                    //Send response to Cloud for Success Transfer
                    jsonPayload[RESPONSE_CODE] = 200;
                    jsonPayload[MESSAGE] =  HARD_TRANSFER_SUCCESS;
                    commandDataObj.jsonPaylod = jsonPayload.dump();
                    hackRouterHooker->SendResponseToCloud(sourceDMA,commandDataObj.jsonPaylod);
                }
            }
        }
        else
        {
            // Not received the discover device callback from router
            //Send response to Cloud for failure
            jsonPayload[RESPONSE_CODE] = ERROR_RES;
            jsonPayload[MESSAGE] =  DISCOVER_CALLBACK_NOT_RECD;
            commandDataObj.jsonPaylod = jsonPayload.dump();
            hackRouterHooker->SendResponseToCloud(sourceDMA,commandDataObj.jsonPaylod);
        }
    }
    catch(std::exception &e)
    {
        json jsonPayload = json::parse(commandDataObj.jsonPaylod);
        jsonPayload[RESPONSE_CODE] = ERROR_RES;
        jsonPayload[MESSAGE] =  CAUGHT_EXCEPTION;
        std::string sourceDMA = jsonPayload[SOURCE_DMA_DID];
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        hackRouterHooker->SendResponseToCloud(sourceDMA,commandDataObj.jsonPaylod);

        //Exception caught in hardTransferDevices command
        //Send the failure response to cloud
        QCC_LogMsg((EXCEPTION_MSG,e.what()));
    }
}

void hardTransferCommand::sendProgressResponse(std::string url, std::string payload , std::string msg)
{
    json jpayload = json::parse(payload);
    jpayload[MESSAGE] = msg;
    std::string payloadStr = jpayload.dump();
    std::string mqtturl = url + COMMANDS_DEVICESTATUS_AFTER;

    if(this->cmdInfoCloudProxyShrdPtr)
        this->cmdInfoCloudProxyShrdPtr->sendDataToCloud(this->cldUniqueName,mqtturl,payloadStr);
}

void hardTransferCommand::DiscoverDeviceCallback(std::string data, std::string msg)
{
    QCC_LogMsg(("Discover command callback reached"));
    QCC_LogMsg(("Discover data %s ", data.c_str()));
    QCC_LogMsg(("Discover msg %s ", msg.c_str()));
    discoveredDevices = data;
    flag = true;
}

void takeOwnershipCommand::execute()
{
    try
    {
        // This function will register the transferred device from other gateways dma apps or loca gateway
        // dma apps.
        json jsonData = json::parse(commandDataObj.jsonPaylod);
        QCC_LogMsg(("reached in registerTransferredDevice"));
        bool executeCommand(false);
        bool localRegister(false);
        for (auto &jsonRef : jsonData[DEVICES]) 	// iterate the array
        {
            std::string deviceId = jsonRef[DEVICE_ID];
            std::string protocolID = jsonRef[PROTOCOL_ID];
            bool found = devConfigInfo->checkDeviceExist(deviceId);
            if(found) // Device is earlier transferred from this dma app and transferred back to original app
            {
                std::cout << "Device found in registerTransferredDevice" << std::endl;
                QCC_LogMsg(("found device in registerTransferredDevice"));
                auto devInfo = devConfigInfo->getDeviceFromList(deviceId);
                devInfo->setState(deviceState::NORMAL);
                localRegister = true;
            }
            else
            {
                // Device is not transferred from this dma app and now this register the device in DMA app.
                std::cout << "Device not found in registerTransferredDevice" << std::endl;
                QCC_LogMsg(("not found device in registerTransferredDevice"));
                deviceInfo devInfo(jsonRef, protocolID);
                devInfo.setState(deviceState::DEV_OWNERSHIP_TAKEN);
                std::string sDevId = jsonData[SOURCE_DMA_DID];
                devInfo.setDeviceDest(sDevId);

                std::string busName = jsonData[BUSNAME];
                devInfo.setBusName(busName);

                devConfigInfo->addDeviceInList(deviceId,devInfo);
                executeCommand = true;
            }
        }

        if(executeCommand)
        {
            // This command will register the topics for newly added device to listen for Cloud control.
            std::unique_ptr<registerTopicsCommand> commandPtr(new registerTopicsCommand);
            commandPtr->commandDataObj.jsonPaylod = commandDataObj.jsonPaylod;
            devConfigInfo->handleCommand(std::move(commandPtr));
            QCC_LogMsg(("after addDeviceInList in registerTransferredDevice"));

            // This command will register the device callbacks in Remote Device Object to receive the telemetry.
            std::unique_ptr<registerDeviceCallback> regCommandDev(new registerDeviceCallback);
            regCommandDev->commandDataObj.jsonPaylod = commandDataObj.jsonPaylod;
            devConfigInfo->handleCommand(std::move(regCommandDev));
        }

        if(localRegister)
        {
            // This command will register the device callbacks in Remote Device Object to receive the telemetry.
            std::unique_ptr<registerDeviceCallback> regCommandDev(new registerDeviceCallback);
            regCommandDev->commandDataObj.jsonPaylod = commandDataObj.jsonPaylod;
            regCommandDev->local = true;
            regCommandDev->setDevicePtr(cmdInfoDevProxyShrdPtr);
            devConfigInfo->handleCommand(std::move(regCommandDev));
        }
    }
    catch(std::exception &e)
    {
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        QCC_LogMsg(("Error in the add device in DMA::  %s",e.what()));
    }
}

void addDeviceCommand::execute()
{
    try
    {
        TransportMask tMask(getTransportMask());
        if(tMask != TRANSPORT_NONE)
        {
            std::string jsonPayload = commandDataObj.jsonPaylod;
            json jsonData = json::parse(jsonPayload);

            json &deviceJson = jsonData[DEVICES];

            for (json::iterator it = deviceJson.begin(); it != deviceJson.end(); ++it) 	// iterate the array
            {
                json &jsonRef = it.value();

                std::string deviceId = jsonRef[DEVICE_ID];
                std::string jsonValue = jsonRef.dump();
                QCC_LogMsg(("INFO : AddDevice JSON  %s", jsonValue.c_str()));

                bool success(false);
                if(tMask == TRANSPORT_EI_ALLJOYN)
                {
                    success = true;
                    alljoynAppManager->AddDevice(deviceId,jsonValue);
                    QCC_LogMsg(("INFO : AddDevice JSON  %s", jsonValue.c_str()));
                }
                else
                {
                    success = cmdInfoDevProxyShrdPtr->addDevice(getTransportMask(), deviceId,jsonValue);
                }

                jsonRef = json::parse(jsonValue);
                if(success)
                {
                    int responseCode = jsonRef[RESPONSE_CODE];
                    if(responseCode == 200)
                    {
                        QCC_LogMsg(("reached in success adddevice"));
                        deviceInfo devInfo(jsonRef, commandDataObj.protocolID);
                        if(devConfigInfo)
                            devConfigInfo->addDeviceInList(deviceId,devInfo);

                        if(requestManager)
                            requestManager->prepareDevIDTopics(deviceId);
                        if(tMask != TRANSPORT_EI_ALLJOYN)
                        {
                            bool ret = cmdInfoDevProxyShrdPtr->registerDeviceCallback(getTransportMask(),deviceId, jsonValue ,
                                                                                      std::bind(&DataTaskHandler::deviceDataCallback, devConfigInfo->dataTaskHndler, std::placeholders::_1, std::placeholders::_2));
                            if(ret)
                            {
                                QCC_LogMsg(("successfully registered callback"));
                                std::cout << "successfully registered callback" << std::endl;
                            }
                        }

                        jsonValue = jsonRef.dump();
                        jsonRef = json::parse(jsonValue);
                    }
                }
            }

            jsonData[RESPONSE_CODE] = 200;
            commandDataObj.jsonPaylod = jsonData.dump();

            QCC_LogMsg(("Json output to cloud ::: %s",commandDataObj.jsonPaylod.c_str()));

            sendToCloud();

            for (json::iterator it = deviceJson.begin(); it != deviceJson.end(); ++it) 	// iterate the array
            {
                json &jsonRef = it.value();

                std::string deviceId = jsonRef[DEVICE_ID];
                std::string jobId = jsonData[JOBID];
                std::string jsonValue = jsonRef.dump();
                QCC_LogMsg(("Json value ::: %s",jsonValue.c_str()));

                bool success(false);
                if(tMask == TRANSPORT_EI_ALLJOYN)
                {
                    success = true;
                    alljoynAppManager->GetDeviceRAML(deviceId,jsonValue);
                    QCC_LogMsg(("INFO : AddDevice JSON :: :: %s",jsonValue.c_str()));
                }
                else
                {
                    success = cmdInfoDevProxyShrdPtr->getDeviceRAML(getTransportMask(), deviceId,jsonValue);

                    jsonRef = json::parse(jsonValue);
                    std::string ramlcontent = jsonRef[RAML];
                    QCC_LogMsg(("raml content :: :: %s",ramlcontent.c_str()));
                    auto appidByPass = this->commandDataObj.destinationMQttURL.substr(0, configurationSettings::dmTopic.size());
                    QCC_LogMsg(("raml content :: :: %s",appidByPass.c_str()));
                    this->commandDataObj.destinationMQttURL = appidByPass + DEVICES_ENTITY_FIRST + SLASH + deviceId + SLASH + RAML_GET_REPLY + jobId ;
                    QCC_LogMsg(("raml content :: :: %s",this->commandDataObj.destinationMQttURL.c_str()));
                    this->commandDataObj.jsonPaylod = ramlcontent;
                    sendToCloud();
                }
            }
        }
        else
        {
            generateJson(ERROR_RES,TRANSPORT_INVALID);
            sendToCloud();
        }
    }
    catch(std::exception &e)
    {
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        QCC_LogMsg((EXCEPTION_MSG,e.what()));
        std::cout<<"\nExceptions: Invalid json in adddevice command" << e.what() << std::endl;
    }
}

void deviceListCommand::execute()
{
    try
    {
        json jsonPay = json::parse(commandDataObj.jsonPaylod);
        json objArray = json::array();
        typedef std::map<std::string, std::shared_ptr<deviceInfo>>::iterator it_type;
        for(it_type iterator = devConfigInfo->deviceMapInfo->begin(); iterator != devConfigInfo->deviceMapInfo->end(); iterator++)
        {
            auto devInfo = iterator->second;
            json jsonRef = devInfo->getJsonPayload();
            objArray += jsonRef;
        }
        jsonPay[DEVICES] = objArray;
        commandDataObj.jsonPaylod = jsonPay.dump();
        sendToCloud();
    }
    catch(std::exception &e)
    {
        std::cout << "Caught exception in deviceListCommand :: "<< e.what() << std::endl;
    }
}

void getAppRAMLCommand::execute()
{
    try
    {
        json jsonPay = json::parse(commandDataObj.jsonPaylod);
        commandDataObj.jsonPaylod = jsonPay.dump();
        sendToCloud();
    }
    catch(std::exception &e)
    {
        std::cout << "Caught exception in getAppRAMLCommand :: "<< e.what() << std::endl;
    }
}

void discoverCommand::execute()
{
    try
    {
        std::cout << "Discover command reached" << std::endl;

        TransportMask tMask(getTransportMask());
        std::cout << "tranport mask value " << tMask << std::endl;
        if(tMask != TRANSPORT_NONE)
        {
            std::cout << "reached inside discover command"<< std::endl;
            if(tMask == TRANSPORT_EI_ALLJOYN)
            {
                std::cout << "Transport is Alljoyn in discover command "<< std::endl;
                alljoynAppManager->Discover(commandDataObj.jsonPaylod);
                std::cout << "INFO : DISCOVER JSON - " << commandDataObj.jsonPaylod << std::endl;
            }
            else
            {
                flag=false;
                std::cout << "command jobid " << commandDataObj.jobID << std::endl;
                std::cout << "command receiving topic " <<  commandDataObj.receivingTopic << std::endl;
                std::cout << "command destination mqtt url " << commandDataObj.destinationMQttURL << std::endl;
                std::cout << "command protocol id " << commandDataObj.protocolID << std::endl;

                std::string busName =  "org.alljoyn.eIAJ.Router.mac" + configurationSettings::gatewayId + "DMA";
                DeviceProxy* deviceProxy = getDeviceProxy(busName);
                if(deviceProxy)
                {
                    deviceProxy->discover(tMask, commandDataObj.jsonPaylod,
                                          std::bind(&discoverCommand::DiscoverDeviceCallback, this, std::placeholders::_1, std::placeholders::_2));

                    int count(0);
                    bool reachedLimit(false);
                    while(!flag)
                    {
                        std::cout << "waiting for discover response from router or library" << std::endl;
                        if(count >= DISCOVER_TIME_OUT)
                        {
                            reachedLimit = true;
                            break;
                        }
                        sleep(1);
                        count++;
                    }
                    std::cout << "came out of waiting for discover response" << std::endl;
                    if(reachedLimit)
                    {
                        generateJson(ERROR_RES,DISCOVER_CALLBACK_NOT_RECD);
                        std::cout << "Discover command waiting period reached limit :: " << commandDataObj.jsonPaylod << std::endl;
                    }
                }
                else
                {
                    std::cout << "Device Proxy not found for busName :: "<< busName << std::endl;
                    generateJson(ERROR_RES,DEVICEPROXY_NOT_FOUND);
                }
            }
        }
        else
        {
            std::cout << "reached inside transport invalid discover command"<< std::endl;
            generateJson(ERROR_RES,TRANSPORT_INVALID);
        }
        sendToCloud();
    }
    catch(std::exception &e)
    {
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        std::cout << "Exception Caught in discover command :: "<< e.what() << std::endl;
    }
}

void discoverCommand::DiscoverDeviceCallback(std::string data, std::string msg)
{
    std::cout << "Discover command callback reached" << std::endl;

    std::cout << "Discover data" << data << std::endl;

    std::cout << "Discover msg" << msg << std::endl;

    std::cout << "cld uniquename" << this->cldUniqueName << std::endl;

    std::cout << "Destinationmqtturl::::" << this->commandDataObj.destinationMQttURL << std::endl;

    this->commandDataObj.jsonPaylod = data;

    flag = true;
}



void restorePersitentDevices::execute()
{
    try
    {
        std::string filePath = JSON_PATH_DEVICES + configurationSettings::AppTopic + SLASH;

        if(!directoryOPS::checkExistence(filePath))
        {
            directoryOPS::createDir(filePath, 0755);
            return;
        }
        else if(directoryOPS::checkExistence(filePath)) {
            std::vector<std::string> vecBuf;

            std::string strPath = filePath;

            directoryOPS::returnDirectoryFilePath(strPath, vecBuf);

            std::map<std::string, json> deviceList;

            for(auto &filePath: vecBuf)
            {
                json jsonObj;

                FileOPS::readFromFile(filePath, jsonObj);

                std::string protocol = jsonObj[PROTOCOL_ID];
                std::string ownershipTaken = jsonObj[DEVICESTATE];

                auto it = deviceList.find(protocol);
                if(it ==  deviceList.end())
                {
                    if(ownershipTaken.compare("OWN_TAKEN") != 0)
                    {
                        deviceList.insert(std::make_pair(protocol,jsonObj));
                        discoverFromFile(jsonObj);
                    }
                }
            }

            for(auto &filePath: vecBuf)
            {
                //std::cout << filePath << std::endl;

                json jsonObj;

                FileOPS::readFromFile(filePath, jsonObj);

                addDevices(jsonObj);
                activateDevices(jsonObj);
            }
        }
    }
    catch(std::exception &e)
    {
        QCC_LogMsg(("Exceptions: Invalid json:: %s",e.what()));
    }
}

void restorePersitentDevices::activateDevices(json &jsonPayLoad)
{
    try
    {
        std::string jsonStr= jsonPayLoad.dump();;
        std::string devID = jsonPayLoad[DEVICE_ID];

        auto devInfo = devConfigInfo->getDeviceFromList(devID);
        if(devInfo)
        {

            if(devInfo->getState() == deviceState::DEV_OWNERSHIP_TAKEN)
                return;

            std::string protocolID = devInfo->getProtocolID();

            TransportMask tMask(transportType(protocolID));
            if(tMask == TRANSPORT_EI_ALLJOYN)
            {
//                alljoynAppManager->SetDeviceStatus(commandDataObj.deviceID,commandDataObj.jsonPaylod);
                QCC_LogMsg(("INFO : SetDeviceStatus JSON -"));
            }
            else
            {
                cmdInfoDevProxyShrdPtr->setDeviceStatus(tMask, devID, jsonStr);
            }
        }
        else
        {
            QCC_LogMsg(("Configuration sensor manager"));
        }
    }
    catch(std::exception &e)
    {
        QCC_LogMsg(("Exceptions: Invalid json:: %s",e.what()));
    }
}

void restorePersitentDevices::addDevices(json &jsonRef)
{
    try
    {
        std::string proto = jsonRef[PROTOCOL_ID];
        TransportMask tMask  = transportType(proto);

        deviceInfo devInfo(jsonRef, proto);

        std::string deviceId = jsonRef[DEVICE_ID];
        std::string jsonValue = jsonRef.dump();

        std::string devState = jsonRef[DEVICESTATE];
        if(devState.compare("NORMAL") == 0)
            devInfo.setState(deviceState::NORMAL);
        else if(devState.compare("TRANSFERED") == 0)
            devInfo.setState(deviceState::DEV_TRANSFERRED_OWNERSHIP);
        else
            devInfo.setState(deviceState::DEV_OWNERSHIP_TAKEN);

        std::string deviceDest = jsonRef[DEVICEDEST];
        devInfo.setDeviceDest(deviceDest);

        std::string destMQTTUrl = jsonRef[DEST_MQTT_URL];
        devInfo.setDestMQTTUrl(destMQTTUrl);

        std::string busName = jsonRef[BUSNAME];
        devInfo.setBusName(busName);

        devConfigInfo->addDeviceInList(deviceId,devInfo);

        requestManager->prepareDevIDTopics(deviceId);

        if(devInfo.getState() == deviceState::DEV_OWNERSHIP_TAKEN)
        {
            std::cout << "IN Device Ownership Taken addDevices" << std::endl;
            if(tMask != TRANSPORT_EI_ALLJOYN)
            {
                DeviceProxy* proxyDevice = getDeviceProxy(busName);
                if(proxyDevice)
                {
                    bool ret = proxyDevice->registerDeviceCallback(tMask,deviceId, jsonValue ,
                                                                   std::bind(&DataTaskHandler::deviceDataCallback, devConfigInfo->dataTaskHndler, std::placeholders::_1, std::placeholders::_2));
                    if(ret)
                    {
                        devInfo.setRemoteDeviceRegisterd(true);
                        std::cout << "Device Ownership taken register with router" << std::endl;
                        QCC_LogMsg(("successfully registered callback"));
                    }
                    else
                    {
                        std::cout << "Device Ownership taken register failed with router" << std::endl;
                    }
                }
                else
                {
                    std::cout << "Device Ownership taken -- DeviceProxy not found" << std::endl;
                }
            }
            return;
        }

        QCC_LogMsg(("Json value ::: %s",jsonValue.c_str()));
        bool success(false);
        if(tMask == TRANSPORT_EI_ALLJOYN)
        {
            success = true;
            //            alljoynAppManager->AddDevice(deviceId, jsonValue);
            QCC_LogMsg(("INFO : AddDevice JSON - ::: %s",jsonValue.c_str()));
        }
        else
        {
            success = cmdInfoDevProxyShrdPtr->addDevice(tMask, deviceId, jsonValue);
        }

        jsonRef = json::parse(jsonValue);
        if(success)
        {
            int responseCode = jsonRef[RESPONSE_CODE];
            if(responseCode == 200)
            {
                QCC_LogMsg(("reached in success adddevice"));

                if(tMask != TRANSPORT_EI_ALLJOYN)
                {
                    if(devInfo.getState() == deviceState::NORMAL)
                    {
                        bool ret = cmdInfoDevProxyShrdPtr->registerDeviceCallback(tMask,deviceId, jsonValue ,
                                                                      std::bind(&DataTaskHandler::deviceDataCallback, devConfigInfo->dataTaskHndler, std::placeholders::_1, std::placeholders::_2));
                        if(ret)
                        {
                            QCC_LogMsg(("successfully registered callback"));
                        }
                    }
                }
            }
        }
    }
    catch(std::exception &e)
    {
        QCC_LogMsg(("Exceptions: Invalid json:: %s",e.what()));
    }
}

void restorePersitentDevices::discoverFromFile(json &jsonPayload)
{
    try
    {
        flag=false;

        std::string protocolType = jsonPayload[PROTOCOL_ID];

        std::string jsonStr = jsonPayload.dump();
        TransportMask pType = transportType(protocolType);

        bool success = cmdInfoDevProxyShrdPtr->discover(pType, jsonStr,
                                            std::bind(&restorePersitentDevices::discoverCallBack, this, std::placeholders::_1, std::placeholders::_2));

        int count(0);

        while(!flag)
        {
            QCC_LogMsg(("waiting for discover response from router or library"));
            if(count >= DISCOVER_TIME_OUT)
            {
                break;
            }
            sleep(1);
            count++;
        }
        QCC_LogMsg(("came out of waiting for discover response from router or library"));
    }
    catch(std::exception &e)
    {
        QCC_LogMsg(("Exceptions: Invalid json:: %s",e.what()));
    }
}

void restorePersitentDevices::discoverCallBack(std::string data, std::string msg)
{
    flag = true;
}

TransportMask restorePersitentDevices::transportType(std::string &protocol)
{
    TransportMask tMask(TRANSPORT_NONE);
    if(protocol.compare(T_BLE) == 0)
        tMask = TRANSPORT_EI_BLE;
    else if(protocol.compare(T_ZIGBEE) == 0)
        tMask = TRANSPORT_EI_ZIGBEE;
    else if(protocol.compare(T_OPC) == 0)
        tMask = TRANSPORT_EI_OPC;
    else if(protocol.compare(T_ZWAVE) == 0)
        tMask = TRANSPORT_EI_ZWAVE;
    else if(protocol.compare(T_ALLJOYN) == 0)
        tMask = TRANSPORT_EI_ALLJOYN;

    return tMask;
}
