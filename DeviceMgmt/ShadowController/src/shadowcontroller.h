#ifndef SHADOW_CONTROLLER_HH
#define SHADOW_CONTROLLER_HH

#include "shadow.h"

class DeviceObj;
class ServiceProvider;

//! This is the shadowController class which manages the shadows against their deviceID and acts as interface between shadows and Devive Object.

class shadowController
{

public:

	/*!
	  Default Constructor.

	  Primarily used for gtest.
	 */

	shadowController();

	/*!
	  Constructor

      \param mask this is the protocol mask.
      \param ptrServiceProvider this is the pointer of service provider.
      \param ptr pointer to device object.
	 */

    shadowController(TransportMask mask, ServiceProvider *ptrServiceProvider, DeviceObj *ptr);

	/*!
	  A function which reads from the persistent storage the shadows and take in the last state of data.

	  Currently, inactive since complete app's persistent storage lies down. However, placeholder is present.
	 */
    void generateShadowsFromPersistent();

	/*!
	  Destructor
	 */

    ~shadowController();

	/*!
	  Creates shadows

      \param devId DeviceID of the device.
      \param raml RAML of the shadow.
	 */

    void createShadow(std::string &devId, std::string &raml);


    void persistantShadow(std::string &macID, std::string &devId);

    void writeToFile(std::string path, std::string payload);

    /*!
      Get device state

      \param devId Device ID of the device.
      \param data Payload of Job request
     */

    void getDeviceState(std::string& devId, std::string& data);

    /*!
      Set device state

      \param devId Device ID of the device.
      \param data Payload of Job request
     */

    bool setDeviceState(std::string& devId, std::string& data);



    void  setMultipleStatus(std::string& data);
    /*!
      Get device status to be implemented. This is for getting the status of shadow.

      \param devId Device ID of the device.
      \param data Payload of Job request
     */

    void getDeviceStatus(std::string& devId, std::string& data);

    /*!
      Set device status to be implemented. This is for activating the shadow.

      \param devId Device ID of the device.
      \param data Payload of Job request
     */

    void setDeviceStatus(std::string& devId, std::string& data);

    /*!
      Telemetry Data function

      \param devId Device ID of the device.
      \param data Payload of telemetry/notification.
     */

    void deviceData(std::string &devId, std::string& data);

    /*!
      Checks whether shadow is already created or not. Depends upon the presence of RAML.

      \param devId Device ID to check the whether that exists or not.
     */
    bool isShadowCreated(std::string &devId);

    /*!
      This function is for sending the direct reply to the end app and for notification as well.

      Thread safe function.

      \param devId Device ID of the device.
      \param data Payload of telemetry.
     */

    void SendTelemetryDataCCAPP(std::string &devId, std::string &payload);

    /*!
      This function calls the interface transport function with payload and everything.

      Thread safe function.

      \param iName this calls the interface of the device.
      \param devId Device ID of the device.
      \param data Payload of telemetry/notification.
     */

    void callTheInterface(InterfaceName iName, std::string &devId, std::string &payload);

    /*!
      This function understands the payload received. Flushes, changes, timer settings etc.

      \param payload this is the settings contained in the payload.
     */

    void adhocSettings(std::string &payload);

    /*!
      This function understands the payload received and sends it back to cloud.

      Thread safe function.

      \param topic cloud topic which is the destination.
      \param payload payload is the string payload.
     */

    void sendToCloud(std::string &topic, std::string &payload);

    /*!
      Deregister Shadow
      \param devId needs the devId
     */

    void deregisterShadow(std::string &devId, std::string &payload);

    void DiscoveredDevices(std::string payload);

    void registerDevice(std::string payload);

private:

    /*!
      Shadow for the specific app would be stored here.
     */

    std::string finalDirPath;
    std::string ramlDirPath;

    json fileJson;

    /*!
      typedef Shadow map table having device id as keys and shadow as shared pointer.
     */

    typedef std::map<std::string, std::shared_ptr<Shadow>> shadowMap;

    /*!
      Shadow Table, key as device ID and value as shadow pointer.
     */

    shadowMap shadowMapObj;

    /*!
      Device object pointer.
     */

    DeviceObj *_DevPtr;

    /*!
      Pointer to service provider.
     */

    ServiceProvider *_ptrServiceProvider;

    /*!
      Device Get State.
     */

    std::mutex mLockCallGetDeviceState;

    /*!
      Device Set State.
     */

    std::mutex mLockCallSetDeviceState;

    /*!
      Service provider call lock.
     */

    std::mutex mLockCallSerProvider;

    /*!
      Device interface call lock.
     */

    std::mutex mLockCallInterface;

    /*!
      Configuration of shadow controller, type json.
     */

    json configurationStore;

    /*!
      A natural status timeout which will update the cloud after every 5 minutes. Disabled.
     */
    int shadowSetStateTimeOut = 300;

    /*!
      The device mask.
     */

    TransportMask _maskSCGlo;

    /*!
      This function receives the configuration of shadow controller.
     */

    void ConfigurationStorage();

    /*!
      This function creates the shadow json object.

      \param devId this is the device ID of the device.
      \param raml this is the raml.
     */

    void CreateShadowJson(const std::string devId);

    std::string filePath = "/opt/GA.conf.d/persistant.conf";

    /*!
      Shadow directory creator
     */

    void createDirectory();

};

#endif

