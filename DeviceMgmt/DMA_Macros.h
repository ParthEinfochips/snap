#ifndef DMA_MQTOPICS_HH
#define DMA_MQTOPICS_HH

//#include "../configSensorManager.h"
//#include "../newSensor.h"
//#include "../dmaController.h"
//#include "CommonHeader/CPPHeaders.h"


/*App id is the listening topic*/
#define OrgId "58c3ce458441383a938c5c75"
#define GWid  "58dbb11c421aa95be32684ca"

//5848dd3184413852939b1f22 my gw id


#define DMA_APPID "DeviceMgmt"
#define WD_APPID "WD"
#define BLE "BLE"

#define DMA_TOPIC "EI/" OrgId "/" GWid "/" DMA_APPID

#define WD_TOPIC "EI/" OrgId "/" GWid "/" WD_APPID

#define HASHED_TOPIC DMA_TOPIC "/#"
#define HASHED_TOPIC_WD WD_TOPIC "/#"



//Entities types, FIRST are the ones that come right after the appid and AFTER those that come after the entity ID.

#define DEVICES_ENTITY_FIRST "/Devices"
#define ACTUATOR_ENTITY_FIRST "/Actuators"
#define PROTOCOLS_ENTITY_FIRST "/Protocols"
#define STATUS_ENTITY_FIRST "/Status"

#define TRANSFER_OWNERSHIP "/TransferOwnership"
#define TRANSFER_DEVICES "/TransferDevices"

#define COMMANDS_CONFIGURATION_AFTER "/Configuration"
#define COMMANDS_DEVICESTATUS_AFTER "/Status"
#define COMMANDS_PROPERTIES_AFTER "/Properties"
#define COMMANDS_RAML_AFTER "/RAML"
#define COMMANDS_STATE_AFTER "/State"

#define COMMANDS_PUT "/Put"
#define COMMANDS_GET "/Get"
#define COMMANDS_DEL "/Del"
#define COMMANDS_DEL_WS "Del"
#define COMMANDS_OWN_CHANGE "OwnChange"

#define COMMANDS_DATA "/Data"

#define REPLY_TOPIC "/Reply"

#define JOBID "JOBID"

#define TELEMETRY_TOPIC "EI/Data/" OrgId "/" GWid "/" DMA_APPID DEVICES_ENTITY_FIRST "/"

//Test to be deleted afterwards
#define discovery_topic DMA_TOPIC PROTOCOLS_ENTITY_FIRST "/" BLE COMMANDS_GET

#define MQTT_HOSTNAME "localhost"
#define MQTT_USERNAME "iotuser"
#define MQTT_PASWRD "ei@123"

#define MQTT_CLOUD_HOSTNAME "10.107.1.164"
#define MQTT_CLOUD_USERNAME "iotuser"
#define MQTT_CLOUD_PASWRD "ei12@"
#define MQTT_CLOUD_PORT 1883

#define HAWKROUTER "HackRouter:"
#define COLON ":"

#define ROUTER_ADV_NAME_PREFIX "org.alljoyn.eIAJ.Router.mac"

#define COULD_CONNECTION_TRY 2

/*
 * New Defines
 * */
#define MQTT_CONFIG_FILE_PATH "/opt/GA.conf.d/"
#define DMA_CONFIG_FILE_PATH "/opt/GA.conf.d/"
#define MQTT_CONFIG_FILE_PRIFIX "MQTT"
#define CONFIG_FILE_PRIFIX ".conf"

/*DMA Topic Prifix*/
#define DTP_DEV_GETDEVICELIST "Devices/GetDeviceList"
#define DTP_DEV_GET "Devices/Get"
#define DTP_DEV_PUT "Devices/Put"
#define DTP_STATUS_GET "Status/Get"
#define DTP_STATUS_PUT "Status/Put"
#define DTP_STATE_GET "State/Get"
#define DTP_STATE_PUT "State/Put"
#define DTP_DEV_DEL "Devices/Del"
#define DTP_RAML_GET "RAML/Get"
#define DTP_RAML_PUT "RAML/Put"
#define DTP_XML_GET "XML/Get"
#define DTP_XML_PUT "XML/Put"
#define DTP_SCCONFIG "SCconfig"
#define DTP_RAML_GET1 "/RAML/Get"
#define DTP_XML_GET1 "/XML/Get"
#define	DTP_BASEURL_UPDATE "Configuration/Baseurl/Update"
#define DTP_BASEURL_TEST   "Configuration/Baseurl/Test"
#define DTP_ADD_CC "Configuration/Cloudconnection/Put"
#define DTP_UPDATE_CC "Configuration/Cloudconnection/Update"
#define DTP_DELETE_CC "Configuration/Cloudconnection/Delete"

#define DTP_ADD_LC "Configuration/Localconnection/Put"
#define DTP_UPDATE_LC "Configuration/Localconnection/Update"
#define DTP_DELETE_LC "Configuration/Localconnection/Delete"


#define DTP_TEST_CC  "Configuration/Connection/Test"
#define DTP_STATUSCHANGE_CC "Devices/StatusChange/Put"
#define DTP_GETALLRAML  "GetDeviceRaml/All"

/*Device Object Command */
#define DOC_STARTDISCOVERY "StartDiscovery"
#define DOC_STOPDISCOVERY "StopDiscovery"
#define DOC_ADDDEVICE "AddDevice"
#define DOC_DELETEDEVICE "DeleteDevice"
#define DOC_GETDEVICERAML "GetDeviceRAML"
#define DOC_SETDEVICERAML "SetDeviceRAML"
#define DOC_GETDEVICEXML "GetDeviceXML"
#define DOC_SETDEVICEXML "SetDeviceXML"
#define DOC_GETDEVICESTATE "GetDeviceState"
#define DOC_SETDEVICESTATE "SetDeviceState"
#define DOC_GETDEVICESTATUS "GetDeviceStatus"
#define DOC_SETDEVICESTATUS "SetDeviceStatus"
#define DOC_REGISTERDEVICEPROPERTIES "RegisterDeviceProperties"
#define DOC_DEREGISTERDEVICEPROPERTIES "DeregisterDeviceProperties"
#define DOC_GETDEVICELIST "GetDeviceList"
#define DOC_DEVICEEXISTS "DeviceExists"
#define DOC_REGISTERDEVICECALLBACK "RegisterTelemetryCallback"
#define DOC_DEREGISTERDEVICECALLBACK "DeregisterTelemetryCallback"
#define DOC_CALLBACKFORDEVICEREGISTER "RegisterDeviceCallback"
#define DOC_DECALLBACKFORDEVICEREGISTER "DeregisterDeviceCallback"
#define DOC_SCCONFIG "SCconfig"
#define DOC_UPDATEBASEURL "UpdateBaseUrl"
#define DOC_TESTBASEURL   "TestBaseUrl"
#define DOC_ADDCC      "AddCC"
#define DOC_UPDATECC    "UpdateCC"
#define DOC_DELETECC     "DeleteCC"

#define DOC_ADDLC      "AddLC"
#define DOC_UPDATELC    "UpdateLC"
#define DOC_DELETELC     "DeleteLC"

#define DOC_TESTCC       "TestCC"
#define DOC_STATUSCHANGE  "StatusChange"
#define DOC_GETALLRAML   "GetAllRaml"

#define DATA "data"
#define BASEURL "baseurl"
#define APPCONFIG "appconfig"
#define CLOUDCONNECTIONS "cloudconnections"

#define TOPIC_PROCESSFLAG "/"

/*
 * @brief: has the command information and is passed onto the
 */

//struct command;


/* Three tasks that I need to lookout for are

1) Discover request comes, I need to just give what I get from the lower sensor. There may be no
2) I get the set status request that is to activate/deactivate the sensor. I may have properties by this time.
3) Add sensor i.e register the sensor for that matter. Discovered or not, doesn't matter. There can be properties,
there may not be.

4) There will be the get properties request and response.



#%RAML 0.8 title: OPC Pressure Node baseUri: PressureNode version: 1 schemas:  - getSchema: |  {  "description" : "Pressure Node provides the atmospheric pressure value in hpa",  "title": "Pressure Node",  "definitions":   {  "pressure":   {  "type": "object",  "description": "Get the value of pressure node",  "properties":   {  "pressure":   {  "type": "number",  "units": "hPa"  }  },  "required": [  "pressure"  ]  }  },  "type": "object",  "anyOf": [  {"$ref": "#definitionspressure"}  ]  } pressure:  get:  responses:   200:  body:   applicationjson:  schema: getSchema  example: |  {  "pressure": 1009.25  }
*/
#endif //DMA_MQTOPICS_HH
