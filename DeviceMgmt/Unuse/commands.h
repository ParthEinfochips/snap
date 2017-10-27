#ifndef COMMANDS_HH
#define COMMANDS_HH

//#include "devicespecificcommands.h"
#include "commandbase.h"
#include "deviceInfo.h"
#include "proxy/cloudProxy.h"
#include "proxy/deviceProxy.h"
//#include "configSensorManager.h"
#include "ajcommon/CustomTransportMask.h"

using namespace std;
using namespace eITM;

class ConfigSensorManager;
class AlljoynAppManager;
class HackRouterHooker;
class RequestManager;

class IDDetails
{
public:
    std::string ORGID;
    std::string GWID;
    std::string APPID;

    std::string APPTopic;

    IDDetails()
    {

    }

    IDDetails(const IDDetails& config)
    {
        this->ORGID = config.ORGID;
        this->GWID = config.GWID;
        this->APPID = config.APPID;
        this->APPTopic = config.APPTopic;
    }

    IDDetails& operator=(const IDDetails& config) // copy assignment
    {
        if (this != &config) { // self-assignment check expected
            this->ORGID = config.ORGID;
            this->GWID = config.GWID;
            this->APPID = config.APPID;
            this->APPTopic = config.APPTopic;
        }
        return *this;
    }
};

/*
 * @brief: Command information .
 */
struct CommandInfo
{
    std::string jobID;
    std::string receivingTopic;
    std::string actuatorID;
    std::string deviceID;
    std::string protocolID;
    std::string destinationMQttURL;
    std::string jsonPaylod;

    deviceInfo *info;

    json jsonPacket;
};

/*
 * @brief: Base class Command , having basic re-usable functions. derived class needs to implement execute function
 */
struct command
{
    virtual void execute() = 0;

    void setCloudPtr(std::string &uniqueName, /*std::shared_ptr<*/CloudProxy*/*> &*/tempPtr)
    {
        cldUniqueName = uniqueName;
        cmdInfoCloudProxyShrdPtr = tempPtr;
    }

    void setDevicePtr(std::shared_ptr<DeviceProxy> &tempPtr)
    {
        cmdInfoDevProxyShrdPtr = tempPtr;
    }

    void sendToCloud()
    {
        if(this->cmdInfoCloudProxyShrdPtr)
            this->cmdInfoCloudProxyShrdPtr->sendDataToCloud(this->cldUniqueName,this->commandDataObj.destinationMQttURL,this->commandDataObj.jsonPaylod);
    }

    COMMAND_TYPE_ENUM getCMDType()
    {
        return cmdType;
    }

    void setConfigManager(ConfigSensorManager* tempPtr)
    {
        devConfigInfo = tempPtr;
    }

    void setAllJoynManager(AlljoynAppManager* tempPtr)
    {
        alljoynAppManager = tempPtr;
    }

    void setHackRouterHooker(HackRouterHooker* value)
    {
        hackRouterHooker = value;
    }

    void setRequestManager(RequestManager* value)
    {
        requestManager = value;
    }

    DeviceProxy* getDeviceProxy(std::string busName);
    TransportMask getTransportMask()
    {
        std::cout << "Protocol id in getTransportMask:::" << commandDataObj.protocolID << std::endl;
        std::string protocol = commandDataObj.protocolID;//.erase(0,1);

        TransportMask tMask(TRANSPORT_NONE);
        if(protocol.compare(T_BLE) == 0)
            tMask = TRANSPORT_EI_BLE;
        else if(protocol.compare(T_ZIGBEE) == 0)
            tMask = TRANSPORT_EI_ZIGBEE;
        else if(protocol.compare(T_OPC) == 0)
            tMask = TRANSPORT_EI_OPC;
        else if(protocol.compare(T_ZWAVE) == 0)
            tMask = TRANSPORT_EI_ZWAVE;
        else if(protocol.compare(T_ALLJOYN) == 0)
            tMask = TRANSPORT_EI_ALLJOYN;
        return tMask;
    }

    std::string convertSpecialCharacters(const std::string& str)
    {
      std::string res;
      std::string::const_iterator it = str.begin()+1;
      while (it != str.end()-1) {
        char ch = *it++;
        if (ch == '\\' && it != str.end()) {
          switch (*it++)
          {
              case '\\': ch = '\\'; break;        // Back Slash
              case 'n':  ch = '\n'; break;        // New Line
              case 't':  ch = '\t'; break;        // Tab
              case '"':  ch = '\"'; break;        // Double Quotes
              case 'b':  ch = '\b'; break;        // Backspace
              case 'r':  ch = '\r'; break;        // Carriage Return
              // all oth er escapes
              default:
              // invalid escape sequence - skip it. alternatively you can copy it as is, throw an exception.
                continue;
          }
        }
        res += ch;
      }
      return res;
    }

    void generateJson(int resCode, std::string resString)
    {
        json jsonData = json::parse(commandDataObj.jsonPaylod);
        jsonData[RESPONSE_CODE] = resCode;
        jsonData[MESSAGE] = resString;
        commandDataObj.jsonPaylod = jsonData.dump();
    }

    CommandInfo commandDataObj;
    ConfigSensorManager* devConfigInfo ;
    AlljoynAppManager* alljoynAppManager= NULL;
    HackRouterHooker* hackRouterHooker= NULL;
    RequestManager* requestManager= NULL;

protected:
    std::shared_ptr<DeviceProxy> cmdInfoDevProxyShrdPtr = nullptr;
    /*std::shared_ptr<*/CloudProxy*/*>*/ cmdInfoCloudProxyShrdPtr = NULL;

    std::string commandType;
    std::string cldUniqueName;

    bool isOperationGet = false;
    bool isOperationPut = false;

    COMMAND_TYPE_ENUM cmdType = COMMAND_TYPE_ENUM::NONE;

private:

};

/*
 * @brief: Hard Transfer of devices, This command is created when the device configuration is transferred
 * from another gateway , device is newly discovered, registered and activated to be functional .
 */
struct hardTransferCommand: public command
{
    bool flag;
    std::string discoveredDevices;
    hardTransferCommand():discoveredDevices("")
    {
        flag=false;
        cmdType = HARD_TRANSFER_COMMAND ;
    }

    void execute();

    // call back from router for discovered devices
    void DiscoverDeviceCallback(std::string data, std::string msg);

    // function will send the progress response to cloud for discover started
    void sendProgressResponse(std::string url, string payload, std::string msg);
    
public:
    void discoverDevices(std::map<std::string, json> deviceList, std::string destMQTTUrl, json jsonPayload, DeviceProxy *deviceProxy);
};

struct takeOwnershipCommand: public command
{
    takeOwnershipCommand()
    {
        cmdType = TAKE_OWNERSHIP_COMMAND ;
    }

    void execute();
};


/*
 * @brief: register Topics for the device registered in DMA
 *
 */
struct registerTopicsCommand: public command
{
    bool flag;
    registerTopicsCommand()
    {
        flag=false;
        cmdType = REGISTER_TOPICS_CLOUD ;
    }

    void execute();
};

/*
 * @brief: register Device callback in router for the devices configuration transferred to DMA
 *
 */
struct registerDeviceCallback: public command
{
    bool flag;
    bool local;
    registerDeviceCallback()
    {
        flag=false;
        local = false;
        cmdType = REGISTER_DEVICE_CALLBACK_ ;
    }

    void execute();
};

/*
 * @brief: register Topics for the device registered in DMA
 *
 */
struct registerDeviceCallbackForRegisterDevice: public command
{
    bool flag;
    registerDeviceCallbackForRegisterDevice()
    {
        flag=false;
        cmdType = REGISTER_DEVICE_CALLBACK_REG_DEV ;
    }

    void execute();
};

/*
 * @brief: Send request response to cloud, this is used when the another app in same gateway or another gateway request is processed
 *
 */
struct sendResponseToCloud : public command
{
    sendResponseToCloud()
    {
        cmdType = SEND_RESPONSE_TO_CLOUD ;
    }

    void execute()
    {
        try
        {
            std::cout <<"\nreached in senddatatocloud command" <<std::endl;
            sendToCloud();
        }
        catch(std::exception &e)
        {
            std::cout<<"\nExceptions: Invalid json" << e.what() << std::endl;
        }
    }
};

/*
 * @brief: send Telemetry to cloud
 *
 */
struct sendDataToCloud : public command
{
    sendDataToCloud()
    {
        cmdType = SEND_DATA_TO_CLOUD ;
    }

    void execute();

    std::string deviceId;
};

/*
 * @brief: Transfer ownership fo devices from one app to another app( app can be on same gateway or another gateway)
 *
 */
struct TransferOwnershipCommand : public command
{

    TransferOwnershipCommand()
    {
        cmdType = TRANSFER_OWNERSHIP_DEVICE ;
    }

    void execute();
};

/*
 * @brief: Transfer devices(hard) from one app to another app( app can be on same gateway or another gateway)
 *
 */
struct TransferDevicesCommand : public command
{

    TransferDevicesCommand()
    {
        cmdType = TRANSFER_DEVICES_DEVICE ;
    }

    void execute();

    void sendProgressResponse(std::string msg);

};

/*
 * @brief: Discover of devices for particular transport or protocol.
 */

struct discoverCommand : public command
{
    bool flag;
    discoverCommand()
    {
        flag=false;
        cmdType = DISCOVER_CMD;
    }

    void execute();

    void DiscoverDeviceCallback(std::string data, std::string msg);


private:

};

/*
 * @brief: Get Devices List.
 */

struct deviceListCommand : public command
{
    deviceListCommand()
    {
        cmdType = DEVICE_LIST;
    }

    void execute();
private:

};

/*
 * @brief: Get Gateway Status, Active or not.
 */

struct getGatewayStatusCommand : public command
{
    getGatewayStatusCommand()
    {
        cmdType = GET_GATEWAY_STATUS;
    }

    void execute();

private:
};

/*
 * @brief: Set Gateway Status.
 */

struct putGatewayStatusCommand : public command
{
    putGatewayStatusCommand()
    {
        cmdType = PUT_GATEWAY_STATUS;
    }

    void execute();
private:


};

/*
 * @brief: Register device in the matrix and in router and send the response to cloud.
 */

struct addDeviceCommand : public command
{
    addDeviceCommand()
    {
        cmdType = DEVICE_ADD;
    }

    void execute();
};

/*
 * @brief: delete device from matrix and from router and send the response to cloud.
 */

struct deleteDeviceCommand : public command
{
    deleteDeviceCommand()
    {
        cmdType = DEVICE_DEL;
    }

    void execute();

private:

};

/*
 * @brief: Get App RAML from gateway and send the response to cloud.
 */

struct getAppRAMLCommand : public command
{
    getAppRAMLCommand()
    {
        cmdType = GET_APP_RAML;
    }

    void execute();

private:

};

/*
 * @brief: delete device from matrix and from router and send the response to cloud.
 */

struct restorePersitentDevices : public command
{
    bool flag;
    restorePersitentDevices()
    {
        flag=false;
        cmdType = RESTORE_PERSISTENT_DEVICES;
    }

    void execute();
    void discoverCallBack(std::string data, std::string msg);
    TransportMask transportType(std::string &protocol);
    void discoverFromFile(json &jsonPayload);
    void addDevices(json &jsonRef);
    void activateDevices(json &jsonPayLoad);

private:

};



#endif // COMMANDS_HH

