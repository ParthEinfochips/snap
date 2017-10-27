#ifndef NEW_SENSOR_H
#define NEW_SENSOR_H

#include "../../Common/CommonHeader/CPPHeaders.h"
#include "../../Common/CommonHeader/directoryOPS.h"

#define INTERVAL_RANGE 300

enum deviceState
{
    NORMAL,
    DEV_TRANSFERRED_OWNERSHIP,
    DEV_OWNERSHIP_TAKEN
};

/*
 * @Brief: This is DMA's JSON rendering unit.
 */

class deviceInfo {
public:
    deviceInfo();
    deviceInfo(json &val, std::string protocol_id);
    deviceInfo(const deviceInfo& devInfo);
    deviceInfo& operator=(const deviceInfo& config);

    void setProtocolID(std::string proID);
    std::string getProtocolID();
    json getJsonPayload(){return valPermanent;}

    void setStatus(std::string status);
    std::string getDestMQTTUrl() const;
    void setDestMQTTUrl(const std::string &value);

    deviceState getState() const;
    void setState(const deviceState &value);

    std::string getDeviceDest() const;
    void setDeviceDest(const std::string &value);

    void removeFile(std::string deviceID);

    std::string getBusName() const;
    void setBusName(const std::string &value);

    bool getRemoteDeviceRegisterd() const;
    void setRemoteDeviceRegisterd(bool value);

private:
    json valPermanent;                                         //This is the sensor configuration.

    json sendingFormat;                                        //This one is for sending the telemetry.

    std::mutex mlock;

    std::string protocolID;

    void preparePacket(json &jsonPacket);
    void writeJson();

    std::vector<std::string> filePathsVec;

    std::time_t getCurrentEpoch();

    std::string destMQTTUrl;
    deviceState state;
    std::string deviceDest;
    std::string busName;
    bool remoteDeviceRegisterd;
};

#endif
