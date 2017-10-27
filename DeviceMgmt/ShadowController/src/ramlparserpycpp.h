#ifndef RAMLPARSERPYCPP_H
#define RAMLPARSERPYCPP_H

#include "pybindingcpp.h"

//! Values inserted RAML parsing.

struct valuesReqRAML: public valuesReq {
	std::string RAMLFilePath;
	std::string jsonFilePath;

	json ObjectOfRAML;

	std::string functionName;
};

//! ramlParserPyCPP this is the class which is derived from the pybindingcpp class. This class passes the raml text to the RAMLFICATIONS library which returns a json.

class ramlParserPyCPP: public PyBindingCPP {
public:

	/*!
		Constructor.
	 */

	ramlParserPyCPP() :
		PyBindingCPP() {

	}

	/*! 
		Destructor
	 */

	~ramlParserPyCPP() {
		deinitializePy();
	}

	/*!
		Get json OF raml file.
	 */

	void getJsonOfRAML(valuesReqRAML *valuesReqRAMLObjTemp) {
		valuesReqRAMLObj = valuesReqRAMLObjTemp;
	}

	/*!
		This function calls loads the module and calls the ramlfications function.

		It returns the pyobject containing the json format.
	 */

	virtual PyObject *callFunction() {

		//Gets the pyObject of the ramlfication parser.
		auto funcPtr = mapFunction["ParseRAMLFile_shadow"];

		PyObject *pResult, *pValue;
		if (PyCallable_Check(funcPtr)) {
			auto &ramlFile = valuesReqRAMLObj->RAMLFilePath;

			pValue = Py_BuildValue("(z)", (char*) ramlFile.c_str());

			PyErr_Print();

			if (pValue) {
				pResult = PyObject_CallObject(funcPtr, pValue);

				if (pResult)
					return pResult;
			} else {
				PyErr_Print();
				return NULL;
			}
		} else {
			PyErr_Print();
			return NULL;
		}
	}

	/*!
		This function imports the ramlfications parser module. 
		This function calls the callFunction for the actual json object creation.

		returns the boolean value.
	 */

	virtual bool usageSpecificFunction() {
		importModule();

		fileFunctionMap();

		//Calls the ramlfication parser for the conversion to json.
		auto pResult = callFunction();

		//Checks whether the pointer received is none or not.
		if (pResult != Py_None)
		{
			try
			{
				valuesReqRAMLObj->ObjectOfRAML = nlohmann::json::parse(
						PyString_AsString(pResult));
			}
			catch (...)
			{
				std::rethrow_exception(std::current_exception());
			}

			auto &jsonShadowFilePath = valuesReqRAMLObj->jsonFilePath;

			//
			std::string status;
			std::fstream FileStrm;
			json dataBuf;

			FileStrm.open(jsonShadowFilePath);

			if(FileStrm.is_open())
			{
				try
				{
					FileStrm >> dataBuf;
					status = dataBuf["status"];
				}
				catch(std::exception &e)
				{
					std::cout << "IN exception" << std::endl;
				}

				FileStrm.close();
			}
			//

			std::ofstream files;
			files.open(jsonShadowFilePath.c_str(), std::ios::out);

			if (files.is_open())
			{
				//
				json tempJson = valuesReqRAMLObj->ObjectOfRAML;

				tempJson["status"] = status;

				valuesReqRAMLObj->ObjectOfRAML = tempJson;
				//

				files << valuesReqRAMLObj->ObjectOfRAML;

				files.close();

				return true;
			}
			else
			{
				throw std::runtime_error("Unable to create shadow json file");

				return false;
			}
		}
		else
		{
			PyErr_Print();

			throw std::runtime_error("RAML is not proper");

			return false;
		}
	}

private:
	valuesReqRAML *valuesReqRAMLObj = nullptr;
};

#endif // RAMLPARSERPYCPP_H
