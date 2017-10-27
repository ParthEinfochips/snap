#ifndef CONFIGURATIONS_HEADERS_H_
#define CONFIGURATIONS_HEADERS_H_


#include "jsonCheck.hpp"

class configurationManage
{
public:

	static std::string gatewayId;

	static std::string appBaseUrl;
	static std::string subTopicPrefix;

	static std::string AppName;

	static json configFunJson;

	static std::map<std::string, std::string> appFunMap;
};

#endif
