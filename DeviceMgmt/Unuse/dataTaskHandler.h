#ifndef DATA_TASK_HANDLER_H
#define DATA_TASK_HANDLER_H

#include "../../Common/CommonHeader/CPPHeaders.h"
#include "../../Common/proxy/cloudProxy.h"
#include "AsyncTaskPool.h"
#include "configSensorManager.h"

/*
 *@brief:  This class processes the data or telemetry received from router and sending it to cloud.
 *
 */

class AsyncTaskPool;
class AsyncTaskQueue;
class ConfigSensorManager;

class DataTaskHandler {
public:
    DataTaskHandler();
    ~DataTaskHandler();

    void setCloudPtr(std::string uniqueName, /*std::shared_ptr<*/CloudProxy*/*>*/ tempPtr,std::string topicdest);

    void deviceDataCallback(std::string deviceID, std::string data);
    void sendToCloud();
    void setConfigSensorManager(ConfigSensorManager* tempPtr);
    void setAlljoynManager(AlljoynAppManager *alljoynMag);

    HackRouterHooker* getHackRouterHooker() const;
    void setHackRouterHooker(HackRouterHooker* value);

private:
    std::string cldUniqueName;
    /*std::shared_ptr<*/CloudProxy*/*>*/ cmdInfoCloudProxyShrdPtr = NULL;
    std::string destinationUrl;
    std::string payLoad;
    AlljoynAppManager* alljoynAppManager;
    ConfigSensorManager* devConfigInfo ;
    HackRouterHooker* hackRouterHooker;
    std::shared_ptr<AsyncTaskQueue> pool = nullptr;
};

#endif //DATA_TASK_HANDLER_H
