/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Tasks/AsyncTaskRunnerPool.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/Tasks/TaskScheduler.h>
#include <Bentley/Tasks/AsyncTaskRunnerFactory.h>

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct AsyncTaskRunnerPool : public ITaskScheduler, public std::enable_shared_from_this<AsyncTaskRunnerPool>
    {
    private:
        friend struct PoolTaskScheduler;

    private:
        struct NotifyingTaskScheduler : TaskScheduler
            {
            private:
                std::weak_ptr<AsyncTaskRunnerPool> m_poolPtr;

            protected:
                void virtual _OnEmpty () override
                    {
                    if (auto pool = m_poolPtr.lock ())
                        {
                        pool->OnSchedulerEmpty ();
                        }
                    };

            public:
                NotifyingTaskScheduler (std::shared_ptr<AsyncTaskRunnerPool> pool) : m_poolPtr (pool) {}

                virtual ~NotifyingTaskScheduler () { }
            };

    private:
        bool m_initializeFlag;
        BeMutex m_mutex;

    protected:
        int         m_runnerCount;
        Utf8String  m_name;
        bool        m_started;

        bvector<std::shared_ptr<ITaskRunner>>    m_runners;
        std::shared_ptr<IAsyncTaskRunnerFactory> m_runnerFactory;

        std::shared_ptr<ITaskScheduler> m_taskScheduler;

        std::shared_ptr<ITaskScheduler> m_defaultWorkerThreadPool;

        std::shared_ptr<AsyncTaskRunnerPool> m_thisPtr;
        BeMutex  m_thisPtrMutex;

    protected:
        BENTLEYDLL_EXPORT void Start ();
        BENTLEYDLL_EXPORT void Stop ();

        BENTLEYDLL_EXPORT void OnBeforeTaskPushed ();
        BENTLEYDLL_EXPORT void OnAfterTaskPushed  ();

        BENTLEYDLL_EXPORT void OnSchedulerEmpty ();

    public:
        BENTLEYDLL_EXPORT AsyncTaskRunnerPool
            (
            int runnerCount = 1, 
            Utf8CP name = nullptr,
            std::shared_ptr<IAsyncTaskRunnerFactory> runnerFactory = nullptr
            );

        BENTLEYDLL_EXPORT virtual ~AsyncTaskRunnerPool ();

        // -----------------------------------------------------------------------------------------------------
        // ITaskScheduler Implementation


        BENTLEYDLL_EXPORT virtual void Push (std::shared_ptr<AsyncTask> task, AsyncTask::Priority priority = AsyncTask::Priority::Inherited) override;

        //! Push task for execution to specific parent task
        BENTLEYDLL_EXPORT virtual void Push (std::shared_ptr<AsyncTask> task, std::shared_ptr<AsyncTask> parentTask, AsyncTask::Priority priority = AsyncTask::Priority::Inherited) override;

        BENTLEYDLL_EXPORT virtual std::shared_ptr<AsyncTask> WaitAndPop () override;

        BENTLEYDLL_EXPORT virtual std::shared_ptr<AsyncTask> TryPop () override;

        BENTLEYDLL_EXPORT virtual int GetQueueTaskCount () const override;

        BENTLEYDLL_EXPORT virtual bool HasRunningTasks () const override;

        BENTLEYDLL_EXPORT virtual AsyncTaskPtr<void> OnEmpty () const override;
    };

END_BENTLEY_TASKS_NAMESPACE
