#ifndef CLOUD_ENDPOINT_H
#define CLOUD_ENDPOINT_H

#include <functional>
#include "cloudWrapper.h"

class cloudEndpoint
{
public:

    cloudEndpoint(ProtocolType, const uint16_t,
                  std::function<void(uint16_t)>,
                  std::function<void(uint16_t)>,
                  std::function<void(uint16_t, std::string)>,
                  std::function<void(uint16_t)>);

    cloudEndpoint(const cloudEndpoint &cloudEndObj);

    cloudWrapper* getPointer();

    void connectionCallback();

    void messageCallback(std::string jsonStr);

    void errorCallback();

    void disconnectionCallback();

private:

    cloudWrapper* mqttPointer;

    uint16_t endpointID;

    std::function<void(uint16_t)> connectionCB;
    std::function<void(uint16_t)> errorCB;
    std::function<void(uint16_t, std::string)> messageCB;
    std::function<void(uint16_t)> disconnectionCB;
};

#endif // CLOUD_ENDPOINT_H
