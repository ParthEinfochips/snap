#ifndef PYBINDINGCPP_H
#define PYBINDINGCPP_H

#include "CommonHeader/CPPHeaders.h"
#include <python2.7/Python.h>


//! This class does the python connectivity.
struct valuesReq {
    std::string valueConversion;
};


//! The base class for python binding it gets inherited by any application needs to inherit and implement its own information.
   
class PyBindingCPP
{
public:

    /*! Constructor
        This initializes python library.
    */

    PyBindingCPP()
    {
        Py_Initialize();
    }

    /*!
        Destructor.
    */

    virtual ~PyBindingCPP()
    {
//    	if(pModule)
//    		delete pModule;
//    	if(pDict)
//    		delete pDict;
//    	if(pFunc)
//    		delete pFunc;

//    	for(auto &obj : mapFunction)
//    	{
//    		delete obj.second;
//    	}
//
//    	mapFunction.clear();
    }

    /*! 
        This imports the module ramlifcation Parser created by gunjan. Returns true if succeeds and false for failure.
    */

    bool importModule()
    {
        std::cout <<  "######" << std::endl;

        try {
            pModule = PyImport_ImportModule((char*)"RamlficationsParser");

            if(pModule == NULL) {
                PyErr_Print();
                return false;
            }

            if(pModule) {
                pDict = PyModule_GetDict(pModule);
            }
            else {
                PyErr_Print();
                return false;
            }
        }
        catch(std::exception &e) {
            std::cout << "exception raised is " << e.what() << std::endl;
        }

        return true;
    }

    /*!
        fileFunctionMap loads and stores the shadow function of the ramlifcations parser.
    */

    bool fileFunctionMap()
    {
        auto objectFunction = PyDict_GetItemString(pDict, (char*)"ParseRAMLFile_shadow");

        if(PyCallable_Check(objectFunction))
        {
            mapFunction["ParseRAMLFile_shadow"] = objectFunction;
        }
    }

    /*!
        deinitializes the python library.
    */

    void deinitializePy()
    {
        Py_DECREF(pModule);

        Py_Finalize();
    }

    /*!
        Pure virtual function that calls the actual function. 
    */
    virtual PyObject *callFunction() = 0;


    /*!
        Pure virtual function that specified by the usage function. 
    */

    virtual bool usageSpecificFunction() = 0;

protected:
    PyObject* pModule = nullptr;
    PyObject* pDict = nullptr;

    PyObject* pFunc = nullptr;

    std::map<std::string, PyObject*> mapFunction;
};

#endif // PYBINDINGCPP_H
