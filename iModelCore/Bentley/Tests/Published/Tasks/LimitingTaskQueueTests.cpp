/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/LimitingTaskQueueTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "LimitingTaskQueueTests.h"
#include <Bentley/Tasks/LimitingTaskQueue.h>
#include <Bentley/Tasks/WorkerThread.h>
#include "TasksTestsHelper.h"

TEST_F (LimitingTaskQueueTests, Push_LimitSetToZeroAndTwoTasks_RunsTasksWhenPushed)
    {
    auto thread1 = WorkerThread::Create ("TestThread");
    auto thread2 = WorkerThread::Create ("TestThread");
    AsyncTestCheckpoint a;

    LimitingTaskQueue<int> queue;
    queue.SetLimit (0);

    int number = 0;
    auto t1 = queue.Push ([&]
        {
        number = 1;
        return thread1->ExecuteAsync<int> ([&]
            {
            a.CheckinAndWait ();
            return 0;
            });
        }, nullptr);

    EXPECT_EQ (1, number);

    a.WaitUntilReached ();

    auto t2 = queue.Push ([&]
        {
        number = 2;
        return thread2->ExecuteAsync<int> ([&]
            {
            return 0;
            });
        }, nullptr);

    EXPECT_EQ (2, number);

    a.Continue ();

    t1->Wait ();
    t2->Wait ();
    }

TEST_F (LimitingTaskQueueTests, Push_LimitSetToOneAndTwoTasks_RunsSecondTaskAfterFirstFinishes)
    {
    auto thread1 = WorkerThread::Create ("TestThread");
    auto thread2 = WorkerThread::Create ("TestThread");
    AsyncTestCheckpoint a;

    LimitingTaskQueue<int> queue;
    queue.SetLimit (1);

    int number = 0;
    auto t1 = queue.Push ([&]
        {
        number = 1;
        return thread1->ExecuteAsync<int> ([&]
            {
            a.CheckinAndWait ();
            return 0;
            });
        }, nullptr);

    EXPECT_EQ (1, number);

    auto t2 = queue.Push ([&]
        {
        number = 2;
        return thread2->ExecuteAsync<int> ([&]
            {
            return 0;
            });
        }, nullptr);

    a.WaitUntilReached ();
    EXPECT_EQ (1, number);
    a.Continue ();

    t1->Wait ();
    t2->Wait ();

    EXPECT_EQ (2, number);
    }

TEST_F (LimitingTaskQueueTests, Push_TaskReturnsValue_SameValueReturned)
    {
    auto thread = WorkerThread::Create ("TestThread");

    LimitingTaskQueue<int> queue;
    queue.SetLimit (0);

    auto result = queue.Push ([&]
        {
        return thread->ExecuteAsync<int> ([&]
            {
            return 42;
            });
        }, nullptr)->GetResult ();

    EXPECT_EQ (42, result);
    }

TEST_F (LimitingTaskQueueTests, Push_QueuedTaskIsCanceled_RunsQuenedTaskOnCancelationEvent)
    {
    auto thread1 = WorkerThread::Create ("TestThread");
    auto thread2 = WorkerThread::Create ("TestThread");
    AsyncTestCheckpoint a;

    LimitingTaskQueue<int> queue;
    queue.SetLimit (1);

    int number = 0;

    auto ct1 = SimpleCancellationToken::Create ();
    auto t1 = queue.Push ([&]
        {
        number = 1;
        return thread1->ExecuteAsync<int> ([&]
            {
            a.CheckinAndWait ();
            return 111;
            });
        }, ct1);

    EXPECT_EQ (1, number);

    auto ct2 = SimpleCancellationToken::Create ();
    auto t2 = queue.Push ([&]
        {
        number = 2;
        return thread2->ExecuteAsync<int> ([&]
            {
            return 222;
            });
        }, ct2);

    a.WaitUntilReached ();
    EXPECT_EQ (1, number);

    ct2->SetCanceled ();
    EXPECT_EQ (2, number);

    a.Continue ();

    EXPECT_EQ (t1->GetResult (), 111);
    EXPECT_EQ (t2->GetResult (), 222);

    EXPECT_EQ (2, number);
    }

TEST_F (LimitingTaskQueueTests, Push_TaskAddedToQueueWithinOtherAsyncTask_OuterAsyncTaskIsBlockedUntilTaskGetsFinished)
    {
    auto thread1 = WorkerThread::Create ("TestThread");
    auto thread2 = WorkerThread::Create ("TestThread2");
    auto thread3 = WorkerThread::Create ("TestThread3");
    AsyncTestCheckpoint a;

    LimitingTaskQueue<int> queue;
    queue.SetLimit (1);

    auto t1 = queue.Push ([&]
        {
        return thread1->ExecuteAsync<int> ([&]
            {
            a.CheckinAndWait ();
            return 0;
            });
        }, nullptr);

    bool outerFinished = false;
    auto outerTask = thread3->ExecuteAsync ([&]
        {
        queue.Push ([&]
            {
            return thread2->ExecuteAsync<int> ([&]
                {
                return 0;
                });
            }, nullptr);
        });

    auto t3 = outerTask->Then ([&]
        {
        outerFinished = true;
        });

    a.WaitUntilReached ();
    thread3->ExecuteAsync ([&]
        {
        // Wait until all previous tasks in thread are finished
        })->Wait ();

    EXPECT_FALSE (outerFinished);

    a.Continue ();

    t1->Wait ();
    t3->Wait ();

    EXPECT_TRUE (outerFinished);
    }
