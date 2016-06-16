/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/UniqueTasksHolderTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UniqueTasksHolderTests.h"
#include "TasksTestsHelper.h"
#include <Bentley/Tasks/UniqueTaskHolder.h>
#include <Bentley/Tasks/WorkerThread.h>

template<typename T>
static bset<T> StubBSet (std::initializer_list<T> list)
    {
    bset<T> set (list.begin (), list.end ());
    return set;
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (UniqueTasksHolderTests, GetTask_CalledWithDifferentIds_ReturnsDifferentTasks)
    {
    UniqueTasksHolder<int, int> holder;

    int timesCalled = 0;
    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        timesCalled++;
        return std::make_shared<PackagedAsyncTask<int>> ([=]
            {
            return 1;
            });
        };

    auto t1 = holder.GetTask (1, newTaskCallback);
    auto t2 = holder.GetTask (2, newTaskCallback);

    EXPECT_FALSE (t1.get () == t2.get ());
    EXPECT_EQ (2, timesCalled);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (UniqueTasksHolderTests, GetTask_CalledWithSameId_ReturnsSameTaskWithoutCallingNewTaskCallbackSecondTime)
    {
    UniqueTasksHolder<int, int> holder;

    int timesCalled = 0;
    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        timesCalled++;
        return std::make_shared<PackagedAsyncTask<int>> ([=]
            {
            return 1;
            });
        };

    auto t1 = holder.GetTask (1, newTaskCallback);
    auto t2 = holder.GetTask (1, newTaskCallback);

    EXPECT_TRUE (t1.get () == t2.get ());
    EXPECT_EQ (1, timesCalled);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (UniqueTasksHolderTests, GetTask_CalledTwiceInSequence_ReturnsSameTaskAndParentTaskGetsBlockedByExistingTaskRetrieval)
    {
    auto thread = WorkerThread::Create();
    auto thread2 = WorkerThread::Create();
    UniqueTasksHolder<int, int> holder;

    AsyncTestCheckpoint checkpointA;
    AsyncTestCheckpoint checkpointB;

    int timesCalled = 0;
    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        timesCalled++;
        return thread2->ExecuteAsync<int> ([&]
            {
            checkpointA.CheckinAndWait ();
            return 42;
            });
        };

    auto t1 = holder.GetTask (1, newTaskCallback);

    bool thenOnParentTaskCalled = false;
    bool thenOnSubTaskCalled = false;

    auto t2 = thread->ExecuteAsync ([&]
        { 
        holder.GetTask (1, newTaskCallback)
        ->Then ([&] (int)
            {
            thenOnSubTaskCalled = true;
            });
        checkpointB.CheckinAndWait ();
        })
    ->Then ([&]
        {
        thenOnParentTaskCalled = true;
        EXPECT_TRUE (thenOnSubTaskCalled);
        });

    checkpointB.WaitUntilReached ();
    checkpointB.Continue ();

    checkpointA.WaitUntilReached ();
    checkpointA.Continue ();

    AsyncTask::WhenAll (StubBSet<std::shared_ptr<AsyncTask>> ({t1, t2}))->Wait ();
    EXPECT_EQ (1, timesCalled);
    EXPECT_TRUE (thenOnSubTaskCalled);
    EXPECT_TRUE (thenOnParentTaskCalled);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (UniqueTasksHolderTests, GetTask_CalledAfterTaskIsFinished_ReturnsDifferentTask)
    {
    UniqueTasksHolder<int, int> holder;

    int timesCalled = 0;
    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        timesCalled++;
        return std::make_shared<PackagedAsyncTask<int>> ([=]
            {
            return 1;
            });
        };

    auto t1 = holder.GetTask (1, newTaskCallback);
    t1->Execute ();
    t1->Wait ();

    auto t2 = holder.GetTask (1, newTaskCallback);

    EXPECT_FALSE (t1.get () == t2.get ());
    EXPECT_EQ (2, timesCalled);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (UniqueTasksHolderTests, GetTask_CalledAfterTaskIsCanceled_ReturnsDifferentTask)
    {
    UniqueTasksHolder<int, int> holder;

    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        return std::make_shared<PackagedAsyncTask<int>> ([=]
            {
            return 1;
            });
        };

    auto t1 = holder.GetTask (1, newTaskCallback);
    holder.CancelTask (1);
    auto t2 = holder.GetTask (1, newTaskCallback);

    EXPECT_FALSE (t1.get () == t2.get ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (UniqueTasksHolderTests, GetTask_CalledAfterSameIdTaskIsCanceledButIsNotFinishedYet_SecondTaskCanAlsoBeCanceled)
    {
    auto thread1 = WorkerThread::Create ();
    auto thread2 = WorkerThread::Create ();

    UniqueTasksHolder<int, int> holder;
    AsyncTestCheckpoint a, b, c;

    auto t1 = holder.GetTask (42, [&] (ICancellationTokenPtr token)
        {
        EXPECT_FALSE(token->IsCanceled ());
        return thread1->ExecuteAsync<int> ([&, token]
            {
            a.CheckinAndWait ();
            EXPECT_TRUE(token->IsCanceled ());
            b.CheckinAndWait ();
            return 0;
            });
        });

    a.WaitUntilReached ();
    holder.CancelTask (42);
    a.Continue ();

    auto t2 = holder.GetTask (42, [&] (ICancellationTokenPtr token)
        {
        EXPECT_FALSE (token->IsCanceled ());
        return thread2->ExecuteAsync<int> ([&, token]
            {
            c.CheckinAndWait ();
            EXPECT_TRUE (token->IsCanceled ());
            return 0;
            });
        });

    b.Continue ();
    t1->Wait ();

    holder.CancelTask (42);

    c.Continue ();
    t2->Wait ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (UniqueTasksHolderTests, CancelTask_CalledForExistingTask_TaskTokenIsSetToCanceled)
    {
    UniqueTasksHolder<int, int> holder;

    auto t1 = holder.GetTask (1, [&] (ICancellationTokenPtr token)
        {
        return std::make_shared<PackagedAsyncTask<int>> ([=]
            {
            EXPECT_TRUE (token->IsCanceled ());

            return 1;
            });
        });
    holder.CancelTask (1);

    t1->Execute ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (UniqueTasksHolderTests, CancelTask_CalledForExistingTask_TaskTokenForNewTaskIsNotCanceled)
    {
    UniqueTasksHolder<int, int> holder;

    auto t1 = holder.GetTask (1, [&] (ICancellationTokenPtr token)
        {
        return std::make_shared<PackagedAsyncTask<int>> ([=]
            {
            EXPECT_TRUE (token->IsCanceled ());

            return 1;
            });
        });
    holder.CancelTask (1);

    auto t2 = holder.GetTask (1, [&] (ICancellationTokenPtr token)
        {
        return std::make_shared<PackagedAsyncTask<int>> ([=]
            {
            EXPECT_FALSE (token->IsCanceled ());

            return 1;
            });
        });

    t2->Execute ();
    t1->Execute ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (UniqueTasksHolderTests, IsTaskRunning_CalledForNotExistingTask_False)
    {
    UniqueTasksHolder<int, int> holder;

    EXPECT_FALSE (holder.IsTaskRunning (42));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (UniqueTasksHolderTests, IsTaskRunning_FinishedTask_False)
    {
    auto thread = WorkerThread::Create ();
    UniqueTasksHolder<int, int> holder;

    auto t1 = holder.GetTask (42, [&] (ICancellationTokenPtr token)
        {
        return thread->ExecuteAsync<int> ([]
            {
            return 0;
            });
        });
    t1->Wait ();

    EXPECT_FALSE (holder.IsTaskRunning (42));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (UniqueTasksHolderTests, IsTaskRunning_TaskNotYetFinished_True)
    {
    auto thread = WorkerThread::Create ();

    UniqueTasksHolder<int, int> holder;
    AsyncTestCheckpoint a;

    auto t1 = holder.GetTask (42, [&] (ICancellationTokenPtr token)
        {
        return thread->ExecuteAsync<int> ([&]
            {
            a.CheckinAndWait ();
            return 0;
            });
        });

    a.WaitUntilReached ();
    EXPECT_TRUE (holder.IsTaskRunning (42));

    a.Continue ();
    t1->Wait ();

    EXPECT_FALSE (holder.IsTaskRunning (42));
    }
