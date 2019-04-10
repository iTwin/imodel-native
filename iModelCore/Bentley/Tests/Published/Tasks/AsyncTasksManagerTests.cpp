/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/AsyncTasksManagerTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/Tasks/Tasks.h>

#include <Bentley/BeThread.h>

#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <Bentley/Tasks/WorkerThread.h>

#include "AsyncTestCheckpoint.h"

USING_NAMESPACE_BENTLEY_TASKS

class AsyncTasksManagerTests : public ::testing::Test {};

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksManagerTests, OnAllShedulersEmpty_TwoThreads_ReturnsWhenThreadsFinishTasks)
    {
    auto t1 = WorkerThread::Create("T1");
    auto t2 = WorkerThread::Create("T2");

    AsyncTestCheckpoint cp1;
    AsyncTestCheckpoint cp2;
    t1->ExecuteAsync([&]
        {
        cp1.CheckinAndWait();
        });
    t2->ExecuteAsync([&]
        {
        cp2.CheckinAndWait();
        });
    cp1.WaitUntilReached();
    cp2.WaitUntilReached();

    auto task = AsyncTasksManager::OnAllSchedulersEmpty();
    BeThreadUtilities::BeSleep(10);
    EXPECT_FALSE(task->IsCompleted());

    cp1.Continue();
    cp2.Continue();

    task->Wait();

    EXPECT_FALSE(t1->HasRunningTasks());
    EXPECT_FALSE(t2->HasRunningTasks());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksManagerTests, OnAllShedulersEmpty_DefaultThread_ReturnsWhenThreadsFinishTasks)
    {
    auto t1 = AsyncTasksManager::GetDefaultScheduler();

    AsyncTestCheckpoint cp1;
    t1->ExecuteAsync([&]
        {
        cp1.CheckinAndWait();
        });
    cp1.WaitUntilReached();

    auto task = AsyncTasksManager::OnAllSchedulersEmpty();
    BeThreadUtilities::BeSleep(10);
    EXPECT_FALSE(task->IsCompleted());

    cp1.Continue();

    task->Wait();

    EXPECT_FALSE(t1->HasRunningTasks());
    }