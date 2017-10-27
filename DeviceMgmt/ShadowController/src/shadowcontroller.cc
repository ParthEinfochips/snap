#include "shadowcontroller.h"
#include "../../DeviceObject/DeviceObj.h"
#include "../../ServiceProvider.h"

#define eI_MODULE "SHADOW_CONTROLLER"

shadowController::shadowController() : _maskSCGlo(TRANSPORT_NONE), _ptrServiceProvider(nullptr), _DevPtr(nullptr)
{
	this->createDirectory();

}

shadowController::shadowController(TransportMask mask, ServiceProvider *ptrServiceProvider, DeviceObj *devPtr)
: _maskSCGlo(mask), _ptrServiceProvider(ptrServiceProvider), _DevPtr(devPtr)

{
	this->createDirectory();
}

shadowController::~shadowController()
{

}

void shadowController::generateShadowsFromPersistent()
{

}

void shadowController::ConfigurationStorage()
{
	shadowSetStateTimeOut = 30;
}

bool shadowController::isShadowCreated(std::string &devId)
{
	auto mapIt = shadowMapObj.find(devId);

	if(mapIt != shadowMapObj.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void shadowController::writeToFile(std::string path, std::string payload)
{
	auto objPtr = std::fopen(path.c_str(), "w+");

	if(objPtr)
	{
		std::fwrite(payload.c_str(), 1, payload.size(), objPtr);

		std::fflush(objPtr);

		std::fclose(objPtr);
	}
}

void shadowController::createShadow(std::string &devId, std::string &raml)
{
	std::cout << "createShadow" << devId << std::endl;
	json objJson = json::parse(raml);
	std::string macID = "";
	json shadowJson;

	if(!jsonChecker::checkJson(objJson, {"raml"}))
	{
		throw std::runtime_error("RAML not present in the packet, shadow cannot be created.");
	}

	std::shared_ptr<Shadow> shadowPtr;

	try
	{

		shadowPtr = std::make_shared<Shadow>(this, devId, macID, objJson, shadowJson, finalDirPath);

		shadowPtr->setConfig(shadowSetStateTimeOut, _maskSCGlo);

		shadowMapObj[devId] = shadowPtr;

		std::cout << "Am I coming here .." << shadowMapObj.size() << std::endl;
	}
	catch(std::exception &e)
	{
		shadowPtr.reset();
		objJson[valStrings::RESPONSE_CODE_STR] = 400;
		objJson[valStrings::MESSAGE_STR] = e.what();
		objJson[valStrings::STATUS_STR] = "failure";

		raml = objJson.dump();
	}
}

void shadowController::persistantShadow(std::string &macID, std::string &devId)
{
	json ramlJson;
	json devJson;
	struct stat fb;

	std::string shadowFile = std::string(finalDirPath) + "/SC_" + devId
			+ ".json";

	FileOPS::readFromFile(shadowFile, devJson);

	std::shared_ptr<Shadow> shadowPtr;


	try
	{
		std::string shadowRAML = ramlDirPath + "raml_" + devId  + ".raml";

		auto exists = stat(shadowRAML.c_str(), &fb);

		if(exists < 0)
		{
			std::cout << "File doesn't exists, no need to update persistant shadow" << std::endl;
		}
		else
		{
			shadowPtr = std::make_shared<Shadow>(this, devId, macID,ramlJson, devJson, finalDirPath);

			shadowPtr->setConfig(shadowSetStateTimeOut, _maskSCGlo);

			shadowMapObj[devId] = shadowPtr;

			std::cout << "Am I coming here .." << shadowMapObj.size() << std::endl;
		}
	}
	catch(std::exception &e)
	{
		shadowPtr.reset();
		std::cout << "Unable to load persistant json" << std::endl;
	}

}


void shadowController::deviceData(std::string &devId, std::string& data)
{
	auto mapIt = shadowMapObj.find(devId);

	if(mapIt != shadowMapObj.end())
	{
		mapIt->second->DataReceivedThreaded(devId, data);
	}
	else
	{
		SendTelemetryDataCCAPP(devId, data);
	}
}

void shadowController::deregisterShadow(std::string &devId, std::string &payload)
{
	//	auto mapIt = shadowMapObj.find(devId);
	//
	//	if(mapIt != shadowMapObj.end())
	//	{
	//		mapIt->second.reset();
	//	}

	//
	json jsonData = json::parse(payload);

	int size = jsonData["devices"].size();

	for(int i =0; i < size; i++ )
	{
		int responseCode = jsonData["devices"][i]["responsecode"];

		if(responseCode == 200)
		{
			std::string id = jsonData["devices"][i]["id"];
			std::string macid = jsonData["devices"][i]["macid"];

			auto mapIt = shadowMapObj.find(id);

			if(mapIt != shadowMapObj.end())
			{
				mapIt->second.reset();
			}

			fileJson.erase(macid);

			std::string shadowFilePath = std::string(finalDirPath) + "/SC_" + id
					+ ".json";

			std::string deleteShadow = "rm -rf " + shadowFilePath;

			system(deleteShadow.c_str());
		}
	}

	this->writeToFile(filePath, fileJson.dump());

}

void shadowController::setDeviceStatus(std::string& devId, std::string& data)
{

	std::cout << "DATA IS IN SETDEVICESTATUS" << data << std::endl;

	json regJson;
	json jsonData = json::parse(data);
	std::string shadowFilePath = std::string(finalDirPath) + "/SC_" + devId
			+ ".json";

	FileOPS::readFromFile(shadowFilePath, regJson);

	if(jsonData["responsecode"] == 200)
	{
		if(jsonData["status"] == "ACTIVE" || jsonData["status"] == "active")
		{
			regJson["status"] = jsonData["status"];
		}
		else if(jsonData["status"] == "DEACTIVE" || jsonData["status"] == "deactive")
		{
			regJson["status"] = jsonData["status"];
		}

		//		auto objPtr = std::fopen(shadowFilePath.c_str(), "w+");
		//
		//		if(objPtr)
		//		{
		//			std::fwrite(regJson.dump().c_str(), 1, regJson.dump().size(), objPtr);
		//
		//			std::fflush(objPtr);
		//
		//			std::fclose(objPtr);
		//		}

		this->writeToFile(shadowFilePath, regJson.dump());
	}
}

void shadowController::setMultipleStatus(std::string &data)
{
	std::cout << "setMultipleStatus devices payload is" << data << std::endl;

	json regJson;
	json jsonData = json::parse(data);

	int size = jsonData["devices"].size();

	for(int i =0 ;i < size; i++ )
	{
		int responseCode = jsonData["devices"][i]["responsecode"];

		if(responseCode == 200)
		{
			std::string id = jsonData["devices"][i]["id"];

			std::string shadowFilePath = std::string(finalDirPath) + "/SC_" + id
					+ ".json";

			FileOPS::readFromFile(shadowFilePath, regJson);

			if(jsonData["devices"][i]["status"] == "ACTIVE" || jsonData["devices"][i]["status"] == "active")
			{
				regJson["status"] = jsonData["devices"][i]["status"];
			}
			else if(jsonData["devices"][i]["status"] == "DEACTIVE" || jsonData["devices"][i]["status"] == "deactive")
			{
				regJson["status"] = jsonData["devices"][i]["status"];
			}

			//			auto objPtr = std::fopen(shadowFilePath.c_str(), "w+");
			//
			//			if(objPtr)
			//			{
			//				std::fwrite(regJson.dump().c_str(), 1, regJson.dump().size(), objPtr);
			//
			//				std::fflush(objPtr);
			//
			//				std::fclose(objPtr);
			//			}

			this->writeToFile(shadowFilePath, regJson.dump());
		}
	}

}


void shadowController::getDeviceStatus(std::string& devId, std::string& data)
{
	auto mapIt = shadowMapObj.find(devId);

	if(mapIt != shadowMapObj.end())
	{
		try
		{
			//    		mapIt->second->getShadowStatus();
		}
		catch(std::exception &e)
		{
			json jData = json::parse(data);

			jData[valStrings::SHADOW_ERR_STR] = e.what();

			data = jData.dump();
		}
	}
	else
	{
		LOGINFO("Device not present as Shadow");

		json jData = json::parse(data);

		jData[valStrings::RESPONSE_CODE_STR] = 400;
		jData[valStrings::MESSAGE_STR] = "Device not present as Shadow";
		jData[valStrings::STATUS_STR] = "failure";

		data = jData.dump();
	}
}

void shadowController::getDeviceState(std::string& devId, std::string& data)
{
	auto mapIt = shadowMapObj.find(devId);

	std::lock_guard<std::mutex> guardObj(mLockCallGetDeviceState);

	if(mapIt != shadowMapObj.end())
	{
		try
		{
			mapIt->second->GetDeviceState(devId, data);
		}
		catch(std::exception &e)
		{
			json jData = json::parse(data);

			jData[valStrings::SHADOW_ERR_STR] = e.what();

			data = jData.dump();
		}
	}
	else
	{
		LOGINFO("Device not present as Shadow");

		json jData = json::parse(data);

		jData[valStrings::RESPONSE_CODE_STR] = 400;
		jData[valStrings::MESSAGE_STR] = "Device not present as Shadow";
		jData[valStrings::STATUS_STR] = "failure";

		data = jData.dump();
	}
}

bool shadowController::setDeviceState(std::string& devId, std::string& data)
{
	LOGINFO2("printing simulator data ", data);

	json obj = json::parse(data);

	std::lock_guard<std::mutex> guardObj(mLockCallSetDeviceState);

	auto mapIt = shadowMapObj.find(devId);

	if(mapIt != shadowMapObj.end())
	{
		try
		{
			mapIt->second->SetDeviceState(devId, data);
		}
		catch(std::exception &e)
		{
			json jData = json::parse(data);

			jData[valStrings::SHADOW_ERR_STR] = e.what();

			data = jData.dump();
		}
	}
	else
	{
		LOGINFO("Device not present as Shadow");

		json jData = json::parse(data);

		jData[valStrings::RESPONSE_CODE_STR] = 400;
		jData[valStrings::MESSAGE_STR] = "Device not present as Shadow";
		jData[valStrings::STATUS_STR] = "failure";

		data = jData.dump();

		data = jData.dump();
	}
}

void shadowController::DiscoveredDevices(std::string payload)
{
	std::cout<< "filehson " << fileJson.dump() << std::endl;
	std::cout << "Discovered devices payload is" << payload << std::endl;
	json discoverJson = json::parse(payload);

	bool newDev = false;

	int size = discoverJson["devices"].size();

	for(int i =0 ;i < size; i++ )
	{
		std::string macid = discoverJson["devices"][i]["macid"];

		for(json::iterator it = fileJson.begin(); it!= fileJson.end(); it++)
		{
			if(macid == it.key())
			{
				newDev = true;
			}
		}

		if(newDev == false)
		{
			fileJson[macid] = "";
		}

		newDev = false;
	}

	std::cout << "File json" << fileJson.dump() << std::endl;

	//	auto objPtr = std::fopen(filePath.c_str(), "w+");
	//
	//	if(objPtr)
	//	{
	//		std::fwrite(fileJson.dump().c_str(), 1, fileJson.dump().size(), objPtr);
	//
	//		std::fflush(objPtr);
	//
	//		std::fclose(objPtr);
	//	}

	this->writeToFile(filePath, fileJson.dump());
}

void shadowController::registerDevice(std::string payload)
{
	json discoveredJson;

	FileOPS::readFromFile(filePath, fileJson);

	std::cout << "registerDevice devices payload is" << payload << std::endl;
	json registerJson = json::parse(payload);

	int size = registerJson["devices"].size();

	for(int i =0 ;i < size; i++ )
	{
		int responseCode = registerJson["devices"][i]["responsecode"];

		if(responseCode == 200)
		{
			std::string id = registerJson["devices"][i]["id"];
			std::string macid = registerJson["devices"][i]["macid"];

			fileJson[macid] = id;

			//			auto objPtr = std::fopen(filePath.c_str(), "w+");
			//
			//			if(objPtr)
			//			{
			//				std::fwrite(fileJson.dump().c_str(), 1, fileJson.dump().size(), objPtr);
			//
			//				std::fflush(objPtr);
			//
			//				std::fclose(objPtr);
			//			}

			this->writeToFile(filePath, fileJson.dump());

			CreateShadowJson(id);
		}
	}
}


void shadowController::CreateShadowJson(const std::string devId)
{
	std::string shadowFilePath = std::string(finalDirPath) + "/SC_" + devId
			+ ".json";

	json statusJson;

	statusJson["status"] = "register";

	//	auto objPtr = std::fopen(shadowFilePath.c_str(), "w+");
	//
	//	if(objPtr)
	//	{
	//		std::fwrite(statusJson.dump().c_str(), 1, statusJson.dump().size(), objPtr);
	//
	//		std::fflush(objPtr);
	//
	//		std::fclose(objPtr);
	//	}

	this->writeToFile(shadowFilePath, statusJson.dump());
}

void shadowController::sendToCloud(std::string &topic, std::string &payload)
{
	//To protect from multiple shadows accessing SendToCloudData, causing pre-emption or inconsistent view.
	//We are using locks.
	std::lock_guard<std::mutex> guardObj(mLockCallSerProvider);

	if(_ptrServiceProvider);
	_ptrServiceProvider->SendToCloudData(topic, payload);
}

void shadowController::SendTelemetryDataCCAPP(std::string &devId, std::string &payload)
{
	//To protect from multiple shadows accessing SendTelemetryData, causing pre-emption or inconsistent view.
	//We are using locks.

	std::lock_guard<std::mutex> guardObj(mLockCallSerProvider);

	if(_ptrServiceProvider);
	_ptrServiceProvider->SendTelemetryData(devId,payload);
}

void shadowController::callTheInterface(InterfaceName iName, std::string &devId, std::string &payload)
{
	//To protect from multiple shadows accessing SendTelemetryData, causing pre-emption or inconsistent view.
	//We are using locks.

	std::lock_guard<std::mutex> guardObj(mLockCallInterface);

	if(_DevPtr)
		_DevPtr->WrappedCallTransportInterface(iName, devId, payload);
}

void shadowController::createDirectory()
{
	FileOPS::readFromFile(filePath , fileJson);

	finalDirPath = configurationSettings::dmAppConfDirPath + SHADOW_JSON_SUBPART;

	if(!directoryOPS::checkExistence(configurationSettings::dmAppConfDirPath))
	{
		directoryOPS::createPath(finalDirPath, 0755);
	}

	ramlDirPath = configurationSettings::dmAppConfDirPath + SHADOW_RAML_TEMP_PATH;

	directoryOPS::createPath(ramlDirPath, 0755);

	//
	//	if(!directoryOPS::checkExistence(configurationSettings::dmAppConfDirPath))
	//	{
	//		directoryOPS::createPath(ramlDirPath, 0755);
	//	}


}

