/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Tasks/AsyncTasksManager.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/WorkerThreadPool.h>

USING_NAMESPACE_BENTLEY_TASKS

bmap<intptr_t, std::weak_ptr<struct ITaskRunner>> AsyncTasksManager::s_runners;
BeMutex AsyncTasksManager::s_runnersCS;
std::weak_ptr<WorkerThreadPool> AsyncTasksManager::s_defaultThreadPool;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::RegisterAsyncTaskRunner (std::shared_ptr<ITaskRunner> runner)
    {
    BeMutexHolder h (s_runnersCS);
    s_runners.Insert (BeThreadUtilities::GetCurrentThreadId (), runner);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::UnregisterCurrentThreadAsyncTaskRunner ()
    {
    BeMutexHolder h (s_runnersCS);
    s_runners.erase (BeThreadUtilities::GetCurrentThreadId ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                        
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ITaskRunner> AsyncTasksManager::GetTaskRunner (intptr_t threadId)
    {
    BeMutexHolder h (s_runnersCS);
    auto runnerIt = s_runners.find (threadId);
    if (runnerIt != s_runners.end ())
        {
        return runnerIt->second.lock ();
        }

    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ITaskRunner> AsyncTasksManager::GetCurrentTaskRunner ()
    {
    return GetTaskRunner(BeThreadUtilities::GetCurrentThreadId ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<AsyncTask> AsyncTasksManager::GetCurrentThreadAsyncTask()
    {
    auto runner = GetCurrentTaskRunner();
    if (runner == nullptr)
        return nullptr;
    
    return runner->GetCurrentRunningTask();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ITaskScheduler> AsyncTasksManager::GetDefaultScheduler()
    {
    BeMutexHolder h (s_runnersCS);
    std::shared_ptr<WorkerThreadPool> sheduler = s_defaultThreadPool.lock ();
    if (sheduler == nullptr)
        {
        sheduler = WorkerThreadPool::Create (2, "Default Worker Pool");
        s_defaultThreadPool = sheduler;
        }

    return sheduler;
    }