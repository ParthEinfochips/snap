#ifndef MQTTBUSOBJECT_H
#define MQTTBUSOBJECT_H

#include <queue>
#include <list>
#include <mutex>
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/BusListener.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/Message.h>
#include <alljoyn/MessageReceiver.h>
#include "ajNameService/nsCallBack.h"
#include "jsonCheck.hpp"
#include "cloudEndpoint.h"
#include "MqttApi.h"
#include <stdlib.h>
#include <sstream>
#include <future>
#include <unordered_map>
#include "configuration.h"
#include "CommonHeader/nsprefix.h"
#include "CommonHeader/CPPHeaders.h"
#include "proxy/methodExecuter.h"
#include "ajNameService/eIBusObject.h"
#include "ajNameService/ajNameService.h"
#include "queueMgmt.h"
#include "directoryOPS.h"

#define CONNECT "Connect"
#define SUBSCRIBE "RegisterTopic"
#define PUBLISH "SendToCloud"
#define UNSUBSCRIBE "DeregisterTopic"
#define DISCONNECT "Disconnect"

#define STATUS "status"
#define MSG "msg"
#define HOSTNAME "hostname"
#define MQTTHOSTNAME "mqtthostname"
#define MQTTPASSWORD "mqttpassword"
#define MQTTUSERNAME "mqttusername"
#define MQTTPORT "mqttport"
#define CAFILE "cafile"
#define CERTFILE "certfile"
#define KEYFILE "keyfile"
//#define ORGID "OrgId"
#define PACKAGEID "packageId"
#define GATEWAYID "gatewayId"
#define ORIGINAL "original"
#define STANDALONE "standalone"
#define USERNAME "username"
#define JOBID "JOBID"
#define PASSWORD "password"
#define PORT "port"
#define RESPONSECODE "responsecode"
#define ENDPOINTID "endpointid"
#define APPNAME "appname"
#define SUCCESS "success"
#define PROCESS "process"
#define FAILURE "failure"
#define MSG "msg"
#define DATA "data"
#define ANDROID_UID "android_uniqueid"
#define CLIENTID "clientid"
#define URL "url"
#define APPNAME "appname"
#define IS_ACTIVE "is_active"
#define CLIENTID "clientid"
#define TOPIC "topic"
#define LOCALCONTROL "LocalControl"
#define ANDROID "Android"
#define PUT "Put"
#define DEL "Del"
#define GATEWAYINFO "gatewayInfo"
#define CONFIGURATION "Configuration"
#define VERSION "version"
#define CIPHER "cipher"

#define CLOUDCONNECTIONCALLBACK "CloudConnectionCallback"
#define CLOUDDISCONNECTIONCALLBACK "CloudDisconnectionCallback"
#define CLOUDERRORCONNECTCALLBACK "CloudErrorcallback"

#define gatewayMetaData "/opt/GA.conf.d/gateway_meta.conf"

struct connectionMap
{
	// Store Connection String ["hostname|port"]
	std::string connectionString;

	// Store MQTT Connection Endpoint
	cloudEndpoint* mqttEndpoint;

	std::string status = "process";

	std::string caCrt;
	std::string cert;
	std::string key;

	// Store ID of Connection Endpoint
	uint16_t endpointID;

	// List of Subscribed Applications to this Endpoint
	std::list<std::string> appList;

	// Probably also store session id corresponding to app
	//SessionId sessionId;

	bool operator ==(const connectionMap &RHS){
		if(this->endpointID == RHS.endpointID)
			return true;
		else
			return false;
	}
};

struct credentials
{
	uint16_t ID;
	std::string username;
	std::string password;
	std::string appName;
};

struct statusInfo
{
	std::string appID;
	uint16_t endptID;
	std::string status;
	std::string username;
	std::string password;
	std::string callback;
	std::string brokerIp;
};

struct signalPacket_t
{
	std::string appname;
	std::string jsonStr;
	int endpointid;
	std::string callbackstatus;
};


struct messagePacket_t
{
	uint16_t endpointID;
	std::string msg;
};

class queueMgmt;
class MqttApi;

class mqttBusObject : public eIBusObject, public asynMethodCallBackHandler, public nsCallBack {

public:

	uint64_t counter = 0;
	mqttBusObject(ajn::BusAttachment* bus, const char* objectName, const char* objectPath,std::string adverName);

	~mqttBusObject();

	QStatus registerBusObject(const char* interfaceName);

	QStatus getMRAMLHandler(std::string &command, std::string &payload);

	QStatus asyncHandler(std::string &command, std::string &payload);

	QStatus syncHandler(std::string &command, std::string &payload);

	QStatus callbackRegisterHandler(std::string &command, std::string &payload);

	void signalHandler(std::string &command, std::string &payload);

	void lostAdvertisement(qcc::String busName/*, ajn::SessionId id*/);

	void foundAdvertisement(qcc::String busName);

	void busDisconnected();

	QStatus processCommand(std::string &command , std::string &payload, std::string appName);

	std::string getAppName(std::string topic);

	QStatus establishConnection(std::string &payload, std::string appName);

	QStatus sendToCloud(std::string &payload, std::string appName);

	QStatus registerTopic(std::string &payload, std::string appName);

	QStatus unregisterTopic(std::string &payload, std::string appName);

	QStatus disconnect(std::string &payload, std::string appName);

	/** Callback functions to be utilized by Threads **/
	void connectionCallback(uint16_t endpointID);

	void messageCallback(uint16_t endpointID, std::string jsonString);

	void errorCallback(uint16_t endpointID);

	void disconnectionCallback(uint16_t endpointID);

	void methodCallerAsyncCallback(std::string status,std::string payload);

	QStatus proxycall(uint16_t ID, std::string appName, std::string jsonString, std::string command, std::string resTopic);

	void sendDataResponse(uint16_t ID, std::string topic, std::string jsonString);

	void signalSender();

	QStatus sendSignal(uint16_t ID, std::string appName, std::string jsonString, std::string command);

	QStatus subUnsubAndroidTopic(uint16_t endpointID, std::string uniqueID, std::string actFlag);

	void setAppName(std::string tempTopic);

	std::string getCommand(std::string topic, std::string appName);

	/** Signal functions that will utilize the queue **/
	void connectionSignalFunction();

	void messageSignalFunction();

	void errorSignalFunction();

	void disconnectSignalFunction();

	MqttApi* GetMqttApiPtr();

	/** Thread Function used in connect() **/
	void connectionThreadFunction(const char *appName);

	std::list<connectionMap> activeConnectionList;

	std::vector<statusInfo> statusVec;

	std::multimap<uint16_t, credentials> credMap;

	std::unordered_map<std::string, queueMgmt*> queueObjMap;


private:

	ajn::BusAttachment* bus = NULL;

	MqttApi *mqttApiPtr;

	FileOPS *filePtr;

	methodExecuter *methodexPtr;

	signalExecuter *sigexPtr;

	json tempJson;

	std::string gatewayid;

	std::string orgID;

	bool temp = true;

	bool tempBool = true;

	bool firstQueueObj = true;
	bool mapQueue = false;


	statusInfo statInfo;

	bool certReq = true;

	bool sendSig = false;

	ajNameService *ajPtr;

	std::string adverName;

	static uint16_t endpointID;

	const ajn::InterfaceDescription* iface = NULL;


	/** Mutex for Callbacks **/
	std::mutex connectionMutex;
	std::mutex messageMutex;
	std::mutex errorMutex;
	std::mutex disconnectionMutex;


	/** Callback Queues **/
	std::queue<uint16_t> connectionQueue;
	std::queue<messagePacket_t> messageQueue;
	std::queue<uint16_t> errorQueue;
	std::queue<uint16_t> disconnectQueue;


	/** Queue Threads **/
	std::thread connectionThread;
	std::thread messageThread;
	std::thread errorThread;
	std::thread disconnectionThread;

	std::thread signalThread;
	std::queue<signalPacket_t> signalQ;
	std::mutex signalMutex;
	std::condition_variable signalVar;

	bool signalThreadRunner = true;
	bool isSignalThreadRunning = true;
};

#endif // MQTTBUSOBJECT_H

