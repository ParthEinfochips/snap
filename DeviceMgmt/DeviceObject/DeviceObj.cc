/**
 * @file
 * This file implements the API interfaces.
 */

/******************************************************************************
 * Copyright eInfochips. All rights reserved.
 *
 ******************************************************************************/

#include "DeviceObj.h"
#include "CommonHeader/debugLog.h"
#include "ajcommon/CustomStd.h"

#define eI_MODULE "DEVICE_OBJ"



using namespace std;
using namespace qcc;

DeviceObj::DeviceObj(TransportMask mask, ServiceProvider *_servicProPtr)
: _TMaskGlo(mask), m_callbackThread(NULL), servicProPtr(_servicProPtr),transportmask_(TRANSPORT_NONE)
{
	/* Add valid transport masks in deviceTransportList */
	deviceTransportList.push_back(TRANSPORT_EI_BLE);
	deviceTransportList.push_back(TRANSPORT_EI_OPC);
	deviceTransportList.push_back(TRANSPORT_EI_ZIGBEE);
	deviceTransportList.push_back(TRANSPORT_EI_ZWAVE);
	deviceTransportList.push_back(TRANSPORT_EI_MODBUS);
	deviceTransportList.push_back(TRANSPORT_EI_COAP);
	deviceTransportList.push_back(TRANSPORT_EI_THREAD);
	deviceTransportList.push_back(TRANSPORT_EI_MQTTSIMULATOR);


#ifdef USE_SHADOW_CONTROLLER
	shadowUniqPtr = std::make_unique<shadowController>(mask, _servicProPtr, this);
#endif

	if(mask == TRANSPORT_EI_MQTTSIMULATOR)
	{
		std::cout << "TRANSPORT_EI_MQTTSIMULATOR" << std::endl;
		this->Init();
	}
}

DeviceObj::~DeviceObj()
{
	//	for (auto it = transportList.begin(); it != transportList.end() /* not hoisted */; /* no increment */)
	//	{
	//		delete it->second;
	//	}
	//transportList.clear();

#ifdef USE_SHADOW_CONTROLLER
	shadowUniqPtr.reset();
#endif
}

QStatus DeviceObj::Init()
{
	LOGDBUG("In DeviceObj Init .. .. ");

	json initJson;

	std::string filePath = "/opt/GA.conf.d/persistant.conf";

	struct stat fb;

	auto retVal = stat(filePath.c_str(), &fb);

	if(retVal < 0)
	{
		std::cout << "File doesn't exists" << std::endl;
	}
	else
	{
		std::cout << "File exists" << std::endl;

		FileOPS::readFromFile(filePath, initJson);

		for(json::iterator it = initJson.begin(); it != initJson.end(); it++)
		{
			std::string macID = it.key();

			if(it.value() != "")
			{
				std::string deviceId = it.value();

				shadowUniqPtr->persistantShadow(macID, deviceId);
			}
			else
			{
				std::cout << "it.value is empty" << std::endl;
			}
		}
	}

	return ER_OK;
}

QStatus DeviceObj::Stop()
{
	LOGDBUG("In DeviceObj Stop .. .. ");
	return ER_OK;
}

QStatus DeviceObj::Join()
{
	LOGDBUG("In DeviceObj Join .. .. ");
	return ER_OK;
}

void DeviceObj::ObjectRegistered(void)
{
	LOGDBUG("In DeviceObj ObjectRegistered .. .. ");
	return ;
}

QStatus DeviceObj::checkAndCreateTransport(TransportMask transportMask, std::string &strReturn)
{
	LOGDBUG("In DeviceObj checkAndCreateTransport .. .. ");

	/* Check for valid Device side Transport Mask */
	bool isValidTransport = this->IsValidTransport(transportMask);
	if(!isValidTransport) {
		LOGEROR("Invalid Transport !!");
		return ER_BUS_TRANSPORT_NOT_AVAILABLE;
	}

	// Create New Transport and add into TransportList
	QStatus status = ER_OK;
	bool transportFound = IsTransportAvailable(transportMask);
	if(!transportFound) {
		status = this->CreateNewTransport(transportMask, strReturn);
		transportmask_ = transportMask;

		//Register callback for telemetry in persistant.......
		//Changed --------------------
		std::string devID = "";
		std::string payload = "";

		this->RegisterDeviceCallback(devID, payload);
		//Changed ---------------------
	}

	if (ER_OK != status) {
		LOGEROR2("(checkAndCreateTransport) Fail to parse msg parameters [strReturn]", strReturn);
	}

	return status;
}

void DeviceObj::StartDiscovery(std::string &devId, std::string &payload)
{
	QCC_UNUSED(payload);
	LOGDBUG("In DeviceObj StartDiscovery .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}

	// Check to see if this endpoint is already discovering this prefix
	bool foundEntry = false;
	std::string sender = "";
	DiscoverMapType::iterator it = m_discoverMap.lower_bound(sender);
	while ((it != m_discoverMap.end()) && (it->first == sender)) {
		if (it->second.sender == sender) {
			if ((it->second.transportMask & transportmask_) == 0) {
				it->second.transportMask = it->second.transportMask | transportmask_;
			}
			foundEntry = true;
		}
		++it;
	}

	if (!foundEntry)
	{
		// This is the fix for multiple discovery issue.
		m_discoverMap.insert(std::make_pair(sender, DiscoverMapEntry(transportmask_, sender)));
	}
	else
	{
		payload = DISCOVER_IN_PROGRESS;
		return;
	}

	// Call respective transport function
	CallTransportInterface(START_DISCOVERY, devId, payload);
}

void DeviceObj::StopDiscovery(std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj StopDiscovery .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}
	CallTransportInterface(STOP_DISCOVERY, devId, payload);
}

void DeviceObj::AddDevice(std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj AddDevice .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}

	//#ifdef USE_SHADOW_CONTROLLER
	//	try
	//	{
	//		shadowUniqPtr->createShadow(devId, payload);
	//	}
	//	catch(std::exception &e)
	//	{
	//		LOGWARN2("Exception thrown", e.what());
	//	}
	//#endif

	QStatus status = CallTransportInterface(ADD_DEVICE, devId, payload);
	if(status == ER_OK)
	{
		json jsonData;
		try
		{
			jsonData = json::parse(payload);



			//CALL REGISTERDEVICE OF SHADOW, MAKE SHADOW FILE WITH ONLY REGISTER STATUS.
			if(enablePersistant == true)
			{
#ifdef USE_SHADOW_CONTROLLER
				try
				{
					//call new function of shadow to form shadow files initially.
					shadowUniqPtr->registerDevice(payload);
				}
				catch(std::exception &e)
				{
					LOGWARN2("Exception thrown", e.what());
				}
#endif
			}


			json tempJson = jsonData[PP_DEVICES];
			int size = tempJson.size();

			for(int j = 0 ; j < size ; j++)
			{
				devId = jsonData[PP_DEVICES][j][PP_DEVID];
				devInfo.strMetaData = jsonData[PP_DEVICES][j].dump();
				devInfo.strDeviceId = devId;
				DeviceListVector.push_back(devInfo);
				devInfo = {};
				for(DeviceInfo &i : DeviceListVector)
				{
					LOGDBUG2("(AddDevice) Add DEVICE Meta Data [METADATA]",i.strMetaData);
				}

				servicProPtr->SendRegDevNotification(devId, const_cast<std::string &> (REGISTERED_NS::DEVICEREGISTERED));
			}

		}
		catch(std::exception &e)
		{
			LOGWARN2("(AddDevice) Unable to process payload for device meta data !!",e.what());
		}
		//m_deviceMap.insert(std::make_pair(devId, DeviceMapEntry(transportmask_)));
	}
}

void DeviceObj::DeleteDevice( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj DeleteDevice .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}

	QStatus status = CallTransportInterface(DELETE_DEVICE, devId, payload);

#ifdef USE_SHADOW_CONTROLLER
	try
	{
		shadowUniqPtr->deregisterShadow(devId,payload);
	}
	catch(std::exception &e)
	{
		LOGWARN2("Exception thrown", e.what());
	}
#endif

	if(status == ER_OK)
	{
		DeviceListVector.erase(
				remove_if(DeviceListVector.begin(),
						DeviceListVector.end(),
						[&](DeviceInfo const& rStr ){ LOGDBUG2("(DeleteDevice) DEVICE ID [Delete]",devId); return (!rStr.strDeviceId.compare(devId));}),
						DeviceListVector.end()
		);


		servicProPtr->SendRegDevNotification(devId, const_cast<std::string &> (REGISTERED_NS::DEVICEDEREGISTERED));
	}
}

void DeviceObj::GetDeviceRAML( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj GetDeviceRAML .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}

	CallTransportInterface(GET_DEVICE_RAML, devId, payload);

#ifdef USE_SHADOW_CONTROLLER
	try
	{
		if(!shadowUniqPtr->isShadowCreated(devId))
			shadowUniqPtr->createShadow(devId, payload);
	}
	catch(std::exception &e)
	{
		LOGWARN2("Exception thrown", e.what());
	}
#endif

}

void DeviceObj::GetAllRaml( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj GetAllRaml .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}

	CallTransportInterface(GET_DEVICE_RAML, devId, payload);

#ifdef USE_SHADOW_CONTROLLER
	try
	{
		if(!shadowUniqPtr->isShadowCreated(devId))
			shadowUniqPtr->createShadow(devId, payload);
	}
	catch(std::exception &e)
	{
		LOGWARN2("Exception thrown", e.what());
	}
#endif

}



void DeviceObj::SetDeviceRAML( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj SetDeviceRAML .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}

	CallTransportInterface(SET_DEVICE_RAML, devId, payload);

#ifdef USE_SHADOW_CONTROLLER
	try
	{
		if(!shadowUniqPtr->isShadowCreated(devId))
			shadowUniqPtr->createShadow(devId, payload);
	}
	catch(...)
	{
		auto excptPtr = std::current_exception();
		LOGWARN("Exception thrown");
	}
#endif

}

void DeviceObj::GetDeviceXML( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj GetDeviceXML .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}
	CallTransportInterface(GET_DEVICE_XML, devId, payload);
}

void DeviceObj::SetDeviceXML( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj SetDeviceXML .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}
	CallTransportInterface(SET_DEVICE_XML, devId, payload);
}

void DeviceObj::GetDeviceState( std::string &devId, std::string &payload)
{
	LOGDBUG2("In DeviceObj GetDeviceState .. .. ", payload);
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}
#ifdef USE_SHADOW_CONTROLLER
	shadowUniqPtr->getDeviceState(devId, payload);
#else
	CallTransportInterface(GET_DEVICE_STATE, devId, payload);
#endif
}

void DeviceObj::SetDeviceState( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj SetDeviceState .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}
#ifdef USE_SHADOW_CONTROLLER
	shadowUniqPtr->setDeviceState(devId, payload);
#else
	CallTransportInterface(SET_DEVICE_STATE, devId, payload);
#endif
}

void DeviceObj::GetDeviceStatus( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj GetDeviceStatus .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}
	CallTransportInterface(GET_DEVICE_STATUS, devId, payload);
}

void DeviceObj::SetDeviceStatus( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj SetDeviceStatus .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}
	CallTransportInterface(SET_DEVICE_STATUS, devId, payload);

	if(enablePersistant == true)
	{
		//CALL SHADOW FUNCTION.
#ifdef USE_SHADOW_CONTROLLER
		shadowUniqPtr->setDeviceStatus(devId, payload);
#endif
	}
}

void DeviceObj::StatusChange( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj StatusChange .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}
	CallTransportInterface(STATUS_CHANGE, devId, payload);

	if(enablePersistant == true)
	{
		//call shadow function
#ifdef USE_SHADOW_CONTROLLER
		shadowUniqPtr->setMultipleStatus(payload);
#endif
	}

}

void DeviceObj::RegisterDeviceProperties( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj RegisterDeviceProperties .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}
	CallTransportInterface(REGISTER_DEVICE_PROPERTIES, devId, payload);
}

void DeviceObj::DeregisterDeviceProperties( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj DeregisterDeviceProperties .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}
	CallTransportInterface(DEREGISTER_DEVICE_PROPERTIES, devId, payload);
}

bool DeviceObj::extractandchktransport(TransportMask& transportMask, std::string &devId)
{
	std::string data;
	bool success(true);
	auto it = m_deviceMap.find(devId);
	if(it !=  m_deviceMap.end())
	{
		transportMask = it->second.transportMask;
	}
	else
	{
		success = false;
	}
	return success;
}

bool DeviceObj::checkDeviceInSensorMap(std::string strSender,std::string devId,TransportMask transportMask)
{
	//Check to see if this device id already registered this sender
	bool foundEntry = false;
	std::string sender = strSender;
	SensorMapType::iterator it = m_sensorMap.begin();
	while (it != m_sensorMap.end()) {
		if (it->second.sender == sender && it->first == devId) {
			if ((it->second.transportMask & transportMask) != 0){
				foundEntry = true;
				break;
			} else {
				it->second.transportMask = it->second.transportMask | transportMask;
			}
		}
		++it;
	}
	return foundEntry;
}

void DeviceObj::RegisterDeviceCallback( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj RegisterDeviceCallback .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}
	if (!checkDeviceInSensorMap("NILESH", devId, transportmask_)) {
		QStatus st = CallTransportInterface(REGISTER_DEVICE_CALLBACK, devId, payload);
		if(st == ER_OK)
		{
			std::string sender = "NILESH"; //::TODO
			// This is the fix for multiple activate issue.
			LOGINFO2I("(RegisterDeviceCallback) Get RegisterDeviceCallback. [TM]", transportmask_);
			LOGINFO2("(RegisterDeviceCallback) Get RegisterDeviceCallback. [Sender]", sender);
			m_sensorMap.insert(std::make_pair(devId, SensorMapEntry(transportmask_, sender)));
		}
	} else {
		payload = "Sensor already registered.";
	}
}

void DeviceObj::DeregisterDeviceCallback( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj DeregisterDeviceCallback .. .. ");
	if(transportmask_ == TRANSPORT_NONE)
	{
		LOGEROR("(StartDiscovery) Transport Not Found !!");
		payload = "Transport Not Found !!";
		return ;
	}
	if (!checkDeviceInSensorMap("iot", devId, transportmask_)) {
		payload = DEVICE_NOT_REGISTERED_IN_DO;
		return ;
	}
	CallTransportInterface(DEREGISTER_DEVICE_CALLBACK, devId, payload);
}

void DeviceObj::CheckDeviceExist(const InterfaceDescription::Member* member, Message& msg)
{
	QCC_UNUSED(member);
	QCC_UNUSED(msg);
	LOGDBUG("In DeviceObj CheckDeviceExist .. .. ");
	std::string devId;
	FetchArguments(msg, devId);

	QStatus  status = ER_FAIL;
	auto it = m_deviceMap.find(devId);
	if(it !=  m_deviceMap.end())
	{
		status = ER_OK;
	} else {
		LOGWARN("(CheckDeviceExist) Device Not Exist !!");
	}
}

void DeviceObj::GetDeviceList( std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj GetDeviceList .. .. ");
	json jsonData;
	json jsonArray = json::array();
	try  {
		jsonData = json::parse(payload);
		for(DeviceInfo &i : DeviceListVector)
		{
			LOGDBUG2("(GetDeviceList) Iterator",i.strMetaData);
			json jsonDevData;
			jsonDevData = json::parse(i.strMetaData);
			LOGDBUG2("(GetDeviceList) Dev Meta Data [STRMETADATA]",i.strMetaData);
			jsonArray += jsonDevData;
		}
		jsonData[PP_DEVICES] = jsonArray;
		jsonData[PP_RESPONSECODE] = RC_SUCCESS;
		payload = jsonData.dump();
	} catch (...) {
		LOGEROR("(GetDeviceList) Failed to process response Json !!");
	}
	return ;
}

QStatus DeviceObj::DeviceExists(std::string &payload, std::string &endpointadd)
{
	LOGDBUG("In DeviceObj DeviceExists .. .. ");
	json jsonData;
	QStatus status = ER_FAIL;
	std::string strDeviceId;
	std::string strMsg;

	try  {
		jsonData = json::parse(payload);
		//{'deviceid': '59633f947123a6432421e57e', 'appname': 'org.alljoyn.eI.RE.GW59633f147123a642f528c9c7.APPRE'}
		if(jsonChecker::checkJson(jsonData,{PP_DEVICEID})) {
			strDeviceId = jsonData[PP_DEVICEID];
			for(DeviceInfo &i : DeviceListVector)
			{
				LOGDBUG3("(DeviceExists) DEVICE ID [ID DEVICEID]",i.strDeviceId,strDeviceId);
				if(!i.strDeviceId.compare(strDeviceId)) {
					status = ER_OK;
					endpointadd = jsonData[PP_APPNAME];
					strMsg = "Device Id Found.";
					break;
				}
			}
		} else {
			status = ER_FAIL;
			LOGEROR("(DeviceExists) Failed to Fetch device ID !!");
			strMsg = "Device Id not Found !!";
		}
	} catch (...) {
		status = ER_FAIL;
		LOGEROR("(DeviceExists) Failed to process Json !!");
	}

	if(status == ER_OK){
		try  {
			jsonData[PP_MSG] =  strMsg;
			jsonData[PP_RESPONSECODE] = RC_SUCCESS;
			jsonData[PP_STATUS] = PP_EXISTS;//{"status":"exists","responsecode":200,"msg":"Device Id Found."}
			payload = jsonData.dump();
			status = ER_OK;
		} catch (...) {
			status = ER_OK;
			LOGEROR("(DeviceExists) Failed to process response ER_OK Json !!!");
		}
	} else {
		try  {
			jsonData[PP_MSG] =  "Device Id not Found !!";
			jsonData[PP_RESPONSECODE] = RC_FAIL;
			//jsonData[PP_STATUS] = PP_EXISTS;//{"responsecode":400,"msg":"Device Id not Found."}
			payload = jsonData.dump();
			status = ER_OK;
		} catch (...) {
			status = ER_OK;
			LOGEROR("(DeviceExists) Failed to process response ER_FAIL Json !!!");
		}
	}
	return status;
}

void DeviceObj::SCconfig(std::string &command, std::string &payload)
{
	LOGDBUG("In DeviceObj SCconfig .. .. ");
	return ;
}

void DeviceObj::DiscoveredDevices(std::string sender, TransportMask transportMask, std::string data, uint32_t ttl)
{
	LOGDBUG("In DeviceObj DiscoveredDevices .. .. ");
	/* Generate a list of name deltas */
	if (0 < ttl)
	{
		if (0 < m_discoverMap.size())
		{
			DiscoverMapType::iterator it = m_discoverMap.begin();
			while (it != m_discoverMap.end()) {
				/* This is the transportMask of the transports that this name was being discovered prior to this FindAdvertisedName call. */
				if (it->first == sender && (it->second.transportMask & transportMask) != 0)
				{
					LOGDBUG2I("(DiscoveredDevices) [TM]",transportMask);
					LOGDBUG2("(DiscoveredDevices) [Sender]", it->second.sender);
					LOGDBUG2("(DiscoveredDevices) [matchingStr]",it->first);
					QStatus status = SendDiscoveredDevices(it->second.sender, data);
					if (ER_OK != status) {
						LOGEROR2("(DiscoveredDevices) Failed to send DiscoveredDevice.[Status]", QCC_StatusText(status));
					}
					it = m_discoverMap.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	}
}

QStatus DeviceObj::SendDiscoveredDevices(const std::string& dest, const std::string& data)
{
	LOGDBUG("In DeviceObj SendDiscoveredDevices .. .. ");
	LOGINFO3("(SendDiscoveredDevices) Got dest and data [dest data]",dest,data);

	//Update the conf file for persistant

	std::cout << "SEND DISCOVERED DEVICES ARE" << data << std::endl;

	if(enablePersistant == true)
	{
#ifdef USE_SHADOW_CONTROLLER
		shadowUniqPtr->DiscoveredDevices(data);
#endif
	}

	servicProPtr->SendDiscoveredDevices(dest,data);
	return ER_OK;
}

void DeviceObj::DeviceData(const std::string& devId, std::string data, uint32_t ttl)
{
	LOGDBUG("In DeviceObj DeviceData .. .. ");
	// Generate a list of name deltas
	if (0 < ttl)
	{
		LOGINFO3("In DeviceObj DeviceData (devId data) -", devId, data);
		SensorMapType::iterator it = m_sensorMap.begin();
		while (it != m_sensorMap.end()) {
			// This is the transportMask of the transports that this name was being discovered prior to this FindAdvertisedName call.
			if (it->first == devId) {
				QStatus status = SendDeviceData(it->second.sender, devId, data);
				if (ER_OK != status) {
					LOGEROR2("Failed to send DiscoveredDevice. [Status]", QCC_StatusText(status));
				}
			}
			++it;
		}
	}
}

void DeviceObj::DeviceData(const std::string &senID, std::string data)
{
	LOGDBUG("In DeviceObj DeviceData .. .. ");
	LOGINFO3("(DeviceData) SendDeviceData Data [dest data]",senID,data);

#ifdef USE_SHADOW_CONTROLLER
	shadowUniqPtr->deviceData(const_cast<std::string &>(senID), data);
#else
	servicProPtr->SendTelemetryData(senID,data);
#endif
}

QStatus DeviceObj::SendDeviceData(const std::string& dest, const std::string& devId, const std::string& data)
{
	LOGDBUG("In DeviceObj SendDeviceData .. .. ");
	LOGINFO2("SendDeviceData Data [devId]",devId);
	LOGINFO3("SendDeviceData Data [dest data]",dest,data);
	std::cout<<"\nAB=> SendDeviceData() " << " sender: "<<dest<<" Device ID: "<<devId<<" data: "<<data<<std::endl;
	//return Signal(dest.c_str(), 0, *deviceDataSignal, args, ArraySize(args));
	return ER_OK;
}

void DeviceObj::FoundNames(const std::string& busAddr, const std::string& guid, TransportMask transport, const std::vector<std::string>* names, uint32_t ttl)
{
	QCC_UNUSED(busAddr);
	QCC_UNUSED(guid);
	QCC_UNUSED(transport);
	QCC_UNUSED(names);
	QCC_UNUSED(ttl);
}

void DeviceObj::AlarmTriggered(const Alarm& alarm, QStatus reason)
{
	QCC_UNUSED(alarm);
	QCC_UNUSED(reason);
	LOGINFO2("(AlarmTriggered) AlarmTriggered (devId) - ", QCC_StatusText(reason));
}

void DeviceObj::FetchArguments(Message& msg, TransportMask& transportMask)
{
	size_t numArgs;
	const MsgArg* args;
	msg->GetArgs(numArgs, args);
	const char* deviceId;
	const char* payload;
	MsgArg::Get(args, numArgs, "qss", &transportMask, &deviceId, &payload);
}

void DeviceObj::FetchArguments(Message& msg, std::string& devId)
{
	size_t numArgs;
	const MsgArg* args;
	msg->GetArgs(numArgs, args);
	const char* deviceId;
	QStatus status = MsgArg::Get(args, numArgs, "s", &deviceId);
	if(ER_OK == status)
	{
		devId = deviceId;
	}
}

void DeviceObj::FetchArguments(Message& msg, std::string& sender, std::string& namePrefix)
{
	size_t numArgs;
	const MsgArg* args;
	msg->GetArgs(numArgs, args);

	sender = msg->GetSender();
	const char* str;
	TransportMask transportMask;
	QStatus status = MsgArg::Get(args, numArgs, "qs", &transportMask, &str);
	if(ER_OK == status) {
		namePrefix = str;
	}
}

void DeviceObj::FetchArguments(Message& msg, TransportMask& transportMask, std::string& devId, std::string& payLoad)
{
	size_t numArgs;
	const MsgArg* args;
	msg->GetArgs(numArgs, args);

	const char* deviceId;
	const char* payload;
	QStatus status = MsgArg::Get(args, numArgs, "qss", &transportMask, &deviceId, &payload);
	if(ER_OK == status) {
		devId = deviceId;
		payLoad = payload;
	}
}

void DeviceObj::FetchArgumentsWT(Message& msg, std::string& devId, std::string& payLoad)
{
	size_t numArgs;
	const MsgArg* args;
	msg->GetArgs(numArgs, args);

	const char* deviceId;
	const char* payload;
	QStatus status = MsgArg::Get(args, numArgs, "ss", &deviceId, &payload);
	if(ER_OK == status) {
		devId = deviceId;
		payLoad = payload;
	}
}

unsigned int DeviceObj::GetTransportErrorCode(InterfaceName iName)
{
	unsigned int errorCode;
	switch(iName)
	{
	case START_DISCOVERY:
		errorCode = DEVICEOBJECT_STARTDISCOVERYY_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case STOP_DISCOVERY:
		errorCode = DEVICEOBJECT_STOPDISCOVERY_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case ADD_DEVICE:
		errorCode = DEVICE_ADDDEVICE_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case DELETE_DEVICE:
		errorCode = DEVICE_DELETEDEVICE_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case GET_DEVICE_RAML:
		errorCode = DEVICE_GETRAML_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case SET_DEVICE_RAML:
		errorCode = DEVICE_SETRAML_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case GET_DEVICE_STATE:
		errorCode = DEVICE_GETSTATE_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case SET_DEVICE_STATE:
		errorCode = DEVICE_SETSTATE_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case GET_DEVICE_STATUS:
		errorCode = DEVICE_GETSTATUS_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case SET_DEVICE_STATUS:
		errorCode = DEVICE_SETSTATUS_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case REGISTER_DEVICE_PROPERTIES:
		errorCode = DEVICE_REGISTERPROPERTIES_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case DEREGISTER_DEVICE_PROPERTIES:
		errorCode = DEVICE_DEREGISTERPROPERTIES_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case REGISTER_DEVICE_CALLBACK:
		errorCode = DEVICE_REGISTERCALLBACK_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case DEREGISTER_DEVICE_CALLBACK:
		errorCode = DEVICE_DEREGISTERCALLBACK_REPLY_TRANSPORT_NOT_AVAILABLE;
		break;
	case STATUS_CHANGE:
		errorCode = DEVICE_STATUSCHANGE_TRANSPORT_NOT_AVAILABLE;
		break;
	default:
		errorCode = 0;
	}
	return errorCode;
}

unsigned int DeviceObj::GetReplyFailErrorCode(InterfaceName iName)
{
	unsigned int errorCode;
	switch(iName)
	{
	case START_DISCOVERY:
		errorCode = DEVICEOBJECT_STARTDISCOVERYY_REPLY_FAILED;
		break;
	case STOP_DISCOVERY:
		errorCode = DEVICEOBJECT_STOPDISCOVERY_REPLY_FAILED;
		break;
	case ADD_DEVICE:
		errorCode = DEVICE_ADDDEVICE_REPLY_FAILED;
		break;
	case DELETE_DEVICE:
		errorCode = DEVICE_DELETEDEVICE_REPLY_FAILED;
		break;
	case GET_DEVICE_RAML:
		errorCode = DEVICE_GETRAML_REPLY_FAILED;
		break;
	case SET_DEVICE_RAML:
		errorCode = DEVICE_SETRAML_REPLY_FAILED;
		break;
	case GET_DEVICE_STATE:
		errorCode = DEVICE_GETSTATE_REPLY_FAILED;
		break;
	case SET_DEVICE_STATE:
		errorCode = DEVICE_SETSTATE_REPLY_FAILED;
		break;
	case GET_DEVICE_STATUS:
		errorCode = DEVICE_GETSTATUS_REPLY_FAILED;
		break;
	case SET_DEVICE_STATUS:
		errorCode = DEVICE_SETSTATUS_REPLY_FAILED;
		break;
	case REGISTER_DEVICE_PROPERTIES:
		errorCode = DEVICE_REGISTERPROPERTIES_REPLY_FAILED;
		break;
	case DEREGISTER_DEVICE_PROPERTIES:
		errorCode = DEVICE_DEREGISTERPROPERTIES_REPLY_FAILED;
		break;
	case REGISTER_DEVICE_CALLBACK:
		errorCode = DEVICE_REGISTERCALLBACK_REPLY_FAILED;
		break;
	case DEREGISTER_DEVICE_CALLBACK:
		errorCode = DEVICE_DEREGISTERCALLBACK_REPLY_FAILED;
		break;
	case STATUS_CHANGE:
		errorCode = DEVICE_STATUSCHANGE_REPLY_FAILED;
		break;
	default:
		errorCode = 0;
	}
	return errorCode;
}

bool DeviceObj::IsValidTransport(const TransportMask& transportMask)
{
	bool isValid(false);
	for(std::vector<TransportMask>::iterator it(deviceTransportList.begin()); it != deviceTransportList.end(); ++it)
	{
		if(transportMask == *it) {
			isValid = true;
			break;
		}
	}

	return isValid;
}

bool DeviceObj::IsTransportAvailable(const TransportMask& transportMask)
{
	bool success(false);
	auto it = transportList.find(transportMask);
	if(it !=  transportList.end())
	{
		LOGDBUG("(IsTransportAvailable) Transport found from map");
		success = true;
	} else {
		LOGWARN("(IsTransportAvailable) Transport not found from map !!");
	}
	return success;
}

QStatus DeviceObj::CreateNewTransport(const TransportMask& transportMask, std::string &strReturn)
{
	LOGDBUG("In CreateNewTransport .. ..");
	QStatus status = ER_OK;
	TransportWrapper* transport = new TransportWrapper(transportMask);

	bool success = transport->Init();
	if(success)
	{
		LOGINFO2I("(CreateNewTransport) Successfully created new Transport [TM]",transportMask);
		transport->SetListener(this);
		transportList.insert(std::make_pair(transportMask,transport));

	} else {
		status = ER_FAIL;
		switch(transport->m_errorCode)
		{
		case 1:
			strReturn = CREATE_TRANSPORT_LIBRARY_NOT_EXIST;
			break;
		case 2:
			strReturn = CREATE_TRANSPORT_LIBRARY_NOT_LOADED;
			break;
		case 3:
			strReturn = CREATE_TRANSPORT_UNABLE_TO_CALL_CREATE;
			break;
		case 4:
			strReturn = CREATE_TRANSPORT_EXCEPTION_RAISED;
			break;
		default:
			strReturn = CREATE_TRANSPORT_UNABLE_TO_CREATE;
			break;
		}
	}
	return status;
}

TransportWrapper* DeviceObj::GetTransport(TransportMask& transportMask)
{
	TransportWrapper* transport(NULL);
	auto it = transportList.find(transportMask);
	if(it !=  transportList.end())
	{
		transport = it->second;
		LOGDBUG("(GetTransport) Transport found from map.");
	} else {
		LOGWARN2I("(GetTransport) Transport not found from map [TM]",transportMask);
	}
	return transport;
}

QStatus DeviceObj::WrappedCallTransportInterface(InterfaceName iName, std::string &devId, std::string &payload)
{
	return this->CallTransportInterface(iName, devId, payload);
}

QStatus DeviceObj::CallTransportInterface(InterfaceName iName, std::string &devId, std::string &payload)
{
	LOGDBUG("In DeviceObj CallTransportInterface .. .. ");
	QStatus status = ER_OK;
	int devStatus;
	std::string iface;
	std::string sender;
	std::string namePrefix;
	TransportWrapper* transport = GetTransport(transportmask_);

	if(!transport)
	{
		status = ER_FAIL;
		devStatus = DEVICE_ADDDEVICE_REPLY_TRANSPORT_NOT_AVAILABLE;
		payload = "Transport not found !!";
		LOGEROR("(CallTransportInterface) Transport ptr not found !!");
	}
	else
	{
		switch(iName)
		{
		case START_DISCOVERY:
			m_callbackThread = new FuncCallbackThread(*this, *transport);
			//m_callbackThread->setInput(namePrefix);
			LOGINFO2("(CallTransportInterface) Pass To lib  [payload]",payload);
			m_callbackThread->setInput(payload);
			m_callbackThread->setSender(sender);
			status = m_callbackThread->Start();
			devStatus = DEVICEOBJECT_STARTDISCOVERYY_REPLY_SUCCESS;
			iface = "StartDiscovery";
			break;
		case STOP_DISCOVERY:
			//status = transport->StopDiscovery(namePrefix.c_str(), transportMask);
			devStatus = DEVICEOBJECT_STOPDISCOVERY_REPLY_SUCCESS;
			iface = "StopDiscovery";
			break;
		case ADD_DEVICE:
			transport->AddDevice(devId, payload);
			devStatus = DEVICE_ADDDEVICE_REPLY_SUCCESS;
			iface = "AddDevice";
			break;
		case DELETE_DEVICE:
			transport->DeleteDevice(devId, payload);
			devStatus = DEVICE_DELETEDEVICE_REPLY_SUCCESS;
			iface = "DeleteDevice";
			break;
		case GET_DEVICE_RAML:
			transport->GetDeviceRAML(devId, payload);
			devStatus = DEVICE_GETRAML_REPLY_SUCCESS;
			iface = "GetDeviceRAML";
			break;
		case SET_DEVICE_RAML:
			transport->SetDeviceRAML(devId, payload);
			devStatus = DEVICE_SETRAML_REPLY_SUCCESS;
			iface = "SetDeviceRAML";
			break;
		case GET_DEVICE_STATE:
			transport->GetDeviceState(devId, payload);
			devStatus = DEVICE_GETSTATE_REPLY_SUCCESS;
			iface = "GetDeviceState";
			break;
		case SET_DEVICE_STATE:
			transport->SetDeviceState(devId, payload);
			devStatus = DEVICE_SETSTATE_REPLY_SUCCESS;
			iface = "SetDeviceState";
			break;
		case GET_DEVICE_STATUS:
			transport->GetDeviceStatus(devId, payload);
			devStatus = DEVICE_GETSTATUS_REPLY_SUCCESS;
			iface = "GetDeviceStatus";
			break;
		case SET_DEVICE_STATUS:
			LOGINFO2("(CallTransportInterface) SET_DEVICE_STATUS [payload]",payload);
			transport->SetDeviceStatus(devId, payload);
			LOGINFO2("(CallTransportInterface) SET_DEVICE_STATUS [payload]",payload);
			devStatus = DEVICE_SETSTATUS_REPLY_SUCCESS;
			iface = "SetDeviceStatus";
			break;
		case REGISTER_DEVICE_PROPERTIES:
			transport->RegisterDeviceProperties(devId, payload);
			devStatus = DEVICE_REGISTERPROPERTIES_REPLY_SUCCESS;
			iface = "RegisterDeviceProperties";
			break;
		case DEREGISTER_DEVICE_PROPERTIES:
			//FetchArgumentsWT(msg, devId, payload);
			transport->DeregisterDeviceProperties(devId, payload);
			devStatus = DEVICE_DEREGISTERPROPERTIES_REPLY_SUCCESS;
			iface = "DeregisterDeviceProperties";
			break;
		case REGISTER_DEVICE_CALLBACK:
			transport->RegisterDeviceCallback(devId, payload);
			devStatus = DEVICE_REGISTERCALLBACK_REPLY_SUCCESS;
			iface = "RegisterDeviceCallback";
			break;
		case DEREGISTER_DEVICE_CALLBACK:
			transport->DeregisterDeviceCallback(devId, payload);
			devStatus = DEVICE_DEREGISTERCALLBACK_REPLY_SUCCESS;
			iface = "DeregisterDeviceCallback";
			break;
		case GET_DEVICE_XML:
			transport->GetDeviceXML(devId, payload);
			devStatus = DEVICE_GETXML_REPLY_SUCCESS;
			iface = "GetDeviceXML";
			break;
		case SET_DEVICE_XML:
			transport->SetDeviceXML(devId, payload);
			devStatus = DEVICE_SETXML_REPLY_SUCCESS;
			iface = "SetDeviceXML";
			break;

		case STATUS_CHANGE:
			transport->StatusChange(devId, payload);
			devStatus = DEVICE_STATUSCHANGE_REPLY_SUCCESS;
			iface = "StatusChange";
			break;
		default:
			cout<<"\nInvalid Interface Name: "<<iName;
			status = ER_FAIL;
			devStatus = INTERFACE_NOT_FOUND_OR_NOT_SUPPORTED;
			payload = "Transport not found !!";

		}
	}
	//::TODO
	//GenerateReply(iName, msg, status, iface, devId, payload, devStatus);
	return status;
}

qcc::ThreadReturn STDCALL DeviceObj::FuncCallbackThread::Run(void* arg)
{
	QCC_UNUSED(arg);
	LOGINFO("(DeviceObj::FuncCallbackThread) Thread - Start.");
	m_transWrapper.Discover(input);
	m_dObj.DiscoveredDevices(sender, m_transWrapper.transport(), input,10000);
	LOGINFO("(DeviceObj::FuncCallbackThread) Thread - End.");
}


