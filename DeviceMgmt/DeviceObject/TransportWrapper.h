#ifndef TRANSPORT_WRAPPER_H
#define TRANSPORT_WRAPPER_H

#include <dlfcn.h>
#include "ajcommon/CustomTransportMask.h"
#include "Status.h"
#include "../../../../Library/PTL/common/inc/devProtocolRequestHandler.h"
#include "DeviceObj.h"
//#include "Transport.h"
//#include "../ShadowController/ShadowController.h"

using namespace eITM;
class DeviceObj;

class TransportWrapper
{
public:
	TransportWrapper(TransportMask transport);
	~TransportWrapper();

	/**
	 * @internal
	 * @brief  Discover the devices.
	 */
	void Discover(std::string &data);

	/**
	 * @internal
	 * @brief  Add the new device.
	 */
	void AddDevice(std::string& deviceID, std::string& payload);

	/**
	 * @internal
	 * @brief  Delete the device.
	 */
	void DeleteDevice(std::string& deviceID, std::string &data);

	/**
	 * @internal
	 * @brief  Get the device model.
	 */
	void GetDeviceRAML(std::string& deviceID, std::string& payload);

	/**
	 * @internal
	 * @brief  Set the device model.
	 */
	void SetDeviceRAML(std::string& deviceID, std::string& payload);

	/**
	 * @internal
	 * @brief  Get the device state.
	 */
	void GetDeviceState(std::string& deviceID, std::string& payload);

	/**
	 * @internal
	 * @brief  Set the device state.
	 */
	void SetDeviceState(std::string& deviceID, std::string& payload);

	/**
	 * @internal
	 * @brief  Get the device status.
	 */
	void GetDeviceStatus(std::string& deviceID, std::string& payload);

	/**
	 * @internal
	 * @brief  Set the device status.
	 */
	void SetDeviceStatus(std::string& deviceID, std::string& payload);

	/**
	 * @internal
	 * @brief  Register the device properties.
	 */
	void RegisterDeviceProperties(std::string& deviceID, std::string& payload);

	/**
	 * @internal
	 * @brief  Deregister the device properties.
	 */
	void DeregisterDeviceProperties(std::string& deviceID, std::string& payload);

	/**
	 * @internal
	 * @brief  Register the device callback.
	 */
	QStatus RegisterDeviceCallback(std::string& devId, std::string& data);

	/**
	 * @internal
	 * @brief  Deregister the device callback.
	 */
	QStatus DeregisterDeviceCallback(std::string& devId, std::string& data);

	/**
	 * @internal
	 * @brief  Will Get Device XML.
	 */
	void GetDeviceXML(std::string& devId, std::string& data);

	/**
	 * @internal
	 * @brief  Will Set Device XML.
	 */
	void SetDeviceXML(std::string& devId, std::string& data);

	/**
	 * @internal
	 * @brief  Change the status of multiple devices.
	 */
	void StatusChange(std::string& devId, std::string& data);

	std::shared_ptr<DevProtocolRequestHandler> LoadBLETransport();
	std::shared_ptr<DevProtocolRequestHandler> LoadOPCTransport();
	std::shared_ptr<DevProtocolRequestHandler> LoadZigbeeTransport();
	std::shared_ptr<DevProtocolRequestHandler> LoadZwaveTransport();
	std::shared_ptr<DevProtocolRequestHandler> LoadModbusTransport();
	std::shared_ptr<DevProtocolRequestHandler> LoadCoAPTransport();
	std::shared_ptr<DevProtocolRequestHandler> LoadThreadTransport();
	std::shared_ptr<DevProtocolRequestHandler> LoadMQTTSimulatorTransport();

	bool Init();

	void DataReceived(std::string devId, std::string data);
	//void SetListener(DeviceListener *listener);
	void SetListener(DeviceObj *listener);

	unsigned int m_errorCode;

	TransportMask transport() const;
	void setTransport(const TransportMask &transport);

private:

	std::shared_ptr<DevProtocolRequestHandler> transportPtr;
	DeviceObj*              m_listener;
	TransportMask           m_transport;
	//    ShadowController*       m_shadowCtrl;

}; // TRANSPORT_WRAPPER_H

#endif
