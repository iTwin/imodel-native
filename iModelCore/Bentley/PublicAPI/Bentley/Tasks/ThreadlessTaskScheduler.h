/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/TaskScheduler.h>

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma      04/18
* For internal use only.
* Scheduler that does not use its own threads.
* Can be used to execute small sections of code that needs to avoid using additional or default threads.
* When used with task->Then() - code is executed in same thread as task or current thread if task is already completed.
* When used with ExecuteAsync() or ExecuteAsyncWithoutAttachingToCurrentTask() - code is exectued in current thread.
+---------------+---------------+---------------+---------------+---------------+------*/
struct ThreadlessTaskScheduler : ITaskScheduler
    {
    void Push(std::shared_ptr<AsyncTask> task, AsyncTask::Priority priority = AsyncTask::Priority::Inherited)
        {
        Push(task, AsyncTasksManager::GetCurrentThreadAsyncTask(), priority);
        };
    void Push(std::shared_ptr<AsyncTask> task, std::shared_ptr<AsyncTask> parentTask, AsyncTask::Priority priority = AsyncTask::Priority::Inherited)
        {
        if (parentTask)
            parentTask->AddSubTask(task);
        task->Execute();
        };
    std::shared_ptr<AsyncTask> WaitAndPop() { return nullptr; };
    std::shared_ptr<AsyncTask> TryPop() { return nullptr; };
    int GetQueueTaskCount() const { return 0; };
    bool HasRunningTasks() const { return false; };
    AsyncTaskPtr<void> OnEmpty() const { return CreateCompletedAsyncTask(); };
    };

END_BENTLEY_TASKS_NAMESPACE
