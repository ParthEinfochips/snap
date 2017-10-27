#include "dataTaskHandler.h"
#include "requestTaskHandler.h"
#include "devicespecificcommands.h"

#define QCC_MODULE "DMA_APP"

DataTaskHandler::DataTaskHandler(){

    RequestTaskHandler *handler = new RequestTaskHandler();
    pool = std::make_shared<AsyncTaskQueue>();
    pool->Start(handler);
}

DataTaskHandler::~DataTaskHandler()
{
    pool->Stop();

}

void DataTaskHandler::sendToCloud()
{
    QCC_LogMsg(("reached in sendToCloud::::"));
//    std::cout << "reached in sendToCloud::::"  << std::endl;
    if(this->cmdInfoCloudProxyShrdPtr)
    {
        this->cmdInfoCloudProxyShrdPtr->sendDataToCloud(this->cldUniqueName,this->destinationUrl,this->payLoad);
    }
    QCC_LogMsg(("reached in sendToCloud:::: 2222"));
//    std::cout << "reached in sendToCloud 222::::"  << std::endl;
}

void DataTaskHandler::setAlljoynManager(AlljoynAppManager* alljoynMag)
{
    alljoynAppManager = alljoynMag;
}

HackRouterHooker* DataTaskHandler::getHackRouterHooker() const
{
    return hackRouterHooker;
}

void DataTaskHandler::setHackRouterHooker(HackRouterHooker *value)
{
    hackRouterHooker = value;
}

void DataTaskHandler::setConfigSensorManager(ConfigSensorManager* tempPtr)
{
    devConfigInfo = tempPtr;
}

void DataTaskHandler::setCloudPtr(std::string uniqueName, /*std::shared_ptr<*/CloudProxy*/*>*/ tempPtr,std::string topicdest)
{
    cldUniqueName = uniqueName;
    cmdInfoCloudProxyShrdPtr = tempPtr;
    destinationUrl = topicdest;
}

void DataTaskHandler::deviceDataCallback(std::string deviceID, std::string data)
{
    QCC_LogMsg(("DeviceID::::%s",deviceID.c_str()));
//    std::cout << "DeviceID::::" << deviceID << std::endl;
    QCC_LogMsg(("Device Data::::%s",data.c_str()));
//    std::cout << "Device Data::::" << data << std::endl;
    std::string url = destinationUrl + deviceID;
    payLoad = data;

    std::unique_ptr<sendDataToCloud> sendDataCMD_uniq(new sendDataToCloud);
    sendDataCMD_uniq->setCloudPtr(cldUniqueName,cmdInfoCloudProxyShrdPtr);
    sendDataCMD_uniq->commandDataObj.destinationMQttURL = url;
    sendDataCMD_uniq->commandDataObj.jsonPaylod = payLoad;

    QCC_LogMsg(("reached in deviceDataCallback command 1"));
//    std::cout <<"\nreached in deviceDataCallback command 1" <<std::endl;
    sendDataCMD_uniq->setAllJoynManager(alljoynAppManager);
    QCC_LogMsg(("reached in deviceDataCallback command 2"));
//    std::cout <<"\nreached in deviceDataCallback command 2" <<std::endl;
    sendDataCMD_uniq->setConfigManager(devConfigInfo);
    QCC_LogMsg(("reached in deviceDataCallback command 3"));
//    std::cout <<"\nreached in deviceDataCallback command 3" <<std::endl;
    sendDataCMD_uniq->deviceId = deviceID;
    QCC_LogMsg(("reached in deviceDataCallback command 4"));
//    std::cout <<"\nreached in deviceDataCallback command 4" <<std::endl;

    sendDataCMD_uniq->setHackRouterHooker(hackRouterHooker);

    pool->Enqueue(std::move(sendDataCMD_uniq));
}






