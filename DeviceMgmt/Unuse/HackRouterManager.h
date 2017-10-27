#ifndef HACKROUTERMANAGER_HH
#define HACKROUTERMANAGER_HH

#include <alljoyn/BusObject.h>
#include <alljoyn/BusAttachment.h>
//#include "../configSensorManager.h"
#include <qcc/Debug.h>

using namespace ajn;

class ConfigSensorManager;

//static const char* HAWKROUTER_INTERFACE_NAME = "org.alljoyn.HackRouter";
//static const char* HAWKROUTER_INTERFACE_PATH = "/HawkEdge/Router";

/**
 * Abstract base class implemented by AllJoyn users and called by AllJoyn to inform
 * users of About interface related events.
 */
class HackRouterManager : public BusObject {

	ConfigSensorManager* devConfigInfo ;

public:
	HackRouterManager(BusAttachment& bus, const char* path);

	/* Respond to remote method call `TransferOwnerShip` by returning the string back to the sender.*/
	void GetTransferOwnerShipSignal(const InterfaceDescription::Member* member, Message& msg);

	/* Respond to remote method call `GetHackDeviceData` by returning the string back to the sender.*/
	void GetRouterDeviceData(const InterfaceDescription::Member* member, Message& msg) ;

	/* Respond to remote method call `TransferDevices` by returning the string back to the sender.*/
	void TransferDevicesHard(const InterfaceDescription::Member* member, Message& msg);

	/* Respond to remote method call `GetDeviceState` by returning the string back to the sender.*/
	void GetDeviceState(const InterfaceDescription::Member* member, Message& msg);

	/* Respond to remote method call `SetDeviceState` by returning the string back to the sender.*/
	void SetDeviceState(const InterfaceDescription::Member* member, Message& msg);

	/* Respond to remote method call `SendDeviceStateResponse` by returning the string back to the sender.*/
	void SendDeviceStateResponse(const InterfaceDescription::Member* member, Message& msg);

    void UpdateSourceDMA(const InterfaceDescription::Member* member, Message& msg);

    void SendResponseToCloud(const InterfaceDescription::Member* member, Message& msg);

	/* Respond to remote method call `GetDMADevicesData` by returning the string back to the sender.*/
	//virtual void GetDMADevicesData(qcc::String const& busName, unsigned short port) = 0;

	/* Respond to remote method call `GetDMADevicesData` by returning the string back to the sender.*/
	//virtual void TransferOwnerShipDMA(qcc::String const& busName, ajn::SessionId id) = 0;

	ConfigSensorManager *getDevConfigInfo() const;

	void setDevConfigInfo(ConfigSensorManager *value);

private:
	QStatus initHRManager(BusAttachment& bus);
};

#endif //HACKROUTERMANAGER_HH
