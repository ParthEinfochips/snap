#ifndef ALLJOYNWRAPPER_H
#define ALLJOYNWRAPPER_H

#include <functional>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/BusListener.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/Init.h>
#include <alljoyn/InterfaceDescription.h>
#include <ajcommon/CustomTransportMask.h>
#include <alljoyn/DBusStd.h>
#include <alljoyn/MsgArg.h>
#include <alljoyn/version.h>
#include <alljoyn/AboutObj.h>
#include "ajcommon/AJCommon.h"
#include "ajNameService/ajNameService.h"
#include "CommonHeader/nsprefix.h"
#include "proxy/cloudProxy.h"
//#include "jsonCheck.hpp"
#include "configuration.h"
#include "CommonHeader/json.hpp"
#include "MqttApi.h"
#include "mqttBusObject.h"
#include "directoryOPS.h"
#include "proxy/methodExecuter.h"
#include "ajcommon/AJCommon.h"


#define configDataPath "/opt/GA.conf.d/"
#define metaDetails "/opt/GA.conf.d/gatewayDtl.conf"

using namespace eITM;
using namespace nlohmann;

//struct argDetails
//{
//	std::string orgId;
//	std::string packageId;
//	std::string gatewayId;
//
//	std::string standalone;
//	std::string original;
//
//	std::string mqqtpassword;
//	std::string mqqthostname;
//	std::string mqqtusername;
//	uint16_t mqqtport;
//
//	std::string AppTopic;
//};


class mqttBusObject;
class MqttApi;

class alljoynWrapper
{
public:

	explicit alljoynWrapper();

	alljoynWrapper(alljoynWrapper &wrapPtr);

	~alljoynWrapper();

	QStatus init();

	void Dinit();

	//ajn::BusAttachment* getBus();

	std::unique_ptr<AJCommon> ajComPtr = nullptr;

private:
	std::unique_ptr<AJCommon> ajCommonPtr = nullptr;

	FileOPS *filePtr;

	ajn::BusAttachment* bus = NULL;

	mqttBusObject* busObject = NULL;

	methodExecuter* methodexPtr = NULL;

	signalExecuter* sigexPtr = NULL;

	bool certRequired = true;

	bool setOrgId = false;

	MqttApi *mqttApiPtr = NULL;

	ajn::InterfaceDescription* ifc = NULL;

	std::string strInterfaceName;
};

#endif // ALLJOYNWRAPPER_H
