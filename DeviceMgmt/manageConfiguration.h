#ifndef MANAGECONFIG_HH
#define MANAGECONFIG_HH

#include "ServiceProvider.h"
#include "CommonHeader/CPPHeaders.h"

class ServiceProvider;

class manageConfiguration{

public:

	manageConfiguration(ServiceProvider *servPtr);

	~manageConfiguration();

	void getConffile(json &jsonObj);

	QStatus updateAppbaseUrl(std::string &payload);

	QStatus testAppbaseUrl(std::string &payload);

	QStatus addCloudConnection(std::string &payload);

	QStatus updateCloudConnection(std::string &payload);

	QStatus deleteCloudConnection(std::string &payload);

	QStatus addLocalConnection(std::string &payload);

	QStatus updateLocalConnection(std::string &payload);

	QStatus deleteLocalConnection(std::string &payload);

	QStatus testCloudConnection(std::string &payload);

	QStatus updateBrokerDetails(json &cloudconnectivityJson);

	QStatus updateLocalBrokerDetails(json &localJson);

	QStatus writeToFile(json &jsonObj);

private:

	json jsonData;
	ServiceProvider *servPtr;
	bool checkConfKey = false;
	bool certRequired =false;


};

#endif //MANAGECONFIG_HH
