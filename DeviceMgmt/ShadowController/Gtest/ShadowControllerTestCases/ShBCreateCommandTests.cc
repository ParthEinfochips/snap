#include "gtest/gtest.h"
#include "../../src/shadow.h"
#include "../../src/shadowcontroller.h"
#include <memory>

/*!
 * This test case is written for testing the "createCommandsForShadowsProperty"
 * function of shadow class
 * Two return statements in source code is added for testing purpose
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
//        std::cout << "raml ========= ========== ======= " << ramlObj.dump() << std::endl;
	}

	std::shared_ptr<Shadow> shadowPtr = nullptr;

	std::shared_ptr<shadowController> SCPtr = nullptr;
	std::string devId = "1234";
	std::string finalDirTempPath = "/tmp";

	json ramlObj;

};


struct createCommandTest: shadowControllerTest
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
TEST_F(createCommandTest, createCommandPositive1)
{
	std::cout << "TEST CASE 1ST  === createCommandsForShadowsProperty" << std::endl;

	bool SUCCESS = true;

	try
	{
		json jobj;
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
		jobj=json::parse(data);
	  SUCCESS = shadowPtr->createCommandsForShadowsProperty(jobj);
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
//
//		SUCCESS = false;
	}

	EXPECT_EQ(true, SUCCESS);

}

//!to check RESPONSE_CODE_STR is 200 or not
TEST_F(createCommandTest, createCommandPositive2)
{
	std::cout << "TEST CASE 2ND  === screateCommandsForShadowsProperty" << std::endl;

	bool SUCCESS = true;
	bool SUCCESS1 = false;
	try
	{
		json jobj;
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
		jobj=json::parse(data);
	  SUCCESS = shadowPtr->createCommandsForShadowsProperty(jobj);
	  if (jobj[valStrings::RESPONSE_CODE_STR] == 200)
		  SUCCESS1 = true;
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;

		SUCCESS = false;
	}

	EXPECT_EQ(true, SUCCESS);
	EXPECT_EQ(true, SUCCESS1);
}

//! Improper key in package
TEST_F(createCommandTest, createCommandNeg1)
{
	std::cout << "TEST CASE 3rd  === screateCommandsForShadowsProperty" << std::endl;

	bool SUCCESS = true;
	try
	{
		json jobj;
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
			  "Level1": 90
			}
		  ]
		})";
	  jobj=json::parse(data);
	  SUCCESS = shadowPtr->createCommandsForShadowsProperty(jobj);
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
	}

	EXPECT_EQ(false, SUCCESS);

}


//! Empty json in package
TEST_F(createCommandTest, createCommandNeg2)
{
	std::cout << "TEST CASE 4th  === screateCommandsForShadowsProperty" << std::endl;

	bool SUCCESS = true;
	try
	{
		json jobj;
		std::string data = R"({

		})";
	  jobj=json::parse(data);
	  SUCCESS = shadowPtr->createCommandsForShadowsProperty(jobj);
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
	}

	EXPECT_EQ(false, SUCCESS);

}

//! properties missing in package
TEST_F(createCommandTest, createCommandNeg3)
{
	std::cout << "TEST CASE 5th  === screateCommandsForShadowsProperty" << std::endl;

	bool SUCCESS = true;
	try
	{
		json jobj;
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
	  jobj=json::parse(data);
	  SUCCESS = shadowPtr->createCommandsForShadowsProperty(jobj);
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
	}

	EXPECT_EQ(false, SUCCESS);

}


//!wrong json
TEST_F(createCommandTest, createCommandNeg4)
{
	std::cout << "TEST CASE 6th  === createCommandsForShadowsProperty" << std::endl;

	bool SUCCESS = true;

	try
	{
		json jobj;
		std::string data = R"({
		  "protocol": "ZWAVE",
		  "gwid": "598070b07123a60532af7944",
		  "CLIENTID": "598070b07123a60532af7944",
		  "JOBID": "598300351d41c82d23bbe59e",
		  "orgid": "58c3ce458441383a938c5c75",
		  "deviceId": "1234",
		  "appid": "zwaveApp_1",
		  "rid": 598300351d41c82d23bbe59e,
		  "properties": [
			{
			  "Level": 90
			}
		  ]
		})";
		jobj=json::parse(data);
	  SUCCESS = shadowPtr->createCommandsForShadowsProperty(jobj);
	}
	catch(std::exception &e)
	{
		std::cout << "Exception is ==== " << e.what() << std::endl;
		SUCCESS = false;
	}

	EXPECT_EQ(false, SUCCESS);

}
