/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Tasks/TaskScheduler.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <Bentley/Tasks/ConcurrentQueue.h>
#include <Bentley/bset.h>

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ITaskScheduler 
    {
    virtual ~ITaskScheduler ()
        {
        }

    virtual void Push (std::shared_ptr<AsyncTask> task, AsyncTask::Priority priority = AsyncTask::Priority::Inherited) = 0;
    virtual void Push (std::shared_ptr<AsyncTask> task, std::shared_ptr<AsyncTask> parentTask, AsyncTask::Priority priority = AsyncTask::Priority::Inherited) = 0;
    virtual std::shared_ptr<AsyncTask> WaitAndPop () = 0;
    virtual std::shared_ptr<AsyncTask> TryPop () = 0;
    virtual int GetQueueTaskCount () const = 0;
    virtual bool HasRunningTasks () const = 0;
    virtual AsyncTaskPtr<void> OnEmpty () const = 0;

    //! Create new task with callback code and execute it 
    template <class T> std::shared_ptr<PackagedAsyncTask<T>> ExecuteAsync (const std::function<T ()>& taskCallback, AsyncTask::Priority priority = AsyncTask::Priority::Inherited)
        {
        auto task = std::make_shared<PackagedAsyncTask<T>> (taskCallback);
        ASYNC_TASK_ADD_DEBUG_INFO(task, 2);
        Push (task, priority);
        return task;
        }

    //! Create new task with callback code and execute it 
    std::shared_ptr<PackagedAsyncTask<void>> ExecuteAsync (const std::function<void (void)>& taskCallback, AsyncTask::Priority priority = AsyncTask::Priority::Inherited)
        {
        auto task = std::make_shared<PackagedAsyncTask<void>> (taskCallback);
        ASYNC_TASK_ADD_DEBUG_INFO(task, 2);
        Push (task, priority);
        return task;
        }

    //! Create new task with callback code and execute it without attaching to current task as subtask.
    //! Current task completion will not be blocked by this new task. 
    template <class T> std::shared_ptr<PackagedAsyncTask<T>> ExecuteAsyncWithoutAttachingToCurrentTask (const std::function<T ()>& taskCallback, AsyncTask::Priority priority = AsyncTask::Priority::Inherited)
        {
        auto task = std::make_shared<PackagedAsyncTask<T>> (taskCallback);
        ASYNC_TASK_ADD_DEBUG_INFO(task, 2);
        Push (task, nullptr, priority);
        return task;
        }

    //! Create new task with callback code and execute it without attaching to current task as subtask.
    //! Current task completion will not be blocked by this new task. 
    std::shared_ptr<PackagedAsyncTask<void>> ExecuteAsyncWithoutAttachingToCurrentTask (const std::function<void (void)>& taskCallback, AsyncTask::Priority priority = AsyncTask::Priority::Inherited)
        {
        auto task = std::make_shared<PackagedAsyncTask<void>> (taskCallback);
        ASYNC_TASK_ADD_DEBUG_INFO(task, 2);
        Push (task, nullptr, priority);
        return task;
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct TaskScheduler : ITaskScheduler, public std::enable_shared_from_this<TaskScheduler>, OnAsyncTaskCompletedListener
    {
    private:
        AsyncTaskPtr<void> m_onEmptyTask;

    private:
        void RegisterTaskStart (std::shared_ptr<AsyncTask> task);
        void ResetOnEmptyTask ();
        bool ProtectedHasRunningTasks () const;        

    protected:
        ConcurrentQueue<std::shared_ptr<AsyncTask>>   m_taskQueue;

        bset<std::shared_ptr<AsyncTask>>             m_startedTasks;

    protected:
        void _OnAsyncTaskCompleted (std::shared_ptr<struct AsyncTask> task) override;
        void virtual _OnEmpty () {} ;

    public:
        BENTLEYDLL_EXPORT TaskScheduler ();
        BENTLEYDLL_EXPORT virtual ~TaskScheduler ();

        //! Push task for execution to current parent task if any
        BENTLEYDLL_EXPORT virtual void Push (std::shared_ptr<AsyncTask> task, AsyncTask::Priority priority = AsyncTask::Priority::Inherited) override;

        //! Push task for execution to specific parent task
        BENTLEYDLL_EXPORT virtual void Push (std::shared_ptr<AsyncTask> task, std::shared_ptr<AsyncTask> parentTask, AsyncTask::Priority priority = AsyncTask::Priority::Inherited) override;

        //! Blocks and returns sheduled task 
        BENTLEYDLL_EXPORT virtual std::shared_ptr<AsyncTask> WaitAndPop () override;

        //! Returns sheduled task or null if no sheduled tasks found
        BENTLEYDLL_EXPORT virtual std::shared_ptr<AsyncTask> TryPop () override;

        BENTLEYDLL_EXPORT virtual int GetQueueTaskCount () const override;

        //! Check if sheduler has running tasks.
        BENTLEYDLL_EXPORT virtual bool HasRunningTasks () const override;

        BENTLEYDLL_EXPORT virtual AsyncTaskPtr<void> OnEmpty () const override;
    };

END_BENTLEY_TASKS_NAMESPACE
