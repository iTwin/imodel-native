/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/WorkerThreadPool.h>
#include "ThreadingLogging.h"


USING_NAMESPACE_BENTLEY_TASKS

bmap<intptr_t, std::weak_ptr<struct ITaskRunner>> AsyncTasksManager::s_runners;
BeMutex AsyncTasksManager::s_runnersMutex;
BeConditionVariable AsyncTasksManager::s_emptyRunnersCV;

std::weak_ptr<ITaskScheduler> AsyncTasksManager::s_defaultThreadPool;

BeMutex AsyncTasksManager::s_threadingStoppingListenersMutex;
bvector<std::function<void()>> AsyncTasksManager::s_onThreadingStoppingListeners;
bool AsyncTasksManager::s_waitForThreadsWhenStopped = true;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::RegisterAsyncTaskRunner (std::shared_ptr<ITaskRunner> runner)
    {
    BeMutexHolder lock(s_runnersMutex);
    s_runners.Insert (BeThreadUtilities::GetCurrentThreadId (), runner);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::UnregisterCurrentThreadAsyncTaskRunner ()
    {
    BeMutexHolder lock(s_runnersMutex);
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
    BeMutexHolder lock(s_runnersMutex);
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
    BeMutexHolder lock(s_runnersMutex);
    ITaskSchedulerPtr sheduler = s_defaultThreadPool.lock();
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
void AsyncTasksManager::SetDefaultScheduler(ITaskSchedulerPtr scheduler)
    {
    BeMutexHolder lock(s_runnersMutex);
    if (scheduler)
        LOG.warningv("Overriding default sheduler");
    s_defaultThreadPool = scheduler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::RegisterOnCompletedListener(const std::function<void()>& listener)
    {
    BeMutexHolder lock(s_threadingStoppingListenersMutex);
    s_onThreadingStoppingListeners.push_back(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::StopThreadingAndWait()
    {
    StopThreading();
    if (s_waitForThreadsWhenStopped)
        {
        WaitForAllTreadRunnersToStop();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                        
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::StopThreading()
    {
    BeMutexHolder lock(s_threadingStoppingListenersMutex);
    for (auto listener : s_onThreadingStoppingListeners)
        {
        listener();
        }
     }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Pranciskus.Ambrazas    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct AreRunnersEmptyPredicate : IConditionVariablePredicate
    {
private:
    bmap<intptr_t, std::weak_ptr<struct ITaskRunner>> const& m_runners;
public:
    AreRunnersEmptyPredicate(bmap<intptr_t, std::weak_ptr<struct ITaskRunner>> const& runners)
        : m_runners(runners) {}
    virtual bool  _TestCondition(BeConditionVariable &cv) override { return m_runners.empty(); }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                         
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::WaitForAllTreadRunnersToStop()
    {
    AreRunnersEmptyPredicate predicate(s_runners);
    s_emptyRunnersCV.WaitOnCondition(&predicate, s_emptyRunnersCV.Infinite);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTasksManager::SetWaitForThreadsWhenStopped(bool value)
    {
    s_waitForThreadsWhenStopped = value;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<AsyncTask> AsyncTasksManager::OnAllSchedulersEmpty()
    {
    bvector<std::shared_ptr<AsyncTask>> tasks;

        {
        BeMutexHolder lock(s_runnersMutex);
        for (auto runnerWeakPtr : s_runners)
            {
            auto runner = runnerWeakPtr.second.lock();
            if (!runner)
                continue;
            tasks.push_back(runner->GetTaskScheduler()->OnEmpty());
            }
        }

    s_defaultThreadPool.reset(); // TODO: this is required because WhenAll() uses default scheduler and deadlocks.
    return AsyncTask::WhenAll(tasks);
    }
