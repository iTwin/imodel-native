/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Tasks/AsyncTasksManager.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/Tasks/ITaskRunner.h>
#include <Bentley/BeThread.h>
#include <Bentley/bmap.h>
#include <condition_variable>
#include <mutex>

BEGIN_BENTLEY_TASKS_NAMESPACE

struct AsyncTask;
struct WorkerThreadPool;
struct ITaskScheduler;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct AsyncTasksManager
    {
    //__PUBLISH_SECTION_END__
    private:
        static bmap<intptr_t, std::weak_ptr<struct ITaskRunner>> s_runners;
        static BeMutex s_runnersMutex;
        static BeConditionVariable s_emptyRunnersCV;

        static BeMutex s_threadingStoppingListenersMutex;
        static bvector<std::function<void()>> s_onThreadingStoppingListeners;

        static std::weak_ptr<WorkerThreadPool> s_defaultThreadPool;

        static void StopThreading();
        static void WaitForAllTreadRunnersToStop();

    public:
        static void RegisterAsyncTaskRunner (std::shared_ptr<ITaskRunner> runner);
        static void UnregisterCurrentThreadAsyncTaskRunner ();

        BENTLEYDLL_EXPORT static void RegisterOnCompletedListener(const std::function<void()>& listener);

    //__PUBLISH_SECTION_START__
    private:
        AsyncTasksManager ();

    public:
        BENTLEYDLL_EXPORT static std::shared_ptr<ITaskRunner> GetTaskRunner (intptr_t threadId);

        BENTLEYDLL_EXPORT static std::shared_ptr<ITaskRunner> GetCurrentTaskRunner ();
        BENTLEYDLL_EXPORT static std::shared_ptr<AsyncTask>   GetCurrentThreadAsyncTask ();

        //! Get default task sheduler used for running tasks not attached to any thread.
        //! Should never be used to perform long running or blocking operations as 
        //! that could cause other code to hang or deadlock.
        BENTLEYDLL_EXPORT static std::shared_ptr<ITaskScheduler> GetDefaultScheduler ();

        BENTLEYDLL_EXPORT static void StopThreadingAndWait();
    };

END_BENTLEY_TASKS_NAMESPACE
