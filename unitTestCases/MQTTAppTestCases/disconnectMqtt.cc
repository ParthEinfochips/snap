#include "gtest/gtest.h"
#include "../../Mqtt/mqttBusObject.h"
#include "../../Mqtt/Status.h"
#include "memory"


struct commandTest : public ::testing::Test
{
	protected:

	virtual void SetUp()
	{

	}
	//ajNameService *ajPtr = NULL;
	std::string objname = "";
	std::string objpath = "";
	std::string gwid = "";
	mqttBusObject *mqttPtr = new mqttBusObject(objname.c_str(),objpath.c_str(),gwid);
};

struct disconnectMqttTest : commandTest
{
	protected:

	virtual void SetUp()
	{
		std::cout << "IN DISCONNECTMQTT SETUP" << std::endl;
	}

	virtual void TearDown() {

	}

};

//PERFECT PACKAGE.
TEST_F(disconnectMqttTest, disconnect)
{

	std::cout << "TEST CASE 1ST - PERFECT PACKAGE." << std::endl;

	std::string jsonobj = R"({
								"CLIENTID":"58ff608d6272d308c4929a23",
								"JOBID":"58ff609d70192a4501400366",
								"ch":"9q9gyvaf2oqobvm2kawiu3by3cs0s72r",
								"appname":"org.alljoyn.EI.GW1234.APPtempApp",
								"clientid":"org.alljoyn.EI.GW1234.APPtempApp",
								"username":"iotuser",
								"password":"ei12@",
								"hostname":"10.100.23.87",
								"port": 1883
								})";
								

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(0, mqttPtr->establishConnection(jsonobj,appName));
	
	sleep(3);
	
	std::string jsonobj1 = R"({
								"CLIENTID":"58ff608d6272d308c4929a23",
								"JOBID":"58ff609d70192a4501400366",
								"ch":"9q9gyvaf2oqobvm2kawiu3by3cs0s72r",
								"appname":"org.alljoyn.EI.GW1234.APPtempApp",
								"endpointid":251
								})";
								
    EXPECT_EQ(0, mqttPtr->disconnect(jsonobj,appName));
    
    sleep(3);
 }
 
 

//ENDPOINT IS NOT PRESENT IN THE PACKAGE.
TEST_F(disconnectMqttTest, NegativeTest1)
{
	std::cout << "TEST CASE 2ND - ENDPOINT IS NOT PRESENT IN THE PACKAGE." << std::endl;

	std::string jsonobj = R"({
								"CLIENTID":"58ff608d6272d308c4929a23",
								"JOBID":"58ff609d70192a4501400366",
								"ch":"9q9gyvaf2oqobvm2kawiu3by3cs0s72r",
								"appname":"org.alljoyn.EI.GW1234.APPtempApp"
								})";

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(1, mqttPtr->disconnect(jsonobj,appName));

}

//invalid json
TEST_F(disconnectMqttTest, NegativeTest4)
{
	std::cout << "TEST CASE 3RD - INVALID JSON" << std::endl;

	std::string jsonobj = R"({
								"CLIENTID":"58ff608d6272d308c4929a23",
								"JOBID":"58ff609d70192a4501400366",
								"ch":"9q9gyvaf2oqobvm2kawiu3by3cs0s72r",
								"appname":"org.alljoyn.EI.GW1234.APPtempApp",
								})";

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(1, mqttPtr->disconnect(jsonobj,appName));

}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
