#include "manageConfiguration.h"

manageConfiguration::manageConfiguration(ServiceProvider *servPtr):servPtr(servPtr)
{

	std::cout << "JSNDATA DUMP " << configurationSettings::connectivityJson << std::endl;
	jsonData = configurationSettings::connectivityJson;
}


manageConfiguration::~manageConfiguration()
{


}

void manageConfiguration::getConffile(json &jsonObj){

	jsonData = jsonObj;
}

QStatus manageConfiguration::updateBrokerDetails(json &cloudJson)
{
	json cloudConnectivityJson = cloudJson[APPCONFIG][CLOUDBROKERS];

	QStatus status;
	int size = cloudConnectivityJson.size();
	int x = 0;

	brokerDetails brokInfo;

	configurationSettings::brokerMap = {};

	for(int i = 0 ; i < size ; i++)
	{
		std::string connectID = cloudConnectivityJson[i][CONNECTID];
		brokInfo.mqqthostname= cloudConnectivityJson[i][HOST];
		brokInfo.mqqtpassword = cloudConnectivityJson[i][PASS];
		brokInfo.mqqtusername = cloudConnectivityJson[i][USER];
		//brokInfo.orgID = cloudConnectivityJson[i][JKF_ORGID];
		brokInfo.mqqtport = cloudConnectivityJson[i][PORT];
		brokInfo.type = cloudConnectivityJson[i][BROKERTYPE];
		brokInfo.brokerNum = ++x;
		brokInfo.totalBroker = size;

		if(certRequired && cloudConnectivityJson[i][JKF_MQTTPORT] != 1883)
		{
			brokInfo.cafile = cloudConnectivityJson[i][JKF_MQTTCAFILE];
			brokInfo.certfile = cloudConnectivityJson[i][JKF_MQTTCERTFILE];
			brokInfo.keyfile = cloudConnectivityJson[i][JKF_MQTTKEYFILE];
			brokInfo.version = "tlsv1.2";
			brokInfo.cipher = "AES256-SHA";
		}

		configurationSettings::brokerMap[connectID] = brokInfo;
	}

	status = ER_OK;

}

QStatus manageConfiguration::updateLocalBrokerDetails(json &localJson)
{
	json localConnectivityJson = localJson[APPCONFIG][LOCALBROKERS];

	std::cout << "LOCALBROKERS IS" << localConnectivityJson.dump() << std::endl;

	QStatus status;
	int size = localConnectivityJson.size();
	int x = 0;

	brokerDetails brokInfo;

	configurationSettings::localBrokerMap = {};

	for(int i = 0 ; i < size ; i++)
	{
		std::string connectID = localConnectivityJson[i][CONNECTID];
		brokInfo.mqqthostname= localConnectivityJson[i][HOST];
		brokInfo.mqqtpassword = localConnectivityJson[i][PASS];
		brokInfo.mqqtusername = localConnectivityJson[i][USER];
		//brokInfo.orgID = localConnectivityJson[i][JKF_ORGID];
		brokInfo.mqqtport = localConnectivityJson[i][PORT];
		brokInfo.type = localConnectivityJson[i][BROKERTYPE];
		brokInfo.brokerNum = ++x;
		brokInfo.totalBroker = size;

		if(certRequired && localConnectivityJson[i][JKF_MQTTPORT] != 1883)
		{
			brokInfo.cafile = localConnectivityJson[i][JKF_MQTTCAFILE];
			brokInfo.certfile = localConnectivityJson[i][JKF_MQTTCERTFILE];
			brokInfo.keyfile = localConnectivityJson[i][JKF_MQTTKEYFILE];
			brokInfo.version = "tlsv1.2";
			brokInfo.cipher = "AES256-SHA";
		}

		configurationSettings::localBrokerMap[connectID] = brokInfo;
	}

	status = ER_OK;

}




QStatus manageConfiguration::writeToFile(json &jsonObj)
{
	std::fstream FileStrm;

	std::string fileMqttPath = MQTT_CONFIG_FILE_PATH + configurationSettings::AppTopic + "/" + configurationSettings::AppTopic + MQTT_CONFIG_FILE_PRIFIX + CONFIG_FILE_PRIFIX;;

	FileStrm.open(fileMqttPath,  std::fstream::out | std::fstream::trunc);

	if(FileStrm.is_open())
	{
		FileStrm << jsonObj.dump() << std::endl;

		FileStrm.close();

		return ER_OK;
	}
	else
	{
		return ER_FAIL;
	}

}

QStatus manageConfiguration::updateAppbaseUrl(std::string &payload)
{
	QStatus status;
	json jsonObj;

	try{

		status = this->testAppbaseUrl(payload);
		std::cout << "testAppBaseURL" << status << std::endl;
		if(status == ER_OK)
		{
			std::string tempStr;

			jsonObj = json::parse(payload);

			std::string appUrl = jsonObj[DATA][BASEURL];
			configurationSettings::appBaseUrl = appUrl;

			jsonData[APPCONFIG][APPBASEURL] = appUrl;


			//			for(keyVec &keyVecObj : configurationSettings::configKeyVec)
			//			{
			//				std::string replaceStr = "$" + keyVecObj.key + "$";
			//				std::size_t repStr = appUrl.find(replaceStr);
			//
			//				appUrl.replace(repStr,replaceStr.size(),keyVecObj.value);
			//			}

			this->writeToFile(jsonData);

			std::string topicToSubscribe = appUrl + "/" + configurationSettings::orgId + "/" + configurationSettings::gatewayId + "/" + configurationSettings::AppTopic + "/#";
			status = servPtr->subscribeConfTopic(topicToSubscribe);

			if(status == ER_OK)
			{
				jsonObj["msg"] = "success";
				jsonObj["responsecode"] = 200;
				jsonObj["status"] = "success";
				payload = jsonObj.dump();
				return ER_OK;
			}
			else
			{
				jsonObj["msg"] = "unable to update topic";
				jsonObj["responsecode"] = 400;
				jsonObj["status"] = "failure";
				payload = jsonObj.dump();
				return ER_FAIL;
			}

		}
		else{
			jsonObj = json::parse(payload);
			jsonObj["msg"] = "unable to update url";
			jsonObj["responsecode"] = 400;
			jsonObj["status"] = "failure";
			payload = jsonObj.dump();
			return ER_FAIL;
		}
	}
	catch(...)
	{
		jsonObj = json::parse(payload);
		jsonObj["msg"] = "unable to update url";
		jsonObj["responsecode"] = 400;
		jsonObj["status"] = "failure";
		payload = jsonObj.dump();
		return ER_FAIL;
	}


}

QStatus manageConfiguration::testAppbaseUrl(std::string &payload)
{

	std::string str1;
	std::string str2;

	QStatus status;
	keyVec keyVectObj;
	bool entryVecBool = true;
	//configurationSettings::configKeyVec = {};

	try
	{
		std::string tempStr;

		json jsonObj;
		jsonObj = json::parse(payload);

		std::string appUrl = jsonObj[DATA][BASEURL];

		if(appUrl == "")
		{
			jsonObj["msg"] = "can't find the key";
			jsonObj["responsecode"] = 400;
			jsonObj["status"] = "failure";
			payload = jsonObj.dump();
			return ER_FAIL;

		}


		//		std::size_t found = appUrl.find_first_of("/");
		//		while(found!=std::string::npos)
		//		{
		//			appUrl = appUrl.substr(found+1);
		//			break;
		//		}
		//		std::size_t found1 = appUrl.find_first_of("$");
		//		while(found1!=std::string::npos)
		//		{
		//			appUrl = appUrl.substr(found1+1);
		//			std::size_t found1 = appUrl.find_first_of("$");
		//			str1 = appUrl.substr(0, found1);
		//
		//			try{
		//				//				if(str1 == "appid")
		//				//				{
		//				//					str2 = configurationSettings::AppTopic;
		//				//					keyVectObj.key = str1;
		//				//					keyVectObj.value = str2;
		//				//					configurationSettings::configKeyVec.push_back(keyVectObj);
		//				//				}
		//				//				else
		//				//				{
		//				str2 = jsonData[APPCONFIG][str1];
		//
		//				keyVectObj.key = str1;
		//				keyVectObj.value = str2;
		//
		//				for(keyVec &keyVecObj : configurationSettings::configKeyVec)
		//				{
		//					if(str1 == keyVecObj.key)
		//					{
		//						entryVecBool = false;
		//						break;
		//					}
		//				}
		//				if(entryVecBool == true)
		//				{
		//					configurationSettings::configKeyVec.push_back(keyVectObj);
		//				}
		//
		//				//				}
		//				entryVecBool = true;
		//
		//			}
		//			catch(...)
		//			{
		//				jsonObj["msg"] = "can't find the key";
		//				jsonObj["responsecode"] = 400;
		//				jsonObj["status"] = "failure";
		//				payload = jsonObj.dump();
		//				return ER_FAIL;
		//
		//			}
		//
		//			std::size_t found2 = appUrl.find_first_of("/");
		//			if(found2!=std::string::npos)
		//			{
		//				appUrl = appUrl.substr(found1+2);
		//			}
		//			else
		//			{
		//				break;
		//			}
		//		}

		jsonObj["msg"] = "success";
		jsonObj["responsecode"] = 200;
		jsonObj["status"] = "success";
		payload = jsonObj.dump();

		return ER_OK;
	}
	catch(...)
	{
		std::cout << "In exception" << std::endl;
	}

}

QStatus manageConfiguration::addCloudConnection(std::string &payload)
{
	std::cout << "payload" << payload << std::endl;

	QStatus status;
	std::string tempStr;

	json jsonObj;

	try{

		jsonObj = json::parse(payload);

		std::string newCloudConnection = jsonObj[DATA][CLOUDBROKER].dump();

		jsonData[APPCONFIG][CLOUDBROKERS].push_back(json::parse(newCloudConnection));

		this->updateBrokerDetails(jsonData);

		status = this->writeToFile(jsonData);

		jsonObj["msg"] = "success";
		jsonObj["responsecode"] = 200;
		jsonObj["status"] = "success";
		payload = jsonObj.dump();

		return ER_OK;
	}
	catch(...)
	{
		jsonObj["msg"] = "failure";
		jsonObj["responsecode"] = 400;
		jsonObj["status"] = "failure";
		payload = jsonObj.dump();

		return ER_FAIL;
	}

}

QStatus manageConfiguration::addLocalConnection(std::string &payload)
{
	std::cout << "payload" << payload << std::endl;

	QStatus status;
	std::string tempStr;

	json jsonObj;

	try{

		jsonObj = json::parse(payload);

		std::string newCloudConnection = jsonObj[DATA][LOCALBROKER].dump();
		std::cout << "newlocslconne" << newCloudConnection << std::endl;

		jsonData[APPCONFIG][LOCALBROKERS].push_back(json::parse(newCloudConnection));

		this->updateLocalBrokerDetails(jsonData);

		status = this->writeToFile(jsonData);

		jsonObj["msg"] = "success";
		jsonObj["responsecode"] = 200;
		jsonObj["status"] = "success";
		payload = jsonObj.dump();

		return ER_OK;
	}
	catch(...)
	{
		jsonObj["msg"] = "failure";
		jsonObj["responsecode"] = 400;
		jsonObj["status"] = "failure";
		payload = jsonObj.dump();

		return ER_FAIL;
	}

}


QStatus manageConfiguration::updateCloudConnection(std::string &payload)
{
	std::cout << "payload" << payload << std::endl;
	QStatus status;
	bool tempbool = false;

	std::string tempStr;

	json jsonUpdate;
	jsonUpdate = json::parse(payload);
	try{
		std::string newCloudConnection = jsonUpdate[DATA][CLOUDBROKER].dump();


		std::string connectID = jsonUpdate[DATA][CLOUDBROKER][CONNECTID];

		std::cout << "connectID" << connectID << std::endl;
		json tempJson;

		int size = jsonData[APPCONFIG][CLOUDBROKERS].size();

		for(int i =0; i < size ; i++ )
		{
			std::string deleteStr = jsonData[APPCONFIG][CLOUDBROKERS][i][CONNECTID];

			std::cout << "deletestr" << deleteStr << std::endl;
			if(connectID != deleteStr)
			{
				std::string saveData = jsonData[APPCONFIG][CLOUDBROKERS][i].dump();

				tempJson[i] = json::parse(saveData);
			}
			else
			{
				tempbool = true;
			}

		}
		tempJson.push_back(json::parse(newCloudConnection));

		jsonData[APPCONFIG][CLOUDBROKERS] = tempJson;

		this->updateBrokerDetails(jsonData);

		status = this->writeToFile(jsonData);

		if(tempbool == false)
		{
			jsonUpdate["msg"] = "unable to update";
			jsonUpdate["responsecode"] = 400;
			jsonUpdate["status"] = "failure";
			payload = jsonUpdate.dump();

			return ER_FAIL;

		}

		jsonUpdate["msg"] = "success";
		jsonUpdate["responsecode"] = 200;
		jsonUpdate["status"] = "success";
		payload = jsonUpdate.dump();

		return ER_OK;
	}
	catch(...)
	{
		jsonUpdate["msg"] = "unable to update";
		jsonUpdate["responsecode"] = 400;
		jsonUpdate["status"] = "failure";
		payload = jsonUpdate.dump();

		return ER_FAIL;
	}

}

QStatus manageConfiguration::updateLocalConnection(std::string &payload)
{
	std::cout << "payload" << payload << std::endl;
	QStatus status;
	bool tempbool = false;

	std::string tempStr;

	json jsonUpdate;
	jsonUpdate = json::parse(payload);
	try{
		std::string newCloudConnection = jsonUpdate[DATA][LOCALBROKER].dump();


		std::string connectID = jsonUpdate[DATA][LOCALBROKER][CONNECTID];

		std::cout << "connectID" << connectID << std::endl;
		json tempJson;

		int size = jsonData[APPCONFIG][LOCALBROKERS].size();

		for(int i =0; i < size ; i++ )
		{
			std::string deleteStr = jsonData[APPCONFIG][LOCALBROKERS][i][CONNECTID];

			std::cout << "deletestr" << deleteStr << std::endl;
			if(connectID != deleteStr)
			{
				std::string saveData = jsonData[APPCONFIG][LOCALBROKERS][i].dump();

				tempJson[i] = json::parse(saveData);
			}
			else
			{
				tempbool = true;
			}

		}
		tempJson.push_back(json::parse(newCloudConnection));

		jsonData[APPCONFIG][LOCALBROKERS] = tempJson;

		this->updateLocalBrokerDetails(jsonData);

		status = this->writeToFile(jsonData);

		if(tempbool == false)
		{
			jsonUpdate["msg"] = "unable to update";
			jsonUpdate["responsecode"] = 400;
			jsonUpdate["status"] = "failure";
			payload = jsonUpdate.dump();

			return ER_FAIL;

		}

		jsonUpdate["msg"] = "success";
		jsonUpdate["responsecode"] = 200;
		jsonUpdate["status"] = "success";
		payload = jsonUpdate.dump();

		return ER_OK;
	}
	catch(...)
	{
		jsonUpdate["msg"] = "unable to update";
		jsonUpdate["responsecode"] = 400;
		jsonUpdate["status"] = "failure";
		payload = jsonUpdate.dump();

		return ER_FAIL;
	}

}


QStatus manageConfiguration::deleteCloudConnection(std::string &payload)
{
	std::cout << "payload" << payload << std::endl;
	QStatus status;
	bool tempBool = false;

	std::string tempStr;
	json deleteJson;
	try
	{
		deleteJson = json::parse(payload);

		std::string newCloudConnection = deleteJson[DATA][CLOUDBROKER].dump();

		std::string connectID = deleteJson[DATA][CLOUDBROKER][CONNECTID];

		std::cout << "CONNECTID" << connectID << std::endl;

		json tempJson;

		int size = jsonData[APPCONFIG][CLOUDBROKERS].size();
		std::cout << "size is" << size << std::endl;
		for(int i =0; i < size ; i++ )
		{
			std::string deleteStr = jsonData[APPCONFIG][CLOUDBROKERS][i][CONNECTID];
			std::cout << "deletestr" << deleteStr << std::endl;

			if(connectID != deleteStr)
			{
				std::string saveData = jsonData[APPCONFIG][CLOUDBROKERS][i].dump();

				tempJson[i] = json::parse(saveData);
			}
			else
			{
				tempBool = true;
			}
		}

		jsonData[APPCONFIG][CLOUDBROKERS] = tempJson;

		this->updateBrokerDetails(jsonData);

		status = this->writeToFile(jsonData);


		if(tempBool == false)
		{
			deleteJson["msg"] = "unable to delete";
			deleteJson["responsecode"] = 400;
			deleteJson["status"] = "failure";
			payload = deleteJson.dump();

			return ER_OK;

		}

		deleteJson["msg"] = "success";
		deleteJson["responsecode"] = 200;
		deleteJson["status"] = "success";
		payload = deleteJson.dump();

		return ER_OK;
	}
	catch(...)
	{
		deleteJson["msg"] = "unable to delete";
		deleteJson["responsecode"] = 400;
		deleteJson["status"] = "failure";
		payload = deleteJson.dump();

		return ER_FAIL;
	}

}

QStatus manageConfiguration::deleteLocalConnection(std::string &payload)
{
	std::cout << "payload" << payload << std::endl;
	QStatus status;
	bool tempBool = false;

	std::string tempStr;
	json deleteJson;
	try
	{
		deleteJson = json::parse(payload);

		std::string newCloudConnection = deleteJson[DATA][LOCALBROKER].dump();

		std::string connectID = deleteJson[DATA][LOCALBROKER][CONNECTID];

		std::cout << "CONNECTID" << connectID << std::endl;

		json tempJson;

		int size = jsonData[APPCONFIG][LOCALBROKERS].size();

		for(int i =0; i < size ; i++ )
		{
			std::string deleteStr = jsonData[APPCONFIG][LOCALBROKERS][i][CONNECTID];
			std::cout << "deletestr" << deleteStr << std::endl;

			if(connectID != deleteStr)
			{
				std::string saveData = jsonData[APPCONFIG][LOCALBROKERS][i].dump();

				tempJson[i] = json::parse(saveData);
			}
			else
			{
				tempBool = true;
			}
		}

		jsonData[APPCONFIG][LOCALBROKERS] = tempJson;

		this->updateLocalBrokerDetails(jsonData);

		status = this->writeToFile(jsonData);


		if(tempBool == false)
		{
			deleteJson["msg"] = "unable to delete";
			deleteJson["responsecode"] = 400;
			deleteJson["status"] = "failure";
			payload = deleteJson.dump();

			return ER_OK;

		}

		deleteJson["msg"] = "success";
		deleteJson["responsecode"] = 200;
		deleteJson["status"] = "success";
		payload = deleteJson.dump();

		return ER_OK;
	}
	catch(...)
	{
		deleteJson["msg"] = "unable to delete";
		deleteJson["responsecode"] = 400;
		deleteJson["status"] = "failure";
		payload = deleteJson.dump();

		return ER_FAIL;
	}

}

QStatus manageConfiguration::testCloudConnection(std::string &payload)
{
	std::string tempStr;
	QStatus status;

	json jsonObj;
	jsonObj = json::parse(payload);

	try{
		status = servPtr->testConnection(jsonObj);

		payload = jsonObj.dump();
		return status;

	}
	catch(...)
	{
		jsonObj["msg"] = "unable to connect";
		jsonObj["responsecode"] = 400;
		jsonObj["status"] = "failures";
		jsonObj = jsonObj.dump();

		return ER_FAIL;
	}

}
