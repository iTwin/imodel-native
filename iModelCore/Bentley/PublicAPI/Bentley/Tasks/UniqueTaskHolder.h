/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/Tasks/UniqueTaskHolder.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/BeThread.h>
#include <Bentley/bmap.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <Bentley/Tasks/TaskScheduler.h>
#include <Bentley/CancellationToken.h>
#include <queue>

BEGIN_BENTLEY_TASKS_NAMESPACE

//--------------------------------------------------------------------------------------+
// Class that simplifies ensuring that only one task is running for given id.
// maxQueue - enables cases where only one task can be ran at the time, but one needs ensure
// that after call addtional work will always be done.
// @bsiclass                                                     Bentley Systems
//---------------+---------------+---------------+---------------+---------------+------+
template <typename ID, typename T, size_t maxQueue = 0>
struct UniqueTasksHolder
    {
    static_assert(maxQueue <= 1, "More than 1 queued tasks is not yet supported");

    private:
        struct QueuedTask
            {
            AsyncTaskPtr<void> runTask;
            AsyncTaskPtr<T> resultTask;
            };

        struct Task
            {
            SimpleCancellationTokenPtr token;
            AsyncTaskPtr<T> runningTask;
            std::queue<QueuedTask> queue;
            };

        struct Impl
            {
            BeMutex taskCS;
            bmap<ID, std::shared_ptr<Task>> tasks;
            };

    private:
        std::shared_ptr<Impl> m_impl;

    public:
        UniqueTasksHolder() : m_impl(std::make_shared<Impl>()) {}

        //! Ensure that only one task is running for given id. Will return running task or get new one using given callback.
        //! @param[in] id unique task id
        //! @param[in] executeNewTaskCallback Callback that should start and return new async task if needed
        AsyncTaskPtr<T> GetTask(const ID& id, std::function<AsyncTaskPtr<T>(ICancellationTokenPtr token)> executeNewTaskCallback)
            {
            auto impl = m_impl;

            BeMutexHolder mutex(impl->taskCS);

            AsyncTaskPtr<T> resultTask;

            auto it = impl->tasks.find(id);
            if (it != impl->tasks.end())
                {
                auto task = it->second;
                if (maxQueue == 0)
                    {
                    resultTask = task->runningTask;
                    }
                else
                    {
                    if (task->queue.size() < maxQueue)
                        {
                        auto finalResult = std::make_shared<T>();
                        QueuedTask queuedTask;
                        queuedTask.runTask = std::make_shared<PackagedAsyncTask<void>>([=]
                            {
                            GetTask(id, executeNewTaskCallback)->Then([=] (T result)
                                {
                                *finalResult = result;
                                });
                            });
                        queuedTask.resultTask = queuedTask.runTask->template Then<T>([=]
                            {
                            return *finalResult;
                            });
                        task->queue.push(queuedTask);
                        }

                    resultTask = task->queue.back().resultTask;
                    }
                }
            else
                {
                auto task = std::make_shared<Task>();
                task->token = SimpleCancellationToken::Create();
                task->runningTask = executeNewTaskCallback(task->token);
                task->runningTask->Then([=] (const T&)
                    {
                    BeMutexHolder mutex(impl->taskCS);

                    if (!task->queue.empty())
                        {
                        QueuedTask queuedTask = task->queue.front();
                        task->queue.pop();
                        AsyncTasksManager::GetDefaultScheduler()->Push(queuedTask.runTask);
                        }

                    auto it = impl->tasks.find(id);
                    if (it != impl->tasks.end() && it->second.get() == task.get())
                        impl->tasks.erase(id);
                    });

                if (!task->runningTask->IsCompleted())
                    impl->tasks[id] = task;

                resultTask = task->runningTask;
                }

            auto parentTask = AsyncTasksManager::GetCurrentThreadAsyncTask();
            if (nullptr != parentTask)
                parentTask->AddSubTask(resultTask);

            return resultTask;
            }

        // Cancel currently running task. Returns task if found, nullptr if not.
        AsyncTaskPtr<T> CancelTask(const ID& id)
            {
            BeMutexHolder mutex(m_impl->taskCS);
            AsyncTaskPtr<T> task;
            auto it = m_impl->tasks.find(id);
            if (it != m_impl->tasks.end())
                {
                task = it->second->runningTask;
                it->second->token->SetCanceled();
                m_impl->tasks.erase(id);
                }
            return task;
            }

        // Check if task is running at a given moment
        bool IsTaskRunning(const ID& id)
            {
            BeMutexHolder mutex(m_impl->taskCS);
            auto it = m_impl->tasks.find(id);
            return it != m_impl->tasks.end();
            }
    };

//--------------------------------------------------------------------------------------+
// Simplified version for one task without identifier
// @bsiclass                                                     Bentley Systems
//---------------+---------------+---------------+---------------+---------------+------+

template <typename T, size_t maxQueue = 0>
struct UniqueTaskHolder : private UniqueTasksHolder<int, T, maxQueue>
    {
    public:
        //! Ensure that only one task is running. Will return running task or get new one using given callback.
        //! @param[in] executeNewTaskCallback Callback that should start and return new async task if needed
        AsyncTaskPtr<T> GetTask(std::function<AsyncTaskPtr<T>()> executeNewTaskCallback)
            {
            return UniqueTasksHolder<int, T, maxQueue>::GetTask(0, [=] (ICancellationTokenPtr token)
                {
                return executeNewTaskCallback();
                });
            }
    };

END_BENTLEY_TASKS_NAMESPACE
