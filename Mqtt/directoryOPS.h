#ifndef __DIRECTORY_OPS_H
#define __DIRECTORY_OPS_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

//struct directoryOPS
//{
//    static bool checkExistence(const std::string &&entityPath)
//    {
//        struct stat fb;
//
//        auto retVal = stat(entityPath.c_str(), &fb);
//
//        if(retVal < 0)
//        {
//            return false;
//        }
//        else
//        {
//            return true;
//        }
//    }
//
//    static bool createDir(const std::string &&entityPath, mode_t mode)
//    {
//            auto retVal = mkdir(entityPath.c_str(), mode);
//
//            if(retVal < 0)
//            {
//                switch(errno)
//                {
//                    case EEXIST:
//                    displayValue2("EEXISTs ", entityPath);
//                    break;
//                    case EACCES:
//                    displayValue2("EACCESS ", entityPath);
//                    break;
//                    case EPERM:
//                    displayValue2("EPERM ", entityPath);
//                    break;
//                    case ENOENT:
//                    displayValue2("ENOENT ", entityPath);
//                    break;
//                }
//            }
//
//    }
//
//    static bool createPath(const std::string &&entityPath, mode_t mode)
//    {
//        auto slashPos = std::string::npos;
//
//        std::cout << entityPath << std::endl;
//
//        while((slashPos = entityPath.find('/', slashPos + 1)) != std::string::npos)
//        {
//            if(slashPos == 0)
//            {
//                display("I am coming here");
//                continue;
//            }
//
//            std::string &&subDirPath = entityPath.substr(0, slashPos);
//
//            displayValue2("The positions are ", subDirPath);
//
//            auto retVal = mkdir(subDirPath.c_str(), mode);
//
//            if(retVal < 0)
//            {
//                switch(errno)
//                {
//                    case EEXIST:
//                    displayValue2("EEXISTs ", subDirPath);
//                    break;
//                    case EACCES:
//                    displayValue2("EACCESS", subDirPath);
//                    break;
//                    case EPERM:
//                    displayValue2("EPERM ", subDirPath);
//                    break;
//                    case ENOENT:
//                    displayValue2("ENOENT ", subDirPath);
//                    break;
//                }
//            }
//        }
//    }
//};

struct FileOPS
{
	//    int createFile(const std::string &filePath)
	//    {
	//        std::fstream FileStrm;
	//
	//        FileStrm.open(filePath);
	//
	//        if(FileStrm.is_open())
	//        {
	//            FileStrm.close();
	//            return 0;
	//        }
	//        else
	//        {
	//            return -3;
	//        }
	//    }
	//
	//    int createFile(const std::string &&filePath)
	//    {
	//        std::fstream FileStrm;
	//
	//        FileStrm.open(filePath);
	//
	//        if(FileStrm.is_open())
	//        {
	//            FileStrm.close();
	//            return 0;
	//        }
	//        else
	//        {
	//            return -3;
	//        }
	//    }



//	bool checkExistence(const std::string &&entityPath)
//	{
//		struct stat fb;
//
//		auto retVal = stat(entityPath.c_str(), &fb);
//
//		if(retVal < 0)
//		{
//			return false;
//		}
//		else
//		{
//			return true;
//		}
//	}

	int readFromFile(const std::string &filePath, json &dataBuf)
	{
		std::fstream FileStrm;

		FileStrm.open(filePath);

		if(FileStrm.is_open())
		{
			try
			{
				FileStrm >> dataBuf;

				displayValue2("j is ", dataBuf);
			}
			catch(std::exception &e)
			{
				displayValue2("In Readfromfile", e.what());
			}

			FileStrm.close();
			return 0;
		}
		else
		{
			return -3;
		}
	}

	int readFromFile(const std::string &&filePath, json &dataBuf)
	{
		std::fstream FileStrm;

		FileStrm.open(filePath);

		if(FileStrm.is_open())
		{
			try
			{
				FileStrm >> dataBuf;

				displayValue2("j is ", dataBuf);
			}
			catch(std::exception &e)
			{
				displayValue2("In Readfromfile", e.what());
			}


			FileStrm.close();
			return 0;
		}
		else
		{
			return -3;
		}
	}

	int writeToFile(const std::string &filePath, const std::string &dataBuf)
	{
		std::fstream FileStrm;
		std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << filePath << "aaaa" << dataBuf << std::endl;
		displayValue2("in the write to file rvalue ", filePath);
		displayValue2("in the write to file rvalue ", dataBuf);

		//if(directoryOPS::checkExistence(filePath))
		//return -5;

		FileStrm.open(filePath, std::ios::out | std::ios::in |std::ios::app);

		if(FileStrm.is_open())
		{
			FileStrm << dataBuf << std::endl;

			FileStrm.close();
			return 0;
		}
		else
		{
			display("In the write to file else ");

			return -3;
		}
	}

	int writeToFile(const std::string &&filePath, const std::string &&dataBuf)
	{
		std::fstream FileStrm;

		displayValue2("in the write to file rvalue ", filePath);
		displayValue2("in the write to file rvalue ", dataBuf);

		//if(directoryOPS::checkExistence(filePath))
		//		return -5;

		FileStrm.open(filePath, std::ios::out | std::ios::in |std::ios::app);

		if(FileStrm.is_open())
		{
			FileStrm << dataBuf << std::endl;

			FileStrm.close();
			return 0;
		}
		else
		{
			display("In the write to file else ");

			return -3;
		}
	}
};


// int myFunction()
// {


//     json jsonOBJ;

//     jsonOBJ["Packages"]  =R"( [
//     {
//         "packageId":"58c7a17729a3a348c82524bf",
//         "packageName":"Cisco OVA",
//         "packageType":"OS",
//         "description":"NA",
//         "version":"Ubuntu 14.04",
//         "status":"INSTALLED"
//     },
//     {
//         "packageId":"58c7a1ba29a3a348c82524c0",
//         "packageName":"eSIA",
//         "packageType":"FW",
//         "description":"NA",
//         "version":"1.4.0",
//         "status":"INSTALLED"
//     }
//     ]  )"_json;

//     std::stringstream strStrm;

//     strStrm << ESIA_PACKAGE_DIR_SLASH << "testFile.pmp";

//     std::remove(strStrm.str().c_str());

//     FileOPS fOPSObj;

//     fOPSObj.writeToFile(strStrm.str(), jsonOBJ.dump());

//     json strPath;

//     fOPSObj.readFromFile(strStrm.str(), strPath);

//     std::cout << std::endl << strPath << std::endl;

//     // try
//     // {
//     //     json  jsonObj = json::parse(strPath);

//     //     std::cout<< jsonObj.dump() << std::endl;
//     // }
//     // catch(...)
//     // {
//     //     std::cout << "Problems in parsing" << std::endl;
//     // }


//     return 0;
// }

#endif
