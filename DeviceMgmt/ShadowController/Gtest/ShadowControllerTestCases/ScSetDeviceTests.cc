#include "gtest/gtest.h"
#include "../../src/shadow.h"
#include "../../src/shadowcontroller.h"
#include <memory>
/*!
 * This test case is written for testing the "setDeviceState"
 * function of shadowController class
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
		system("sudo rm /tmp/raml_1234.raml");

        readFromFile(ramlObj);
//        std::cout << "raml ========= ========== ======= " << ramlObj.dump() << std::endl;

	}

	std::shared_ptr<Shadow> shadowPtr = nullptr;

	std::shared_ptr<shadowController> SCPtr = nullptr;
	std::string devId = "1234";
	std::string finalDirTempPath = "/tmp";

	json ramlObj;

};

struct SetDeviceState : shadowControllerTest
{
	protected:
	shadowController *ptr = nullptr;

	virtual void SetUp()
	{
		try
		{
			shadowControllerTest::SetUp();
//			shadowPtr = std::make_shared<Shadow> (ptr, devId, ramlObj ,finalDirTempPath);

			SCPtr = std::make_shared<shadowController> ();
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
	}
};


//!PERFECT PACKAGE
TEST_F(SetDeviceState, SetDeviceStatePositive1)
{
	testing::internal::CaptureStdout();

	std::string output = testing::internal::GetCapturedStdout();
	std::cout << "TEST CASE 1ST  === set device" << std::endl;

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
		std::string data1 = ramlObj.dump();
		SCPtr->createShadow(devId,data1);
	    SCPtr->setDeviceState(devId,data);
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;

		SUCCESS = false;
	}

	EXPECT_EQ(true, SUCCESS);

}

//!Wrong devId
TEST_F(SetDeviceState, SetDeviceStateneg1)
{
	testing::internal::CaptureStdout();


	std::cout << "TEST CASE 2nd  === set device" << std::endl;

	bool SUCCESS = true;

	try
	{

		std::string data = R"({
		  "protocol": "ZWAVE",
		  "gwid": "598070b07123a60532af7944",
		  "CLIENTID": "598070b07123a60532af7944",
		  "JOBID": "598300351d41c82d23bbe59e",
		  "orgid": "58c3ce458441383a938c5c75",
		  "deviceId": "123456",
		  "appid": "zwaveApp_1",
		  "rid": "598300351d41c82d23bbe59e",
		  "properties": [
			{
			  "Level": 90
			}
		  ]
		})";
		std::string devId1 = "123456";
		std::string data1 = ramlObj.dump();
		SCPtr->createShadow(devId,data1);
	    SCPtr->setDeviceState(devId1,data);
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;

		SUCCESS = false;
	}
	std::string output = testing::internal::GetCapturedStdout();
	std::cout<<"op-------------------"<<output<<std::endl;
	std::size_t found = output.find(std::string("Device not present as Shadow"));
	if (found!=std::string::npos)
		std::cout<<"inside if"<<std::endl;
		SUCCESS = false;
	EXPECT_EQ(false, SUCCESS);

}

//!Wrong devId
TEST_F(SetDeviceState, SetDeviceStateneg2)
{
	std::cout << "TEST CASE 3rd  === set device" << std::endl;

	bool SUCCESS = true;

	try
	{

		std::string data = R"({
		  "protocol": "ZWAVE",
		  "gwid": "598070b07123a60532af7944",
		  "CLIENTID": "598070b07123a60532af7944",
		  "JOBID": "598300351d41c82d23bbe59e",
		  "orgid": "58c3ce458441383a938c5c75",
		  "deviceId": "123456",
		  "appid": "zwaveApp_1",
		  "rid": "598300351d41c82d23bbe59e",
		  "properties": [
			{
			  "Level": 90
			}
		  ]
		})";
		std::string devId1 = "123456";
		std::string data1 = ramlObj.dump();
		SCPtr->createShadow(devId,data1);
	    SCPtr->setDeviceState(devId1,data);
		json objTemp = nlohmann::json::parse(data);
		if (objTemp[valStrings::STATUS_STR] == "failure")
			SUCCESS = false;
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;

		SUCCESS = false;
	}

	EXPECT_EQ(false, SUCCESS);

}

//! To check if Properties is missing in package than STATUS_STR is failure or not
TEST_F(SetDeviceState, SetDeviceStateneg3)
{
	std::cout << "TEST CASE 4th  === set device" << std::endl;

	bool SUCCESS = true;

	try
	{

		std::string data = R"({
		  "protocol": "ZWAVE",
		  "gwid": "598070b07123a60532af7944",
		  "CLIENTID": "598070b07123a60532af7944",
		  "JOBID": "598300351d41c82d23bbe59e",
		  "orgid": "58c3ce458441383a938c5c75",
		  "deviceId": "123456",
		  "appid": "zwaveApp_1",
		  "rid": "598300351d41c82d23bbe59e"

		})";
		std::string devId1 = "123456";
		std::string data1 = ramlObj.dump();
		SCPtr->createShadow(devId,data1);
	    SCPtr->setDeviceState(devId,data);
		json objTemp = nlohmann::json::parse(data);
		if (objTemp[valStrings::STATUS_STR] == "failure")
			SUCCESS = false;
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;

		SUCCESS = false;
	}

	EXPECT_EQ(false, SUCCESS);

}

//! To check if json is empty in package than STATUS_STR is failure or not
TEST_F(SetDeviceState, SetDeviceStateneg4)
{
	std::cout << "TEST CASE 5th  === set device" << std::endl;

	bool SUCCESS = true;

	try
	{

		std::string data = R"({

		})";
		std::string devId1 = "123456";
		std::string data1 = ramlObj.dump();
		SCPtr->createShadow(devId,data1);
	    SCPtr->setDeviceState(devId,data);
		json objTemp = nlohmann::json::parse(data);
		if (objTemp[valStrings::STATUS_STR] == "failure")
			SUCCESS = false;
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;

		SUCCESS = false;
	}

	EXPECT_EQ(false, SUCCESS);

}


//! To check if package is perfect than RESPONSE_CODE_STR is 200 or not
TEST_F(SetDeviceState, SetDeviceStatePos2)
{
	std::cout << "TEST CASE 6th  === set device" << std::endl;
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
		std::string data1 = ramlObj.dump();
		SCPtr->createShadow(devId,data1);
	    SCPtr->setDeviceState(devId,data);
	    json objTemp = nlohmann::json::parse(data);
	  if (objTemp[valStrings::RESPONSE_CODE_STR] == 200)
			SUCCESS1 = true;
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
	}
	EXPECT_EQ(true, SUCCESS1);

}

//! To check if package is perfect than JOB_STATUS_VAL_STR is "JOB_QUEUED" or not
TEST_F(SetDeviceState, SetDeviceStatePos3)
{
	std::cout << "TEST CASE 7th  === set device" << std::endl;
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
		std::string data1 = ramlObj.dump();
		SCPtr->createShadow(devId,data1);
	    SCPtr->setDeviceState(devId,data);
	    json objTemp = nlohmann::json::parse(data);
	  if (objTemp[valStrings::JOB_STATUS_VAL_STR] == "JOB_QUEUED")
			SUCCESS1 = true;
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
	}

	EXPECT_EQ(true, SUCCESS1);

}

