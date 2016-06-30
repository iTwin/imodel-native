/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/UniqueTaskHolderTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UniqueTaskHolderTests.h"

#include <Bentley/Tasks/UniqueTaskHolder.h>
#include <Bentley/Tasks/WorkerThread.h>

TEST_F (UniqueTaskHolderTests, GetRunningTask_CalledTwice_ExecutesJustFirstTask)
    {
    auto thread = WorkerThread::Create ("TestThread");

    UniqueTaskHolder<int> taskHolder;

    bset<std::shared_ptr<AsyncTask>> tasks;
    BeAtomic<bool> block (true);

    tasks.insert (taskHolder.GetTask ([&]
        {
        return
            thread->ExecuteAsync<int> ([&]
            {
            while (block);
            return 0;
            });
        }));

    tasks.insert (taskHolder.GetTask ([&]
        {
        BeAssert (false);
        return nullptr;
        }));

    block.store(false);

    AsyncTask::WhenAll (tasks)->Wait ();
    }

TEST_F (UniqueTaskHolderTests, GetRunningTask_CalledSecondTimeAfterFirstFinished_ExecutesSecondTime)
    {
    auto thread = WorkerThread::Create ("TestThread");

    UniqueTaskHolder<int> taskHolder;

    BeAtomic<int> i (0);

    taskHolder.GetTask ([&]
        {
        return thread->ExecuteAsync<int> ([&]
            {
            EXPECT_EQ (1, ++i);
            return 0;
            });
        })->Wait ();

    taskHolder.GetTask ([&]
        {
        return thread->ExecuteAsync<int> ([&]
            {
            EXPECT_EQ (2, ++i);
            return 0;
            });
        })->Wait ();

    EXPECT_EQ (2, i);
    }

TEST_F (UniqueTaskHolderTests, GetRunningTask_CalledTwiceWithThenTasks_ReturnsValueToBothThens)
    {
    auto thread = WorkerThread::Create ("TestThread");

    UniqueTaskHolder<Utf8String> taskHolder;

    bset<std::shared_ptr<AsyncTask>> tasks;

    BeAtomic<int> i (0);
    BeAtomic<bool> block (true);

    tasks.insert (taskHolder.GetTask ([&]
        {
        return
            thread->ExecuteAsync<Utf8String> ([&]
            {
            while (block);
            return "UniqueTaskResult";
            });
        })
            ->Then ([&] (Utf8String result)
            {
            EXPECT_EQ (1, ++i);
            EXPECT_EQ ("UniqueTaskResult", result);
            }));

        tasks.insert (taskHolder.GetTask (nullptr)
                      ->Then ([&] (Utf8String result)
            {
            EXPECT_EQ (2, ++i);
            EXPECT_EQ ("UniqueTaskResult", result);
            }));

    block.store(false);

    AsyncTask::WhenAll (tasks)->Wait ();
    EXPECT_EQ (2, i);
    }

TEST_F (UniqueTaskHolderTests, GetRunningTask_CalledOnceAndExecutorDeleted_DoesNotCrashAndExecutesTask)
    {
    auto thread = WorkerThread::Create ("TestThread");

    auto taskHolder = new UniqueTaskHolder<int> ();

    BeAtomic<bool> block (true);
    auto task = taskHolder->GetTask ([&]
        {
        return
            thread->ExecuteAsync<int> ([&]
            {
            while (block);
            return 42;
            });
        });

    delete taskHolder;
    block.store(false);

    EXPECT_EQ (42, task->GetResult ());
    }

