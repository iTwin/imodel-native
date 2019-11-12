/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/Tasks/Tasks.h>
#include <Bentley/Tasks/ThreadlessTaskScheduler.h>
#include <Bentley/Tasks/WorkerThread.h>

#include "AsyncTestCheckpoint.h"

USING_NAMESPACE_BENTLEY_TASKS

struct ThreadlessTaskSchedulerTests : ::testing::Test {};

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(ThreadlessTaskSchedulerTests, WaitAndPop_Empty_ReturnsNullAsTasksAreNotQueued)
    {
    auto scheduler = std::make_shared<ThreadlessTaskScheduler>();
    EXPECT_TRUE(nullptr == scheduler->WaitAndPop());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(ThreadlessTaskSchedulerTests, TryPop_Empty_ReturnsNullAsTasksAreNotQueued)
    {
    auto scheduler = std::make_shared<ThreadlessTaskScheduler>();
    EXPECT_TRUE(nullptr == scheduler->TryPop());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(ThreadlessTaskSchedulerTests, GetQueueTaskCount_Empty_ReturnsZeroAsTasksAreNotQueued)
    {
    auto scheduler = std::make_shared<ThreadlessTaskScheduler>();
    EXPECT_EQ(0, scheduler->GetQueueTaskCount());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(ThreadlessTaskSchedulerTests, OnEmpty_Empty_ReturnsCompletedTaskAsTasksAreNotQueued)
    {
    auto scheduler = std::make_shared<ThreadlessTaskScheduler>();
    EXPECT_TRUE(scheduler->OnEmpty()->IsCompleted());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(ThreadlessTaskSchedulerTests, HasRunningTasks_Empty_False)
    {
    auto scheduler = std::make_shared<ThreadlessTaskScheduler>();
    EXPECT_FALSE(scheduler->HasRunningTasks());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(ThreadlessTaskSchedulerTests, Push_MultipleTasks_ExecutedInOriginalOrder)
    {
    auto scheduler = std::make_shared<ThreadlessTaskScheduler>();

    BeAtomic<int> i(0);

    auto t1 = std::make_shared<PackagedAsyncTask<void>>([&]
        {
        EXPECT_EQ(1, (int)++i);
        });
    auto t2 = std::make_shared<PackagedAsyncTask<void>>([&]
        {
        EXPECT_EQ(2, (int)++i);
        });
    auto t3 = std::make_shared<PackagedAsyncTask<void>>([&]
        {
        EXPECT_EQ(3, (int)++i);
        });

    scheduler->Push(t1);
    scheduler->Push(t2);
    scheduler->Push(t3);

    t1->Wait();
    t2->Wait();
    t3->Wait();

    EXPECT_EQ(3, i);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(ThreadlessTaskSchedulerTests, Push_MultipleThenTasks_ExecutedInOriginalOrder)
    {
    auto scheduler = std::make_shared<ThreadlessTaskScheduler>();

    BeAtomic<int> i(0);

    auto t1 = std::make_shared<PackagedAsyncTask<void>>([&]
        {
        EXPECT_EQ(1, (int)++i);
        });
    auto t2 = t1->Then([&]
        {
        EXPECT_EQ(2, (int)++i);
        });
    auto t3 = t2->Then([&]
        {
        EXPECT_EQ(3, (int)++i);
        });

    scheduler->Push(t1);

    t1->Wait();
    t2->Wait();
    t3->Wait();

    EXPECT_EQ(3, i);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(ThreadlessTaskSchedulerTests, Push_ViaThenForNotCompletedTask_TaskIsExecutedInsideTasksThread)
    {
    auto scheduler = std::make_shared<ThreadlessTaskScheduler>();
    auto thread = WorkerThread::Create("TestThread");

    AsyncTestCheckpoint cp1, cp2;

    auto task = thread->ExecuteAsync([&]
        {
        cp1.CheckinAndWait();
        })
        ->Then(scheduler, [&]
            {
            EXPECT_EQ(thread->GetThreadId(), BeThreadUtilities::GetCurrentThreadId());
            cp2.Checkin();
            });

        cp1.WaitUntilReached();
        cp1.Continue();

        task->Wait();
        thread->OnEmpty()->Wait();
        EXPECT_TRUE(cp2.WasReached());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(ThreadlessTaskSchedulerTests, Push_ViaThen_TaskIsExecutedInSameThread)
    {
    auto scheduler = std::make_shared<ThreadlessTaskScheduler>();
    auto thread = WorkerThread::Create("TestThread");

    AsyncTestCheckpoint cp2;

    auto task0 = thread->ExecuteAsync([&] {});
    task0->Wait();

    auto currentThreadId = BeThreadUtilities::GetCurrentThreadId();

    auto task = task0->Then(scheduler, [&]
        {
        EXPECT_EQ(currentThreadId, BeThreadUtilities::GetCurrentThreadId());
        cp2.Checkin();
        });

    task->Wait();
    thread->OnEmpty()->Wait();
    EXPECT_TRUE(cp2.WasReached());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(ThreadlessTaskSchedulerTests, Push_WithinParentTask_ParentExecutionProlonged)
    {
    auto scheduler = std::make_shared<ThreadlessTaskScheduler>();
    auto thread1 = WorkerThread::Create("TestThread");
    auto thread2 = WorkerThread::Create("TestThread");

    AsyncTestCheckpoint cp;

    auto task = thread1->ExecuteAsync([&]
        {
        AsyncTestCheckpoint delay;
        thread2->ExecuteAsync([&]
            {
            delay.CheckinAndWait();
            })
            ->Then(scheduler, [&]
                {
                cp.CheckinAndWait();
                });
            delay.WaitUntilReached();
            delay.Continue();
        });

    cp.WaitUntilReached();
    BeThreadUtilities::BeSleep(100);
    EXPECT_FALSE(task->IsCompleted());
    cp.Continue();

    task->Wait();
    thread1->OnEmpty()->Wait();
    thread2->OnEmpty()->Wait();
    }
