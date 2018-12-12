/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/StubTaskScheduler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <Bentley/Tasks/Tasks.h>

BEGIN_BENTLEY_TASKS_NAMESPACE

struct StubTaskScheduler : ITaskScheduler
    {
    void Push(std::shared_ptr<AsyncTask> task, AsyncTask::Priority priority)
        {
        EXPECT_TRUE(false);
        task->Execute();
        };
    void Push(std::shared_ptr<AsyncTask> task, std::shared_ptr<AsyncTask> parentTask, AsyncTask::Priority priority)
        {
        EXPECT_TRUE(false);
        if (parentTask)
            parentTask->AddSubTask(task);
        task->Execute();
        };
    std::shared_ptr<AsyncTask> WaitAndPop() { EXPECT_TRUE(false); return nullptr; };
    std::shared_ptr<AsyncTask> TryPop() { EXPECT_TRUE(false); return nullptr; };
    int GetQueueTaskCount() const { EXPECT_TRUE(false); return 0; };
    bool HasRunningTasks() const { EXPECT_TRUE(false); return false; };
    AsyncTaskPtr<void> OnEmpty() const { EXPECT_TRUE(false); return CreateCompletedAsyncTask(); };
    };

END_BENTLEY_TASKS_NAMESPACE
