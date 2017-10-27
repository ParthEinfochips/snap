/**
 * @file
 * DeviceObj is for implementing the daemon-to-daemon inteface org.alljoyn.device
 */

/******************************************************************************
 * Copyright eInfochips. All rights reserved.
 *
 ******************************************************************************/
#ifndef _ALLJOYN_DEVICEOBJ_H
#define _ALLJOYN_DEVICEOBJ_H

#include <map>
#include <set>
#include <queue>
#include <qcc/String.h>
#include <qcc/Timer.h>
#include <qcc/platform.h>
#include <qcc/Logger.h>
#include <qcc/String.h>
#include <qcc/Util.h>
#include <qcc/atomic.h>
#include <qcc/LockLevel.h>
#include <qcc/Thread.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/Message.h>
#include <alljoyn/SessionPortListener.h>
#include <alljoyn/SessionListener.h>
#include "CommonHeader/CPPHeaders.h"
#include "TransportWrapper.h"
#include "../ServiceProvider.h"
#include "../ShadowController/src/shadowcontroller.h"

#define TRANPORT_INVALID_ERROR_MSG "Transport not available or invalid."
#define INVALID_MSG_ARG "Invalid Message arguments."
#define CREATE_TRANSPORT_LIBRARY_NOT_EXIST "library does not exist."
#define CREATE_TRANSPORT_LIBRARY_NOT_LOADED "library does not exist."
#define CREATE_TRANSPORT_UNABLE_TO_CALL_CREATE "Cannot load symbol create()."
#define CREATE_TRANSPORT_EXCEPTION_RAISED "Exception raised while loading library"
#define CREATE_TRANSPORT_UNABLE_TO_CREATE "Unable to create Transport object."
#define DISCOVER_IN_PROGRESS "Discovery already inprogress."
#define DISCOVER_REQUEST_QUEUED "Discovery request queued."
#define DISCOVER_REQUEST_ERROR "Something wrong in discovery."
#define DEVICE_NOT_REGISTERED_IN_DO "Device not registered."

using namespace eITM;
using namespace ajn;

//#define USE_SHADOW_CONTROLLER_TEST //This disables the simulated environment.
#define USE_SHADOW_CONTROLLER //To disable shadow controller, disable this flag.

static const struct code_map divice_api_codes[] =
{ { 201, "StartDiscovery" },
  { 202, "StopDiscovery" },
  { 203, "AddDevice" },
  { 204, "DeleteDevice" },
  { 205, "GetDeviceRAML" },
  { 206, "SetDeviceRAML" },
  { 207, "GetDeviceXML" },
  { 208, "SetDeviceXML" },
  { 209, "GetDeviceState" },
  { 301, "SetDeviceState" },
  { 302, "GetDeviceStatus" },
  { 303, "SetDeviceStatus" },
  { 304, "RegisterDeviceProperties" },
  { 305, "DeregisterDeviceProperties" },
  { 306, "GetDeviceList" },
  { 307, "RegisterDeviceCallback" },
  { 307, "DeregisterDeviceCallback" },
  {   0, "NULL" }
};

class ServiceProvider;
class TransportWrapper;

/**
 * BusObject responsible for implementing the standard AllJoyn interface org.alljoyn.sl.
 */
class DeviceObj {

public:
	/**
	 * Constructor
	 *
	 */
	DeviceObj(TransportMask mask, ServiceProvider *_servicProPtr);

	/**
	 * Destructor
	 */
	~DeviceObj();

	/**
	 * Initialize and register this DBusObj instance.
	 *
	 * @return ER_OK if successful.
	 */
	QStatus Init();

	/**
	 * Stop DeviceObj.
	 *
	 * @return ER_OK if successful.
	 */
	QStatus Stop();

	/**
	 * Join DeviceObj.
	 *
	 * @return ER_OK if successful.
	 */
	QStatus Join();

	/**
	 * Called when object is successfully registered.
	 */
	void ObjectRegistered(void);

	/**-----------------------------------------------------------------------------------------**/

	/**
	 * Start for device protocol discovery.
	 *
	 * The input Message (METHOD_CALL) is expected to contain the following parameters:
	 *   sessionPort  sessionPort    SessionPort identifier.
	 *   opts         SessionOpts    SessionOpts that must be agreeable to any joiner.
	 *
	 * The output Message (METHOD_REPLY) contains the following parameters:
	 *   resultCode   uint32   An ALLJOYN_BINDSESSIONPORT_* reply code (see AllJoynStd.h).
	 *
	 * @param  devId     Device Id.
	 * @param  devId     The incoming message.
	 */
	void StartDiscovery(std::string &devId, std::string &payload);

	/**
	 * Stop for device protocol discovery.
	 *
	 * The input Message (METHOD_CALL) is expected to contain the following parameters:
	 *   sessionPort  sessionPort    SessionPort identifier.
	 *   opts         SessionOpts    SessionOpts that must be agreeable to any joiner.
	 *
	 * The output Message (METHOD_REPLY) contains the following parameters:
	 *   resultCode   uint32   An ALLJOYN_BINDSESSIONPORT_* reply code (see AllJoynStd.h).
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void StopDiscovery( std::string &devId, std::string &payload);

	/**
	 * Add new device.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void AddDevice( std::string &devId, std::string &payload);

	/**
	 * Delete device.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void DeleteDevice( std::string &devId, std::string &payload);

	/**
	 * Get device model.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void GetDeviceRAML( std::string &devId, std::string &payload);

	/**
	 * Get device model.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void SetDeviceRAML( std::string &devId, std::string &payload);

	/**
	 * Get device model.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void GetDeviceXML( std::string &devId, std::string &payload);

	/**
	 * Get device model.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */

	void GetAllRaml(std::string &devId, std::string &payload);

	void SetDeviceXML( std::string &devId, std::string &payload);

	/**
	 * Get device state.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void GetDeviceState( std::string &devId, std::string &payload);

	/**
	 * Set device state.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void SetDeviceState( std::string &devId, std::string &payload);

	/**
	 * Get the device status.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */

	void StatusChange( std::string &devId, std::string &payload);



	void GetDeviceStatus( std::string &devId, std::string &payload);

	/**
	 * Set the device status.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void SetDeviceStatus( std::string &devId, std::string &payload);

	/**
	 * Register the device properties.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void RegisterDeviceProperties( std::string &devId, std::string &payload);

	/**
	 * Deregister the device properties.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void DeregisterDeviceProperties( std::string &devId, std::string &payload);

	/**
	 * Register the device callback.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void RegisterDeviceCallback( std::string &devId, std::string &payload);

	/**
	 * Deregister the device callback.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void DeregisterDeviceCallback( std::string &devId, std::string &payload);

	/**
	 * Check Device Registered or not.
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void CheckDeviceExist(const InterfaceDescription::Member* member, Message& msg);

	/**
	 * Get Device List .
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void GetDeviceList( std::string &devId, std::string &payload);

	/**
	 * Device Exists .
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	QStatus DeviceExists(std::string &payload,std::string &endpointadd);

	/**
	 * Device Exists .
	 *
	 * @param  member  Member.
	 * @param  msg     The incoming message.
	 */
	void SCconfig(std::string &command, std::string &payload);
	/**-----------------------------------------------------------------------------------------**/

	/**
	 * Get TransportMask from DeviceMapEntry for particular device id.
	 *
	 * @param[out]  TransportMask  Transport Mask to be filled.
	 * @param[in]  msg     The incoming message.
	 */
	bool extractandchktransport(TransportMask &transportMask, std::string &devId);

	/**
	 * Generate Reply for the MethodCall.
	 *
	 * @param[in]  InterfaceName  request type.
	 * @param[in]  msg     The incoming message.
	 * @param[in]  status  status of request.
	 * @param[in]  devId   device id in request.
	 * @param[in]  payload   payload in request.
	 * @param[in]  devStatus  request status .
	 */
	//void GenerateReply(InterfaceName iName, Message& msg, QStatus status, String iface, String devId, String payload, int devStatus);

	/**
	 * Check whether device is registered or not
	 *
	 * @param[in]  msg     The incoming message.
	 * @param[in]  devId   Device unique id.
	 * @param[in]  TransportMask transport type.
	 */
	bool checkDeviceInSensorMap(std::string strSender, std::string devId, TransportMask transportMask);

	/**
	 * Check Transport is Valid and if valid then create transport if not available
	 *
	 * @param[in]  msg     The incoming message.
	 * @param[in]  InterfaceName  request type.
	 * @param[in]  TransportMask transport type.
	 */
	QStatus checkAndCreateTransport(TransportMask transportMask, std::string &strReturn);

	/**
	 * Utility function used to send a single DiscoveredDevices signal.
	 *
	 * @param dest        Unique name of destination.
	 * @param name        Well-known name that was found.
	 * @param transport   The transport that received the advertisment.
	 * @param namePrefix  Well-known name prefix used in call to StartDiscovery() that triggered this notification.
	 * @return ER_OK if succssful.
	 */
	QStatus SendDiscoveredDevices(const std::string& dest, const std::string& data);

	/**
	 * Receive notification of a activated device via TransportListener.
	 * Internal use only.
	 *
	 * @param   sender    Destination where data sent
	 *          transport Transport that received the data.
	 *          macAddr   Mac Address of Device.
	 *          data      Data list of the activated device.
	 */
	void DeviceData(const std::string &senID, std::string data);

	QStatus WrappedCallTransportInterface(InterfaceName iName, std::string &devId, std::string &payload);

	ServiceProvider *servicProPtr;

	/*
	 * Device Object's _Transport Mask.
	 */

	TransportMask _TMaskGlo;

	TransportMask transportmask_;

#ifdef USE_SHADOW_CONTROLLER
	std::unique_ptr<shadowController> shadowUniqPtr = nullptr;
#endif

private:

	/**
	 * Receive notification of a new device discovered via TransportListener.
	 * Internal use only.
	 *
	 * @param   transport Transport that received the advertisement.
	 * @param   names     List of device names advertised by the discovered device.
	 * @param   ttl       Number of seconds before this advertisement expires
	 *                    (0 means expire immediately, numeric_limits<uint32_t>::max() means never expire)
	 */
	void DiscoveredDevices(std::string sender, TransportMask transportMask, std::string data, uint32_t ttl);

	/**
	 * Receive notification of a activated device via TransportListener.
	 * Internal use only.
	 *
	 * @param   sender    Destination where data sent
	 *          transport Transport that received the data.
	 *          macAddr   Mac Address of Device.
	 *          data      Data list of the activated device.
	 *          ttl       Number of seconds before this data signal expires
	 *                    (0 means expire immediately, numeric_limits<uint32_t>::max() means never expire)
	 */
	void DeviceData(const std::string &senID, std::string data, uint32_t ttl);

	/**
	 * Utility function used to send a single DeviceData signal.
	 *
	 * @param dest        Unique name of destination.
	 * @param transport   The transport that received the advertisment.
	 *        macAddr     Mac Address of the Device.
	 *        data        Data to be send on specified mac address.
	 * @return ER_OK if succssful.
	 */
	QStatus SendDeviceData(const std::string& dest, const std::string& devId, const std::string &data);

	/**
	 * Receive notification of a new bus instance via TransportListener.
	 * Internal use only.
	 *
	 * @param   busAddr   Address of discovered bus.
	 * @param   guid      GUID of daemon that sent the advertisement.
	 * @param   transport Transport that received the advertisement.
	 * @param   names     Vector of bus names advertised by the discovered bus.
	 * @param   ttl       Number of seconds before this advertisement expires
	 *                    (0 means expire immediately, numeric_limits<uint32_t>::max() means never expire)
	 */
	void FoundNames(const std::string& busAddr, const std::string& guid, TransportMask transport, const std::vector<std::string>* names, uint32_t ttl);


	/**
	 * DeviceObj worker.
	 *
	 * @param alarm  The alarm object for the timeout that expired.
	 */
	void AlarmTriggered(const qcc::Alarm& alarm, QStatus reason);

	/**
	 * Fetch arguments from Message objects
	 */
	void FetchArguments(Message& msg, TransportMask& transportMask);

	/**
	 * Fetch arguments from Message objects
	 */
	void FetchArguments(Message& msg, std::string& sender, std::string& namePrefix);

	/**
	 * Fetch arguments from Message objects
	 */
	void FetchArguments(Message& msg, TransportMask& transportMask, std::string& devId, std::string& payLoad);

	/**
	 * Fetch arguments from Message objects
	 */
	void FetchArguments(Message& msg, std::string& devId);

	/**
	 * Fetch Argument from Message
	 *
	 * @param[in]   Message   message.
	 * @param[out]   String    device id .
	 * @param[out]   String    payload .
	 */
	void FetchArgumentsWT(Message& msg, std::string& devId, std::string& payLoad);

	/**
	 * Get Transport error code as per Interface
	 */
	unsigned int GetTransportErrorCode(InterfaceName iName);

	/**
	 * Get Transport error code as per Interface
	 */
	unsigned int GetReplyFailErrorCode(enum InterfaceName iName);

	/**
	 * Check the transport is valid or not
	 *
	 * @param   transport Transport to be validate.
	 * @return true if valid transport else false
	 */
	bool IsValidTransport(const TransportMask& transportMask);

	/**
	 * Check the transport is available into Transport Factory list
	 *
	 * @param   transport Transport to be found.
	 * @return true if found else false
	 */
	bool IsTransportAvailable(const TransportMask& transportMask);

	/**
	 * Create new transport and add into Transport Factory list
	 *
	 * @param   transport Transport to be created.
	 * @return ER_OK if successful.
	 */
	QStatus CreateNewTransport(const TransportMask& transportMask, std::string &strReturn);

	/**
	 * Get the respective transport object
	 */
	TransportWrapper* GetTransport(TransportMask& transportMask);

	/**
	 * Call Interface of respective transport
	 */
	QStatus CallTransportInterface(InterfaceName iName, std::string &devId, std::string &payload);

	std::vector<TransportMask> deviceTransportList; /**< The Transport List */

	std::map<TransportMask,TransportWrapper*> transportList; /** Map containing the TransportWrapper for particular transport type */

	//const InterfaceDescription* deviceIface;  /**< com.einfochips.Device interface */

	//const InterfaceDescription::Member* deviceConnectionLostSignal;      /**< com.einfochips.Device.ConnectionLost signal */


	/** Map of active discovery names to requesting local endpoint's permitted transport mask(s) and name(s) */
	struct DiscoverMapEntry {
		TransportMask transportMask;
		std::string sender;
		DiscoverMapEntry(TransportMask transportMask, const std::string& sender) :
			transportMask(transportMask),
			sender(sender) { }
	};
	typedef std::multimap<std::string, DiscoverMapEntry> DiscoverMapType;
	DiscoverMapType m_discoverMap;

	/** Map of active sensors for storing routing the sensor data to particual endpoint */
	struct SensorMapEntry {
		TransportMask transportMask;
		std::string sender;
		SensorMapEntry(TransportMask transportMask, const std::string& sender) :
			transportMask(transportMask),
			sender(sender) { }
	};
	typedef std::multimap<std::string, SensorMapEntry> SensorMapType;
	SensorMapType m_sensorMap;

	/** Map of active devices for storinbg transport mask */
	struct DeviceMapEntry {
		TransportMask transportMask;
		DeviceMapEntry(TransportMask transportMask) :
			transportMask(transportMask) { }
	};
	typedef std::map<std::string, DeviceMapEntry> DeviceMapType;
	DeviceMapType m_deviceMap;

	/** Vector Will Store Device List. */
	struct DeviceInfo {
		std::string strDeviceId;
		std::string strMetaData;
	};
	DeviceInfo devInfo;

	std::vector<DeviceInfo> DeviceListVector;

	bool enablePersistant = true;

protected:
	/** FuncCallbackThread handles a function callback request on a separate thread */
	class FuncCallbackThread : public qcc::Thread
	{
	public:
		FuncCallbackThread(DeviceObj& dObj, TransportWrapper& transW)
	: m_dObj(dObj)
	, m_transWrapper(transW)
	{}
		void setInput(std::string data){
			input = data;
		}
		void setSender(std::string send)
		{
			sender = send;
		}

	protected:
		qcc::ThreadReturn STDCALL Run(void* arg);

	private:
		DeviceObj&          m_dObj;
		TransportWrapper&   m_transWrapper;
		std::string         input;
		std::string         sender;
	};

	FuncCallbackThread*     m_callbackThread;
};

#endif //_ALLJOYN_DEVICEOBJ_H
