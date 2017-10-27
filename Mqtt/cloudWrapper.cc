#include "cloudWrapper.h"

#include <unistd.h>
#define CTL_LIB_PATH "/opt/ctl/lib/libMessageFactory.so"

cloudWrapper::cloudWrapper()
:protocolPointer(NULL)
{
	//protocolPointer = new MessageInterface();

}

cloudWrapper::cloudWrapper(const cloudWrapper &cloudWrapObj)
{
	protocolPointer = NULL;
}

cloudWrapper::~cloudWrapper()
{
    // Destructor
	if(protocolPointer)
		delete protocolPointer;

}

void cloudWrapper::initCloudWrapper(ProtocolType protocol)
{
    /** Open Provided Dynamic Library (CTL) */
    void *messageFactoryPtr= dlopen(CTL_LIB_PATH, RTLD_LAZY);
    
    std::stringstream strErrStream;

    if (!messageFactoryPtr) {
		
        strErrStream << "Cannot load library: " << dlerror() << '\n';
		
		throw std::runtime_error(strErrStream.str());
    }

    dlerror();

    createfactory_t * createFactoryFunc = (createfactory_t *) dlsym(messageFactoryPtr, "create");

    const char *dlsym_error = dlerror();
    if (dlsym_error) {
		std::stringstream strErrStream;		
		
        strErrStream << "Cannot load symbol create: " << dlsym_error << std::endl;
        
		throw std::runtime_error(strErrStream.str());
    }

    MessageFactory* factory = createFactoryFunc();
    //QCC_ASSERT(factory && "Unable to open message factory!");

    if(!factory)
    {
		std::stringstream strErrStream;
		
        strErrStream << "Factory pointer null";
        
		throw std::runtime_error(strErrStream.str());
    }

    protocolPointer = factory->CreateProtocol(protocol);

    if(!protocolPointer)
    {
		std::stringstream strErrStream;
		
        strErrStream << "protocol pointer null";
        
		throw std::runtime_error(strErrStream.str());
    }	
}

QStatus cloudWrapper::Connect (std::string jsonString)
{
	std::cout << "IN CLOUD WRAPPER CONNECT" << std::endl;
    return (QStatus)protocolPointer->Connect(jsonString);
}

QStatus cloudWrapper::SendData (std::string jsonString)
{
    return (QStatus)protocolPointer->SendData(jsonString);
}

QStatus cloudWrapper::RegisterListener (std::string jsonString)
{
    return (QStatus)protocolPointer->RegisterListener(jsonString);
}

QStatus cloudWrapper::CancelListener (std::string jsonString)
{
    return (QStatus)protocolPointer->CancelListener(jsonString);
}

QStatus cloudWrapper::ConfigureTLS (std::string jsonString)
{
    return (QStatus)protocolPointer->ConfigureTLS(jsonString);
}

QStatus cloudWrapper::Disconnect()
{
    return (QStatus)protocolPointer->Disconnect();
}

// Callback Forwarders
void cloudWrapper::addMessageCallback(std::function<void(std::string jsonStr)> functionPointer)
{
    protocolPointer->addMessageCallback(functionPointer);
}

void cloudWrapper::addConnectionCallback (std::function<void(void)> functionPointer)
{
    protocolPointer->addConnectionCallback(functionPointer);
}

void cloudWrapper::addDisconnectionCallback (std::function<void(void)> functionPointer)
{
    protocolPointer->addDisconnectionCallback(functionPointer);
}

void cloudWrapper::addErrorCallback (std::function<void(void)> functionPointer)
{
    protocolPointer->addErrorCallback(functionPointer);
}

