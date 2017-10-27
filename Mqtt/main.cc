#include <signal.h>
#include <iostream>
#include <alljoyn/AboutObj.h>
#include "alljoynWrapper.h"
#include "cloudWrapper.h"
#include "mqttBusObject.h"
#include <unistd.h>
#include <stdlib.h>
#include "CommonHeader/debugLog.h"
#include "CommonHeader/CPPHeaders.h"
#include "configuration.h"
#define eI_MODULE "MAIN"
//#include "headers/aboutListener.h"

using namespace ajn;

//static volatile sig_atomic_t s_interrupt = false;
//
//static void CDECL_CALL SigIntHandler(int sig) {
//	QCC_UNUSED(sig);
//
//	s_interrupt = true;
//}

static bool StartMQTT = false;

void signalHandler( int signum )
{
	system("rm -rf /opt/secCer");
	LOGWARN("Got Interrupt Signal !! App will be goes down.. ..");
	LOGWARN2I("Got Signal [Signal Number] ",signum);
	StartMQTT = false;
	exit(signum);
}

/** Main entry point */
int main(int argc, char* argv[])
{
	DebugLog::instance();
	DebugLog::instance()->createLogFile("MQTT");

	/* Handle kernel signal */
	signal(SIGINT, signalHandler);
	signal(SIGKILL, signalHandler);
	signal(SIGTERM, signalHandler);
	StartMQTT = true;

	if(argc == 1)
	{
		LOGINFO("No arguments.You should run this program in terminal with several arguments");
		return 1;
	}

	else if(argc == 2)
	{
		LOGINFO("The arguments are");

		for(int index = 0; index<argc ;index++)
		{
			switch(index)
			{
			case 1:
				configurationManage::AppName = argv[index];
				cout<< configurationManage::AppName <<std::endl;
				break;
			}

			cout<<argv[index]<<endl;
		}
	}

	QStatus status;
	// Initialize Alljoyn Bus
	alljoynWrapper *alljoyn = new alljoynWrapper();
	status = alljoyn->init();
	sleep(2);
	//alljoyn->makeConnection();
	//alljoyn->ConnectMqtt();

	//sleep(20);
	//alljoyn->CancleNameService();
	/* Perform the service asynchronously until the user signals for an exit. */

	if (ER_OK == status) {
		while (StartMQTT)
			usleep(300);
	}
	else
	{
		LOGINFO("Init failure");
		return 0;
	}

	alljoyn->Dinit();
	if(alljoyn)
		delete alljoyn;
	//delete busObject;
	//delete alljoyn;
	return 0;
}
