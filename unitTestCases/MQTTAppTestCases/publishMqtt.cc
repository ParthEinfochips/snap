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

struct publishMqttTest : commandTest
{
	protected:

	virtual void SetUp()
	{
		std::cout << "IN PUBLISH-MQTT SETUP" << std::endl;
	}

	virtual void TearDown() {

	}

};

//PERFECT PACKAGE.
TEST_F(publishMqttTest, publishTopic)
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
									"JOBID":"598c23461d41c87e5f05eebb",
									"endpointid":251,
									"topic":"EI/ORGID/GWID/TEMP",
									"msg":"{\"JOBID\":\"598c23461d41c87e5f05eebb\",\"Data\":\"hello\"}"
								})";
	
	EXPECT_EQ(0, mqttPtr->sendToCloud(jsonobj1,appName));
	
	sleep(3);

 }
 
 
//ENDPOINTID IS NOT PRESENT IN THE PACKAGE.
TEST_F(publishMqttTest, NegativeTest1)
{
	std::cout << "TEST CASE 2ND - ENDPOINTID IS NOT PRESENT IN THE PACKAGE" << std::endl;

	std::string jsonobj = R"({
									"JOBID":"598c23461d41c87e5f05eebb",
									"topic":"EI/ORGID/GWID/TEMP",
									"msg":"{\"JOBID\":\"598c23461d41c87e5f05eebb\",\"Data\":\"hello\"}"
								})";

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(1, mqttPtr->registerTopic(jsonobj,appName));

}


//TOPIC IS NOT PRESENT IN THE PACKAGE.
TEST_F(publishMqttTest, NegativeTest2)
{
	std::cout << "TEST CASE 3RD - TOPIC IS NOT PRESENT IN THE PACKAGE." << std::endl;

	std::string jsonobj = R"({
									"JOBID":"598c23461d41c87e5f05eebb",
									"endpointid":251,
									"msg":"{\"JOBID\":\"598c23461d41c87e5f05eebb\",\"Data\":\"hello\"}"
								})";

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(1, mqttPtr->registerTopic(jsonobj,appName));

}

//TOPIC IS EMPTY
TEST_F(publishMqttTest, NegativeTest3)
{
	std::cout << "TEST CASE 4TH - TOPIC IS EMPTY" << std::endl;

	std::string jsonobj = R"({
									"JOBID":"598c23461d41c87e5f05eebb",
									"endpointid":251,
									"topic":"",
									"msg":"{\"JOBID\":\"598c23461d41c87e5f05eebb\",\"Data\":\"hello\"}"
								})";

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(1, mqttPtr->registerTopic(jsonobj,appName));

}

//INVALID JSON
TEST_F(publishMqttTest, NegativeTest4)
{
	std::cout << "TEST CASE 5TH - INVALID JSON" << std::endl;

	std::string jsonobj = R"({
									"JOBID":"598c23461d41c87e5f05eebb",
									"endpointid":
									"topic":"EI/ORGID/GWID/TEMP",
									"msg":"{\"JOBID\":\"598c23461d41c87e5f05eebb\",\"Data\":\"hello\"}"
								})";

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(1, mqttPtr->registerTopic(jsonobj,appName));
}
/*
//JOBID IS NOT PRESENT
TEST_F(publishMqttTest, NegativeTest5)
{
	std::cout << "TEST CASE 6TH" << std::endl;

	std::string jsonobj = R"({
									"JOBID":"598c23461d41c87e5f05eebb",
									"endpointid": 251,
									"topic":"EI/ORGID/GWID/TEMP",
									"msg":"{\"Data\":\"hello\"}"
								})";

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(1, mqttPtr->registerTopic(jsonobj,appName));
}

//JOBID IS EMPTY
TEST_F(publishMqttTest, NegativeTest6)
{
	std::cout << "TEST CASE 7TH" << std::endl;

	std::string jsonobj = R"({
									"JOBID":"598c23461d41c87e5f05eebb",
									"endpointid": 251,
									"topic":"EI/ORGID/GWID/TEMP",
									"msg":"{\"JOBID\":\"\",\"Data\":\"hello\"}"
								})";

	std::string appName = "org.alljoyn.EI.GW1234.APPtempApp";

	EXPECT_EQ(1, mqttPtr->registerTopic(jsonobj,appName));
}

*/

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
