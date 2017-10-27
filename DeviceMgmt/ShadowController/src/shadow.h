#ifndef SHADOW_H
#define SHADOW_H

#include <map>
#include <iostream>
#include <string.h>
#include <fstream>
#include <future>
#include "ramlparserpycpp.h"
#include "CommonHeader/directoryOPS.h"
#include "CommonHeader/json.hpp"
#include "CommonHeader/jsonCheck.hpp"
#include "CommonHeader/debugLog.h"

class shadowController;

///*!
// * A temporary timer class used for measuring the time that has already elapsed.
// */
//
//class myTimer
//{
//public:
//    myTimer() : beg_(clock_::now()) {}
//    void reset() { beg_ = clock_::now(); }
//    double elapsed() const {
//        return std::chrono::duration_cast<second_>
//            (clock_::now() - beg_).count(); }
//
//private:
//    typedef std::chrono::high_resolution_clock clock_;
//    typedef std::chrono::duration<double, std::ratio<1> > second_;
//    std::chrono::time_point<clock_> beg_;
//};

#define SHADOW_RAML_TEMP_PATH "/ShadowRaml/"
#define SHADOW_JSON_SUBPART "/ShadowController/"

#define PERSISTANT_FILE "/opt/GA.conf.d/persistant.conf"

using namespace nlohmann;

enum SHADOW_SITUATIONS
{
    SHADOW_NOT_PRESENT = 0x678,
    SHADOW_PRESENT = 0x679,
    RAML_NOT_PRESENT = 0x680,
    PROPERTY_NOT_PRESENT = 0x681
};

/*!
    Enumeration for determining whether the command has come from cloud or topic.
*/

enum DELTA_SITUATIONS
{
    DELTA_PRESENT = 0x770,
    NO_DELTA_PRESENT = 0x771,
	FALSE_COMMAND	= 0x772
};


/*!
    Enumeration for determining whether the command has come from cloud or topic.
*/

enum TOPIC_R_COMMAND
{
    IT_IS_TOPIC = 0x900,
    IT_IS_COMMAND = 0x901
};

/*!
    Namespace statusesSC to hold the job status.
*/

namespace statusesSC
{
	const std::string STATE_ALREADY_SET_SC = "STATE_ALREADY_SET";   //!State already set.
	const std::string DELTA_N_FIRED_SC = "DELTA_N_COMMAND_FIRED_SUCCESSFULLY";  //!Delta is found and command is fired.
	// const std::string NO_UPDATE_FROM_DEVICE = "NO_UPDATE_FROM_DEVICE";  //!No update from the device.
	// const std::string COMMAND_COMPLETED_SC = "JOB_DONE";    //!Job is fone
	const std::string COMMAND_QUEUED_SC = "JOB_QUEUED"; //!Job is queued
	const std::string GET_COMMAND_SC = "GET_COMMAND_COMPLETED";
	const std::string NONNOTIFY_GET_COMMAND_SC = "NON_NOTIFIY_GET_COMMAND_COMPLETED";
	const std::string PROPERTY_NOT_REGISTERED = "PROPERTY NOT PRESENT"; //!Property was not found.
	const std::string PROPERTY_NOT_POSTABLE = "PROPERTY NOT POSTABLE"; //!Property is not postable.
	const std::string INVALID_JSON_SC = "INVALID_JSON_SC";  //!Json is invalid.
};

/*!
    A namespace to hold the json keys.
*/

namespace valStrings
{
    const std::string REQUESTED_VAL_STR 	 	    = "requested_by";   //! Requester key.
    const std::string ORIGINATOR_VAL_STR 	 	    = "originator";     //! Originator key.
    const std::string USERNAME_VAL_STR 	 	        = "username";       //! Username key.

    const std::string MANAGED_VAL_STR 	 	        = "managed_by";     //! Managed By key.
    const std::string CURRENT_STAGE_VAL_STR 	    = "current_stage";  //! Current Stage key.

    const std::string JOB_STATUS_VAL_STR            = "job_status";     //! Job Status key.
    const std::string JOB_REQUEST_VAL_STR           = "job_request";    //! Job Request key.
    const std::string JOB_RESPONSE_VAL_STR          = "job_response";   //! Job Response key.
    const std::string DEVICE_ID_VAL_STR             = "deviceid";       //! Device ID key.
    const std::string GATEWAY_ID_VAL_STR            = "gwid";           //! gateway ID key.
    const std::string ORGANIZATION_ID_VAL_STR       = "orgid";          //! Organization ID key.
    const std::string CREATED_TS_VAL_STR            = "created_ts";     //! Created Timestamp key.

	const std::string REPORTED_VAL_STR 	 	        = "reported_value"; //! Reported Value key.
	const std::string DESIRED_VAL_STR 	 	        = "desired_value";  //! Desired Value key.
	const std::string NOTIFY_VAL_STR                = "notify";         //! Notify property.
	const std::string PROPERTIES_VAL_STR 	        = "properties";     //! Properties key.
	const std::string SUBPROPERTIES_VAL_STR         = "subproperties";  //! Subproperties key.
	const std::string CONFIG_VAL_STR 	 	        = "config";         //! Configuration key.
	const std::string PROPERTYNAME_STR 	 	        = "propertyName";   //! Property Name key.
	const std::string ONLY_PROPERTY_STR		        = "property";       //! Property key.
	const std::string VALUE_STR 		 	        = "value";          //! Value of the property key.

	const std::string JOBSTATUS_STR 	 	= "jobStatus";              //! Job status key.
	const std::string DevID_STR 		 	= "deviceId";               //! Device ID key.
	const std::string JOBID_STR  		 	= "JOBID";                  //! JOB ID key.
	const std::string CLIENTID_STR  		= "CLIENTID";               //! Client ID key.
	const std::string DESIREDVALUE_STR 	 	= "desiredValue";           //! Desired Value key. 
	const std::string CURRENT_REP_STR 	 	= "value";                  //! value key.
	const std::string OBSERVEDTIME_STR 	 	= "observedtime";           //! Observed Time key.
	// const std::string STATE_CHANGED_TIME 	= "StateChangeFinishTime";  //! State Changed time finish key.
	// const std::string STATE_REQUEST_TIME 	= "StateRequestTime";       //! key.
	// const std::string ACTION			 	= "action";                 //! 
	// const std::string REPLY_STR				= "Reply";
    // const std::string APP_NAME_STR          = "app_name";


	const std::string MESSAGE_STR			= "msg";                    //! msg key.
	const std::string RESPONSE_CODE_STR		= "responsecode";           //! response key.
	const std::string STATUS_STR			= "status";                 //! status key.

    const std::string SHADOW_ERR_STR        = "shadowError";            //! shadow error key.
};


/*!
    A static function that returns the current time. 
 */

static std::string GetObservedTime()
{
	std::time_t result;

	time(&result);

    char buffer[sizeof("2011-10-08T07:07:09Z")];

    strftime(buffer, sizeof(buffer), "%FT%TZ", gmtime(&result));

	return (std::string)buffer;

}

//! A class which constructs the blob packets.

class prepareResponse
{
    public:
    /*!
        Prepare the status response only.

        /param obj of the json
        /param status this is the status of the job.
    */
	static void prepareResponseOnlyStatus(json &obj, const std::string &status)
	{
		obj[valStrings::JOBSTATUS_STR] = status;
	}

    /*!
        Blob packet static formation.

        /param nameOfOriginator Name of the originator.
        /param devIDPerm Permanent device ID of the shadow.
        /param theCommand Job request payload.
        /param theResponse Job Response payload.
        /param status Job Status of the job.
        /param theCommandPayload Destination payload in which the blob is to be created.
    */
    
    static void formTheBlobPacket(const std::string &nameOfOriginator, const std::string &devIDPerm, const json &theCommand, const json &theResponse, const std::string &status, json &theCommandPayload)
    {
        theCommandPayload[valStrings::REQUESTED_VAL_STR][valStrings::ORIGINATOR_VAL_STR]    = nameOfOriginator;

        json currentStage;
        currentStage["current_stage"] 														= "GATEWAY_SHADOW";

        theCommandPayload[valStrings::MANAGED_VAL_STR]                                      = currentStage;
        theCommandPayload[valStrings::JOBID_STR]	                                        = theCommand[valStrings::JOBID_STR];
    	theCommandPayload[valStrings::JOB_STATUS_VAL_STR]                                   = status;
    
    	theCommandPayload[valStrings::CREATED_TS_VAL_STR]                                   = GetObservedTime();

    	theCommandPayload[valStrings::DEVICE_ID_VAL_STR]                                    = devIDPerm;

        theCommandPayload[valStrings::JOB_REQUEST_VAL_STR]                                  = theCommand;

        theCommandPayload[valStrings::JOB_RESPONSE_VAL_STR]                                 = theResponse;

        theCommandPayload[valStrings::ORGANIZATION_ID_VAL_STR]                              = configurationSettings::orgId;
        theCommandPayload[valStrings::GATEWAY_ID_VAL_STR]                                   = configurationSettings::gatewayId;
    }
};


//! This is the shadow class which is managing the device and performs the delta information.

class Shadow
{
public:

	/*!
	  Constructor. An exception throwing default constructor.

	 */
    Shadow();
	/*!
	  Constructor. An exception throwing parameterized constructor.

      \sa Shadow()
      \param SCPtr pointer to shadow controller i.e this class's parent.
      \param devId ID of the device which shadow is representing.
      \param raml RAML from which the shadow would be generated.
      \param finalDirPathTemp Final Directory where the shadow file would be placed.

	 */
    Shadow(shadowController *SCPtr, std::string &devId, std::string &macID, json &raml, json &shadowJson, const std::string &finalDirPathTemp);

	/*!
	  Destructor

	 */
    ~Shadow();

    void setConfig(int secs, TransportMask mask);

    /*!
	  A asynchronous task runner function for getDeviceStateActualTask.

	  return true if successfully executes the command, false if it faces any issue.

      \param devId ID of the device which shadow is representing.
      \param data Data payload.

	 */

    bool GetDeviceState(std::string& devId, std::string& data);

    /*!
	  An exception throwing get device state function.

	  return true if successfully executes the command, false if it faces any issue.

      \param devId ID of the device which shadow is representing.
      \param data Data payload.

	 */

    bool GetDeviceStateActualTask(std::string& devId, std::string& data);

    //! A data receiving public function.
    /*!
      An asynchronous task based function that will be spawned.

      The arguments are deviceID and the data is telemetry/notification data sent by the device.

      \sa DataReceivedThreaded()
      \param devId the first argument.
      \param data the second argument.
    */

    void DataReceivedThreaded(std::string &devId, std::string &data);

    /*!
     * Here the desired property would be set.

     \param devId ID of the Device.
     \param data payload for the device.

     */
    bool SetDeviceState(std::string& devId, std::string& data);

     /*!
     * This is the function that would receive the data.

     \param devId ID of the Device.
     \param data payload for the device.

     */

    void DataReceived(std::string devId, std::string data);

    /*!
      gets the active status.

      returns a boolean.
     */

    bool getShadowStatus() const;

    /*!
      Sets the status device.

      \param status, boolean for the shadow being active.
     */

    void setShadowStatus(std::string payload);

    /*!
      An unescape function which removes unwanted slash, tab etc.

      \param str this takes in the payload.
     */

    std::string Unescape(const std::string& str);

    /*!
      A simple function for printing the commands.
     */

    void printCommandQueue() const;

private:
    std::string devIDPerm;

    std::string macID;

    std::string raml;

    std::string ramlFilePath;

    std::string jsonShadowFilePath;

    uint16_t tempID = 0;

    int shadowSetStateTimeOut = 0;

    std::string ramlPath;

    // threadedImpl timerObj;

    /*!
     * this function sends the telemetry to Cloud.
     */
    void sendSnapShot();

    /*!
     * Permanent shadow object.
     */

    json jsonShadowObj;

    /*!
     * Shadow controller pointer
     */

    shadowController *_SCPtr;

    /*!
     * This function creates the json Object from RAML data given.

     \param devId it is the ID of the device.
     \param raml this takes the string format of RAML.
     */

    bool  CreateShadowJson(const std::string &devId, nlohmann::json &raml);

    /*!
     * This function contains the python & C++ binding for converting RAML to JSON.

     \param devId it is the ID of the device.
     */
    bool CreateShadowControlFile(const std::string &devId);

    /*!
      This function removes the initial command.
     */

    void removeFirstDesired();

    /*!
      This checks the jsonShadowObject whether that particular property is present or not.

      \param objTemp, json object of the payload.
     */
    std::string getValue(json &objTemp);

    /*!
      Checks if the array of desired value is empty or not.

      \param key this checks whether there is already a job in the desired or not.
     */

    bool checkIFDesiredEmpty(std::string &key);

    /*!
     Checks whether the key can be set or not.

     \param key whether desired value is setable or not.
     */
    bool checkDesiredVal(std::string &key);

    /*!
     Checks if the command that has come is from a cloud or mesh App.

     \param objJson

     This returns the ENUM TOPIC_R_COMMAND.
     */

    TOPIC_R_COMMAND checkTopicRCommand( json &objJson);

    /*!
      Multiple properties in a single object.

      \param JOBID_STR takes in the JOB ID string.
      \param objTemp takes in the particular object whose having size greater than 1.
     */

    bool multiplePropertiesInSingleObject(const std::string &JOBID_STR, json &objTemp); 

    /*!
       Property set device state.

      \param objTemp A json object which has the job request.
     */

    bool createCommandsForShadowsProperty(json &objTemp);

    /*!
        Task based thread that only turns on when there is a request coming in.

     */
    void threadDataSigReceiver();

    /*!
        final directory path is the path where the shadow json would be created.
     */
    std::string finalDirPath;

    /*!
    Disabled.
     */
    std::string getDeviceStateNFlush(std::string &data);

    void CreateShadowJson(const std::string &devId, const std::string &raml);

    bool activeShadow = false;

    /*!
      The blob topic.
    */

    std::string blobTopic;

    /*!
     The telemetry topic.
    */

    std::string telemetryTopic;

    /*!
        Protocol
    */

    TransportMask maskVal;

    /*!
        Notification Enabled or not.
     */
    bool NotificationEnabled = true;

    /*!
        Paired Values type definition.
     */

    using pairedVals = std::pair<std::string, json>;

    /*!
        std::queue for queuing the commands.
     */

     std::list<pairedVals> commandQueue;

     /*!
        The thread which will manage the tasks.
      */

     std::thread t;

     /*!
        List for the keys that will 
      */

     std::vector<std::string> keysVect;

     /*!A mutex lock for the getdevice state.
      */

     std::mutex mutexGetDeviceState;

     /*!
      * Remove this function whenever you get the chance.
      */
     void Filewrite(std::string val);

     /*!
      * Updates the shadow file.
      */

     int writeJsonToSTDFile(const std::string &filePath, const json &dataBuf);
};



#endif // SHADOW_H




// ============== Disabled section.

// struct shadowCommand
// {
//     shadowCommand(std::string &idTemp, json &jsonObjTemp, std::string propertyNameTemp, TOPIC_R_COMMAND tmpCmdRTopic)
//         : id(idTemp),
// 		  jsonObjPermanentShadow(jsonObjTemp),
// 		  propertyName(propertyNameTemp),
//           valCmdRTopic(tmpCmdRTopic)
//     {
//     	lastTelemetryReceivedEpoch = std::time(nullptr);
//     }

//     /*
//      * This gets the app.
//      */

//     // void setCommandPayload()
//     // {
//     // 	theCommandPayload[valStrings::JOBID_STR]	= jsonObjPermanentShadow[valStrings::JOBID_STR];
//     // 	theCommandPayload[valStrings::JOBSTATUS_STR] = statusesSC::DELTA_N_FIRED_SC;
//     // 	theCommandPayload[valStrings::ACTION]	= "setState";
//     // 	theCommandPayload[valStrings::PROPERTYNAME_STR] = propertyName;
//     // 	theCommandPayload[valStrings::DevID_STR] = id;
//     // 	theCommandPayload[valStrings::STATE_REQUEST_TIME] = GetObservedTime();
//     //     theCommandPayload[PP_APPNAME] = jsonObjPermanentShadow[PP_APPNAME];
//     // }

//     /*
//      * Set reply.
//      */

//     // json &GetReply()
//     // {
//     //     return theReply;
//     // }

//     /*
//      * Set reply.
//      */

//     // void SetReply(const std::string &reply)
//     // {
//     //     theReply = json::parse(reply);
//     // }

//      /*
//       * Get topic or command value.
//       */

//     TOPIC_R_COMMAND getTopicRCMDVal() const
//     {
//         return valCmdRTopic;
//     }

//     /*
//      * returns PropertyName function
//      */

//     std::string GetPropertyName() const
//     {
//     	return propertyName;
//     }

//     /*
//      * Get last telemetry epoch.
//      */

//     uint64_t getLastTelemetryEpoch() const
//     {
//     	return lastTelemetryReceivedEpoch;
//     }

//     /*
//      * Returns the original command payload, once the delta is over.
//      */

//     // json &giveCommandPayload()
//     // {
//     // 	return theCommandPayload;
//     // }

//     /*
//      * returns the previous delta calculation.
//      */

//     DELTA_SITUATIONS currentDeltaSituation() const
//     {
//     	return situation;
//     }

//     /*
//      * This compares the value and gives out the result.
//      */

//     DELTA_SITUATIONS compareVals(json &obj)
//     {
//         std::cout << "1" << jsonObjPermanentShadow.dump() << std::endl;

//         std::cout << "Comparing the values " << jsonObjPermanentShadow[valStrings::DESIREDVALUE_STR] << obj[propertyName][valStrings::REPORTED_VAL_STR];

//     	if(jsonObjPermanentShadow[valStrings::DESIREDVALUE_STR].empty())
//     	{
//     		situation = DELTA_SITUATIONS::FALSE_COMMAND;

//     		return situation;
//     	}

//     	lastTelemetryReceivedEpoch = std::time(nullptr);

//     	if(jsonObjPermanentShadow[valStrings::DESIREDVALUE_STR] == obj[propertyName][valStrings::REPORTED_VAL_STR])
//     	{
//             if(valCmdRTopic == TOPIC_R_COMMAND::IT_IS_COMMAND)
//             {
//                 theCommandPayload[valStrings::JOBSTATUS_STR] = statusesSC::COMMAND_COMPLETED_SC;
//                 theCommandPayload[valStrings::DESIREDVALUE_STR] = obj[propertyName][valStrings::REPORTED_VAL_STR];
//                 theCommandPayload[valStrings::CURRENT_REP_STR] = obj[propertyName][valStrings::REPORTED_VAL_STR];
//                 theCommandPayload[valStrings::STATE_CHANGED_TIME] = GetObservedTime();
//             }

//     		situation = DELTA_SITUATIONS::NO_DELTA_PRESENT;

//     		return situation;
//     	}
//     	else
//     	{
//     		theCommandPayload[valStrings::CURRENT_REP_STR] = obj[propertyName][valStrings::REPORTED_VAL_STR];

//     		situation = DELTA_SITUATIONS::DELTA_PRESENT;

//     		return situation;
//     	}
//     }
//     /*
//      * transport if called, this returns true else false.
//      */

//     bool getTransPortWrapperCalled()
//     {
//         return transportWrapperCalled;
//     }

// private:
//     /*
//      * Property it is maintaining.
//      */

//     std::string propertyName;

//     /*
//      * Dev ID.
//      */
     
//     std::string &id;

//     json jsonObjPermanentShadow, theCommandPayload, theReply;

//     DELTA_SITUATIONS situation = DELTA_SITUATIONS::NO_DELTA_PRESENT;

//     bool transportWrapperCalled = false;

//     /*
//      * Epoch of the last telemetry;
//      */

//     uint64_t lastTelemetryReceivedEpoch;

//     /*
//      * ENUM for command and topic
//      */
//      TOPIC_R_COMMAND valCmdRTopic;

// };

/*!
    This class manages the shadow file. Shadow file is in json format.

    Class has a clear defined interface and all the functions throw exceptions.
*/
