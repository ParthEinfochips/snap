#include "gtest/gtest.h"
#include "../../src/shadow.h"
#include <memory>

struct shadowControllerTest : public ::testing::Test
{
	protected:

	virtual void SetUp()
	{

	}

	std::shared_ptr<Shadow> shadowPtr = nullptr;

	std::shared_ptr<shadowController> SCPtr = nullptr;
	std::string devId = "1234";
	std::string finalDirTempPath = "/tmp";

};

//PERFECT PACKAGE
struct createShadowTest: shadowControllerTest
{
	protected:

	virtual void SetUp()
	{
		std::string jsonobj = R"({
				  "CLIENTID": "598070b07123a60532af7944",
				  "raml": "#%RAML 0.8 title: device RAML baseUri: example version: V1.00 schemas : - getSchema: | { "definitions": { "Start Level": { "properties": { "Start Level": { "minimum": 0, "type": "integer", "maximum": 255 } } }, "Level": { "properties": { "Level": { "minimum": 0, "type": "integer", "maximum": 255 } } }, "Ignore Start Level": { "properties": { "Ignore Start Level": { "type": "boolean" } } }, "Switch": { "properties": { "Switch": { "type": "boolean" } } }, "config": { "properties": { "config": {} } }, "Switch All": { "properties": { "Switch All": { "enum": [ "Disabled", "Off Enabled", "On Enabled", "On and Off Enabled" ], "type": "string" } } } }, "type": "object", "anyOf": [ { "$ref": "#/definitions/Switch" }, { "$ref": "#/definitions/Level" }, { "$ref": "#/definitions/Ignore Start Level" }, { "$ref": "#/definitions/Start Level" }, { "$ref": "#/definitions/Switch All" }, { "$ref": "#/definitions/config" } ] } - postSchema: | { "definitions": { "Dim": { "properties": { "Dim": {} } }, "Bright": { "properties": { "Bright": {} } }, "Level": { "properties": { "Level": { "minimum": 0, "type": "integer", "maximum": 255 } } }, "Switch": { "properties": { "Switch": { "type": "boolean" } } }, "Ignore Start Level": { "properties": { "Ignore Start Level": { "type": "boolean" } } }, "Switch All": { "properties": { "Switch All": { "enum": [ "Disabled", "Off Enabled", "On Enabled", "On and Off Enabled" ], "type": "string" } } }, "Start Level": { "properties": { "Start Level": { "minimum": 0, "type": "integer", "maximum": 255 } } } }, "type": "object", "anyOf": [ { "$ref": "#/definitions/Switch" }, { "$ref": "#/definitions/Level" }, { "$ref": "#/definitions/Bright" }, { "$ref": "#/definitions/Dim" }, { "$ref": "#/definitions/Ignore Start Level" }, { "$ref": "#/definitions/Start Level" }, { "$ref": "#/definitions/Switch All" } ] } /resources: get: responses: 200: body: application/json: schema: getSchema example : | { "Start Level" : 255, "Level" : 255, "Ignore Start Level" : true, "Switch" : true, "Switch All" : "Disabled" } post: body: application/json: schema: postSchema example : | { "Level" : 255, "Switch" : true, "Ignore Start Level" : true, "Switch All" : "Disabled", "Start Level" : 255 } responses: 200: body: application/json: schema: postSchema example : | { "Level" : 255, "Switch" : true, "Ignore Start Level" : true, "Switch All" : "Disabled", "Start Level" : 255 }",
				  
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

	}

	json ramlObj;

};

//PERFECT PACKAGE
TEST_F(createShadowTest, CreateShadowJson)
{
	std::cout << "TEST CASE 1ST" << std::endl;

	bool SUCCESS = true;

	try
	{
		shadowPtr = std::make_shared<Shadow> (nullptr, devId, ramlObj ,finalDirTempPath);
	}
	catch(std::exception &e)
	{
		SUCCESS = false;
	}

	EXPECT_EQ(true, SUCCESS);

}


//Faulty RAML
struct createShadowTestFaultRAML: shadowControllerTest
{
	protected:

	virtual void SetUp()
	{
		std::string jsonobj = R"({
				  "CLIENTID": "598070b07123a60532af7944",
				  "raml": "#%RAML 0.8 title: device RAML baseUri: example version: V1.00 schemas : - getSchema: | { "definitions": { "Start Level": { "properties": { "Start Level": { "minimum": 0, "type": "integer", "maximum": 255 } } }, "Level": { "properties": { "Level": { "minimum": 0, "type": "integer", "maximum": 255 } } }, "Ignore Start Level": { "properties": { "Ignore Start Level": { "type": "boolean" } } }, "Switch": { "properties": { "Switch": { "type": "boolean" } } }, "config": { "properties": { "config": {} } }, "Switch All": { "properties": { "Switch All": { "enum": [ "Disabled", "Off Enabled", "On Enabled", "On and Off Enabled" ], "type": "string" } } } }, "type": "object", "anyOf": [ { "$ref": "#/definitions/Switch" }, { "$ref": "#/definitions/Level" }, { "$ref": "#/definitions/Ignore Start Level" }, { "$ref": "#/definitions/Start Level" }, { "$ref": "#/definitions/Switch All" }, { "$ref": "#/definitions/config" } ] } - postSchema: | { "definitions": { "Dim": { "properties": { "Dim": {} } }, "Bright": { "properties": { "Bright": {} } }, "Level": { "properties": { "Level": { "minimum": 0, "type": "integer", "maximum": 255 } } }, "Switch": { "properties": { "Switch": { "type": "boolean" } } }, "Ignore Start Level": { "properties": { "Ignore Start Level": { "type": "boolean" } } }, "Switch All": { "properties": { "Switch All": { "enum": [ "Disabled", "Off Enabled", "On Enabled", "On and Off Enabled" ], "type": "string" } } }, "Start Level": { "properties": { "Start Level": { "minimum": 0, "type": "integer", "maximum": 255 } } } }, "type": "object", "anyOf": [ { "$ref": "#/definitions/Switch" }, { "$ref": "#/definitions/Level" }, { "$ref": "#/definitions/Bright" }, { "$ref": "#/definitions/Dim" }, { "$ref": "#/definitions/Ignore Start Level" }, { "$ref": "#/definitions/Start Level" }, { "$ref": "#/definitions/Switch All" } ] } /resources: get: responses: 200: body: application/json: schema: getSchema example : | { "Start Level" : 255, "Level" : 255, "Ignore Start Level" : true, "Switch" : true, "Switch All" : "Disabled" } post: body: application/json: schema: postSchema example : | { "Level" : 255, "Switch" : true, "Ignore Start Level" : true, "Switch All" : "Disabled", "Start Level" : 255 } responses: 200: body: application/json: schema: postSchema example : | { "Level" : 255, "Switch" : true, "Ignore Start Level" : true, "Switch All" : "Disabled", "Start Level" : 255 }",
				  
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

	}

	json ramlObj;

};

//Faulty RAML
TEST_F(createShadowTestFaultRAML, CreateShadowJson)
{
	std::cout << "TEST CASE 1ST" << std::endl;

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

//Faulty RAML
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

	}

	json ramlObj;

};

//! The following test determines when RAML is not proper, do we get the "RAML is not Proper" exception or not.
TEST_F(RAMLNotProperExceptionCatching, CreateShadowJson)
{
	std::cout << "TEST CASE 1ST" << std::endl;

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

//struct setDeviceStateTestPositive: shadowControllerTest
//{
//
//};

//int main(int argc, char **argv)
//{
//	::testing::InitGoogleTest(&argc, argv);
//	return RUN_ALL_TESTS();
//}
