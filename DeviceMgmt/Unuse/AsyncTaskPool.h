#ifndef ASYNC_TASK_POOL_H
#define ASYNC_TASK_POOL_H

//#include "headers/DMA_MQTopics.h"
#include "devicespecificcommands.h"

/**
 * class AsyncTask
 * User must derived from this class and implement its virtual functions
 */

struct command;
class IDDetails;

class AsyncTask 
{
  public:
    AsyncTask() { }
    virtual ~AsyncTask() { }
    /**
     * OnEmptyQueue - handling 'queue become empty' events.
     */
    virtual void OnEmptyQueue() = 0;
    /**
     * OnEmptyQueue - handling 'queue got new message' events.
     *  @param taskdata - pointer to the data that currently processed.
     */
    virtual void OnTask(std::unique_ptr<command> command ) = 0;
};

/*
 * @brief: Pool which has multiple threads running.
 * Status: Might not be used further, since we might use Alljoyn thread pool.
 */
class AsyncTaskQueue {
  public:
    /**
     * AsyncTaskQueue constructor
     *  @param asyncTask - pointer to the class which callbacks will be called.
     *  @param ownersheap - if true, the queue will delete the data after calling to callbacks.
     */
    AsyncTaskQueue(bool ownersheap = true);
    /**
     * AsyncTaskQueue destructor
     */
    ~AsyncTaskQueue();
    /**
     * Start
     */
    void Start(AsyncTask *async);
    /**
     * Stop
     */
    void Stop();
    /**
     * Enqueue data
     */
    void Enqueue(std::unique_ptr<command> cmdPtr);
  private:
    /**
     * The thread responsible for receiving messages
     */
#ifdef _WIN32
    HANDLE m_handle;
#else
    pthread_t m_Thread;
#endif

    /**
     * A Queue that holds the commands
     */
    std::queue<std::unique_ptr<command>> m_MessageQueue;

    /**
     * The mutex Lock
     */
    pthread_mutex_t m_Lock;

    /**
     * The Queue Changed thread condition
     */
    pthread_cond_t m_QueueChanged;

    /**
     * is the thread in the process of shutting down
     */
    bool m_IsStopping;

    /**
     * A wrapper for the receiver Thread
     * @param context
     */
    static void* ReceiverThreadWrapper(void* context);

    /**
     * The function run in the Receiver
     */
    void Receiver();

    /**
     * class to report about events to the client
     */
    AsyncTask* m_AsyncTask;

    /**
     * is the thread is the owner of the objects in the queue and those will delete them.
     */
    bool m_ownersheap;
};





#endif
