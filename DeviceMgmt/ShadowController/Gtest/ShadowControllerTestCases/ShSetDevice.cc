#include "gtest/gtest.h"
#include "../../src/shadow.h"
#include "../../src/shadowcontroller.h"
#include <memory>
/*!
 * This test case is written for testing the "setDeviceState"
 * function of shadow class
*/
struct shadowControllerTest : public ::testing::Test
{
	protected:

	void readFromFile(json &objJson)
    {
    	// std::string objJsonTemp;

        auto retVal = FileOPS::readFromFile("/tmp/json.example", objJson);

        if(retVal < 0)
        {
            std::cout << "Couldn't read raml file" << std::endl;
        }

        // objJson["raml"] = objJsonTemp;
    }

	virtual void SetUp()
	{

        readFromFile(ramlObj);
        std::cout << "raml ========= ========== ======= " << ramlObj.dump() << std::endl;
	}

	std::shared_ptr<Shadow> shadowPtr = nullptr;

	std::shared_ptr<shadowController> SCPtr = nullptr;
	std::string devId = "1234";
	std::string finalDirTempPath = "/tmp";

	json ramlObj;

};

struct setDeviceStateTest: shadowControllerTest
{
	protected:

	shadowController *ptr = nullptr;

	virtual void SetUp()
	{

		try
		{
			shadowControllerTest::SetUp();
			system("sudo rm /tmp/raml_1234.raml");

			ptr = new shadowController;
			shadowPtr = std::make_shared<Shadow> (ptr, devId, ramlObj ,finalDirTempPath);
			// ramlObj.clear();
		}
		catch(std::exception &e)
		{
			std::cout << "Error is " << e.what();
		}
	}

	virtual void TearDown() {
		std::cout << "tearing down" << std::endl;

		sleep(3);

		shadowPtr.reset();

		delete ptr;
	}
};

//!PERFECT PACKAGE
TEST_F(setDeviceStateTest, SetDeviceStatePositive1)
{
	std::cout << "TEST CASE 1ST  === sh set device" << std::endl;

	bool SUCCESS = true;

	try
	{
		std::string data = R"({
		  "protocol": "ZWAVE",
		  "gwid": "598070b07123a60532af7944",
		  "CLIENTID": "598070b07123a60532af7944",
		  "JOBID": "598300351d41c82d23bbe59e",
		  "orgid": "58c3ce458441383a938c5c75",
		  "deviceId": "1234",
		  "appid": "zwaveApp_1",
		  "rid": "598300351d41c82d23bbe59e",
		  "properties": [
			{
			  "Level": 90
			}
		  ]
		})";
	  SUCCESS = shadowPtr->SetDeviceState(devId,data);
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;

		SUCCESS = false;
	}

	EXPECT_EQ(true, SUCCESS);

}

//! Properties missing in package
TEST_F(setDeviceStateTest, SetDeviceStateNeg1)
{
	std::cout << "TEST CASE 2nd  ===  sh set device" << std::endl;

	bool SUCCESS = true;

	try
	{
		std::string data = R"({
		  "protocol": "ZWAVE",
		  "gwid": "598070b07123a60532af7944",
		  "CLIENTID": "598070b07123a60532af7944",
		  "JOBID": "598300351d41c82d23bbe59e",
		  "orgid": "58c3ce458441383a938c5c75",
		  "deviceId": "1234",
		  "appid": "zwaveApp_1",
		  "rid": "598300351d41c82d23bbe59e"

		})";
		SUCCESS = shadowPtr->SetDeviceState(devId, data);
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
	}

	EXPECT_EQ(false, SUCCESS);
}

//! To check if Properties is missing in package than STATUS_STR is failure or not
TEST_F(setDeviceStateTest, SetDeviceStateNeg2)
{
	std::cout << "TEST CASE 3rd  === sh  set device" << std::endl;

	bool SUCCESS = true;
	bool SUCCESS1 = true;

	try
	{
		std::string data = R"({
		  "protocol": "ZWAVE",
		  "gwid": "598070b07123a60532af7944",
		  "CLIENTID": "598070b07123a60532af7944",
		  "JOBID": "598300351d41c82d23bbe59e",
		  "orgid": "58c3ce458441383a938c5c75",
		  "deviceId": "1234",
		  "appid": "zwaveApp_1",
		  "rid": "598300351d41c82d23bbe59e"

		})";
	  SUCCESS = shadowPtr->SetDeviceState(devId,data);
	  json objTemp = nlohmann::json::parse(data);
	  if (objTemp[valStrings::STATUS_STR] == "failure")
			SUCCESS1 = false;
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
	}

	// EXPECT_EQ(false, SUCCESS);
	EXPECT_EQ(false, SUCCESS1);

}

//! To check if Properties is missing in package than STATUS_STR is "failure" or not
TEST_F(setDeviceStateTest, SetDeviceStateNeg3)
{
	std::cout << "TEST CASE 4th  === sh  set device" << std::endl;

	bool SUCCESS = true;
	bool SUCCESS1 = true;

	try
	{
		std::string data = R"({
		  "protocol": "ZWAVE",
		  "gwid": "598070b07123a60532af7944",
		  "CLIENTID": "598070b07123a60532af7944",
		  "JOBID": "598300351d41c82d23bbe59e",
		  "orgid": "58c3ce458441383a938c5c75",
		  "deviceId": "1234",
		  "appid": "zwaveApp_1",
		  "rid": "598300351d41c82d23bbe59e"

		})";
	  SUCCESS = shadowPtr->SetDeviceState(devId,data);
	  json objTemp = nlohmann::json::parse(data);
	  if (objTemp[valStrings::STATUS_STR] == "failure")
			SUCCESS1 = false;
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
	}

	// EXPECT_EQ(false, SUCCESS);
	EXPECT_EQ(false, SUCCESS1);

}

//! To check if Properties is missing in package than RESPONSE_CODE_STR is 400 or not
TEST_F(setDeviceStateTest, SetDeviceStateNeg4)
{
	std::cout << "TEST CASE 5th  === sh  set device" << std::endl;

	bool SUCCESS = true;
	bool SUCCESS1 = true;

	try
	{
		std::string data = R"({
				  "protocol": "ZWAVE",
				  "gwid": "598070b07123a60532af7944",
				  "CLIENTID": "598070b07123a60532af7944",
				  "JOBID": "598300351d41c82d23bbe59e",
				  "orgid": "58c3ce458441383a938c5c75",
				  "deviceId": "1234",
				  "appid": "zwaveApp_1",
				  "rid": "598300351d41c82d23bbe59e"
				})";
	  SUCCESS = shadowPtr->SetDeviceState(devId,data);
	  json objTemp = nlohmann::json::parse(data);
	  if (objTemp[valStrings::RESPONSE_CODE_STR] == 400)
			SUCCESS1 = false;
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
	}

	EXPECT_EQ(false, SUCCESS);
	EXPECT_EQ(false, SUCCESS1);

}

//! To check if package is perfect than RESPONSE_CODE_STR is 200 or not
TEST_F(setDeviceStateTest, SetDeviceStatePos2)
{
	std::cout << "TEST CASE 6th  === sh  set device" << std::endl;

	bool SUCCESS = true;
	bool SUCCESS1 = false;

	try
	{
		std::string data = R"({
				  "protocol": "ZWAVE",
				  "gwid": "598070b07123a60532af7944",
				  "CLIENTID": "598070b07123a60532af7944",
				  "JOBID": "598300351d41c82d23bbe59e",
				  "orgid": "58c3ce458441383a938c5c75",
				  "deviceId": "1234",
				  "appid": "zwaveApp_1",
				  "rid": "598300351d41c82d23bbe59e",
				  "properties": [
					{
					  "Level": 90
					}
				  ]
				})";
	  SUCCESS = shadowPtr->SetDeviceState(devId,data);
	  json objTemp = nlohmann::json::parse(data);
	  if (objTemp[valStrings::RESPONSE_CODE_STR] == 200)
			SUCCESS1 = true;
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
	}

	EXPECT_EQ(true, SUCCESS);
	EXPECT_EQ(true, SUCCESS1);

}

//! To check if package is perfect than JOB_STATUS_VAL_STR is "JOB_QUEUED" or not
TEST_F(setDeviceStateTest, SetDeviceStatePos3)
{
	std::cout << "TEST CASE 7th  === sh set device" << std::endl;

	bool SUCCESS = true;
	bool SUCCESS1 = false;

	try
	{
		std::string data = R"({
				  "protocol": "ZWAVE",
				  "gwid": "598070b07123a60532af7944",
				  "CLIENTID": "598070b07123a60532af7944",
				  "JOBID": "598300351d41c82d23bbe59e",
				  "orgid": "58c3ce458441383a938c5c75",
				  "deviceId": "1234",
				  "appid": "zwaveApp_1",
				  "rid": "598300351d41c82d23bbe59e",
				  "properties": [
					{
					  "Level": 90
					}
				  ]
				})";
	  SUCCESS = shadowPtr->SetDeviceState(devId,data);
	  json objTemp = nlohmann::json::parse(data);
	  if (objTemp[valStrings::JOB_STATUS_VAL_STR] == "JOB_QUEUED")
			SUCCESS1 = true;
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
	}

	EXPECT_EQ(true, SUCCESS);
	EXPECT_EQ(true, SUCCESS1);

}

//struct shadowControllerStress
//{
//	protected:
//
//	void readFromFile(json &objJson)
//    {
//        auto retVal = FileOPS::readFromFile("/tmp/json.example", objJson);
//
//        if(retVal < 0)
//        {
//            std::cout << "Couldn't read raml file" << std::endl;
//        }
//    }
//
//	std::shared_ptr<Shadow> shadowPtr = nullptr;
//
//	shadowController *SCPtr = nullptr;
//	std::string devId = "1234";
//	std::string finalDirTempPath = "/tmp";
//
//	json ramlObj;
//
//};
//
//std::vector<std::string> vectDevIdList;
//
//struct setDeviceStateStressTest: shadowControllerStress
//{
//	public:
//
//	void executeSetDeviceState(std::string devId)
//	{
//
//		for(int i = 0; i < 10000; i++)
//		{
//			std::string data = R"({
//							  "protocol": "ZWAVE",
//							  "gwid": "598070b07123a60532af7944",
//							  "CLIENTID": "598070b07123a60532af7944",
//							  "JOBID": "598300351d41c82d23bbe59e",
//							  "orgid": "58c3ce458441383a938c5c75",
//							  "deviceId": "1234",
//							  "appid": "zwaveApp_1",
//							  "rid": "598300351d41c82d23bbe59e",
//							  "properties": [
//								{
//								  "Level": 90
//								}
//							  ]
//							})";
//					SCPtr->setDeviceState(devId,data);
//		}
//	}
//
//	void SetUp()
//	{
//		try
//		{
//			system("sudo rm /tmp/raml_1234.raml");
//
//			readFromFile(ramlObj);
//
//			SCPtr = new shadowController;
//
//			auto data = ramlObj.dump();
//
//			int count = 12345;
//
//			for(auto i = 0; i < 30; i++)
//			{
//				count += i;
//
//				auto str = std::to_string(count);
//
//				vectDevIdList.push_back(str);
//
//				SCPtr->createShadow(str,data);
//			}
//
//		}
//		catch(std::exception &e)
//		{
//			std::cout << "Error is " << e.what();
//		}
//	}
//	void TearDown()
//	{
//		std::cout << "tearing down" << std::endl;
//
//		shadowPtr.reset();
//
//		delete SCPtr;
//	}
//};


//int main(int argc, char **argv)
//{
//	::testing::InitGoogleTest(&argc, argv);
//	return RUN_ALL_TESTS();

//	setDeviceStateStressTest test;
//
//	test.SetUp();
//
//	for(auto i = 0; i < 10000; i++)
//	{
//		for(auto devIdIndex = 0; devIdIndex < vectDevIdList.size(); devIdIndex++)
//		{
//			auto str = vectDevIdList[devIdIndex];
//
//			auto futureTemp = std::async(std::launch::async, &setDeviceStateStressTest::executeSetDeviceState, test, str);
//		}
//	}
//
//	while(1) sleep(0.1);
//
//	test.TearDown();
//
//}
