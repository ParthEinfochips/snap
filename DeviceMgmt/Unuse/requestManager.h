#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H

/*
 *@brief:  Request Manager is responsible for executing the requests received from the Cloud/Apps etc.
 *
 * It forms the task of type requestTaskHandler and puts them in AsyncTaskQueue thread pooler.
 */

#include "devicespecificcommands.h"
#include "AsyncTaskPool.h"
#include "dmaController.h"
//#include "headers/ThreadEnabler.h"
//#include "proxy/ProxyCloudObj.h"
#include "CommonHeader/fileoperations.h"

struct topicCMDnDevID
{
    topicCMDnDevID(std::string devIDTemp, std::string protocolType, COMMAND_TYPE_ENUM commandEnum)
        : devID(devIDTemp), cmdType(commandEnum), protocolIDD(protocolType)
    {

    }

    topicCMDnDevID(std::string devIDTemp, COMMAND_TYPE_ENUM commandEnum)
        : devID(devIDTemp), cmdType(commandEnum), protocolIDD("")
    {

    }

    std::string protocolIDD;
    std::string devID;
    COMMAND_TYPE_ENUM cmdType;

};

class AsyncTaskQueue;
class RequestManager {
public:

    /*
     *@brief: Constructor
     */

    RequestManager(std::shared_ptr<DMAController> &dmaCtrlTmpPtr,
                   std::shared_ptr<AsyncTaskQueue> &tempPool);

    /*
     *@brief: Destructor
     */

    ~RequestManager();

    /*
     *@brief: Callback registered with the Cloud and Alljoyn object.
     *
     * This will receive the request and further process that.
     */

    void IncomingRequestReceiverCB(std::string topic, std::string msg);

    std::string getCloudUniqueName(){return cloudUniqueName;}
    /*std::shared_ptr<*/CloudProxy*/*>*/ getCloudProxy(){return reqMgrCloudProxyShrdPtr;}

    void prepareDevIDTopics(const string &devID);

    std::map<std::string, std::unique_ptr<topicCMDnDevID>> mapTopic;
    std::string convertSpecialCharacters(const std::string& str);
    void setCommandParameters(std::string topic, std::string msg,topicCMDnDevID *ptrTopicCMDnDevID, std::unique_ptr<command> ptr);

    void discoverCommandParameters(std::string topic, topicCMDnDevID *ptrTopicCMDnDevID, std::string msg);

    void addDeviceCommandParameters(std::string msg, topicCMDnDevID *ptrTopicCMDnDevID, std::string topic);

    void setCommonCommandParameters(std::string topic, std::string msg, std::unique_ptr<command> ptr);

private:
    int side = 9;

    friend class ConfigSensorManager;

    /*std::shared_ptr<*/CloudProxy*/*>*/ reqMgrCloudProxyShrdPtr;
    std::shared_ptr<DeviceProxy> reqMgrDevProxyShrdPtr;
    std::shared_ptr<DMAController> &dmaCtrlObjShrdPtr;
    std::shared_ptr<AsyncTaskQueue> &pool;
    std::string cloudUniqueName;
    std::function<void(std::unique_ptr<command>)> functPtr;

    void decisionOnForwarding(std::unique_ptr<command> uniqCommandPtr);

    void startRequestTaskHandler();
    void stopRequestTaskHandler();

    void connectWithCloudObj();

    void obtainDevObject();

    /*
     *@brief: register configuration manager callback here
     */

    void registerConfigMgrCallBack(std::function<void(std::unique_ptr<command>)> functTempPtr);

    void readConfiguration();
    void prepareFixTopics();
    topicCMDnDevID *getCommandInfo(const std::string &topic);
    void registerDeviceCommand(COMMAND_TYPE_ENUM commandType, std::string topic,const std::string &devID);
    void registerCommand(COMMAND_TYPE_ENUM commandType, string topic, string protocol);
};

#endif //REQUEST_MANAGER_H
