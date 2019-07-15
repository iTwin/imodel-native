/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "AsyncTaskTests.h"

#include <Bentley/BeDebugLog.h>
#include <Bentley/BeTimeUtilities.h>

#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/WorkerThread.h>
#include <Bentley/Tasks/TaskScheduler.h>

#include <atomic>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, IsCompleted_TaskNotExecuted_False)
    {
    auto task = std::make_shared<PackagedAsyncTask<void>>([] { });
    EXPECT_FALSE(task->IsCompleted());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, IsCompleted_TaskExecuted_True)
    {
    auto task = std::make_shared<PackagedAsyncTask<void>>([] {});
    task->Execute();
    EXPECT_TRUE(task->IsCompleted());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, Wait_TaskPushedToScheduler_BlocksUntilTaskIsExecuted)
    {
    bool executed = false;

    auto workerThread = WorkerThread::Create();
    auto task = workerThread->ExecuteAsync([&] { executed = true; });
    
    task->Wait();

    EXPECT_TRUE(executed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, Wait_OnExecutedTask_DoesNotBlock)
    {
    bool executed = false;

    auto task = std::make_shared<PackagedAsyncTask<void>>([&] { executed = true; });

    task->Execute();

    task->Wait();
    task->Wait(); // Test if multiple calls to Wait() does not block.

    EXPECT_TRUE(executed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, GetPriority_ConstructedWithoutPriority_PriorityNormal)
    {
    auto task = std::make_shared<PackagedAsyncTask<void>>([] {});
    EXPECT_TRUE(task->GetPriority() == AsyncTask::Priority::Normal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, GetPriority_Constructed_PriorityNormalByDefault)
    {
    auto task = std::make_shared<PackagedAsyncTask<void>>([] {});
    EXPECT_TRUE(task->GetPriority() == AsyncTask::Priority::Normal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, Then_ThenCallbackAdded_ExecutedAfterOriginalTaskWasExecuted)
    {
    BeAtomic<int> result (0);

    auto task = std::make_shared<PackagedAsyncTask<void>>([&] { result.store(1); });

    auto lastTask = task->Then([&] { result.store(2); });

    result.store(3);

    task->Execute();

    lastTask->Wait();

    EXPECT_TRUE(result == 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, Then_MultipleThensChained_AllExecutedInCorrectOrder)
    {
    BeAtomic<int> lastCompletedThenTask (0);

    auto task = std::make_shared<PackagedAsyncTask<void>>([&] {});

    auto lastTask = 
    task->Then([&] 
            {
            EXPECT_TRUE(lastCompletedThenTask == 0);
            lastCompletedThenTask.store(1);
            })
        ->Then([&] 
            {
            EXPECT_TRUE(lastCompletedThenTask == 1);
            lastCompletedThenTask.store(2);
            })
        ->Then([&] 
            {
            EXPECT_TRUE(lastCompletedThenTask == 2);
            lastCompletedThenTask.store(3);
            });

    task->Execute();

    lastTask->Wait();

    EXPECT_TRUE(lastCompletedThenTask == 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, Then_MultipleChildrenAdded_ThenExecutedAfterAllChildrenTasksCompleted)
    {
    BeAtomic<int> childrenCompleted (0);

    auto task = std::make_shared<PackagedAsyncTask<void>>([&] { });

    auto childrenTask1 = std::make_shared<PackagedAsyncTask<void>>([&] { childrenCompleted++; });
    auto childrenTask2 = std::make_shared<PackagedAsyncTask<void>>([&] { childrenCompleted++; });
    childrenTask2->Then([&]
        { 
        childrenCompleted++; 
        });

    task->AddSubTask(childrenTask1);
    task->AddSubTask(childrenTask2);

    bool thenExecuted = false;
    auto lastTask = task->Then([&]
        {
        EXPECT_TRUE(childrenCompleted == 3); 
        thenExecuted = true;
        });

    task->Execute();
    childrenTask1->Execute();
    childrenTask2->Execute();

    lastTask->Wait();

    EXPECT_TRUE(thenExecuted);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, WhenAll_MultipleTasksUsed_ThenExecutedAfterAllTasksCompleted)
    {
    BeAtomic<int> tasksCompleted(0);

    auto task1 = std::make_shared<PackagedAsyncTask<void>>([&] { tasksCompleted++; });
    auto task2 = std::make_shared<PackagedAsyncTask<void>>([&] { tasksCompleted++; });

    bool thenExecuted = false;
    auto lastTask = AsyncTask::WhenAll(task1, task2)->Then([&]
        {
        EXPECT_TRUE(tasksCompleted == 2);
        thenExecuted = true;
        });

    task1->Execute();
    task2->Execute();

    lastTask->Wait();

    EXPECT_TRUE(thenExecuted);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, WhenAll_MultipleTasksAdded_ThenExecutedAfterAllTasksCompleted)
    {
    BeAtomic<int> tasksCompleted (0);

    auto task1 = std::make_shared<PackagedAsyncTask<void>>([&] { tasksCompleted++; });
    auto task2 = std::make_shared<PackagedAsyncTask<void>>([&] { tasksCompleted++; });

    bset<std::shared_ptr<AsyncTask>> tasks;
    tasks.insert (task1);
    tasks.insert (task2);

    bool thenExecuted = false;
    auto lastTask = AsyncTask::WhenAll(tasks)->Then([&]
        {
        EXPECT_TRUE(tasksCompleted == 2);
        thenExecuted = true;
        });

    task1->Execute();
    task2->Execute();

    lastTask->Wait();

    EXPECT_TRUE(thenExecuted);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, WhenAll_MultipleTasksInVectorAdded_ThenExecutedAfterAllTasksCompleted)
    {
    BeAtomic<int> tasksCompleted(0);

    auto task1 = std::make_shared<PackagedAsyncTask<void>>([&] { tasksCompleted++; });
    auto task2 = std::make_shared<PackagedAsyncTask<void>>([&] { tasksCompleted++; });

    bvector<std::shared_ptr<PackagedAsyncTask<void>>> tasks;
    tasks.push_back(task1);
    tasks.push_back(task2);

    bool thenExecuted = false;
    auto lastTask = AsyncTask::WhenAll(tasks)->Then([&]
        {
        EXPECT_TRUE(tasksCompleted == 2);
        thenExecuted = true;
        });

    task1->Execute();
    task2->Execute();

    lastTask->Wait();

    EXPECT_TRUE(thenExecuted);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, WhenAll_MultipleTasksInSetAddedWithReturnValues_GetResultWorksOnThem)
    {
    bvector<std::shared_ptr<PackagedAsyncTask<int>>> tasks;
    tasks.push_back(std::make_shared<PackagedAsyncTask<int>>([&] { return 1; }));
    tasks.push_back(std::make_shared<PackagedAsyncTask<int>>([&] { return 2; }));

    auto lastTask = AsyncTask::WhenAll(tasks);
    for (auto task : tasks)
        task->Execute();
    lastTask->Wait();

    int i=0;
    for (auto task : tasks)
        EXPECT_EQ(++i, task->GetResult());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsyncTaskTests, Then_MultipleThenCallbackAdded_AllThenCallbacksExecuted)
    {
    auto task = std::make_shared<PackagedAsyncTask<void>> ([&]
        {
        });

    bool task1Executed = false;
    auto thenTask1 = task->Then ([&]
        {
        task1Executed = true;
        });

    bool task2Executed = false;
    auto thenTask2 = task->Then ([&]
        {
        task2Executed = true;
        });

    task->Execute ();

    thenTask1->Wait ();
    thenTask2->Wait ();

    EXPECT_TRUE (task1Executed);
    EXPECT_TRUE (task2Executed);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(AsyncTaskTests, Push_GetCount_Pop)
    {
    bool executed = false;
    auto task = std::make_shared<PackagedAsyncTask<void>>([&]
        {
        executed = true;
        });
    auto task2 = std::make_shared<PackagedAsyncTask<void>>([&]
        {
        executed = true;
        });
    task2->Execute();
    TaskScheduler scheduler;
    ASSERT_FALSE(scheduler.HasRunningTasks());
    ASSERT_EQ(0, scheduler.GetQueueTaskCount());
    scheduler.Push(task);
    scheduler.Push(task2);
    ASSERT_TRUE(scheduler.HasRunningTasks());
    ASSERT_TRUE(2 == scheduler.GetQueueTaskCount());
    task2->Wait();
    ASSERT_TRUE(task2->IsCompleted());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    julius.cepukenas               11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AsyncTaskTests, Then_MultipleTasks_AllExecuted)
    {
    bool taskExecuted = false;
    bool thenExecuted = false;

    auto task = std::make_shared<PackagedAsyncTask<void>>([&]
        {
        taskExecuted = true;
        });

    auto task2 = task->Then([&]
        {
        thenExecuted = true;
        });

    task->Execute();

    task2->Wait();

    ASSERT_TRUE(taskExecuted);
    ASSERT_TRUE(thenExecuted);
    }
