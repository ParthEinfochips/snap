#ifndef DEVICESPECIFICCOMMANDS
#define DEVICESPECIFICCOMMANDS

#include "commandbase.h"
//#include "configSensorManager.h"
#include "commands.h"
#include "DeviceObject/DeviceObj.h"

//struct command;
//class ConfigSensorManager;

/*
 * @brief: Get Device Status according to the deviceID given. Active or not.
 */

struct getDeviceStatusCommand : public command
{
    getDeviceStatusCommand()
    {
        cmdType = GET_DEVICE_STATUS_;
    }

    void execute();

private:
};

/*
 * @brief: Set Device Status(Active or Deactive) according to the deviceID given.
 */

struct putDeviceStatusCommand : public command
{
    putDeviceStatusCommand()
    {
        cmdType = PUT_DEVICE_STATUS;
    }

    void execute();

private:

};

/*
 * @brief: Get Device RAML according to the deviceID given.
 */

struct getDeviceRAMLCommand : public command
{
    getDeviceRAMLCommand()
    {
        cmdType = GET_DEVICE_RAML_;
    }

    void execute();

private:
};

/*
 * @brief: Set Device RAML according to the deviceID given.
 */

struct putDeviceRAMLCommand : public command
{
    putDeviceRAMLCommand()
    {
        cmdType = SET_DEVICE_RAML_;
    }

    void execute();

private:
};


/*
 * @brief: Get Device State according to the deviceID given.
 */

struct getDeviceStateCommand : public command
{
    getDeviceStateCommand()
    {
        cmdType = GET_DEVICE_STATE_;
    }

    void execute();

private:
};

/*
 * @brief: Set Device State according to the deviceID given.
 */

struct setDeviceStateCommand : public command
{
    setDeviceStateCommand()
    {
        cmdType = SET_DEVICE_STATE_;
    }

    void execute();

private:

};


/*
 * @brief: delete Device Properties according to the deviceID given.
 */

struct deleteDevicePropertiesCommand : public command
{
    deleteDevicePropertiesCommand()
    {
        cmdType = DEL_DEVICE_PROPERTIES;
    }

    void execute();

private:
};

/*
 * @brief: Set Device Properties according to the deviceID given.
 */

struct setDevicePropertiesCommand : public command
{
    setDevicePropertiesCommand()
    {
        cmdType = PUT_DEVICE_PROPERTIES ;
    }

    void execute();

private:
};


#endif // DEVICESPECIFICCOMMANDS

