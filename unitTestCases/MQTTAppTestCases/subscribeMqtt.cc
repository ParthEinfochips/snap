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

struct subscribeMqttTest : commandTest
{
	protected:

	virtual void SetUp()
	{
		std::cout << "IN SUBSCRIBE-MQTT SETUP" << std::endl;
	}

	virtual void TearDown() {

	}

};

//PERFECT PACKAGE.
TEST_F(subscribeMqttTest, subscribeTopic)
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
	
	sleep(2);
	
	std::string jsonobj1 = R"({
								"CLIENTID":"58ff608d6272d308c4929a23",
								"JOBID":"58ff609d70192a4501400366",
								"ch":"9q9gyvaf2oqobvm2kawiu3by3cs0s72r",
								"endpointid":251,
								"topic": "EI/1234"
								})";
	
	EXPECT_EQ(0, mqttPtr->registerTopic(jsonobj1,appName));
	
	sleep(3);

 }

//ENDPOINTID IS NOT PRESENT IN THE PACKAGE.
TEST_F(subscribeMqttTest, NegativeTest1)
{
	std::cout << "TEST CASE 2ND - ENDPOINTID IS NOT PRESENT IN THE PACKAGE." << std::endl;

	std::string jsonobj = R"({
								"CLIENTID":"58ff608d6272d308c4929a23",
								"JOBID":"58ff609d70192a4501400366",
								"ch":"9q9gyvaf2oqobvm2kawiu3by3cs0s72r",
								"topic": "EI/1234"
								})";

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(1, mqttPtr->registerTopic(jsonobj,appName));

}


//TOPIC IS NOT PRESENT IN THE PACKAGE.
TEST_F(subscribeMqttTest, NegativeTest2)
{
	std::cout << "TEST CASE 3RD - TOPIC IS NOT PRESENT IN THE PACKAGE." << std::endl;

	std::string jsonobj = R"({
								"CLIENTID":"58ff608d6272d308c4929a23",
								"JOBID":"58ff609d70192a4501400366",
								"ch":"9q9gyvaf2oqobvm2kawiu3by3cs0s72r",
								"endpointid":251
								})";

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(1, mqttPtr->registerTopic(jsonobj,appName));

}

//TOPIC IS EMPTY
TEST_F(subscribeMqttTest, NegativeTest3)
{
	std::cout << "TEST CASE 4TH - TOPIC IS EMPTY" << std::endl;

	std::string jsonobj = R"({
								"CLIENTID":"58ff608d6272d308c4929a23",
								"JOBID":"58ff609d70192a4501400366",
								"ch":"9q9gyvaf2oqobvm2kawiu3by3cs0s72r",
								"endpointid":251,
								"topic": ""
								})";

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(1, mqttPtr->registerTopic(jsonobj,appName));

}

//INVALID JSON
TEST_F(subscribeMqttTest, NegativeTest4)
{
	std::cout << "TEST CASE 5TH - INVALID JSON" << std::endl;

	std::string jsonobj = R"({
								"CLIENTID":"58ff608d6272d308c4929a23",
								"JOBID":"58ff609d70192a4501400366",
								"ch":"9q9gyvaf2oqobvm
								"endpointid":251,
								"topic": "aqwe"
								})";

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(1, mqttPtr->registerTopic(jsonobj,appName));
}



int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
