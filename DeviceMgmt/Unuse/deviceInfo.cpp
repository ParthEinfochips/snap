#include "deviceInfo.h"

#define QCC_MODULE "DMA_APP"

deviceInfo::deviceInfo(json &val, std::string protocol_id) : valPermanent(val),protocolID(protocol_id),
    destMQTTUrl(""),state(deviceState::NORMAL),remoteDeviceRegisterd(false)
{
    std::string filePath = JSON_PATH_DEVICES + configurationSettings::AppTopic + SLASH;

    if(!directoryOPS::checkExistence(filePath))
    {
        directoryOPS::createPath(filePath, 0755);
    }

    valPermanent[DEVICEDEST] = "";
    valPermanent[DEST_MQTT_URL] = "";
    valPermanent[DEVICESTATE] = "NORMAL";
    valPermanent[BUSNAME] = "";

    writeJson();
}

deviceInfo::deviceInfo() : protocolID(""),destMQTTUrl(""),state(deviceState::NORMAL),remoteDeviceRegisterd(false){
}

deviceInfo::deviceInfo(const deviceInfo &devInfo)
{
    this->sendingFormat = devInfo.sendingFormat;
    this->valPermanent = devInfo.valPermanent;
    this->protocolID = devInfo.protocolID;
    this->destMQTTUrl = devInfo.destMQTTUrl;
    this->state = devInfo.state;
    this->deviceDest = devInfo.deviceDest;
    this->busName = devInfo.busName;
    this->remoteDeviceRegisterd = devInfo.remoteDeviceRegisterd;
}

deviceInfo& deviceInfo::operator=(const deviceInfo& config) // copy assignment
{
    if (this != &config) { // self-assignment check expected
        this->sendingFormat = config.sendingFormat;
        this->valPermanent = config.valPermanent;
        this->protocolID = config.protocolID;
        this->destMQTTUrl = config.destMQTTUrl;
        this->state = config.state;
        this->deviceDest = config.deviceDest;
        this->busName = config.busName;
        this->remoteDeviceRegisterd = config.remoteDeviceRegisterd;
    }
    return *this;
}

void deviceInfo::preparePacket(json &jsonPacket)
{

}

void deviceInfo::setStatus(std::string status)
{
    valPermanent[STATUS] = status;

    writeJson();
}

void deviceInfo::setProtocolID(std::string proID)
{
    valPermanent[PROTOCOL_ID] = protocolID;
    protocolID = proID;
}

std::string deviceInfo::getProtocolID()
{
    return protocolID;
}

void deviceInfo::writeJson()
{
    FileOPS fileOPSObj;

    valPermanent[PROTOCOL_ID] = protocolID;

    QCC_LogMsg(("Device Info writejson ProtocolID :: %s",protocolID.c_str()));
    std::string deviceID = valPermanent["id"];

    std::string filePath = JSON_PATH_DEVICES + configurationSettings::AppTopic + SLASH + deviceID;

    QCC_LogMsg(("File Path in writejson :: %s",filePath.c_str()));
    if(!directoryOPS::checkExistence(filePath))
    {
        QCC_LogMsg(("File does not exist in writejson :: %s",filePath.c_str()));
        fileOPSObj.writeToFile(filePath, valPermanent.dump());
        filePathsVec.push_back(filePath);
    }
    else
    {
        std::remove(filePath.c_str());
        fileOPSObj.writeToFile(filePath, valPermanent.dump());
    }
}

void deviceInfo::removeFile(std::string deviceID)
{
    std::string filePath = JSON_PATH_DEVICES + configurationSettings::AppTopic + SLASH + deviceID;
    std::remove(filePath.c_str());
}

std::time_t deviceInfo::getCurrentEpoch()
{
}

bool deviceInfo::getRemoteDeviceRegisterd() const
{
    return remoteDeviceRegisterd;
}

void deviceInfo::setRemoteDeviceRegisterd(bool value)
{
    remoteDeviceRegisterd = value;
}

std::string deviceInfo::getBusName() const
{
    return busName;
}

void deviceInfo::setBusName(const std::string &value)
{
    busName = value;
    valPermanent[BUSNAME] = value;
    writeJson();
}

std::string deviceInfo::getDeviceDest() const
{
    return deviceDest;
}

void deviceInfo::setDeviceDest(const std::string &value)
{
    valPermanent[DEVICEDEST] = value;

    deviceDest = value;
    writeJson();
}

deviceState deviceInfo::getState() const
{
    return state;
}

void deviceInfo::setState(const deviceState &value)
{
    switch(value)
    {
    case deviceState::NORMAL:
        valPermanent[DEVICESTATE] = "NORMAL";
        break;
    case deviceState::DEV_TRANSFERRED_OWNERSHIP:
        valPermanent[DEVICESTATE] = "TRANSFERED";
        break;
    case deviceState::DEV_OWNERSHIP_TAKEN:
        valPermanent[DEVICESTATE] = "OWN_TAKEN";
        break;
    }
    writeJson();

    state = value;
}

std::string deviceInfo::getDestMQTTUrl() const
{
    return destMQTTUrl;
}

void deviceInfo::setDestMQTTUrl(const std::string &value)
{
    valPermanent[DEST_MQTT_URL] = value;

    writeJson();

    destMQTTUrl = value;
}
