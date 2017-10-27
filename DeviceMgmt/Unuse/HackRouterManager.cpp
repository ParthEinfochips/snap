#include "HackRouterManager.h"
#include "configSensorManager.h"

#define QCC_MODULE "DMA_HRM"
/**
 * Abstract base class implemented by AllJoyn users and called by AllJoyn to inform
 * users of About interface related events.
 */
HackRouterManager::HackRouterManager(BusAttachment& bus, const char* path)
: BusObject(path), devConfigInfo(NULL)
{
	initHRManager(bus);
}

QStatus HackRouterManager::initHRManager(BusAttachment& bus)
{
	QStatus status;

    /* Create org.alljoyn.eIAJ.Router interface */
    qcc::String ifaceStr= "<node>"
                    "<interface name='" + qcc::String(HAWKROUTER_INTERFACE_NAME) + "'>"
                    "  <method name='TransferOwnerShip'>"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "  </method>"
                    "  <method name='GetHackDeviceData'>"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "  </method>"
                    "  <method name='TransferDevices'>"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "  </method>"
                    "  <method name='GetDeviceState'>"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "  </method>"
                    "  <method name='SetDeviceState'>"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "  </method>"
                    "  <method name='SendDeviceStateResponse'>"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "  </method>"
                    "  <method name='UpdateSourceDMA'>"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "  </method>"
                    "  <method name='SendResponseToCloud'>"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='out_arg' type='s' direction='in' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "    <arg name='return_arg' type='s' direction='out' />"
                    "  </method>"
                    "</interface>"
                    "</node>";

    status = bus.CreateInterfacesFromXml(ifaceStr.c_str());
    if (ER_OK != status) {
            std::cout << "EROR : Failed to parse the xml interface definition. Status - " << QCC_StatusText(status) << std::endl;
            QCC_LogMsg(("EROR : Failed to parse the xml interface definition (%s)",QCC_StatusText(status)));
    }


	const InterfaceDescription* iface = bus.GetInterface(HAWKROUTER_INTERFACE_NAME);

	if((iface)
			&& (ER_OK == (status = AddInterface(*iface, ANNOUNCED))))
	{
		{
			/* Register the method handlers with the object */
			const MethodEntry methodEntries[] = {
					{ iface->GetMember("TransferOwnerShip"), static_cast<MessageReceiver::MethodHandler>(&HackRouterManager::GetTransferOwnerShipSignal) }
			};
			AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
		}
		{
			/* Register the method handlers with the object */
			const MethodEntry methodEntries[] = {
					{ iface->GetMember("GetHackDeviceData"), static_cast<MessageReceiver::MethodHandler>(&HackRouterManager::GetRouterDeviceData) }
			};
			AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
		}
		{
			/* Register the method handlers with the object */
			const MethodEntry methodEntries[] = {
					{ iface->GetMember("TransferDevices"), static_cast<MessageReceiver::MethodHandler>(&HackRouterManager::TransferDevicesHard) }
			};
			AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
		}

		{
			/* Register the method handlers with the object */
			const MethodEntry methodEntries[] = {
					{ iface->GetMember("GetDeviceState"), static_cast<MessageReceiver::MethodHandler>(&HackRouterManager::GetDeviceState) }
			};
			AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
		}

		{
			/* Register the method handlers with the object */
			const MethodEntry methodEntries[] = {
					{ iface->GetMember("SetDeviceState"), static_cast<MessageReceiver::MethodHandler>(&HackRouterManager::SetDeviceState) }
			};
			AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
		}

		{
			/* Register the method handlers with the object */
			const MethodEntry methodEntries[] = {
					{ iface->GetMember("SendDeviceStateResponse"), static_cast<MessageReceiver::MethodHandler>(&HackRouterManager::SendDeviceStateResponse) }
			};
			AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
		}

        {
            /* Register the method handlers with the object */
            const MethodEntry methodEntries[] = {
                    { iface->GetMember("UpdateSourceDMA"), static_cast<MessageReceiver::MethodHandler>(&HackRouterManager::UpdateSourceDMA) }
            };
            AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
        }

        {
            /* Register the method handlers with the object */
            const MethodEntry methodEntries[] = {
                    { iface->GetMember("SendResponseToCloud"), static_cast<MessageReceiver::MethodHandler>(&HackRouterManager::SendResponseToCloud) }
            };
            AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
        }

	} else {
		std::cout << "EROR : Failed to add" << HAWKROUTER_INTERFACE_NAME << " interface to the BusObject." << std::endl;
		throw std::runtime_error("Fail to initialize HR Manager !!");
		QCC_ASSERT(iface != NULL);
	}

    status = bus.RegisterBusObject(*this);
    if (ER_OK == status) {
        std::cout << "INFO : (DMA) RegisterBusObject - " << "Succeeded." << std::endl;
    } else {
        std::cout << "EROR : (DMA) RegisterBusObject - " << "Failed _ Status -" <<  QCC_StatusText(status ) << std::endl;
        return status;
    }

	return status;
}

void HackRouterManager::GetTransferOwnerShipSignal(const InterfaceDescription::Member* member, Message& msg) {
	QCC_UNUSED(member);

	std::cout << "INFO : GetTransferOwnerShipSignal ..  " << std::endl;

	const ajn::MsgArg* returnArgs;
	size_t numArgs;
	msg->GetArgs(numArgs, returnArgs);
	if(numArgs == 2)
	{
		std::cout << "INFO : GET Payload -   "<< msg->GetArg(0)->v_string.str << std::endl;
		std::cout << "INFO : GET DevID   -   "<< msg->GetArg(1)->v_string.str << std::endl;
        std::unique_ptr<takeOwnershipCommand> commandPtr(new takeOwnershipCommand);
        commandPtr->commandDataObj.jsonPaylod = msg->GetArg(0)->v_string.str;
//        commandPtr->setDevicePtr(devConfigInfo->getDeviceProxyPtr());
        commandPtr->setConfigManager(devConfigInfo);
        commandPtr->execute();
//        devConfigInfo->registerTransferredDevice(msg->GetArg(0)->v_string.str,msg->GetArg(1)->v_string.str);
    }
    else
    {
		std::cout << "EROR : Failed to proceed GetTransferOwnerShipSignal()." << std::endl;
	}

	std::string str1 = R"({"CLIENTID":"1234","JOBID":"1234","properties":[{"property":"GetTransferOwnerShipSignal"}]})";
	std::string str2 = "HackRouter:64:00:6A:09:DC:A6";

	MsgArg args[2];
	args[0].Set("s", str1.c_str());
	args[1].Set("s", str2.c_str());

	QStatus status;
	if (!(msg->GetFlags() & ALLJOYN_FLAG_NO_REPLY_EXPECTED)) {
		status = MethodReply(msg, args, 2);
		if (status != ER_OK) {
			std::cout << "EROR : Failed to created MethodReply." << std::endl;
		}
	}
}

/* Testing API */
/* Respond to remote method call `Echo` by returning the string back to the sender.*/
void HackRouterManager::GetRouterDeviceData(const InterfaceDescription::Member* member, Message& msg) {
	QCC_UNUSED(member);

	std::cout << "INFO : GetRouterDeviceData ..  " << std::endl;
	const ajn::MsgArg* returnArgs;
	size_t numArgs;
	msg->GetArgs(numArgs, returnArgs);

	if(numArgs == 2)
	{
		std::cout << "INFO : GET Payload -   "<< msg->GetArg(0)->v_string.str << std::endl;
		std::cout << "INFO : GET DevID   -   "<< msg->GetArg(1)->v_string.str << std::endl;
		devConfigInfo->sendTelemetry(msg->GetArg(0)->v_string.str);
	} else {
		std::cout << "EROR : Failed to proceed GetRouterDeviceData()." << std::endl;
	}


	std::string str1 = R"({"CLIENTID":"1234","JOBID":"1234","properties":[{"property":"GetRouterDeviceData"}]})";
	std::string str2 = "HackRouter:64:00:6A:09:DC:A6";

	MsgArg args[2];
	args[0].Set("s", str1.c_str());
	args[1].Set("s", str2.c_str());

	QStatus status;
	if (!(msg->GetFlags() & ALLJOYN_FLAG_NO_REPLY_EXPECTED)) {
		status = MethodReply(msg, args, 2);
		if (status != ER_OK) {
			std::cout << "EROR : Failed to created MethodReply." << std::endl;
		}
	}
}

void HackRouterManager::TransferDevicesHard(const InterfaceDescription::Member* member, Message& msg) {
	QCC_UNUSED(member);

	std::cout << "INFO : TransferDevicesHard ..  " << std::endl;
	const ajn::MsgArg* returnArgs;
	size_t numArgs;
	msg->GetArgs(numArgs, returnArgs);

	if(numArgs == 2)
	{
		std::cout << "INFO : GET Payload -   "<< msg->GetArg(0)->v_string.str << std::endl;
		std::cout << "INFO : GET DevID   -   "<< msg->GetArg(1)->v_string.str << std::endl;
		devConfigInfo->transferDevicess(msg->GetArg(0)->v_string.str);
	} else {
		std::cout << "EROR : Failed to proceed TransferDevicesHard()." << std::endl;
	}

	std::string str1 = R"({"CLIENTID":"1234","JOBID":"1234","properties":[{"property":"TransferDevicesHard"}]})";
	std::string str2 = "HackRouter:64:00:6A:09:DC:A6";

	MsgArg args[2];
	args[0].Set("s", str1.c_str());
	args[1].Set("s", str2.c_str());

	QStatus status;
	if (!(msg->GetFlags() & ALLJOYN_FLAG_NO_REPLY_EXPECTED)) {
		status = MethodReply(msg, args, 2);
		if (status != ER_OK) {
			std::cout << "EROR : Failed to created MethodReply." << std::endl;
		}
	}
}

void HackRouterManager::GetDeviceState(const InterfaceDescription::Member* member, Message& msg) {
	QCC_UNUSED(member);

	std::cout << "INFO : GetDeviceState ..  " << std::endl;
	std::string payLoad;
	std::string DevID;
	const ajn::MsgArg* returnArgs;
	size_t numArgs;
	msg->GetArgs(numArgs, returnArgs);

	if(numArgs == 2)
	{
		std::cout << "INFO : GET Payload -   "<< msg->GetArg(0)->v_string.str << std::endl;
		std::cout << "INFO : GET DevID   -   "<< msg->GetArg(1)->v_string.str << std::endl;
		payLoad = msg->GetArg(0)->v_string.str;
		devConfigInfo->getDeviceState(payLoad);
		DevID = msg->GetArg(1)->v_string.str;
	} else {
		std::cout << "EROR : Failed to proceed TransferDevicesHard()." << std::endl;
	}

	MsgArg args[2];
	args[0].Set("s", payLoad.c_str());
	args[1].Set("s", DevID.c_str());

	QStatus status;
	if (!(msg->GetFlags() & ALLJOYN_FLAG_NO_REPLY_EXPECTED)) {
		status = MethodReply(msg, args, 2);
		if (status != ER_OK) {
			std::cout << "EROR : Failed to created MethodReply." << std::endl;
		}
	}
}

void HackRouterManager::SetDeviceState(const InterfaceDescription::Member* member, Message& msg) {
	QCC_UNUSED(member);

	std::cout << "INFO : SetDeviceState ..  " << std::endl;
	std::string payLoad;
	std::string DevID;
	const ajn::MsgArg* returnArgs;
	size_t numArgs;
	msg->GetArgs(numArgs, returnArgs);

	if(numArgs == 2)
	{
		std::cout << "INFO : GET Payload -   "<< msg->GetArg(0)->v_string.str << std::endl;
		std::cout << "INFO : GET DevID   -   "<< msg->GetArg(1)->v_string.str << std::endl;
		payLoad = msg->GetArg(0)->v_string.str;
		devConfigInfo->setDeviceState(payLoad);
		DevID = msg->GetArg(1)->v_string.str;
	} else {
		std::cout << "EROR : Failed to proceed TransferDevicesHard()." << std::endl;
	}

	MsgArg args[2];
	args[0].Set("s", payLoad.c_str());
	args[1].Set("s", DevID.c_str());

	QStatus status;
	if (!(msg->GetFlags() & ALLJOYN_FLAG_NO_REPLY_EXPECTED)) {
		status = MethodReply(msg, args, 2);
		if (status != ER_OK) {
			std::cout << "EROR : Failed to created MethodReply." << std::endl;
		}
	}
}


void HackRouterManager::SendDeviceStateResponse(const InterfaceDescription::Member* member, Message& msg) {
	QCC_UNUSED(member);

	std::cout << "INFO : SendDeviceStateResponse ..  " << std::endl;
	std::string payLoad;
	std::string DevID;
	const ajn::MsgArg* returnArgs;
	size_t numArgs;
	msg->GetArgs(numArgs, returnArgs);

	if(numArgs == 2)
	{
		std::cout << "INFO : GET Payload -   "<< msg->GetArg(0)->v_string.str << std::endl;
		std::cout << "INFO : GET DevID   -   "<< msg->GetArg(1)->v_string.str << std::endl;
		payLoad = msg->GetArg(0)->v_string.str;
		devConfigInfo->sendDeviceStateResponse(payLoad);
		DevID = msg->GetArg(1)->v_string.str;
	} else {
		std::cout << "EROR : Failed to proceed TransferDevicesHard()." << std::endl;
	}

	MsgArg args[2];
	args[0].Set("s", payLoad.c_str());
	args[1].Set("s", DevID.c_str());

	QStatus status;
	if (!(msg->GetFlags() & ALLJOYN_FLAG_NO_REPLY_EXPECTED)) {
		status = MethodReply(msg, args, 2);
		if (status != ER_OK) {
			std::cout << "EROR : Failed to created MethodReply." << std::endl;
		}
	}
}

void HackRouterManager::SendResponseToCloud(const InterfaceDescription::Member* member, Message& msg) {
    QCC_UNUSED(member);

    std::cout << "INFO : SendResponseToCloud ..  " << std::endl;
    std::string payLoad;
    std::string DevID;
    const ajn::MsgArg* returnArgs;
    size_t numArgs;
    msg->GetArgs(numArgs, returnArgs);

    if(numArgs == 2)
    {
        std::cout << "INFO : GET Payload -   "<< msg->GetArg(0)->v_string.str << std::endl;
        std::cout << "INFO : GET DevID   -   "<< msg->GetArg(1)->v_string.str << std::endl;
        payLoad = msg->GetArg(0)->v_string.str;
        devConfigInfo->SendResponseToCloud(payLoad);
        DevID = msg->GetArg(1)->v_string.str;
    } else {
        std::cout << "EROR : Failed to proceed TransferDevicesHard()." << std::endl;
    }

    MsgArg args[2];
    args[0].Set("s", payLoad.c_str());
    args[1].Set("s", DevID.c_str());

    QStatus status;
    if (!(msg->GetFlags() & ALLJOYN_FLAG_NO_REPLY_EXPECTED)) {
        status = MethodReply(msg, args, 2);
        if (status != ER_OK) {
            std::cout << "EROR : Failed to created MethodReply." << std::endl;
        }
    }
}

void HackRouterManager::UpdateSourceDMA(const InterfaceDescription::Member* member, Message& msg) {
    QCC_UNUSED(member);

    std::cout << "INFO : UpdateSourceDMA ..  " << std::endl;
    std::string payLoad;
    std::string DevID;
    const ajn::MsgArg* returnArgs;
    size_t numArgs;
    msg->GetArgs(numArgs, returnArgs);

    if(numArgs == 2)
    {
        std::cout << "INFO : GET Payload -   "<< msg->GetArg(0)->v_string.str << std::endl;
        std::cout << "INFO : GET DevID   -   "<< msg->GetArg(1)->v_string.str << std::endl;
        payLoad = msg->GetArg(0)->v_string.str;
//        devConfigInfo->updateSourceDMA(payLoad);
        DevID = msg->GetArg(1)->v_string.str;
    } else {
        std::cout << "EROR : Failed to proceed TransferDevicesHard()." << std::endl;
    }

    MsgArg args[2];
    args[0].Set("s", payLoad.c_str());
    args[1].Set("s", DevID.c_str());

    QStatus status;
    if (!(msg->GetFlags() & ALLJOYN_FLAG_NO_REPLY_EXPECTED)) {
        status = MethodReply(msg, args, 2);
        if (status != ER_OK) {
            std::cout << "EROR : Failed to created MethodReply." << std::endl;
        }
    }
}

/* Respond to remote method call `GetDMADevicesData` by returning the string back to the sender.*/
//virtual void GetDMADevicesData(qcc::String const& busName, unsigned short port) = 0;

/* Respond to remote method call `GetDMADevicesData` by returning the string back to the sender.*/
//virtual void TransferOwnerShipDMA(qcc::String const& busName, ajn::SessionId id) = 0;
ConfigSensorManager *HackRouterManager::getDevConfigInfo() const{
	return devConfigInfo;
}
void HackRouterManager::setDevConfigInfo(ConfigSensorManager *value){
	devConfigInfo = value;
}
