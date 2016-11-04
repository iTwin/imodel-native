/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Tasks/AsyncTaskRunnerPool.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <Bentley/Tasks/AsyncTaskRunnerPool.h>

USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                         
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskRunnerPool::AsyncTaskRunnerPool
(
int runnerCount,
Utf8CP name,
std::shared_ptr<IAsyncTaskRunnerFactory> runnerFactory
)
: 
m_runnerCount (runnerCount), 
m_name (name),
m_started (false), 
m_runnerFactory (runnerFactory),
m_initializeFlag(false)
    {
    if (m_runnerFactory == nullptr)
        {
        m_runnerFactory = std::shared_ptr<AsyncTaskRunnerFactory<AsyncTaskRunner>> (new AsyncTaskRunnerFactory<AsyncTaskRunner> ());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                      
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskRunnerPool::~AsyncTaskRunnerPool ()
    {
    Stop ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTaskRunnerPool::Start ()
    {
    m_started = true;

    auto thisPtr = shared_from_this ();

    m_taskScheduler = std::make_shared <NotifyingTaskScheduler> (thisPtr);

    auto defaultPool = AsyncTasksManager::GetDefaultScheduler ();
    if (defaultPool != thisPtr)
        {
        m_defaultWorkerThreadPool = defaultPool;
        }

    for (int i = 0; i < m_runnerCount; i++)
        {
        auto runner = m_runnerFactory->CreateRunner ();

        Utf8String runnerName;
        if (m_runnerCount > 1)
            {
            runnerName = Utf8PrintfString("%s #%d", m_name.c_str(), i);
            }
        else
            {
            runnerName = m_name;
            }

        runner->Start (m_taskScheduler, runnerName.c_str ());
        m_runners.push_back (runner);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTaskRunnerPool::Stop ()
    {
    if (!m_started)
        return;

    for (auto runner : m_runners)
        {
        runner->Stop ();
        }

    for (auto runner : m_runners)
        {
        runner->WakeUp();
        }

    m_started = false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTaskRunnerPool::OnBeforeTaskPushed ()
    {
    BeMutexHolder lock(m_mutex);
    if (!m_initializeFlag)
        {
        Start ();
        m_initializeFlag = true;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTaskRunnerPool::OnAfterTaskPushed ()
    {
    BeMutexHolder lock (m_thisPtrMutex);

    if (m_thisPtr == nullptr)
        {
        m_thisPtr = shared_from_this ();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTaskRunnerPool::OnSchedulerEmpty ()
    {
    BeMutexHolder lock (m_thisPtrMutex);

    m_thisPtr = nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTaskRunnerPool::Push (std::shared_ptr<AsyncTask> task, AsyncTask::Priority priority)
    {
    OnBeforeTaskPushed ();

    m_taskScheduler->Push (task, priority);

    OnAfterTaskPushed ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTaskRunnerPool::Push (std::shared_ptr<AsyncTask> task, std::shared_ptr<AsyncTask> parentTask, AsyncTask::Priority priority)
    {
    OnBeforeTaskPushed ();

    m_taskScheduler->Push (task, parentTask, priority);

    OnAfterTaskPushed ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                        
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<AsyncTask> AsyncTaskRunnerPool::WaitAndPop ()
    {
    return m_taskScheduler->WaitAndPop ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                         
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<AsyncTask> AsyncTaskRunnerPool::TryPop ()
    {
    return m_taskScheduler->TryPop ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                   
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> AsyncTaskRunnerPool::OnEmpty () const
    {
    if (!m_taskScheduler)
        {
        return CreateCompletedAsyncTask ();
        }

    return m_taskScheduler->OnEmpty ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                          
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsyncTaskRunnerPool::HasRunningTasks () const
    {
    if (!m_taskScheduler)
        {
        return false;
        }

    return m_taskScheduler->HasRunningTasks ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                       
+---------------+---------------+---------------+---------------+---------------+------*/
int AsyncTaskRunnerPool::GetQueueTaskCount () const
    {
    if (!m_taskScheduler)
        {
        return 0;
        }

    return m_taskScheduler->GetQueueTaskCount ();
    }
    
