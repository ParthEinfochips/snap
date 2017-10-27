#ifndef DMA_CONTROLLER_H
#define DMA_CONTROLLER_H

/*
 *@brief:  This initializes request manager, sensor matrix/configuration, classes responsible for
 * alljoyn bus attachment, Device Proxy Object, Cloud Proxy Object.
 */

#include <app/AlljoynAppManager.h>
#include "proxy/deviceProxy.h"
#include "proxy/cloudProxy.h"
#include "ajcommon/AJInitializer.h"
#include "CommonHeader/CPPHeaders.h"
#include "ajcommon/AJCommon.h"
#include "ajNameService/ajNameService.h"
#include "ServiceProvider.h"


class DMAController {

public:

	/*
	 *@brief: Constructor
	 */

	DMAController();


	/*
	 *@brief: Destructor
	 */

	~DMAController();


	/*
	 *@brief: This function is responsible for initializing everything in DMA.
	 */

	QStatus Init();

	/*
	 *@brief: This function is responsible for initializing everything in DMA.
	 */

	void DInit();

	/*
	 *@brief: This function is responsible for provide BusAttachment object.
	 */

	ajn::BusAttachment *getBusAttachmentPtr() const;

	/*
	 *@brief: Initialize name service.
	 */

	QStatus InitNS();

	/*
	 *@brief: Get Configuration from file.
	 */

	QStatus GetConfigurationFromConfigFile();

	/*
	 *@brief: Test Function for testing other functions.
	 */

	void FunctionTest();

private:
	std::unique_ptr<AJInitializer> initUniqPtr = nullptr;

	std::unique_ptr<AJCommon> ajCommonPtr = nullptr;

	ajn::BusAttachment *busAttachPtr = nullptr;

	ajNameService *nameService = NULL;

	ServiceProvider *servProPtr = NULL;

	std::string appName;

	json jsonData;
	std::string prefixTopic;
	std::string telemetryPrefix;

	bool certRequired = true;

	TransportMask transportMast;
};

#endif //DMA_CONTROLLER_H
