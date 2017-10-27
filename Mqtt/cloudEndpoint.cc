#include "cloudEndpoint.h"
#include <unistd.h>

cloudEndpoint::cloudEndpoint(ProtocolType type, const uint16_t id,
                             std::function<void(uint16_t)> connectionCB,
                             std::function<void(uint16_t)> errorCB,
                             std::function<void(uint16_t, std::string)> messageCB,
                             std::function<void(uint16_t)> disconnectionCB):
    endpointID(id),
    connectionCB(connectionCB),
    errorCB(errorCB),
    messageCB(messageCB),
    disconnectionCB(disconnectionCB)
{
	QStatus status;
    mqttPointer = new cloudWrapper();

    try 
    {
		/** Register Callbacks */
		mqttPointer->initCloudWrapper(type);
		mqttPointer->addConnectionCallback(std::bind(&cloudEndpoint::connectionCallback, this));
		mqttPointer->addDisconnectionCallback(std::bind(&cloudEndpoint::disconnectionCallback, this));
		mqttPointer->addMessageCallback(std::bind(&cloudEndpoint::messageCallback, this, std::placeholders::_1));
		mqttPointer->addErrorCallback(std::bind(&cloudEndpoint::errorCallback, this));
	}
    catch(std::exception &e)
    {
		std::cerr << "EROR : (CloudEndpoint) Issue while initialize cloud wrapper" << e.what() << std::endl;
	}
}

cloudEndpoint::cloudEndpoint(const cloudEndpoint &cloudEndObj)
{
	mqttPointer = new cloudWrapper();
	mqttPointer = cloudEndObj.mqttPointer;

	endpointID = cloudEndObj.endpointID;

}


cloudWrapper *cloudEndpoint::getPointer()
{
    return mqttPointer;
}


/** Callback Functions **/
void cloudEndpoint::connectionCallback()
{
    std::cerr << "INFO : (cloudEndpoint) connectionCallback()" << std::endl;;

    connectionCB(endpointID);
}

void cloudEndpoint::messageCallback(std::string jsonStr)
{
    std::cerr << "INFO : (cloudEndpoint) messageCallback()" << std::endl;;

    messageCB(endpointID, jsonStr);
}

void cloudEndpoint::errorCallback()
{
    std::cerr << "INFO : (cloudEndpoint) errorCallback()" << std::endl;;

    errorCB(endpointID);
}

void cloudEndpoint::disconnectionCallback()
{
    std::cerr << "INFO : (cloudEndpoint) disconnectionCallback()" << std::endl;;

    disconnectionCB(endpointID);
}
