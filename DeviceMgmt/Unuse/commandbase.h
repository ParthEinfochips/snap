#ifndef COMMANDBASE_HH
#define COMMANDBASE_HH

#include "DMA_Macros.h"
#include "CommonHeader/CPPHeaders.h"
//#include "configSensorManager.h"

using namespace std;
//class ConfigSensorManager;
//class AlljoynAppManager;

enum COMMAND_TYPE_ENUM
{
    NONE = 0x263,
    DISCOVER_CMD,
    DEVICE_LIST,
    GET_APP_RAML,
    DEVICE_ADD,
    DEVICE_DEL,
    GET_DEVICE_STATUS_,
    PUT_DEVICE_STATUS,
    DEL_DEVICE_PROPERTIES,
    PUT_DEVICE_PROPERTIES,
    GET_DEVICE_RAML_,
    SET_DEVICE_RAML_,
    GET_DEVICE_STATE_,
    SET_DEVICE_STATE_,
    SEND_DATA_TO_CLOUD,
    GET_GATEWAY_STATUS,
    PUT_GATEWAY_STATUS,
    DEVICE_OWN_CHANGE,
    TRANSFER_OWNERSHIP_DEVICE,
    TRANSFER_DEVICES_DEVICE,
    HARD_TRANSFER_COMMAND,
    TAKE_OWNERSHIP_COMMAND,
    SEND_RESPONSE_TO_CLOUD,
    REGISTER_TOPICS_CLOUD,
    REGISTER_DEVICE_CALLBACK_,
    REGISTER_DEVICE_CALLBACK_REG_DEV,
    REGISTER_DEVICE_CALLBACK_LOCAL,
    RESTORE_PERSISTENT_DEVICES
};


#endif // COMMANDBASE_HH

