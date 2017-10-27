#include "gtest/gtest.h"
#include "../../src/shadow.h"
#include "../../src/shadowcontroller.h"
#include <memory>
/*!
 * This test case is written for testing the constructor of shadow class
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
        std::cout << "raml ========= ========== ======= " << ramlObj.dump() << std::endl;
	}

	std::shared_ptr<Shadow> shadowPtr = nullptr;

	std::shared_ptr<shadowController> SCPtr = nullptr;
	std::string devId = "1234";
	std::string finalDirTempPath = "/tmp";

	json ramlObj;

};

 //!PERFECT PACKAGE
 struct createShadowTest: shadowControllerTest
 {
 	protected:

 	virtual void SetUp()
 	{

 		try
 		{
 			shadowControllerTest::SetUp();

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
 TEST_F(createShadowTest, CreateShadowJson)
 {
 	std::cout << "TEST CASE 1ST---------------" << std::endl;

 	bool SUCCESS = true;

 	try
 	{
 		shadowPtr = std::make_shared<Shadow> (nullptr, devId, ramlObj ,finalDirTempPath);
 	}
 	catch(std::exception &e)
 	{
 		std::cout << "Exception is ==== " << e.what() << std::endl;

 		SUCCESS = false;
 	}

 	EXPECT_EQ(true, SUCCESS);

 }


 //!Faulty RAML
 struct createShadowTestFaultRAML: shadowControllerTest
 {
 	protected:

 	virtual void SetUp()
 	{

 		try
 		{
 			shadowControllerTest::SetUp();

 			ramlObj.clear();
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

 //!Faulty RAML
 TEST_F(createShadowTestFaultRAML, CreateShadowJson)
 {
 	std::cout << "TEST CASE 1ST-=============" << std::endl;

 	bool SUCCESS = true;

 	try
 	{
 		shadowPtr = std::make_shared<Shadow> (nullptr, devId, ramlObj ,finalDirTempPath);
 	}
 	catch(std::exception &e)
 	{
 		SUCCESS = false;
 	}

 	EXPECT_EQ(false, SUCCESS);

 }

 //!Faulty RAML
 struct RAMLNotProperExceptionCatching: shadowControllerTest
 {
 	protected:

 	virtual void SetUp()
 	{
 		std::string jsonobj = R"({
 				  "CLIENTID": "598070b07123a60532af7944",
 				  "raml": "clkdmcls",
 				  "devices": [
 				    {
 				      "displayname": "FGD212 Dimmer 2",
 				      "address": "",
 				      "macid": "010f0102100018",
 				      "appid": "zwaveApp_1",
 				      "protocol": "ZWAVE",
 				      "id": "1234"
 				    }
 				  ],
 				  "JOBID": "5982fd601d41c82d23bbe597"
 				})";

 		try
 		{
 			ramlObj = json::parse(jsonobj);
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

 //! The following test determines when RAML is not proper, do we get the "RAML is not Proper" exception or not.
 TEST_F(RAMLNotProperExceptionCatching, CreateShadowJson)
 {
 	std::cout << "TEST CASE 1ST999999999999999999999" << std::endl;

 	bool SUCCESS = false;

 	try
 	{
 		shadowPtr = std::make_shared<Shadow> (nullptr, devId, ramlObj ,finalDirTempPath);
 	}
 	catch(std::exception &e)
 	{
 		if(e.what() == std::string("RAML is not proper"))
 			SUCCESS = true;
 	}

 	EXPECT_EQ(true, SUCCESS);
 }


