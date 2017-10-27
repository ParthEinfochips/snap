#include <fstream>
#include "TransportWrapper.h"
//#include "ShadowController.h"
#include "CommonHeader/debugLog.h"

#define eI_MODULE "TransportWrapper"

TransportWrapper::TransportWrapper(TransportMask transport)
: transportPtr(nullptr), m_listener(NULL), m_transport(transport)
/*, m_shadowCtrl(NULL)*/, m_errorCode(0)
{
	LOGDBUG("In TransportWrapper Constructor .. .. ");
}

TransportWrapper::~TransportWrapper()
{
}

bool TransportWrapper::Init()
{
	bool success(true);
	switch(m_transport)
	{
	case TRANSPORT_EI_BLE:
		transportPtr = LoadBLETransport();
		break;
	case TRANSPORT_EI_OPC:
		transportPtr = LoadOPCTransport();
		break;
	case TRANSPORT_EI_ZIGBEE:
		transportPtr = LoadZigbeeTransport();
		break;
	case TRANSPORT_EI_ZWAVE:
		transportPtr = LoadZwaveTransport();
		break;
	case TRANSPORT_EI_MODBUS:
		transportPtr = LoadModbusTransport();
		break;
	case TRANSPORT_EI_COAP:
		transportPtr = LoadCoAPTransport();
		break;
	case TRANSPORT_EI_THREAD:
		transportPtr = LoadThreadTransport();
		break;
	case TRANSPORT_EI_MQTTSIMULATOR:
	{
		transportPtr = LoadMQTTSimulatorTransport();
		std::string configPath= configurationSettings::dmAppConfDirPath + "/ShadowController/";

		transportPtr->GetPersistantDevice(configPath);
		break;
	}
	default:
		LOGEROR("(TransportWrapper) Unknown Transport !!");
	}

	if(transportPtr == nullptr)
	{
		LOGEROR("Fail to Create DevProtocolRequestHandler Ptr !!");
		success = false;
	} else {
		LOGINFO("Successfully Create DevProtocolRequestHandler.");
	}
	return success;
	}

	void TransportWrapper::Discover(std::string& data)
	{
		LOGDBUG("In TransportWrapper Discover .. .. ");
		if(transportPtr){
			transportPtr->Discover(data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::AddDevice(std::string &deviceID, std::string& data)
	{
		LOGDBUG("In TransportWrapper AddDevice .. .. ");
		if(transportPtr)
		{
			std::string raml = data;
			transportPtr->AddDevice(deviceID, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::DeleteDevice(std::string &deviceID, std::string& data)
	{
		LOGDBUG("In TransportWrapper DeleteDevice .. .. ");
		if(transportPtr) {
			transportPtr->DeleteDevice(deviceID, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::GetDeviceRAML(std::string &deviceID, std::string& data)
	{
		LOGDBUG("In TransportWrapper GetDeviceRAML .. .. ");
		if(transportPtr) {
			transportPtr->GetDeviceRAML(deviceID, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::SetDeviceRAML(std::string &deviceID, std::string& data)
	{
		LOGDBUG("In TransportWrapper SetDeviceRAML .. .. ");
		if(transportPtr) {
			transportPtr->SetDeviceRAML(deviceID, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::GetDeviceState(std::string &deviceID, std::string& data)
	{
		LOGDBUG("In TransportWrapper GetDeviceState .. .. ");
		if(transportPtr) {
			transportPtr->GetDeviceState(deviceID, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::SetDeviceState(std::string &deviceID, std::string& data)
	{
		LOGDBUG("In TransportWrapper SetDeviceState .. .. ");
		if(transportPtr) {
			transportPtr->SetDeviceState(deviceID, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::GetDeviceStatus(std::string &deviceID, std::string& data)
	{
		LOGDBUG("In TransportWrapper GetDeviceStatus .. .. ");
		if(transportPtr){
			transportPtr->GetDeviceStatus(deviceID, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::SetDeviceStatus(std::string &deviceID, std::string& data)
	{
		LOGDBUG("In TransportWrapper SetDeviceStatus.. .. ");
		if(transportPtr){
			transportPtr->SetDeviceStatus(deviceID, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::RegisterDeviceProperties(std::string &deviceID, std::string& data)
	{
		LOGDBUG("In TransportWrapper RegisterDeviceProperties.. .. ");
		if(transportPtr) {
			transportPtr->RegisterDeviceProperties(deviceID, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::DeregisterDeviceProperties(std::string &deviceID, std::string& data)
	{
		LOGDBUG("In TransportWrapper DeregisterDeviceProperties.. .. ");
		if(transportPtr) {
			transportPtr->DeregisterDeviceProperties(deviceID, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::GetDeviceXML(std::string& devId, std::string& data)
	{
		LOGDBUG("In TransportWrapper GetDeviceXML.. .. ");
		if(transportPtr) {
			transportPtr->GetDeviceXML(devId, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::SetDeviceXML(std::string& devId, std::string& data)
	{
		LOGDBUG("In TransportWrapper SetDeviceXML.. .. ");
		if(transportPtr) {
			transportPtr->SetDeviceXML(devId, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	void TransportWrapper::StatusChange(std::string& devId, std::string& data)
	{
		LOGDBUG("In TransportWrapper StatusChange.. .. ");
		if(transportPtr) {
			transportPtr->StatusChange(devId, data);
		} else {
			LOGEROR("DevProtocolRequestHandler Ptr Not Exits !!");
		}
	}

	QStatus TransportWrapper::RegisterDeviceCallback(std::string &devId, std::string& data)
	{
		QStatus status = ER_FAIL;
		LOGDBUG("In TransportWrapper RegisterDeviceCallback.. .. ");
		if(transportPtr) {
			LOGDBUG("(TransportWrapper) AddDataCallback .. .. DataReceived");
			transportPtr->AddDataCallback(std::bind(&TransportWrapper::DataReceived, this, std::placeholders::_1, std::placeholders::_2));
			LOGDBUG("(TransportWrapper) AddDataCallback .. .. DataReceived DONE");
		}
		return status;
	}

	QStatus TransportWrapper::DeregisterDeviceCallback(std::string &devId, std::string& data)
	{
		QStatus status = ER_FAIL;
		LOGDBUG("In TransportWrapper DeregisterDeviceCallback.. .. ");
		//    if(m_shadowCtrl) {
		//        status = m_shadowCtrl->DeregisterDeviceCallback(devId, data);
		//    }
		return status;
	}

	void TransportWrapper::SetListener(DeviceObj* listener)
	{
		m_listener = listener;
	}

	TransportMask TransportWrapper::transport() const
	{
		return m_transport;
	}

	void TransportWrapper::setTransport(const TransportMask &transport)
	{
		m_transport = transport;
	}

	void TransportWrapper::DataReceived(std::string devId, std::string data)
	{
		LOGDBUG("In TransportWrapper DataReceived.. .. ");
		LOGINFO3("(DataReceived) Got Device Data [devId : data]", devId, data);
		if(m_listener)
		{
			std::string senQC = devId;
			std::string dataQC = data;
			m_listener->DeviceData(senQC, dataQC);
		} else {
			LOGEROR("DeviceListener Ptr Not Exits !!");
		}
	}

	std::shared_ptr<DevProtocolRequestHandler> TransportWrapper::LoadBLETransport()
	{
		LOGDBUG("In TransportWrapper LoadBLETransport.. .. ");
		try
		{
			const char *file = "/opt/ptl/lib/libBLE.so";
			std::ifstream stream(file);
			if (!stream.good()) {
				LOGEROR("BLE library does not exist !!");
				m_errorCode = 1;
				return nullptr;
			}

			//void* BLEClientLibPtr = dlopen("/home/pi/arm/release/dist/cpp/lib/libBLE.so", RTLD_LAZY);
			void* BLEClientLibPtr = dlopen(file, RTLD_LAZY);
			if (!BLEClientLibPtr)
			{
				LOGEROR2("Cannot load BLE library !! dlerror - ", dlerror());
				m_errorCode = 2;
				return nullptr;
			}
			dlerror();
			create_t *createBLEfunc = (create_t *) dlsym(BLEClientLibPtr, "create");
			const char *dlsym_error = dlerror();
			if (dlsym_error)
			{
				LOGEROR2("Cannot load BLE symbol create() !! dlerror - ", dlsym_error);
				m_errorCode = 3;
				return nullptr;
			}
			return std::shared_ptr<DevProtocolRequestHandler>(createBLEfunc());
		}
		catch (std::exception &e)
		{
			LOGEROR2("(LoadBLETransport) Exception - ", e.what());
		}
	}

	std::shared_ptr<DevProtocolRequestHandler> TransportWrapper::LoadOPCTransport()
	{
		LOGDBUG("In TransportWrapper LoadOPCTransport.. .. ");
		try
		{
			const char *file = "/opt/ptl/lib/libOPC.so";
			std::ifstream stream(file);
			if (!stream.good()) {
				LOGEROR("OPC library does not exist !!");
				m_errorCode = 1;
				return nullptr;
			}

			//void* OPCClientLibPtr = dlopen("/home/pi/arm/release/dist/cpp/lib/libOPC.so", RTLD_LAZY);
			void* OPCClientLibPtr = dlopen(file, RTLD_LAZY);
			if (!OPCClientLibPtr)
			{
				LOGEROR2("Cannot load OPC library !! dlerror - ", dlerror());
				m_errorCode = 2;
				return nullptr;
			}

			dlerror();
			create_t *createOPCfunc = (create_t *) dlsym(OPCClientLibPtr, "create");
			const char *dlsym_error = dlerror();
			if (dlsym_error)
			{
				LOGEROR2("Cannot load OPC symbol create() !! dlerror - ", dlsym_error);
				m_errorCode = 3;
				return nullptr;
			}

			return std::shared_ptr<DevProtocolRequestHandler>(createOPCfunc());
		}
		catch (std::exception &e)
		{
			LOGEROR2("(LoadOPCTransport) Exception - ", e.what());
		}
	}

	std::shared_ptr<DevProtocolRequestHandler> TransportWrapper::LoadZigbeeTransport()
	{
		LOGDBUG("In TransportWrapper LoadZigbeeTransport.. .. ");
		try
		{
			const char *file = "/opt/ptl/lib/libZigbee.so";
			std::ifstream stream(file);
			if (!stream.good()) {
				LOGEROR("ZIGBEE library does not exist !!");
				m_errorCode = 1;
				return nullptr;
			}

			//void* ZIGBEEClientLibPtr = dlopen("/home/pi/arm/release/dist/cpp/lib/libZigbee.so", RTLD_LAZY);
			void* ZIGBEEClientLibPtr = dlopen(file, RTLD_LAZY);
			if (!ZIGBEEClientLibPtr)
			{
				LOGEROR2("Cannot load ZIGBEE library !! dlerror - ", dlerror());
				m_errorCode = 2;
				return nullptr;
			}
			dlerror();
			create_t *createZIGBEEfunc = (create_t *) dlsym(ZIGBEEClientLibPtr, "create");
			const char *dlsym_error = dlerror();
			if (dlsym_error)
			{
				LOGEROR2("Cannot load ZIGBEE symbol create() !! dlerror - ", dlsym_error);
				m_errorCode = 3;
				return nullptr;
			}
			return std::shared_ptr<DevProtocolRequestHandler>(createZIGBEEfunc());
		}
		catch (std::exception &e)
		{
			m_errorCode = 4;
			LOGEROR2("(LoadZigbeeTransport) Exception - ", e.what());
		}
	}

	std::shared_ptr<DevProtocolRequestHandler> TransportWrapper::LoadZwaveTransport()
	{
		LOGDBUG("In TransportWrapper LoadZwaveTransport.. .. ");
		try
		{
			const char *file = "/opt/ptl/lib/libZwave.so";
			std::ifstream stream(file);
			if (!stream.good()) {
				LOGEROR("ZWAVE library does not exist !!");
				m_errorCode = 1;
				return nullptr;
			}

			//void* ZwaveClientLibPtr = dlopen("/home/pi/arm/release/dist/cpp/lib/libZwave.so", RTLD_LAZY);
			void* ZwaveClientLibPtr = dlopen(file, RTLD_LAZY);
			if (!ZwaveClientLibPtr)
			{
				LOGEROR2("Cannot load ZWAVE library !! dlerror - ", dlerror());
				m_errorCode = 2;
				return nullptr;
			}
			dlerror();
			create_t *createZwavefunc = (create_t *) dlsym(ZwaveClientLibPtr, "create");
			const char *dlsym_error = dlerror();
			if (dlsym_error)
			{
				LOGEROR2("Cannot load ZWAVE symbol create() !! dlerror - ", dlsym_error);
				m_errorCode = 3;
				return nullptr;
			}
			return std::shared_ptr<DevProtocolRequestHandler>(createZwavefunc());
		}
		catch (std::exception &e)
		{
			LOGEROR2("(LoadZwaveTransport) Exception - ", e.what());
		}
	}

	std::shared_ptr<DevProtocolRequestHandler> TransportWrapper::LoadModbusTransport()
	{
		LOGDBUG("In TransportWrapper LoadModbusTransport.. .. ");
		try
		{
			const char *file = "/opt/ptl/lib/libModbus.so";
			std::ifstream stream(file);
			if (!stream.good()) {
				LOGEROR("Modbus library does not exist !!");
				m_errorCode = 1;
				return nullptr;
			}

			//void* ZwaveClientLibPtr = dlopen("/home/pi/arm/release/dist/cpp/lib/libZwave.so", RTLD_LAZY);
			void* ZwaveClientLibPtr = dlopen(file, RTLD_LAZY);
			if (!ZwaveClientLibPtr)
			{
				LOGEROR2("Cannot load Modbus library !! dlerror - ", dlerror());
				m_errorCode = 2;
				return nullptr;
			}
			dlerror();
			create_t *createZwavefunc = (create_t *) dlsym(ZwaveClientLibPtr, "create");
			const char *dlsym_error = dlerror();
			if (dlsym_error)
			{
				LOGEROR2("Cannot load Modbus symbol create() !! dlerror - ", dlsym_error);
				m_errorCode = 3;
				return nullptr;
			}
			return std::shared_ptr<DevProtocolRequestHandler>(createZwavefunc());
		}
		catch (std::exception &e)
		{
			LOGEROR2("(LoadModbusTransport) Exception - ", e.what());
		}
	}

	std::shared_ptr<DevProtocolRequestHandler> TransportWrapper::LoadCoAPTransport()
	{
		LOGDBUG("In TransportWrapper LoadModbusTransport.. .. ");
		try
		{
			const char *file = "/opt/ptl/lib/libCoAP.so";
			std::ifstream stream(file);
			if (!stream.good()) {
				LOGEROR("CoAP library does not exist !!");
				m_errorCode = 1;
				return nullptr;
			}

			//void* ZwaveClientLibPtr = dlopen("/home/pi/arm/release/dist/cpp/lib/libZwave.so", RTLD_LAZY);
			void* ZwaveClientLibPtr = dlopen(file, RTLD_LAZY);
			if (!ZwaveClientLibPtr)
			{
				LOGEROR2("Cannot load CoAP library !! dlerror - ", dlerror());
				m_errorCode = 2;
				return nullptr;
			}
			dlerror();
			create_t *createZwavefunc = (create_t *) dlsym(ZwaveClientLibPtr, "create");
			const char *dlsym_error = dlerror();
			if (dlsym_error)
			{
				LOGEROR2("Cannot load CoAP symbol create() !! dlerror - ", dlsym_error);
				m_errorCode = 3;
				return nullptr;
			}
			return std::shared_ptr<DevProtocolRequestHandler>(createZwavefunc());
		}
		catch (std::exception &e)
		{
			LOGEROR2("(LoadCoAPTransport) Exception - ", e.what());
		}
	}

	std::shared_ptr<DevProtocolRequestHandler> TransportWrapper::LoadThreadTransport()
	{
		LOGDBUG("In TransportWrapper LoadThreadTransport.. .. ");
		try
		{
			const char *file = "/opt/ptl/lib/libThread.so";
			std::ifstream stream(file);
			if (!stream.good()) {
				LOGEROR("Thread library does not exist !!");
				m_errorCode = 1;
				return nullptr;
			}

			//void* ZwaveClientLibPtr = dlopen("/home/pi/arm/release/dist/cpp/lib/libZwave.so", RTLD_LAZY);
			void* ZwaveClientLibPtr = dlopen(file, RTLD_LAZY);
			if (!ZwaveClientLibPtr)
			{
				LOGEROR2("Cannot load Thread library !! dlerror - ", dlerror());
				m_errorCode = 2;
				return nullptr;
			}
			dlerror();
			create_t *createZwavefunc = (create_t *) dlsym(ZwaveClientLibPtr, "create");
			const char *dlsym_error = dlerror();
			if (dlsym_error)
			{
				LOGEROR2("Cannot load Thread symbol create() !! dlerror - ", dlsym_error);
				m_errorCode = 3;
				return nullptr;
			}
			return std::shared_ptr<DevProtocolRequestHandler>(createZwavefunc());
		}
		catch (std::exception &e)
		{
			LOGEROR2("(LoadThreadTransport) Exception - ", e.what());
		}
	}

	std::shared_ptr<DevProtocolRequestHandler> TransportWrapper::LoadMQTTSimulatorTransport()
	{
		LOGDBUG("In TransportWrapper LoadMQTTSimulatorTransport.. .. ");
		try
		{
			const char *file = "/opt/ptl/lib/libSimulator.so";
			std::ifstream stream(file);
			if (!stream.good()) {
				LOGEROR("MQTTSimulator library does not exist !!");
				m_errorCode = 1;
				return nullptr;
			}

			void* MqttSimulatorLibPtr = dlopen(file, RTLD_LAZY);
			if (!MqttSimulatorLibPtr)
			{
				LOGEROR2("Cannot load MQTTSimulator library !! dlerror - ", dlerror());
				m_errorCode = 2;
				return nullptr;
			}
			dlerror();
			create_t *createMQTTSimulatorfunc = (create_t *) dlsym(MqttSimulatorLibPtr, "create");
			const char *dlsym_error = dlerror();
			if (dlsym_error)
			{
				LOGEROR2("Cannot load MQTTSimulator symbol create() !! dlerror - ", dlsym_error);
				m_errorCode = 3;
				return nullptr;
			}

			return std::shared_ptr<DevProtocolRequestHandler>(createMQTTSimulatorfunc());
		}
		catch (std::exception &e)
		{
			LOGEROR2("(LoadMQTTSimulatorTransport) Exception - ", e.what());
		}
	}
