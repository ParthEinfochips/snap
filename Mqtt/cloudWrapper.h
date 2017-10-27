#ifndef CLOUDWRAPPER_H
#define CLOUDWRAPPER_H

#include <SnapBricks/ctl/messageFactory.h>
#include "Status.h"
#include <sstream>

class cloudWrapper
{
public:

    cloudWrapper();

    cloudWrapper(const cloudWrapper &cloudWrapObj);

    ~cloudWrapper();
    
    void initCloudWrapper (ProtocolType protocol);

    QStatus Connect (std::string jsonString);

    QStatus SendData (std::string jsonString);

    QStatus RegisterListener (std::string jsonString);

    QStatus CancelListener(std::string jsonString);

    QStatus ConfigureTLS (std::string jsonString);

    QStatus Disconnect();

    /** Common Callbacks */
    void addMessageCallback (std::function<void(std::string jsonStr)> functionPointer);

    void addConnectionCallback (std::function<void(void)> functionPointer);

    void addDisconnectionCallback (std::function<void(void)> functionPointer);

    void addErrorCallback (std::function<void(void)> functionPointer);

private:

    MessageInterface* protocolPointer;

};

#endif // CLOUDWRAPPER_H
