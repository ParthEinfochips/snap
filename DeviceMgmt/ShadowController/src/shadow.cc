#include "shadow.h"
#include <python2.7/Python.h>
#include "ajcommon/CustomTransportMask.h"
#include "shadowcontroller.h"
#include <queue>

#define std_MODULE "SHADOW"
#define eI_MODULE "SHADOW"

std::mutex FileOPS::mLockFileOPS;

namespace varsCondMutex {
std::condition_variable condVar;
std::mutex mLock;
}

using namespace varsCondMutex;

bool stopThread = false;
bool runningSigThread = false;

Shadow::Shadow(shadowController *SCPtr, std::string &devId, std::string &macID,
		nlohmann::json &raml, nlohmann::json &shadowJson, const std::string &finalDirPathTemp) :
																						_SCPtr(SCPtr), devIDPerm(devId), macID(macID), finalDirPath(finalDirPathTemp), blobTopic(
																								"blob") {
	std::string ramlStr = raml.dump();

	//ADDED HERE=== NEED TO TEST
	jsonShadowFilePath = std::string(finalDirPath) + "/SC_" + devId
			+ ".json";

	ramlPath =  "/opt/GA.conf.d/" + configurationSettings::AppTopic + "/" + configurationSettings::AppTopic + std::string(SHADOW_RAML_TEMP_PATH);


	if(raml != NULL)
	{
		std::cout << "File doesn't exists, create new shadow" << std::endl;
		auto retVal = this->CreateShadowJson(devIDPerm, raml);

		if (!retVal)
			throw std::runtime_error("Shadow controller unable to create shadow.");
	}
	else
	{
		std::cout << "shadow already present so load it" << std::endl;
		jsonShadowObj = shadowJson;
	}

	LOGDBUG("Failing at this point ");

	//if (!retVal)
	//	throw std::runtime_error("Shadow controller unable to create shadow.");

	//!Telemetry Topic Preparation.
	//	telemetryTopic = "EI/Data/";
	//	telemetryTopic += configurationSettings::orgId;
	//	telemetryTopic += "/";
	//	telemetryTopic += configurationSettings::gatewayId;
	//	telemetryTopic += "/";
	//	telemetryTopic += configurationSettings::AppTopic;
	//	telemetryTopic += "/Devices/";

	//	telemetryTopic = configurationSettings::pubTelemetryTopic;
	//	telemetryTopic += devId;

	//!Blob Topic Preparation.
	blobTopic += "/" + configurationSettings::orgId;

	LOGINFO2("The blobbbbbbbbb  topic is ", blobTopic);

	//!Thread Starts.
	threadDataSigReceiver();
}

Shadow::~Shadow() {
	//!Gets the lock and stops the thread.

	LOGINFO("In the destructor");
	{
		std::lock_guard < std::mutex > lockGuarded(mLock);

		stopThread = true;
	}

	LOGINFO("In the destructor 2");

	condVar.notify_all();

	LOGINFO("In the destructor 3");

	if (t.joinable()) {
		t.join();
	}

	LOGINFO("In the destructor 4");
}

void Shadow::setConfig(int secs, TransportMask mask) {
	maskVal = mask;
}

bool Shadow::getShadowStatus() const {
	//!Future feature for device active or not.
	return activeShadow;
}

void Shadow::setShadowStatus(std::string payload) {
	//!Future feature for device active or not.
	//activeShadow = status;

	//need to add in file the status........


}

void Shadow::removeFirstDesired() {
	//Removes the job execution related info from the shadow file.
	for (auto &key : keysVect) {
		LOGEROR2("The key name is ", key);

		if (jsonChecker::checkJson(jsonShadowObj, { key })) {
			if (!checkDesiredVal(key)) {
				LOGEROR("Value cannot be set, need to get back from here");

				continue;
			}

			if (jsonShadowObj[key][valStrings::DESIRED_VAL_STR].size() >= 1) {
				jsonShadowObj[key][valStrings::DESIRED_VAL_STR].erase(0);

				std::cout << "jsonShadowfilePath" << jsonShadowFilePath << std::endl;
				writeJsonToSTDFile(jsonShadowFilePath, jsonShadowObj);

				//				std::cout << "======= " << jsonShadowObj[key][valStrings::DESIRED_VAL_STR].size() << jsonShadowObj[key][valStrings::DESIRED_VAL_STR] << std::endl;
			}
		} else {
			LOGEROR2("The unable to find key name is ", key);
		}
	}

	keysVect.clear();
}

bool Shadow::GetDeviceState(std::string& devId, std::string& data) {
	std::lock_guard < std::mutex > guardObj(mutexGetDeviceState);

	return GetDeviceStateActualTask(devId, data);
}

bool Shadow::GetDeviceStateActualTask(std::string& devId, std::string& data) {
	json objTemp;
	auto callInterface = false;

	try {
		objTemp = json::parse(data);

		auto COMMAND_TOPIC = checkTopicRCommand(objTemp);

		//Checks if the JOBID is there or not.
		if (!jsonChecker::checkJson(objTemp, { valStrings::JOBID_STR })) {
			throw std::runtime_error("JOBID Not present, job invalid.");
		}

		//Checks if the properties are there or not.
		if (!jsonChecker::checkJson(objTemp,
				{ valStrings::PROPERTIES_VAL_STR })) {
			throw std::runtime_error("Properties Not present, job invalid.");
		}

		//Goes over the properties one by one.
		for (auto &keyProperty : objTemp[valStrings::PROPERTIES_VAL_STR])
		{
			std::string key = keyProperty[valStrings::ONLY_PROPERTY_STR];

			if (jsonChecker::checkJson(jsonShadowObj, { key })) {

				int notifyVal = jsonShadowObj[key][valStrings::NOTIFY_VAL_STR];
				//Here if the value is null and if device is of notification type only then the reported value will be read from the shadow.
				if (!jsonShadowObj[key][valStrings::REPORTED_VAL_STR].is_null()  /* && if the device is of notification type*/ && notifyVal != 0) {
					auto str1 = jsonShadowObj[key][valStrings::REPORTED_VAL_STR];
					std::cout << "notify is not zero" << jsonShadowObj[key][valStrings::NOTIFY_VAL_STR].dump() << std::endl;
					keyProperty.clear();

					keyProperty[key] = str1;
				}
				else
				{
					std::cout << "notify is zero" << std::endl;
					callInterface = true;
					break;
				}
			}
		}

		json Packet;
		if(callInterface)
		{
			std::cout << "Calling the interface 1" << std::endl;

			_SCPtr->callTheInterface(GET_DEVICE_STATE,devId, data);

			COMMAND_TOPIC == TOPIC_R_COMMAND::IT_IS_COMMAND ?
					prepareResponse::formTheBlobPacket(objTemp[PP_APPNAME],
							devIDPerm, objTemp, std::move(json::parse(data)),
							statusesSC::GET_COMMAND_SC, Packet) :
							prepareResponse::formTheBlobPacket("cloudShadow", devIDPerm,
									objTemp, std::move(json::parse(data)),
									statusesSC::GET_COMMAND_SC, Packet);


			//----FOR UPDATING FILE AND SENDING SNAPSHOT IN GET(when no telemetry)--//
			std::cout << "data payload is" << data << std::endl;

			json jsonData;
			json propJson = json::parse(data);

			json tempJson = propJson["properties"][0];

			for(json::iterator j =  tempJson.begin(); j != tempJson.end(); j++)
			{
				if(j.key() != "observedtime")
				{
					jsonData["propertyName"] = j.key();
					jsonData["value"] = j.value();
				}
			}
			std::cout << "jsonDATA.dump" << jsonData.dump() << std::endl;
			DataReceived(devId, jsonData.dump());

			//----------------------------------------------------------------------
		}
		else
		{
			objTemp[valStrings::RESPONSE_CODE_STR] = 200;

			data = objTemp.dump();

			COMMAND_TOPIC == TOPIC_R_COMMAND::IT_IS_COMMAND ?
					prepareResponse::formTheBlobPacket(objTemp[PP_APPNAME],
							devIDPerm, std::move(json::parse(data)), objTemp,
							statusesSC::GET_COMMAND_SC, Packet) :
							prepareResponse::formTheBlobPacket("cloudShadow", devIDPerm,
									std::move(json::parse(data)), objTemp,
									statusesSC::GET_COMMAND_SC, Packet);
		}

		std::string blobPacket(Packet.dump());

		//Blob is sent to cloud.
		_SCPtr->sendToCloud(blobTopic, blobPacket);

		return true;
	} catch (std::exception &e) {
		LOGEROR2(
				"Exception raised at outer Get Device State.------------------",
				e.what());

		objTemp[valStrings::RESPONSE_CODE_STR] = 400;
		objTemp[valStrings::MESSAGE_STR] = e.what();
		objTemp[valStrings::STATUS_STR] = "failure";

		data = objTemp.dump();
	}

	return false;
}

std::string Shadow::getValue(json &objTemp) {
	for (auto it = objTemp.begin(); it != objTemp.end(); it++) {
		try {
			decltype(it.key()) str = it.key();

			std::cout << str << std::endl;

			if (jsonChecker::checkJson(jsonShadowObj, { str })) {
				return str;
			} else {
				return "";
			}
		} catch (...) {
			auto e = std::current_exception();

			std::rethrow_exception(e);
		}
	}

	return "";
}

bool Shadow::checkDesiredVal(std::string &key) {
	if (jsonChecker::checkJson(jsonShadowObj[key],
			{ valStrings::DESIRED_VAL_STR })) {
		return true;
	} else {
		return false;
	}
}

bool Shadow::checkIFDesiredEmpty(std::string &key) {
	if (jsonShadowObj[key][valStrings::DESIRED_VAL_STR].empty())
		return true;
	else
		return false;
}

TOPIC_R_COMMAND Shadow::checkTopicRCommand(json &objJson) {

	// The origin is checked and category of the origin is decided.
	if (jsonChecker::checkJson(objJson, { PP_TOPIC })) {
		LOGINFO("In topic");

		return TOPIC_R_COMMAND::IT_IS_TOPIC;
	} else if (jsonChecker::checkJson(objJson, { PP_APPNAME })) {
		LOGINFO("In Command");
		return TOPIC_R_COMMAND::IT_IS_COMMAND;
	}
}

bool Shadow::multiplePropertiesInSingleObject(const std::string &JOBID_STR,
		json &objTemp) {
	auto allSet = true;

	// A simple loop to go over all the keys in a single object.
	for (auto it = objTemp.begin(); it != objTemp.end(); it++) {
		try {
			decltype(it.key()) key = it.key();

			std::cout << key << std::endl;

			if (jsonChecker::checkJson(jsonShadowObj, { key })) {
				if (!checkDesiredVal(key)) {
					LOGEROR("Value cannot be set, need to get back from here");

					continue;
				}

				if (objTemp[key]
							== jsonShadowObj[key][valStrings::REPORTED_VAL_STR]) {
					continue;
				}

				allSet = false;

				keysVect.emplace_back(key);

				jsonShadowObj[key][valStrings::DESIRED_VAL_STR].push_back(
						json( { { valStrings::JOBID_STR, JOBID_STR }, { "value",
								objTemp[key] } }));
			}
		} catch (...) {
			std::rethrow_exception(std::current_exception());
		}
	}

	return allSet;
}

bool Shadow::createCommandsForShadowsProperty(json &objTemp) {
	try {
		auto COMMAND_TOPIC = checkTopicRCommand(objTemp);

		bool allSet = true;

		//Iterates over properties array.
		for (auto &keyProperty : objTemp[valStrings::PROPERTIES_VAL_STR]) {
			//If the object has multiple properties or size is greater than 1. If yes, continues
			if (keyProperty.size() > 1) {
				allSet = multiplePropertiesInSingleObject(
						objTemp[valStrings::JOBID_STR], keyProperty);

				continue;
			}

			auto key = getValue(keyProperty);

			if (!checkDesiredVal(key)) {
				LOGEROR("Value cannot be set, need to get back from here");

				continue;
			}

			if (keyProperty[key]
							== jsonShadowObj[key][valStrings::REPORTED_VAL_STR]) {
				continue;
			}

			allSet = false;

			keysVect.emplace_back(key);

			jsonShadowObj[key][valStrings::DESIRED_VAL_STR].push_back(json( { {
				valStrings::JOBID_STR, objTemp[valStrings::JOBID_STR] }, {
						"value", keyProperty[key] } }));
		}

		json Packet;

		//If all are set then only setAlreadyState for the Job is sent. However, if anyone property is not set, command is fired.
		if (!allSet) {
			std::cout << "IN allSET" << std::endl;
			sendSnapShot();

			std::string data(objTemp.dump());

			std::cout << "data in ALLSET2" << data << std::endl;
			_SCPtr->callTheInterface(SET_DEVICE_STATE, devIDPerm, data);

			std::cout << "IN allset 3" << std::endl;
			//UPDATE THE SHADOW FILE WITH LATEST VALUE.........
			//			std::cout << "IN SHADOW CONTROLLER--break--packet--" << data << std::endl;
			//
			//			try
			//			{
			//				json tempJson = json::parse(data);
			//
			//				auto properties = tempJson[valStrings::PROPERTIES_VAL_STR][0];
			//
			//				auto keyProp = getValue(properties);
			//
			//				auto latestKeyValue = properties[keyProp];
			//
			//				jsonShadowObj[keyProp][valStrings::REPORTED_VAL_STR] = properties[keyProp];
			//
			//				writeJsonToSTDFile(jsonShadowFilePath, jsonShadowObj);
			//
			//			}
			//			catch(...)
			//			{
			//				std::cout << "Exception raised" << std::endl;
			//
			//			}

			COMMAND_TOPIC == TOPIC_R_COMMAND::IT_IS_COMMAND ?
					prepareResponse::formTheBlobPacket(objTemp[PP_APPNAME],
							devIDPerm, objTemp, std::move(json::parse(data)),
							statusesSC::DELTA_N_FIRED_SC, Packet) :
							prepareResponse::formTheBlobPacket("cloudShadow", devIDPerm,
									objTemp, std::move(json::parse(data)),
									statusesSC::DELTA_N_FIRED_SC, Packet);
		} else {
			COMMAND_TOPIC == TOPIC_R_COMMAND::IT_IS_COMMAND ?
					prepareResponse::formTheBlobPacket(objTemp[PP_APPNAME],
							devIDPerm, objTemp, "",
							statusesSC::STATE_ALREADY_SET_SC, Packet) :
							prepareResponse::formTheBlobPacket("cloudShadow", devIDPerm,
									objTemp, "", statusesSC::STATE_ALREADY_SET_SC,
									Packet);
		}

		std::string blobPacket(Packet.dump());

		//Blob is sent to cloud.
		_SCPtr->sendToCloud(blobTopic, blobPacket);

		//Jobs are then being removed.
		removeFirstDesired();

		//Jobs removed latest snapshot sent.
		sendSnapShot();

		return true;
	} catch (std::exception &e) {
		LOGEROR2(
				"Handle exception in the shadow and return false after updating the packet",
				e.what());

		objTemp[valStrings::RESPONSE_CODE_STR] = 400;
		objTemp[valStrings::MESSAGE_STR] = e.what();
		objTemp[valStrings::STATUS_STR] = "failure";

		std::string blobPacket(objTemp.dump());

		//Blob is sent to cloud.
		_SCPtr->sendToCloud(blobTopic, blobPacket);

		//Jobs are then being removed.
		removeFirstDesired();

		return false;
	}
}

void Shadow::threadDataSigReceiver() {
	// A sleeping a thread, only gets activated when a job arrives.
	// Subject to change as a task.
	t =	std::thread(
			[this]() {
		int count = 0;

		while(1)
		{
			std::unique_lock<std::mutex> uniLock(mLock);

			condVar.wait(uniLock, [this] {
				if(!commandQueue.empty() || stopThread) return true;
				else return false;
			});

			if(stopThread)
			{
				if(commandQueue.empty())
					break;
			}

			auto jsonObjPair = commandQueue.front();

			commandQueue.pop_front();

			uniLock.unlock();

			//							Filewrite(std::to_string(count++));

			createCommandsForShadowsProperty(jsonObjPair.second);

			LOGINFO("In the thread of Shadow Controller");
		}
	});
}

bool Shadow::SetDeviceState(std::string &devId, std::string& data) {
	json objTemp;

	try {
		objTemp = nlohmann::json::parse(data);

		//Checks if the JOBID is there or not.
		if (!jsonChecker::checkJson(objTemp, { valStrings::JOBID_STR })) {
			throw std::runtime_error("JOBID Not present, job invalid.");
		}

		//Checks if the properties are there or not.
		if (!jsonChecker::checkJson(objTemp,
				{ valStrings::PROPERTIES_VAL_STR })) {
			throw std::runtime_error("Properties Not present, job invalid.");
		}

		//Sends an immediate response of job queued, This is ought to change.
		objTemp[valStrings::RESPONSE_CODE_STR] = 200;
		objTemp[valStrings::JOB_STATUS_VAL_STR] = statusesSC::COMMAND_QUEUED_SC;

		data = objTemp.dump();

		objTemp.erase(valStrings::JOB_STATUS_VAL_STR);

		std::lock_guard < std::mutex > LockGuardObj(mLock);

		//Emplaces the command back in the queue.
		commandQueue.emplace_back(std::make_pair(devId, std::move(objTemp)));

		condVar.notify_all();

		std::cout << "IN 111 SETDEVICE STATE RETURN" << data << std::endl;
		return true;
	}
	catch (std::exception &e)
	{
		LOGINFO("This will be the collect pointing for all exceptions.");

		objTemp[valStrings::RESPONSE_CODE_STR] = 400;
		objTemp[valStrings::MESSAGE_STR] = e.what();
		objTemp[valStrings::STATUS_STR] = "failure";

		data = objTemp.dump();
	}

	return false;
}

void Shadow::printCommandQueue() const {
	for (auto &i : commandQueue) {
		std::cout << "Jobs scheduled for == " << i.second << std::endl;
	}
}

void Shadow::DataReceivedThreaded(std::string &devId, std::string &data) {
	auto future = std::async(std::launch::async, &Shadow::DataReceived, this,
			devId, data);
}

void Shadow::sendSnapShot() {
	std::cout << "IN sendsnap shot" << std::endl;
	json objSnapShot;

	objSnapShot["device_status"] = jsonShadowObj;

	std::string snapShot(objSnapShot.dump());

	std::cout << "SNAPSHOT PAYLAOAD" << snapShot << std::endl;

	telemetryTopic = configurationSettings::pubTelemetryTopic;
	telemetryTopic += devIDPerm;

	LOGINFO2(
			" =======>>>>>>>>>>>>>>>>>>>> >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",
			telemetryTopic);

	_SCPtr->sendToCloud(telemetryTopic, snapShot);
}

//Change will happen here in this function.
void Shadow::DataReceived(std::string devId, std::string data) {
	json objTemp;

	try {
		objTemp = json::parse(data);

		std::cout << "jsonShadowObj is" << jsonShadowObj << std::endl;
		std::string key = objTemp[valStrings::PROPERTYNAME_STR];

		jsonShadowObj[key][valStrings::REPORTED_VAL_STR] =
				objTemp[valStrings::VALUE_STR];

		sendSnapShot();

		//Updates the shadow file.
		writeJsonToSTDFile(jsonShadowFilePath, jsonShadowObj);
	} catch (std::exception &e) {
		LOGEROR2("Telemetry packet is wrong. SHADOW CONTROLLER", e.what());

		objTemp[valStrings::RESPONSE_CODE_STR] = 400;
		objTemp[valStrings::MESSAGE_STR] = e.what();
		objTemp[valStrings::STATUS_STR] = "failure";

		data = objTemp.dump();
	}

	_SCPtr->SendTelemetryDataCCAPP(devId, data);

	return;
}

bool Shadow::CreateShadowJson(const std::string &devId,
		nlohmann::json &jsonObj) {

	ramlFilePath = ramlPath + "raml_" + devId
			+ ".raml";

	std::cout << "IN CREATESHADOWJSON" << ramlFilePath << std::endl;
	//	ramlFilePath = std::string(SHADOW_RAML_TEMP_PATH) + "/raml_" + devId
	//			+ ".raml";

	std::fstream files;
	files.open(ramlFilePath, std::ios::out | std::ios::in | std::ios::app);

	if (files.is_open()) {
		files << this->Unescape(jsonObj["raml"].dump());
		files.close();
	} else {
		throw std::runtime_error("RAML file unable to created");
	}

	try {
		auto retVal = CreateShadowControlFile(devId);
		return retVal;
	} catch (...) {
		auto e = std::current_exception();

		std::rethrow_exception(e);
	}
}

bool Shadow::CreateShadowControlFile(const std::string &devId) {
	valuesReqRAML valueObj;

	valueObj.RAMLFilePath = ramlFilePath;
	valueObj.jsonFilePath = std::string(finalDirPath) + "/SC_" + devId
			+ ".json";

	jsonShadowFilePath = valueObj.jsonFilePath;

	LOGINFO2(valueObj.RAMLFilePath, valueObj.jsonFilePath);

	ramlParserPyCPP objRamlParserCPPPy;

	objRamlParserCPPPy.getJsonOfRAML(&valueObj);

	bool retVal = false;

	try {
		retVal = objRamlParserCPPPy.usageSpecificFunction();
	} catch (...) {
		auto e = std::current_exception();

		std::rethrow_exception(e);
	}

	jsonShadowObj = valueObj.ObjectOfRAML;

	LOGDBUG("In the shadow njdckjdskjnckjsdn ");

	if (jsonShadowObj.empty())
		retVal = false;

	std::cout << "In the shadow njdckjdskjnckjsdn 2 " << retVal << std::endl;

	return retVal;
}

std::string Shadow::Unescape(const std::string& str) {
	std::string res;
	std::string::const_iterator it = str.begin() + 1;
	while (it != str.end() - 1) {
		char ch = *it++;
		if (ch == '\\' && it != str.end()) {
			switch (*it++) {
			case '\\':
				ch = '\\';
				break;        // Back Slash
			case 'n':
				ch = '\n';
				break;        // New Line
			case 't':
				ch = '\t';
				break;        // Tab
			case '"':
				ch = '\"';
				break;        // Double Quotes
			case 'b':
				ch = '\b';
				break;        // Backspace
			case 'r':
				ch = '\r';
				break;        // Carriage Return
				// all oth er escapes
			default:
				// invalid escape sequence - skip it. alternatively you can copy it as is, throw an exception.
				continue;
			}
		}
		res += ch;
	}
	return res;
}

int Shadow::writeJsonToSTDFile(const std::string &filePath, const json &dataBuf)
{
	auto objPtr = std::fopen(filePath.c_str(), "w+");

	if(objPtr)
	{
		std::fwrite(dataBuf.dump().c_str(), 1, dataBuf.dump().size(), objPtr);

		std::fflush(objPtr);

		std::fclose(objPtr);
	}
	return 0;
}

void Shadow::Filewrite(std::string val)
{

	std::string filePath = "/tmp/" + devIDPerm + ".sample";

	val += "\n";

	std::fstream FileStrm;

	FileStrm.open(filePath, std::ios::out | std::ios::in |std::ios::app);

	if(FileStrm.is_open())
	{
		FileStrm << val;

		FileStrm.close();
		return 0;
	}
}

//=====================================Disabled function================================================

// bool Shadow::createCommandsForShadowsProperty(json &objTemp, std::string &devId, std::string &data)
// {
// 	try
// 	{
// 		auto &keyProperty = objTemp[valStrings::PROPERTIES_VAL_STR][0];

// 		auto key = getValue(keyProperty);

// 		auto COMMAND_TOPIC = checkTopicRCommand(objTemp);

// 		std::string actionStr = "setDeviceState";

// 		std::cout << "======== = ============= = = ===================== 1 ==== " << COMMAND_TOPIC << std::endl;

// 		if(!checkDesiredVal(key))
// 		{
// 			LOGEROR("Value cannot be set, need to get back from here");

// 			objTemp[valStrings::RESPONSE_CODE_STR] = 400;
// 			objTemp[valStrings::MESSAGE_STR] = "Property not for set.";
// 			objTemp[valStrings::STATUS_STR] = "failure";

// 			data = objTemp.dump();

// 			return false;
// 		}

// 		LOGINFO("======== = ============= = = ===================== 2");

// 		if(keyProperty[key] == jsonShadowObj[key][valStrings::REPORTED_VAL_STR])
// 		{
// 			prepareResponse::prepareResponseOnlyStatus(objTemp, statusesSC::STATE_ALREADY_SET_SC);

// 			data = objTemp.dump();

// 			auto COMMAND_TOPIC = checkTopicRCommand(objTemp);

// 			if(COMMAND_TOPIC == TOPIC_R_COMMAND::IT_IS_COMMAND) 
// 			{
// 				json tempJson;

// 				prepareResponse::formTheBlobPacket(objTemp, tempJson, devId, actionStr, key, statusesSC::STATE_ALREADY_SET_SC);

// 				std::string jsonStr = tempJson.dump();

// 				std::cout << jsonStr << std::endl;

// 				_SCPtr->sendToCloud(telemetryTopic, jsonStr);
// 			}

// 			return false;
// 		}

// 		json tempObj;

// 		tempObj[valStrings::JOBID_STR] =  objTemp[valStrings::JOBID_STR];
// 		tempObj[valStrings::DESIREDVALUE_STR] = keyProperty[key];
// 		tempObj[valStrings::STATE_REQUEST_TIME] = GetObservedTime();

// 		(COMMAND_TOPIC == TOPIC_R_COMMAND::IT_IS_TOPIC) ? tempObj[PP_TOPIC] = objTemp[PP_TOPIC] : tempObj[PP_APPNAME] = objTemp[PP_APPNAME];

// 		tempObj[valStrings::STATUS_STR] = statusesSC::DELTA_N_FIRED_SC;

// 		jsonShadowObj[key][valStrings::DESIRED_VAL_STR].push_back(tempObj);

// 		maskVal == eITM::TRANSPORT_EI_MODBUS ? prepareResponse::prepareResponseOnlyStatus(objTemp, statusesSC::COMMAND_COMPLETED_SC) : prepareResponse::prepareResponseOnlyStatus(objTemp, statusesSC::DELTA_N_FIRED_SC);

// 		auto ptrObj = std::make_shared<shadowCommand>(devId, tempObj, key, COMMAND_TOPIC);

// 		data = objTemp.dump();

// 		_SCPtr->callTheInterface(SET_DEVICE_STATE, devId, data);

//         if(COMMAND_TOPIC == TOPIC_R_COMMAND::IT_IS_COMMAND)
//         {
// 			json tempJson;

// 			prepareResponse::formTheBlobPacket(objTemp, tempJson, devId, actionStr, key, statusesSC::DELTA_N_FIRED_SC);

// 			std::string jsonStr = tempJson.dump();

// 			_SCPtr->sendToCloud(telemetryTopic, jsonStr);

//             ptrObj->setCommandPayload();
//         }
// 		else 
// 		{
// 			ptrObj->SetReply(data);
// 		}

// 		incomingShadowRequests[key] = ptrObj;

// 		return true;
// 	}
// 	catch(std::exception &e)
// 	{
//         std::cout << "Let us find out the exce" << e.what() << std::endl;

// 		objTemp[valStrings::RESPONSE_CODE_STR] = 400;
// 		objTemp[valStrings::MESSAGE_STR] = e.what();
// 		objTemp[valStrings::STATUS_STR] = "failure";

// 		data = objTemp.dump();
// 	}
// }

// bool Shadow::createCommandsForShadowsProperties(json &objTemp, std::string &devId, std::string &data)
// {
// 	json deviceStateObj;

// 	try
// 	{
// 		int count  = 0;

// 		deviceStateObj[valStrings::PROPERTIES_VAL_STR] = objTemp[valStrings::PROPERTIES_VAL_STR];

// 		objTemp.erase(valStrings::PROPERTIES_VAL_STR);

// 		objTemp[valStrings::PROPERTIES_VAL_STR] = json::array();

// 		for(auto &keyProperty: deviceStateObj[valStrings::PROPERTIES_VAL_STR])
// 		{
// 			auto key = getValue(keyProperty);

// 			if(!checkDesiredVal(key))
// 			{
// 				LOGEROR("Value cannot be set, need to get back from here");

// 				continue;
// 			}

// 			if(keyProperty[key] == jsonShadowObj[key][valStrings::REPORTED_VAL_STR])
// 			{
// 				prepareResponse::prepareResponseOnlyStatus(keyProperty, statusesSC::STATE_ALREADY_SET_SC);

// 				continue;
// 			}

// 			json tempObj;

// 			tempObj[valStrings::JOBID_STR] =  objTemp[valStrings::JOBID_STR];
// 			tempObj[valStrings::DESIREDVALUE_STR] = keyProperty[key];
// 			tempObj[valStrings::STATE_REQUEST_TIME] = GetObservedTime();

// 			if(!checkIFDesiredEmpty(key))
// 			{
// 				tempObj[valStrings::JOBSTATUS_STR] = statusesSC::COMMAND_QUEUED_SC;
// 				jsonShadowObj[key][valStrings::DESIRED_VAL_STR].push_back(tempObj);

// 				prepareResponse::prepareResponseOnlyStatus(keyProperty, statusesSC::COMMAND_QUEUED_SC);

// 				continue;
// 			}

// 			tempObj[valStrings::JOBSTATUS_STR] = statusesSC::DELTA_N_FIRED_SC;

// 			jsonShadowObj[key][valStrings::DESIRED_VAL_STR].push_back(tempObj);

// 			prepareResponse::prepareResponseOnlyStatus(keyProperty, statusesSC::DELTA_N_FIRED_SC);

// 			objTemp[valStrings::PROPERTIES_VAL_STR].push_back(keyProperty);

// 			deviceStateObj[valStrings::PROPERTIES_VAL_STR][count].clear();

// 			auto ptrObj = std::make_shared<shadowCommand>(devId, jsonShadowObj, key, TOPIC_R_COMMAND::IT_IS_COMMAND);

// 			if(incomingShadowRequests.empty())
// 				timerObj.startSimulation();

// 			incomingShadowRequests[key] = ptrObj;

// 			count++;

// 		}
// 	}
// 	catch(std::exception &e)
// 	{
// 		LOGEROR2("Handle exception in the shadow and return false after updating the packet", e.what());

// 		prepareResponse::prepareResponseConvertString(data, statusesSC::INVALID_JSON_SC);

// 		return false;
// 	}

// 	try {

// 		data = objTemp.dump();

// 		_SCPtr->callTheInterface(SET_DEVICE_STATE, devId, data);

// 		json finalResponse = json::parse(data);

// 		for(auto &keyProperty : deviceStateObj[valStrings::PROPERTIES_VAL_STR])
// 		{
// 			if(keyProperty.empty())
// 				continue;

// 			finalResponse[valStrings::PROPERTIES_VAL_STR].push_back(keyProperty);
// 		}

// 		data = finalResponse.dump();

// 		return true;

// 	}
// 	catch(std::exception &e)
// 	{

// 	}

// 	return false;
// }

// bool Shadow::SetDeviceState(std::string &devId, std::string& data)
// {
// 	json objTemp;

//     try
//     {
//     	objTemp = json::parse(data);

//         if(jsonChecker::checkJson(objTemp,{valStrings::PROPERTIES_VAL_STR}))
//         {
//         	if(objTemp[valStrings::PROPERTIES_VAL_STR].size() > 1)
//         	{
//         		createCommandsForShadowsProperties(objTemp, devId, data);
//         	}
//         	else if(objTemp[valStrings::PROPERTIES_VAL_STR].size() == 1)
//         	{
//         		createCommandsForShadowsProperty(objTemp, devId, data);
//         	}
//         }
//         if(jsonChecker::checkJson(objTemp,{valStrings::SUBPROPERTIES_VAL_STR}))
// 		{
// //        	return createCommandsForSubProperties(objTemp);
// 		}
//     }
//     catch(std::exception &e)
//     {
//     	LOGINFO("This will be the collect pointing for all exceptions.");

// 		objTemp[valStrings::RESPONSE_CODE_STR] = 400;
// 		objTemp[valStrings::MESSAGE_STR] = e.what();
// 		objTemp[valStrings::STATUS_STR] = "failure";

// 		data = objTemp.dump();
//     }

//     return false;
// }

// std::shared_ptr<shadowCommand> Shadow::getSCHPtr(std::string &key)
// {
// 	auto it = incomingShadowRequests.find(key);

// 	if(it != incomingShadowRequests.end())
// 	{
// 		return it->second;
// 	}

// 	return nullptr;
// }

// void Shadow::timedResponseBackToCloud()
// {
// 	if(incomingShadowRequests.empty())
// 	{
// 		timerObj.stopTimer();

// 		return;
// 	}

// 	json objFinal;

// 	objFinal["SHADOW_SET_STATUS"] = json::array();

// 	for(auto &pair: incomingShadowRequests)
// 	{
// 		auto epoch = pair.second->getLastTelemetryEpoch();

// 		uint64_t resultEpoch = std::time(nullptr);

// 		auto diff = resultEpoch - epoch;

// 		std::cout << "The information is " << diff << std::endl;

// 		if(diff > shadowSetStateTimeOut)
// 		{
// 			auto jsonObj = pair.second->giveCommandPayload();

// 			std::cout << jsonObj.dump() << std::endl;

// 			prepareResponse::prepareResponseOnlyStatus(jsonObj, statusesSC::NO_UPDATE_FROM_DEVICE);

// 			objFinal["SHADOW_SET_STATUS"].push_back(jsonObj);
// 		}
// 	}

// 	LOGINFO2("OBJ FINAL is ", objFinal.dump());

// //	callTheServiceProvider()
// }

// std::string Shadow::flushCommands(std::string &data)
// {
// 	json objFinal = json::parse(data);

// 	for(auto &pair: incomingShadowRequests)
// 	{
// //		get the command,

// 		auto &key = pair.first;
// 		auto &ptrShadowCommand = pair.second;

// 		if(ptrShadowCommand)
// 		{
// 			if(ptrShadowCommand->currentDeltaSituation() == DELTA_PRESENT)
// 			{
// 				if(!key.empty())
// 				{
// 					jsonShadowObj[key];
// 				}
// 			}
// 		}

// //		get property of the command,
// //		set the value,
// //		remove the first element of the desired value and then set the command.

// 	}

// 	std::cout << "OBJ FINAL is " << objFinal.dump() << std::endl;

// }

// std::string Shadow::getDeviceStateNFlush(std::string &data)
// {
// 	json objFinal = json::parse(data);

// 	for(auto &pair: incomingShadowRequests)
// 	{

// 		//I will simply update the value and close the command
// 	}

// 	std::cout << "OBJ FINAL is " << objFinal.dump() << std::endl;
// }

// void Shadow::DataReceived(std::string devId, std::string data)
// {
// 	json objTemp;
// 	std::string cloudOnlyVal;
// 	std::string topicForCloud;

//     try
//     {
//     	objTemp = json::parse(data);

//     	std::string key = objTemp[valStrings::PROPERTYNAME_STR];

//         jsonShadowObj[key][valStrings::REPORTED_VAL_STR] = objTemp[valStrings::VALUE_STR];

//         if(checkDesiredVal(key))
//         {
//         	auto ptrShadowCommand = this->getSCHPtr(key);

//         	if(ptrShadowCommand)
//         	{
//         		auto situationDelta = ptrShadowCommand->compareVals(jsonShadowObj);

//         		if(situationDelta == DELTA_SITUATIONS::NO_DELTA_PRESENT)
//         		{
// 					auto cmdRTopic = ptrShadowCommand->getTopicRCMDVal();

//         			// auto JobId = jsonObj[valStrings::JOBID_STR]; Temporarily Disabled.

// 					if(cmdRTopic == TOPIC_R_COMMAND::IT_IS_TOPIC) 
// 					{
// 						auto jsonObj = ptrShadowCommand->GetReply();

// 						prepareResponse::prepareResponseOnlyStatus(jsonObj, statusesSC::COMMAND_COMPLETED_SC);

// 						cloudOnlyVal = jsonObj.dump();

// 						topicForCloud = jsonObj[PP_TOPIC];

// 						LOGDBUG2("============ =============== =========== topic", cloudOnlyVal);

// 						_SCPtr->sendToCloud(topicForCloud, cloudOnlyVal);
// 					}
// 					else if(cmdRTopic == TOPIC_R_COMMAND::IT_IS_COMMAND) 
// 					{
// 						auto jsonObj = ptrShadowCommand->giveCommandPayload();

// 						prepareResponse::prepareResponseOnlyStatus(jsonObj, statusesSC::COMMAND_COMPLETED_SC);

// 						cloudOnlyVal = jsonObj.dump();

// 						_SCPtr->sendToCloud(telemetryTopic, cloudOnlyVal);

// 						LOGDBUG2("============ =============== ===========   ", cloudOnlyVal);
// 					}

//         			incomingShadowRequests.erase(key);
//         		}
//         	}
//         }
//         // FileOPS::writeJsonToSTDFile(jsonShadowFilePath, jsonShadowObj);  Temporarily disabled.
//     }
//     catch(std::exception &e)
//     {
//     	LOGEROR2("Telemetry packet is wrong. SHADOW CONTROLLER", e.what());

// 		objTemp[valStrings::RESPONSE_CODE_STR] = 400;
// 		objTemp[valStrings::MESSAGE_STR] = e.what();
// 		objTemp[valStrings::STATUS_STR] = "failure";

// 		data = objTemp.dump();
//     }

//     _SCPtr->SendTelemetryDataCCAPP(devId, data);

//     return;
// }

//bool Shadow::createCommandsForSubProperties(json &objTemp)
//{
//	try
//	{
//		for(auto keyProperty : objTemp[valStrings::SUBPROPERTIES_VAL_STR][valStrings::CONFIG_VAL_STR])
//		{
//			auto key = getValue(keyProperty);
//
//			if(!checkDesiredVal(key))
//			{
//				LOGEROR("Value not set, need to get back from here");
//
//				return false;
//			}
//
//			if(keyProperty[key] == jsonShadowObj[key][valStrings::REPORTED_VAL_STR])
//			{
//				prepareResponse::prepareResponseOnlyStatus(objTemp, statusesSC::STATE_ALREADY_SET_SC);
//
//				return false;
//			}
//
//			_SCPtr->callTheInterface(SET_DEVICE_STATE, devId, data);
//
//			prepareResponse::prepareResponseConvertString(data, statusesSC::DELTA_N_FIRED_SC);
//
//			std::cout << "=====================================" << keyProperty[key] << jsonShadowObj[key]["reported_value"];
//
//			jsonShadowObj[key][valStrings::DESIRED_VAL_STR] = keyProperty[key];
//
//			auto ptrObj = std::make_shared<shadowCommand>(devId, jsonShadowObj, key, objTemp);
//
//			incomingShadowRequests[key] = ptrObj; //Until the data doesn't resolve it will stay here.
//		}
//	}
//	catch(std::exception &e)
//	{
//		LOGEROR2("Handle exception in the shadow and return false after updating the packet", e.what());
//	}
//}

//
//if(statusVal == STATUS_SUCCESS)
//	{
//		return DEVICE_SITUATIONS::SUCCESS;
//	}
//	else if (statusVal == STATUS_FAILURE)
//	{
//		return DEVICE_SITUATIONS::FAILED;
//	}
//	else if (statusVal == STATUS_IN_PROGRESS)
//	{
//		return DEVICE_SITUATIONS::INPROGRESS;
//	}
//=================================== Old code, may not be in use.
