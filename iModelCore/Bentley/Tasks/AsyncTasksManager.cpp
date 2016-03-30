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
std::mutex AsyncTasksManager::s_runnersMutex;
std::condition_variable AsyncTasksManager::s_emptyRunnersCV;

std::weak_ptr<WorkerThreadPool> AsyncTasksManager::s_defaultThreadPool;

std::mutex AsyncTasksManager::s_threadingStoppingListenersMutex;
bvector<std::function<void()>> AsyncTasksManager::s_onThreadingStoppingListeners;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::RegisterAsyncTaskRunner (std::shared_ptr<ITaskRunner> runner)
    {
    std::lock_guard<std::mutex> lock(s_runnersMutex);
    s_runners.Insert (BeThreadUtilities::GetCurrentThreadId (), runner);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::UnregisterCurrentThreadAsyncTaskRunner ()
    {
    std::lock_guard<std::mutex> lock(s_runnersMutex);
    s_runners.erase (BeThreadUtilities::GetCurrentThreadId ());

    if (s_runners.empty())
        {
        s_emptyRunnersCV.notify_all();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                        
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ITaskRunner> AsyncTasksManager::GetTaskRunner (intptr_t threadId)
    {
    std::lock_guard<std::mutex> lock(s_runnersMutex);
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
    std::lock_guard<std::mutex> lock(s_runnersMutex);
    std::shared_ptr<WorkerThreadPool> sheduler = s_defaultThreadPool.lock ();
    if (sheduler == nullptr)
        {
        sheduler = WorkerThreadPool::Create (2, "Default Worker Pool");
        s_defaultThreadPool = sheduler;
        }

    return sheduler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::RegisterOnCompletedListener(const std::function<void()>& listener)
    {
    std::lock_guard<std::mutex> lock(s_threadingStoppingListenersMutex);
    s_onThreadingStoppingListeners.push_back(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::StopThreadingAndWait()
    {
    StopThreading();
    WaitForAllTreadRunnersToStop();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                        
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::StopThreading()
    {
    std::lock_guard<std::mutex> lock(s_threadingStoppingListenersMutex);
    for (auto listener : s_onThreadingStoppingListeners)
        {
        listener();
        }
     }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                         
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::WaitForAllTreadRunnersToStop()
    {
    std::unique_lock<std::mutex> lk(s_runnersMutex);
    s_emptyRunnersCV.wait(lk, [&]
        {
        return s_runners.empty();
        });
    }

