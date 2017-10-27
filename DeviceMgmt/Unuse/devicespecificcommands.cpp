#include "devicespecificcommands.h"
#include "commandbase.h"
#include "configSensorManager.h"

#define QCC_MODULE "DMA_APP"

void getDeviceStatusCommand::execute()
{
    try
    {
        if(devConfigInfo)
        {
            QCC_LogMsg(("device id returned::: :: %s",commandDataObj.deviceID.c_str()));
            auto devInfo = devConfigInfo->getDeviceFromList(commandDataObj.deviceID);
            if(devInfo)
            {
                commandDataObj.protocolID = devInfo->getProtocolID();
                QCC_LogMsg(("protocol id returned::: :: %s",commandDataObj.protocolID.c_str()));

                TransportMask tMask(getTransportMask());
                if(tMask == TRANSPORT_EI_ALLJOYN)
                {
                    if(alljoynAppManager)
                    {
                        alljoynAppManager->GetDeviceStatus(commandDataObj.deviceID,commandDataObj.jsonPaylod);
                        QCC_LogMsg(("INFO : SetDeviceStatus JSON -  %s",commandDataObj.jsonPaylod.c_str()));
                    }
                    else
                    {
                        generateJson(ERROR_RES,ALLJOYN_MANAGER_NOT_REGISTERED);
                    }
                }
                else
                {
                    std::string busName = devInfo->getBusName();
                    DeviceProxy* deviceProxy = getDeviceProxy(busName);
                    if(deviceProxy)
                    {
                        deviceProxy->getDeviceStatus(tMask, commandDataObj.deviceID,commandDataObj.jsonPaylod);
                    }
                    else
                    {
                        generateJson(ERROR_RES,DEIVCEPROXY_NOT_REGISTERED);
                    }
                }
            }
            else
            {
                generateJson(ERROR_RES,DEVICE_NOT_REGISTERED);
            }
        }
        else
        {
             generateJson(ERROR_RES,CONFIGSENSORMANAGER_NOT_REGISTERED);
        }
        sendToCloud();
    }
    catch(std::exception &e)
    {
         generateJson(ERROR_RES,CAUGHT_EXCEPTION);
         sendToCloud();
         QCC_LogMsg(("Exceptions: Invalid json :: %s",e.what()));
         std::cout<<"\nExceptions: Invalid json in getDeviceStatusCommand" << e.what() << std::endl;
    }
}


void putDeviceStatusCommand::execute()
{
    try
    {
        QCC_LogMsg(("deviceid returned- :: %s",commandDataObj.deviceID.c_str()));
        if(devConfigInfo)
        {
            auto devInfo = devConfigInfo->getDeviceFromList(commandDataObj.deviceID);
            if(devInfo)
            {
                commandDataObj.protocolID = devInfo->getProtocolID();

                TransportMask tMask(getTransportMask());
                if(tMask == TRANSPORT_EI_ALLJOYN)
                {
                    if(alljoynAppManager)
                    {
                        alljoynAppManager->SetDeviceStatus(commandDataObj.deviceID,commandDataObj.jsonPaylod);
                        QCC_LogMsg(("INFO : SetDeviceStatus JSON -",commandDataObj.jsonPaylod.c_str()));
                    }
                    else
                    {
                        generateJson(ERROR_RES,ALLJOYN_MANAGER_NOT_REGISTERED);
                    }
                }
                else
                {
                    std::string busName = devInfo->getBusName();
                    DeviceProxy* deviceProxy = getDeviceProxy(busName);
                    if(deviceProxy)
                    {
                        deviceProxy->setDeviceStatus(tMask, commandDataObj.deviceID,commandDataObj.jsonPaylod);
                    }
                    else
                    {
                        generateJson(ERROR_RES,DEIVCEPROXY_NOT_REGISTERED);
                    }
                }

                json jsonData = json::parse(commandDataObj.jsonPaylod);
                devInfo->setStatus(jsonData[STATUS]);
            }
            else
            {
                generateJson(ERROR_RES,DEVICE_NOT_REGISTERED);
            }
        }
        else
        {
            generateJson(ERROR_RES,CONFIGSENSORMANAGER_NOT_REGISTERED);
        }
        sendToCloud();
    }
    catch(std::exception &e)
    {
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        QCC_LogMsg(("Exceptions: Invalid json :: %s",e.what()));
        std::cout<<"\nExceptions: Invalid json in putDeviceStatusCommand" << e.what() << std::endl;
    }
}


void getDeviceRAMLCommand::execute()
{
    try
    {
        QCC_LogMsg(("deviceid returned- :: %s",commandDataObj.deviceID.c_str()));
        if(devConfigInfo)
        {
            auto devInfo = devConfigInfo->getDeviceFromList(commandDataObj.deviceID);
            if(devInfo)
            {
                commandDataObj.protocolID = devInfo->getProtocolID();
                TransportMask tMask(getTransportMask());
                if(tMask == TRANSPORT_EI_ALLJOYN)
                {
                    if(alljoynAppManager)
                    {
                        alljoynAppManager->GetDeviceRAML(commandDataObj.deviceID,commandDataObj.jsonPaylod);
                    }
                    else
                    {
                        generateJson(ERROR_RES,ALLJOYN_MANAGER_NOT_REGISTERED);
                    }
                }
                else
                {
                    std::string busName = devInfo->getBusName();
                    DeviceProxy* deviceProxy = getDeviceProxy(busName);
                    if(deviceProxy)
                    {
                        deviceProxy->getDeviceRAML(tMask, commandDataObj.deviceID,commandDataObj.jsonPaylod);
                    }
                    else
                    {
                         generateJson(ERROR_RES,DEIVCEPROXY_NOT_REGISTERED);
                    }
                }
                commandDataObj.jsonPaylod = convertSpecialCharacters(commandDataObj.jsonPaylod);
            }
            else
            {
                generateJson(ERROR_RES,DEVICE_NOT_REGISTERED);
            }
        }
        else
        {
            generateJson(ERROR_RES,CONFIGSENSORMANAGER_NOT_REGISTERED);
        }

        sendToCloud();
    }
    catch(std::exception &e)
    {
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        QCC_LogMsg(("Exceptions: Invalid json :: %s",e.what()));
        std::cout<<"\nExceptions: Invalid json in getDeviceRAMLCommand" << e.what() << std::endl;
    }
}


void putDeviceRAMLCommand::execute()
{
    try
    {
        QCC_LogMsg(("deviceid returned- :: %s",commandDataObj.deviceID.c_str()));
        std::cout << "device id returned in putDeviceRAML:::" << commandDataObj.deviceID << std::endl;
        if(devConfigInfo)
        {
            auto devInfo = devConfigInfo->getDeviceFromList(commandDataObj.deviceID);
            if(devInfo)
            {
                commandDataObj.protocolID = devInfo->getProtocolID();
                TransportMask tMask(getTransportMask());
                if(tMask == TRANSPORT_EI_ALLJOYN)
                {
                    if(alljoynAppManager)
                    {
                        alljoynAppManager->GetDeviceRAML(commandDataObj.deviceID,commandDataObj.jsonPaylod);
                    }
                    else
                    {
                        generateJson(ERROR_RES,ALLJOYN_MANAGER_NOT_REGISTERED);
                    }
                }
                else
                {
                    std::string busName = devInfo->getBusName();
                    DeviceProxy* deviceProxy = getDeviceProxy(busName);
                    if(deviceProxy)
                    {
                        deviceProxy->setDeviceRAML(tMask, commandDataObj.deviceID,commandDataObj.jsonPaylod);
                    }
                    else
                    {
                        generateJson(ERROR_RES,DEIVCEPROXY_NOT_REGISTERED);
                    }
                }
                commandDataObj.jsonPaylod = convertSpecialCharacters(commandDataObj.jsonPaylod);
            }
            else
            {
                generateJson(ERROR_RES,DEVICE_NOT_REGISTERED);
            }
        }
        else
        {
            generateJson(ERROR_RES,CONFIGSENSORMANAGER_NOT_REGISTERED);
        }

        sendToCloud();
    }
    catch(std::exception &e)
    {
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        QCC_LogMsg(("Exceptions: Invalid json :: %s",e.what()));
        std::cout<<"\nExceptions: Invalid json in putDeviceRAMLCommand" << e.what() << std::endl;
    }
}

void getDeviceStateCommand::execute()
{
    try
    {
        QCC_LogMsg(("deviceid returned- :: %s",commandDataObj.deviceID.c_str()));
        if(devConfigInfo)
        {
            auto devInfo = devConfigInfo->getDeviceFromList(commandDataObj.deviceID);
            if(devInfo)
            {
                commandDataObj.protocolID = devInfo->getProtocolID();
                devInfo->setDestMQTTUrl(commandDataObj.destinationMQttURL);
                QCC_LogMsg(("protocol id returned::: :: %s",commandDataObj.protocolID.c_str()));

                TransportMask tMask(getTransportMask());
                if(tMask == TRANSPORT_EI_ALLJOYN)
                {
                    if(alljoynAppManager)
                    {
                        alljoynAppManager->GetDeviceState(commandDataObj.deviceID,commandDataObj.jsonPaylod);
                        QCC_LogMsg(("INFO : GetDeviceState JSON -",commandDataObj.jsonPaylod.c_str()));
                    }
                    else
                    {
                        generateJson(ERROR_RES,ALLJOYN_MANAGER_NOT_REGISTERED);
                    }
                }
                else
                {
                    deviceState state = devInfo->getState();
                    switch(state)
                    {
                    case deviceState::NORMAL:
                    {
                        std::string busName = devInfo->getBusName();
                        DeviceProxy* deviceProxy = getDeviceProxy(busName);
                        if(deviceProxy)
                        {
                            deviceProxy->getDeviceState(getTransportMask(), commandDataObj.deviceID,
                                                                   commandDataObj.jsonPaylod);
                        }
                        else
                        {
                            generateJson(ERROR_RES,DEIVCEPROXY_NOT_REGISTERED);
                        }
                        sendToCloud();
                    }
                        break;
                    case deviceState::DEV_OWNERSHIP_TAKEN:
                    {
                        json jsonData = json::parse(this->commandDataObj.jsonPaylod);
                        jsonData[DEVICE_ID] = commandDataObj.deviceID;
                        commandDataObj.jsonPaylod = jsonData.dump();
                        if(devConfigInfo)
                        {
                            std::string busName = devInfo->getBusName();
                            DeviceProxy* proxyDevice = getDeviceProxy(busName);
                            if(proxyDevice)
                            {
                                proxyDevice->getDeviceState(getTransportMask(), commandDataObj.deviceID,
                                                            commandDataObj.jsonPaylod);
                                sendToCloud();
                                std::cout << "GetDeviceState Output:: " << commandDataObj.jsonPaylod;
                            }
                            else
                            {
                                generateJson(ERROR_RES,DEIVCEPROXY_NOT_REGISTERED);
                            }
                        }
                    }
                        break;
                    case deviceState::DEV_TRANSFERRED_OWNERSHIP:
                    {
                        std::string busName = devInfo->getBusName();
                        DeviceProxy* deviceProxy = getDeviceProxy(busName);
                        if(deviceProxy)
                        {
                            deviceProxy->getDeviceState(getTransportMask(), commandDataObj.deviceID,
                                                                   commandDataObj.jsonPaylod);
                        }
                        else
                        {
                            generateJson(ERROR_RES,DEIVCEPROXY_NOT_REGISTERED);
                        }

                        QCC_LogMsg(("INFO : reached in setdevicestate ownership transferred"));
                        std::string gatewayDest = devInfo->getDeviceDest();
                        if(hackRouterHooker)
                        {
                            hackRouterHooker->SendDeviceStateResponse( gatewayDest,commandDataObj.jsonPaylod);
                        }
                        else
                        {
                            generateJson(ERROR_RES,HACK_ROUTER_HOOKER_NOT_REGISTERED);
                        }
                        break;
                    }
                   }
                }
            }
            else
            {
                generateJson(ERROR_RES,DEVICE_NOT_REGISTERED);
                sendToCloud();
            }
        }
        else
        {
            generateJson(ERROR_RES,CONFIGSENSORMANAGER_NOT_REGISTERED);
            sendToCloud();
        }
    }
    catch(std::exception &e)
    {
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        QCC_LogMsg(("Exceptions: Invalid json :: %s",e.what()));
        std::cout<<"\nExceptions: Invalid json in getDeviceStateCommand" << e.what() << std::endl;
    }
}

void setDeviceStateCommand::execute()
{
    try
    {
        if(devConfigInfo)
        {
            QCC_LogMsg(("deviceid returned- :: %s",commandDataObj.deviceID.c_str()));
            auto devInfo = devConfigInfo->getDeviceFromList(commandDataObj.deviceID);
            if(devInfo)
            {
                commandDataObj.protocolID = devInfo->getProtocolID();
                devInfo->setDestMQTTUrl(commandDataObj.destinationMQttURL);
                QCC_LogMsg(("protocolID returned- :: %s",commandDataObj.protocolID.c_str()));

                TransportMask tMask(getTransportMask());
                if(tMask == TRANSPORT_EI_ALLJOYN)
                {
                    if(alljoynAppManager)
                    {
                        alljoynAppManager->SetDeviceState(commandDataObj.deviceID,commandDataObj.jsonPaylod);
                    }
                    else
                    {
                        generateJson(ERROR_RES,ALLJOYN_MANAGER_NOT_REGISTERED);
                    }
                    QCC_LogMsg(("INFO : GetDeviceState JSON - :: %s",commandDataObj.jsonPaylod.c_str()));
                }
                else
                {
                    deviceState state = devInfo->getState();
                    switch(state)
                    {
                    case deviceState::NORMAL:
                    {
                        std::string busName = devInfo->getBusName();
                        DeviceProxy* deviceProxy = getDeviceProxy(busName);
                        if(deviceProxy)
                        {
                            deviceProxy->setDeviceState(getTransportMask(), commandDataObj.deviceID,
                                                        commandDataObj.jsonPaylod);
                        }
                        else
                        {
                            generateJson(ERROR_RES,DEIVCEPROXY_NOT_REGISTERED);
                        }
                        sendToCloud();
                    }
                        break;
                    case deviceState::DEV_OWNERSHIP_TAKEN:
                    {
                        QCC_LogMsg(("INFO : reached in OwnershipTaken JSON"));
                        json jsonData = json::parse(this->commandDataObj.jsonPaylod);
                        jsonData[DEVICE_ID] = commandDataObj.deviceID;
                        commandDataObj.jsonPaylod = jsonData.dump();
                        if(devConfigInfo)
                        {
                            std::string busName = devInfo->getBusName();
                            DeviceProxy* proxyDevice = getDeviceProxy(busName);
                            if(proxyDevice)
                            {
                                proxyDevice->setDeviceState(getTransportMask(), commandDataObj.deviceID,
                                                            commandDataObj.jsonPaylod);
                                std::cout << "GetDeviceState Output:: " << commandDataObj.jsonPaylod;
                            }
                            else
                            {
                                generateJson(ERROR_RES,DEIVCEPROXY_NOT_REGISTERED);
                            }
                            sendToCloud();
                        }
                    }
                        break;
                    case deviceState::DEV_TRANSFERRED_OWNERSHIP:
                    {
                        std::string busName = devInfo->getBusName();
                        DeviceProxy* deviceProxy = getDeviceProxy(busName);
                        if(deviceProxy)
                        {
                            deviceProxy->setDeviceState(getTransportMask(), commandDataObj.deviceID,
                                                        commandDataObj.jsonPaylod);
                        }
                        else
                        {
                            generateJson(ERROR_RES,DEIVCEPROXY_NOT_REGISTERED);
                        }

                        QCC_LogMsg(("INFO : reached in setdevicestate ownership transferred"));
                        std::string gatewayDest = devInfo->getDeviceDest();
                        if(hackRouterHooker)
                        {
                            hackRouterHooker->SendDeviceStateResponse( gatewayDest,commandDataObj.jsonPaylod);
                        }
                        else
                        {
                            generateJson(ERROR_RES,HACK_ROUTER_HOOKER_NOT_REGISTERED);
                        }
                    }
                        break;
                    }
                }
            }
            else
            {
                generateJson(ERROR_RES,DEVICE_NOT_REGISTERED);
                sendToCloud();
            }
        }
        else
        {
            generateJson(ERROR_RES,CONFIGSENSORMANAGER_NOT_REGISTERED);
            sendToCloud();
        }
    }
    catch(std::exception &e)
    {
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        QCC_LogMsg(("Exceptions: Invalid json :: %s",e.what()));
        std::cout<<"\nExceptions: Invalid json in setDeviceStateCommand" << e.what() << std::endl;
    }
}

void deleteDevicePropertiesCommand::execute()
{
    try
    {
        QCC_LogMsg(("deviceid returned- :: %s",commandDataObj.deviceID.c_str()));
        auto devInfo = devConfigInfo->getDeviceFromList(commandDataObj.deviceID);
        if(devInfo)
        {
            commandDataObj.protocolID = devInfo->getProtocolID();
            QCC_LogMsg(("protocolID returned- :: %s",commandDataObj.protocolID.c_str()));

            TransportMask tMask(getTransportMask());
            if(tMask == TRANSPORT_EI_ALLJOYN)
            {
                alljoynAppManager->DeregisterDeviceProperties(commandDataObj.deviceID,commandDataObj.jsonPaylod);
                QCC_LogMsg(("INFO : GetDeviceState JSON - :: %s",commandDataObj.jsonPaylod.c_str()));
            }
            else
            {
                std::string busName = devInfo->getBusName();
                DeviceProxy* deviceProxy = getDeviceProxy(busName);
                if(deviceProxy)
                {
                    deviceProxy->deregisterDeviceProperties(tMask, commandDataObj.deviceID,commandDataObj.jsonPaylod);
                }
                else
                {
                    generateJson(ERROR_RES,DEVICEPROXY_NOT_FOUND);
                }
            }
        }
        else
        {
            generateJson(ERROR_RES,DEVICE_NOT_REGISTERED);
        }
        sendToCloud();
    }
    catch(std::exception &e)
    {
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        QCC_LogMsg(("Exceptions: Invalid json :: %s",e.what()));
        std::cout<<"\nExceptions: Invalid json in deleteDeviceProperties command" << e.what() << std::endl;
    }
}

void setDevicePropertiesCommand::execute()
{
    try
    {
        QCC_LogMsg(("deviceid returned- :: %s",commandDataObj.deviceID.c_str()));
        auto devInfo = devConfigInfo->getDeviceFromList(commandDataObj.deviceID);
        if(devInfo)
        {
            commandDataObj.protocolID = devInfo->getProtocolID();
            QCC_LogMsg(("protocolID returned- :: %s",commandDataObj.protocolID.c_str()));

            TransportMask tMask(getTransportMask());
            if(tMask == TRANSPORT_EI_ALLJOYN)
            {
                alljoynAppManager->RegisterDeviceProperties(commandDataObj.deviceID,commandDataObj.jsonPaylod);
                QCC_LogMsg(("INFO : GetDeviceState JSON - :: %s",commandDataObj.jsonPaylod.c_str()));
            }
            else
            {
                std::string busName = devInfo->getBusName();
                DeviceProxy* deviceProxy = getDeviceProxy(busName);
                if(deviceProxy)
                {
                    deviceProxy->registerDeviceProperties(getTransportMask(), commandDataObj.deviceID,
                                                                 commandDataObj.jsonPaylod);
                }
                else
                {
                    generateJson(ERROR_RES,DEVICEPROXY_NOT_FOUND);
                }
            }
        }
        else
        {
            generateJson(ERROR_RES,DEVICE_NOT_REGISTERED);
        }
        sendToCloud();
    }
    catch(std::exception &e)
    {
        generateJson(ERROR_RES,CAUGHT_EXCEPTION);
        sendToCloud();
        QCC_LogMsg(("Exceptions: Invalid json :: %s",e.what()));
        std::cout<<"\nExceptions: Invalid json in setDevicePropertiesCommand" << e.what() << std::endl;
    }
}
