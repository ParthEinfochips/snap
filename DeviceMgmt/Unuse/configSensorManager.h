#ifndef CONFIG_SENSOR_MANAGER_H
#define CONFIG_SENSOR_MANAGER_H

#include "deviceInfo.h"
#include "dmaController.h"
#include "requestManager.h"
#include "../../Common/CommonHeader/CPPHeaders.h"
#include "dataTaskHandler.h"
#include "../../Common/proxy/HackRouterHooker.h"

#include <time.h>

typedef std::map<std::string ,std::shared_ptr<deviceInfo> > mapdeviceInfo;


 /*
 *@brief: This stores the configuration of sensor(Matrix), data collection from device proxy object
 * and creates data task handler.
 *
 *@Status: This is the class which will take care of the sensor shadows.
 */
class IDDetails;
struct command;
class AsyncTaskQueue;
class RequestManager;
class DataTaskHandler;
class ConfigSensorManager {

public:
    ConfigSensorManager(std::shared_ptr<DMAController> &dmaCtrlTmpPtr);
    ~ConfigSensorManager();

    AlljoynAppManager *getAlljoynmanager() const;
    void setAlljoynmanager(AlljoynAppManager* alljoynmanager);

    HackRouterHooker *getHackRouterHooker() const;
    void setHackRouterHooker(HackRouterHooker* value);

    void setGatewayMacId(std::string macId);
    std::string getGatewayMacId();

    bool getGatewayStatus() const;
    void setGatewayStatus(bool value);

    void handleCommand(std::unique_ptr<command> command);

    void addDeviceInList(std::string deviceID, deviceInfo devInfo);
    void deleteDeviceFromList(std::string deviceID);
    std::shared_ptr<deviceInfo> getDeviceFromList(std::string deviceID);
    bool checkDeviceExist(std::string deviceID);

    void sendTelemetry(std::string jsonPayload);
    void updateSourceDMA(std::string jsonPayload);

    void transferDevicess(std::string jsonPayload);

    void setDeviceState(std::string& jsonPayload);
    void getDeviceState(string &jsonPayload);

    void sendDeviceStateResponse(std::string& jsonPayload);
    void SendResponseToCloud(std::string& jsonPayload);

    void threadFunction();
    std::string getBusUniqueName();
    DeviceProxy* getDeviceProxyPtr(std::string busName);
    void registerDeviceWithRouter(string busName);
    void writeRuleEngineInfo();
    mapdeviceInfo* deviceMapInfo;

private: //functions
    void doTheAbout();
    void readFromDir();
    void discoverCallBack(std::string data, std::string msg);
    TransportMask transportType(std::string &protocol);
    void discoverFromFile(json &jsonPayload);
    void addDevices(json &jsonRef);
    void activateDevices(json &jsonPayLoad);

public: // variables
    std::shared_ptr<DataTaskHandler> dataTaskHndler = nullptr;

private: // variables
    std::unique_ptr<AJCommon> ajCommonPtr = nullptr;
    std::shared_ptr<AsyncTaskQueue> pool = nullptr;
    std::shared_ptr<RequestManager> RmMgr = nullptr;
    std::shared_ptr<DMAController> dmaCtrlPtr;

    ajn::BusAttachment *busAttachPtr = NULL;

    std::shared_ptr<CommonBusListener> busListenerPtr = nullptr;
    AlljoynAppManager*_alljoynmanager = NULL;
    HackRouterHooker* hackRouterHooker = NULL;

    std::mutex mlock;
    bool gatewayStatus;
    bool flag=false;
    std::string gatewayMacId;
    std::string jsonPayld;
};



#endif //CONFIG_SENSOR_MANAGER_H
