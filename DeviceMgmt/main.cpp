#include <memory>
#include <csignal>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include "dmaController.h"
//#include "configSensorManager.h"
#include "CommonHeader/directoryOPS.h"
#include "DeviceObject/DeviceObj.h"
#include "CommonHeader/debugLog.h"
#include "ajNameService/ajNameService.h"

#define eI_MODULE "MAIN"
#define discovery_topic DMA_TOPIC PROTOCOLS_ENTITY_FIRST "/" BLE COMMANDS_GET

static bool StartDMA = false;

void signalHandler( int signum )
{
	LOGWARN("Got Interrupt Signal !! App will be goes down.. ..");
	LOGWARN2I("Got Signal [Signal Number] ",signum);
	StartDMA = false;
	exit(signum);
}

int main(int argc,char *argv[])
{
	/*Initialize Debug log*/
	DebugLog::instance();
	DebugLog::instance()->createLogFile("DMA");

	/* Handle kernel signal */
	signal(SIGINT, signalHandler);
	signal(SIGKILL, signalHandler);
	signal(SIGTERM, signalHandler);

	if(argc == 1)
	{
		LOGEROR("No arguments.You should run this program in terminal with App name as arguments.");
		exit(1);
	}
	else
	{
		/*for loop,each loop print a argument once a time. Note that the loop begin with argv[1],
	        because argv[0] represent the program's name.*/
		if((argc > 2) || (argc == 2))
		{
			LOGDBUG2I("The total number of argument",argc);
			configurationSettings::AppTopic = argv[1];
			LOGINFO2("AppTopic",configurationSettings::AppTopic);
		} else {
			LOGWARN("Something Went Wrong !!");
		}
	}

	/*enable start process.*/
	int retStatus;
	StartDMA = true;

	/*Initialize DMA Controller will initialize all remaining services.*/
	std::shared_ptr<DMAController> dmaPtr(new DMAController());
	retStatus = dmaPtr->Init();
	LOGINFO("Start Application..");
	/*Test Simple */
	//sleep(2);
	//	dmaPtr->FunctionTest();
	while(StartDMA && (ER_OK == retStatus ))
		usleep(400);
	dmaPtr->DInit();
	LOGWARN("DeInit all working process .. ..DOWN");
	return retStatus;
}

