#include "AsyncTaskPool.h"

AsyncTaskQueue::AsyncTaskQueue(bool ownersheap) :
    m_IsStopping(true), m_ownersheap(ownersheap),m_AsyncTask(NULL)
{

}

AsyncTaskQueue::~AsyncTaskQueue()
{

}

void AsyncTaskQueue::Enqueue(std::unique_ptr<command> cmdPtr)
{
#ifdef _WIN32
    EnterCriticalSection(&m_Lock);
    m_MessageQueue.push(std::move(cmdPtr);
    WakeConditionVariable(&m_QueueChanged);
    LeaveCriticalSection(&m_Lock);
#else
    pthread_mutex_lock(&m_Lock);
    m_MessageQueue.push(std::move(cmdPtr));
    pthread_cond_signal(&m_QueueChanged);
    pthread_mutex_unlock(&m_Lock);
#endif
}

void AsyncTaskQueue::Start(AsyncTask *async)
{
    if (!m_IsStopping) {
        return;
    }

    m_AsyncTask = async;

    m_IsStopping = false;

#ifdef _WIN32
    InitializeCriticalSection(&m_Lock);
    InitializeConditionVariable(&m_QueueChanged);
    m_handle = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 256 * 1024, (unsigned int (__stdcall*)(void*))ReceiverThreadWrapper, this, 0, NULL));
#else
    pthread_mutex_init(&m_Lock, NULL);
    pthread_cond_init(&m_QueueChanged, NULL);
    pthread_create(&m_Thread, NULL, ReceiverThreadWrapper, this);
#endif
}

void AsyncTaskQueue::Stop()
{
    if (m_IsStopping) {
        return;
    }

#ifdef _WIN32
    EnterCriticalSection(&m_Lock);
    while (!m_MessageQueue.empty()) {
        m_MessageQueue.pop();
    }
    m_IsStopping = true;
    WakeConditionVariable(&m_QueueChanged);
    LeaveCriticalSection(&m_Lock);
    WaitForSingleObject(m_handle, INFINITE);
    CloseHandle(m_handle);
    DeleteCriticalSection(&m_Lock);
#else
    pthread_mutex_lock(&m_Lock);
    while (!m_MessageQueue.empty()) {
        m_MessageQueue.pop();
    }
    m_IsStopping = true;
    pthread_cond_signal(&m_QueueChanged);
    pthread_mutex_unlock(&m_Lock);
    pthread_join(m_Thread, NULL);
    pthread_cond_destroy(&m_QueueChanged);
    pthread_mutex_destroy(&m_Lock);
#endif
}

void* AsyncTaskQueue::ReceiverThreadWrapper(void* context)
{
    AsyncTaskQueue* asyncTask = reinterpret_cast<AsyncTaskQueue*>(context);
    if (asyncTask == NULL) { // should not happen
        return NULL;
    }
    asyncTask->Receiver();
    return NULL;
}

void AsyncTaskQueue::Receiver()
{
#ifdef _WIN32
    EnterCriticalSection(&m_Lock);
    while (!m_IsStopping) {
        while (!m_MessageQueue.empty()) {
            TaskData const* taskData = m_MessageQueue.front();
            m_MessageQueue.pop();
            LeaveCriticalSection(&m_Lock);
            m_AsyncTask->OnTask(taskData);
            if (m_ownersheap) {
                delete taskData;
            }
            EnterCriticalSection(&m_Lock);
        }
        m_AsyncTask->OnEmptyQueue();

        // it's possible m_IsStopping changed while executing OnTask() (which is done unlocked)
        //  therefore we have to check it again here, otherwise we potentially deadlock here
        if (!m_IsStopping) {
            //pthread_testcancel(); //for win only?
            SleepConditionVariableCS(&m_QueueChanged, &m_Lock, INFINITE);
        }
    }
    LeaveCriticalSection(&m_Lock);
#else
    pthread_mutex_lock(&m_Lock);
    while (!m_IsStopping) {
        while (!m_MessageQueue.empty()) {

            auto requestObj = std::move(m_MessageQueue.front());

            m_MessageQueue.pop();
            pthread_mutex_unlock(&m_Lock);
            m_AsyncTask->OnTask(std::move(requestObj));
//            if (m_ownersheap) {
//                delete taskData;
//            }
            pthread_mutex_lock(&m_Lock);
        }
        m_AsyncTask->OnEmptyQueue();

        // it's possible m_IsStopping changed while executing OnTask() (which is done unlocked)
        //  therefore we have to check it again here, otherwise we potentially deadlock here
        if (!m_IsStopping) {
            pthread_cond_wait(&m_QueueChanged, &m_Lock);
        }
    }
    pthread_mutex_unlock(&m_Lock);
#endif
}
