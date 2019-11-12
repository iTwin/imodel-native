/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <Bentley/CancellationToken.h>
#include <Bentley/BeThread.h>
#include <deque>
#include <memory>

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct LimitingTaskQueue
    {
    private:
        struct Task : public PackagedAsyncTask<T>
            {
            std::shared_ptr<ICancellationListener> cancellationListener;
            std::function<AsyncTaskPtr<T> ()> createAsyncTask;

            Task (size_t stackDepth = 0) : PackagedAsyncTask<T> (nullptr, ++stackDepth) {};

            void OnFinished (const T& result)
                {
                PackagedAsyncTask<T>::m_result = result;
                PackagedAsyncTask<T>::Execute ();
                };

            virtual void _OnExecute () {};
            };

        struct Impl
            {
            Impl(ITaskSchedulerPtr sheduler) : sheduler(sheduler) {};
            BeMutex implCS;
            std::deque<std::shared_ptr<Task>> tasks;
            size_t limit = 0;
            size_t runningTasks = 0;
            ITaskSchedulerPtr sheduler;
            };

    private:
        std::shared_ptr<Impl> m_impl;

    private:
        static void CheckQueue (std::shared_ptr<Impl> impl)
            {
            BeMutexHolder mutex (impl->implCS);

            if (impl->limit != 0 &&
                impl->runningTasks >= impl->limit)
                {
                return;
                }

            auto tasksToRun = impl->limit - impl->runningTasks;
            while (tasksToRun-- && !impl->tasks.empty ())
                {
                auto task = impl->tasks.front ();
                impl->tasks.pop_front ();
                RunTask (impl, task);
                }
            };

        static void RunTask (std::shared_ptr<Impl> impl, std::shared_ptr<Task> task)
            {
            BeMutexHolder mutex (impl->implCS);
            impl->runningTasks++;

            // TODO: Then() could have overload to execute in same scheduler as parent task, avoiding need for "sheduler" parameter here.
            task->createAsyncTask()->Then(impl->sheduler, [=] (const T& result)
                {
                BeMutexHolder mutex (impl->implCS);
                impl->runningTasks--;

                task->OnFinished (result);

                task->createAsyncTask = nullptr;
                task->cancellationListener = nullptr;

                CheckQueue (impl);
                });
            };

        static void OnTaskCancelled (std::shared_ptr<Impl> impl, std::shared_ptr<Task> task)
            {
            BeMutexHolder mutex (impl->implCS);

            auto it = std::find (impl->tasks.begin (), impl->tasks.end (), task);
            if (it != impl->tasks.end ())
                {
                impl->tasks.erase (it);
                RunTask (impl, task);
                }
            };

    public:
        //! Create queue that limits tasks.
        //! @param[in] scheduler - optional scheduler to for running async queue code. Uses default scheduler if null.
        LimitingTaskQueue(ITaskSchedulerPtr scheduler = nullptr) : m_impl(std::make_shared<Impl>(scheduler)) {}

        //! Set how many tasks to run at one time. Zero will indicate no limit and all pushed tasks will be run.
        void SetLimit (size_t limit)
            {
            BeMutexHolder mutex (m_impl->implCS);
            m_impl->limit = limit;
            };

        //! Ensure that no more than set limit of tasks are running.
        //! @param[in] createAsyncTask - callback that should start and return new async task.
        //! @param[in] token - cancellation token to be used to invoke running of canceled task so it could finalize instead of waiting in queue.
        AsyncTaskPtr<T> Push (const std::function<AsyncTaskPtr<T> ()>& createAsyncTask, ICancellationTokenPtr token = nullptr)
            {
            auto impl = m_impl;
            BeMutexHolder mutex (impl->implCS);

            if (impl->limit == 0)
                {
                return createAsyncTask ();
                }

            auto task = std::make_shared<Task> ();
            impl->tasks.push_back (task);

            task->createAsyncTask = createAsyncTask;

            if (token)
                {
                task->cancellationListener = std::make_shared<SimpleCancellationListener> ([=]
                    {
                    OnTaskCancelled (impl, task);
                    });
                token->Register (task->cancellationListener);
                }

            auto parentTask = AsyncTasksManager::GetCurrentThreadAsyncTask ();
            if (parentTask)
                {
                parentTask->AddSubTask (task);
                }

            CheckQueue (impl);
            return task;
            };
    };

END_BENTLEY_TASKS_NAMESPACE
