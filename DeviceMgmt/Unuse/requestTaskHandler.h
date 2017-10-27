#ifndef REQUEST_TASK_HANDLER_H
#define REQUEST_TASK_HANDLER_H

/*
 *@brief:  This class executes the request received through request manager.
 *
 * Inherited from AsyncTask, so that it may run on AsyncTaskQueue Thread pool.
 */

#include "AsyncTaskPool.h"
#include "configSensorManager.h"
class RequestTaskHandler : public AsyncTask
{

public:
    RequestTaskHandler();
    /**
     * OnEmptyQueue - handling 'queue become empty' events.
     */
    void OnEmptyQueue();
    /**
     * OnEmptyQueue - handling 'queue got new message' events.
     *  @param taskdata - pointer to the data that currently processed.
     */
    void OnTask(std::unique_ptr<command> command);

private:

};

#endif //REQUEST_TASK_HANDLER_H
