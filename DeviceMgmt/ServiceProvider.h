#ifndef SERVICEPROVIDER_HH
#define SERVICEPROVIDER_HH

#include <qcc/Thread.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/BusAttachment.h>
#include <condition_variable>
#include <alljoyn/Session.h>
#include "ajNameService/eIBusObject.h"
#include "ajNameService/nsCallBack.h"
#include "proxy/methodExecuter.h"
#include "CommonHeader/nsprefix.h"
#include "CommonHeader/jsonCheck.hpp"
#include "DeviceObject/DeviceObj.h"
#include "CommonHeader/CPPHeaders.h"
#include "ajNameService/ajNameService.h"
#include "manageConfiguration.h"
#include "DMA_Macros.h"
#include <mutex>

using namespace ajn;
using namespace eITM;

enum eITOPIC{
	EI_TOPIC_HASH  = 1,
	EI_TOPIC_DEV   = 2,
	EI_TOPIC_DATA  = 3
};

class DeviceObj;
class manageConfiguration;


struct manageConnections{

	uint16_t endpointid;
	std::string brokerID;
	std::string type;


	friend bool operator==(const manageConnections& lhs, const manageConnections& rhs)
	{
		return lhs.endpointid == rhs.endpointid;
	}
};

/**
 * Abstract derived class implemented by Device users and called by AllJoyn to inform
 * users of About interface related events as well as handle Interface call.
 */
class ServiceProvider : public eIBusObject, public nsCallBack, public asynMethodCallBackHandler {

public:
	ServiceProvider(TransportMask mask, ajn::BusAttachment *_pBusAttach, const char* _interfaceName, const char* _path);

	~ServiceProvider();

	/* RAML Handler will process `GetMRAML` request and returning the string back to the sender.*/
	QStatus getMRAMLHandler(std::string &command, std::string &payload);

	/* AsyncListener Handler will process `AsyncListener` request and returning the string back to the sender.*/
	QStatus asyncHandler(std::string &command, std::string &payload);

	/* SyncListener Handler will process `SyncListener` request and returning the string back to the sender.*/
	QStatus syncHandler(std::string &command, std::string &payload);

	/* CallbackRegister Handler will process `CallbackRegister` request and returning the string back to the sender.*/
	QStatus callbackRegisterHandler(std::string &command, std::string &payload);

	/* CallbackListener Handler will process `CallbackListener` request.*/
	void signalHandler(std::string &command, std::string &payload);

	/* DiscoveredDevices Handler will process `DiscoveredDevices' And send to request end point.*/
	QStatus SendDiscoveredDevices(const qcc::String& dest, const qcc::String& data);

	/* Get notification after new advertise find.*/
	void foundAdvertisement(qcc::String busName);

	/* Get notification after loss advertise.*/
	void lostAdvertisement(qcc::String busName);

	void busDisconnected();

	QStatus subscribeConfTopic(std::string topicPrefix);

	QStatus testConnection(json &jsonObj);

	QStatus getTopicPrefix(std::string &prefix,std::string appID, json &jsonData);

	/*Get notification from Asyn Method Call reply.*/
	void methodCallerAsyncCallback(std::string retStatus, std::string payload);

	/*This API will connect cloud And Register Topic.*/
	QStatus connectingCloudAndRegisterTopic(std::string busName);

	QStatus connectingLocalAndRegisterTopic(std::string busName);

	/*This API will connect cloud And Register Topic.*/
	QStatus disConnectingCloud(std::string busName);

	/*Get MethodExecuter ptr.*/
	methodExecuter* GetMethodExecuterPtr();

	/*Get SignalExecuter ptr.*/
	signalExecuter* GetSignalExecuterPtr();

	/*Get DeviceObj ptr.*/
	DeviceObj* GetDeviceObjPtr();

	/*Initialize Transport Lib.*/
	QStatus loadLIB();

	ServiceProvider *getSrvPtr();


	/*Will Update ajNameService ptr. by DMAController*/
	void updateAJNameServicePtr(ajNameService* _ajNameSerPtr);

	/*Will send telemetry to Register App.*/
	void SendTelemetryData(const std::string &senID, std::string data);

	/*Will Send Data to Cloud App.*/
	void SendToCloudData(const std::string &senID, std::string data);

	/*Will Send Data to Register App.*/
	void SendToAppData(const std::string &senID, std::string data);

	/*Will send Data to Register App.*/
	std::string processTopicString(std::string Command);

	/*Will process signal.*/
	void isDeviceExists(std::string command, std::string& payload, std::string mcCommand);

	/*Test Function.*/
	void FunctionTest();

	/*This sends the notification to the apps which wants to know whether any device has been registered or not.*/
	void SendRegDevNotification(const std::string &devID, std::string &status);

	/*This function is called by DMA Controller class which sets the advertised name of the DO App.*/
	void setAdvName(const std::string &advAppName);

	void startCloudConnection();

	void startLocalConnection();

	QStatus getAllRaml(std::string payload, std::string commandPayload);

	ajn::BusAttachment *pBusAttach = NULL;

	BusObject *busObj = NULL;

	methodExecuter *methoExePtr = NULL;

	signalExecuter *signalExePtr = NULL;

	DeviceObj *deviceObjPtr = NULL;

	ajNameService* ajNameSerPtr = NULL;

	TransportMask transportMast;

	bool checkBool = false;

	bool certRequired = true;

	static std::mutex conditionMut;

	static std::condition_variable cv;

	bool ready = false;

	manageConfiguration *manageConfig;

	std::vector<std::string> cloudAppInVicinityVector;

	std::vector<std::string> personalApp;
	std::vector<std::string> vicinityApp;

	std::vector<manageConnections> connManage;

	std::thread cloudConnectionThread;
	std::thread localConnectionThread;

	bool cloudThreadRunner;
	bool localThreadRunner;

	//std::shared_ptr<manageConfiguration> manageConfig;

private:
	/*Cloud Connection Status CallBack Handler. */
	void CloudConnectionStatus(bool);

	/*Will provide topic ID. */
	std::string GetTopicID(eITOPIC eItopic);

	/*Will process request.*/
	QStatus GetDevIDFromJson(std::string &devId, std::string payload);

	/*Will process cloud Callback.*/
	QStatus processCloudCallback(std::string &command, std::string &payload);

	/*Will process request.*/
	QStatus processTopic(std::string &command, std::string &payload);

	/*Will process request.*/
	QStatus processCommand(std::string &command, std::string &payload);

	/*Will process request.*/
	QStatus serve_request(std::string DevId, std::string &command, std::string &payload);

	/*add Register Callback. */
	QStatus addRegisterCallbackApp(std::string&);

	/*Json Formatter. */
	QStatus removeRegisterCallbackApp(std::string& payload);

	/*Adds any app which wants to know whether any device has been registered or not.*/
	QStatus CallbackRegDev(std::string& payload);

	/*Removes any app which wants to know whether any device has been registered or not.*/
	QStatus removeCallbackRegDev(std::string& payload);

	/*Json Formatter. */
	std::string unescape(const std::string& s);

	/*Advertised Name*/
	std::string advName;

	std::string persistantFp;

	std::mutex cloudMutex;

	std::mutex testMutex;

	void init();

	void updatePersistantFile(std::string command, std::string payload);

private:
	const char* interfacePath = NULL;

	const char* interfaceName = NULL;

	std::string connectBrokerNum;

	std::string localBrokerNum;

	bool isDiscovryDone;

	bool isCloudConnected;

	bool isLocalConnected;

	bool isCloudConnectedSignalGot;

	bool isLocalConnectedSignalGot;

	bool isTestConnected;

	bool lossCloudConnection;

	bool lossLocalConnection;

	bool isCloudThreadStart;

	bool isNewCloudAppInVicinity;

	std::string rProtocol;

	std::string discovryTopic;

	std::mutex mLockSendToCloud;
	std::mutex mLockSendTelemetry;
	std::mutex mLockProcessTopic;
	std::mutex mLockProcessCommands;

	bool isPTLload;

	SessionId presentSessionID;

	configurationSettings *confPtr;

	struct cloudEndPoint {
		bool ConnectionStatus;
		int iCloudEndpoint;
		std::string topic;
		std::string cloudAppName;
	};
	cloudEndPoint cloudEpInfo;


	struct localEndPoint {
		bool ConnectionStatus;
		int ilocalEndpoint;
		std::string topic;
		std::string cloudAppName;
	};
	localEndPoint localEpInfo;


	struct appEndPoint {
		std::string strDevId;
		std::string appName;
	};
	appEndPoint appEPInfo;
	std::vector<appEndPoint> appEPVec;

	std::vector<std::string> appRegDevVec;

	std::map<std::string, std::string> configMap;

	std::multimap<std::string, std::string> connectMap;

	std::mutex cloudInitMutex;
	std::condition_variable cloudInitVar;

	std::mutex localInitMutex;
	std::condition_variable localInitVar;

	std::condition_variable testInitVar;
	bool waitingForCloudApp = true;


protected:
	/** processAsynTaskThread handles a function callback request on a separate thread */
	class AsynTaskThread : public qcc::Thread
	{
	public:
		AsynTaskThread(ServiceProvider& _sproPtr, DeviceObj& _deviceObjPtr)
	: sproPtr(_sproPtr), deviceObjPtr(_deviceObjPtr){}
		void setDevID(std::string data) {
			devId = data;
		}

		void setPayload(std::string send) {
			payload = send;
		}

	protected:
		qcc::ThreadReturn STDCALL Run(void* arg);

	private:
		ServiceProvider&    sproPtr;
		DeviceObj&		 	deviceObjPtr;
		std::string         devId;
		std::string         payload;
	};

	/** CloudConnectionThread handles a function callback request on a separate thread */
	//	class CloudConnectionThread : public qcc::Thread
	//	{
	//	public:
	//		CloudConnectionThread(ServiceProvider& _sproPtr, MethodExecuter& _methExPtr)
	//	: sproPtr(_sproPtr), methExPtr(_methExPtr) {}
	//
	//		void updateAppNmae(qcc::String send) {
	//			sender = send;
	//			cloudAppInVicinityVector.push_back(sender);
	//		}
	//
	//	protected:
	//		qcc::ThreadReturn STDCALL Run(void* arg);
	//
	//	private:
	//		ServiceProvider&    sproPtr;
	//		MethodExecuter&     methExPtr;
	//		std::string         sender;
	//		std::string         connectingAppName;
	//		std::vector<std::string> cloudAppInVicinityVector;
	//		std::mutex cctlock;
	//	};

	AsynTaskThread*	callbackThread;

	//	CloudConnectionThread* cloudconnectionThread;

};

#endif //SERVICEPROVIDER_HH
