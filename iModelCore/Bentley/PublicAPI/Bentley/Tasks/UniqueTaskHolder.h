/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/Tasks/UniqueTaskHolder.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/BeThread.h>
#include <Bentley/bmap.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <Bentley/Tasks/CancellationToken.h>

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ID, typename T>
struct UniqueTasksHolder
    {
    private:
        struct Task
            {
            SimpleCancellationTokenPtr token;
            AsyncTaskPtr<T> asyncTask;
            };

        struct Impl
            {
            BeMutex taskCS;
            bmap<ID, std::shared_ptr<Task>> tasks;
            };

    private:
        std::shared_ptr<Impl> m_impl;

    public:
        UniqueTasksHolder () : m_impl (std::make_shared<Impl> ())
            {
            }

        //! Ensure that only one task is running for given id. Will return running task or get new one using given callback.
        //! @param[in] id unique task id
        //! @param[in] executeNewTaskCallback Callback that should start and return new async task if needed
        AsyncTaskPtr<T> GetTask (const ID& id, const std::function<AsyncTaskPtr<T> (ICancellationTokenPtr token)>& executeNewTaskCallback)
            {
            auto impl = m_impl;

            BeMutexHolder mutex (impl->taskCS);
            std::shared_ptr<Task> task;

            auto it = impl->tasks.find (id);
            if (it != impl->tasks.end ())
                {
                task = it->second;
                }
            else
                {
                task = std::make_shared<Task> ();
                task->token = SimpleCancellationToken::Create ();
                task->asyncTask = executeNewTaskCallback (task->token);
                task->asyncTask->Then ([=] (const T&)
                    {
                    BeMutexHolder mutex (impl->taskCS);
                    auto it = impl->tasks.find (id);
                    if (it != impl->tasks.end () && it->second.get () == task.get ())
                        {
                        impl->tasks.erase (id);
                        }
                    });

                if (!task->asyncTask->IsCompleted ())
                    {
                    impl->tasks[id] = task;
                    }
                }

            auto parentTask = AsyncTasksManager::GetCurrentThreadAsyncTask ();
            if (nullptr != parentTask)
                {
                parentTask->AddSubTask (task->asyncTask);
                }

            return task->asyncTask;
            }

        // Cancels task and removes it
        void CancelTask (const ID& id)
            {
            BeMutexHolder mutex (m_impl->taskCS);
            auto it = m_impl->tasks.find (id);
            if (it != m_impl->tasks.end ())
                {
                it->second->token->SetCanceled ();
                m_impl->tasks.erase (id);
                }
            }

        // Check if task is running at a given moment
        bool IsTaskRunning (const ID& id)
            {
            BeMutexHolder mutex (m_impl->taskCS);
            auto it = m_impl->tasks.find (id);
            return it != m_impl->tasks.end ();
            }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct UniqueTaskHolder : private UniqueTasksHolder <int, T>
    {
    public:
        //! Ensure that only one task is running. Will return running task or get new one using given callback.
        //! @param[in] executeNewTaskCallback Callback that should start and return new async task if needed
        AsyncTaskPtr<T> GetTask (const std::function<AsyncTaskPtr<T> ()>& executeNewTaskCallback)
            {
            return UniqueTasksHolder <int, T>::GetTask (0, [&] (ICancellationTokenPtr token)
                {
                return executeNewTaskCallback ();
                });
            }
    };

END_BENTLEY_TASKS_NAMESPACE
