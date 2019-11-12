/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include "AsyncTestCheckpoint.h"
#include <Bentley/Tasks/UniqueTaskHolder.h>
#include <Bentley/Tasks/WorkerThread.h>

struct UniqueTasksHolderTests : ::testing::Test {};

template<typename T>
static bset<T> StubBSet(std::initializer_list<T> list)
    {
    bset<T> set(list.begin(), list.end());
    return set;
    }

template<typename T>
AsyncTaskPtr<T> StubStartedAsyncTask(T returnValue)
    {
    return std::make_shared<PackagedAsyncTask<T>>([=]
        {
        return returnValue;
        });
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, GetTask_CalledWithDifferentIds_ReturnsDifferentTasks)
    {
    UniqueTasksHolder<int, int> holder;

    int timesCalled = 0;
    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        timesCalled++;
        return StubStartedAsyncTask<int>(1);
        };

    auto t1 = holder.GetTask(1, newTaskCallback);
    auto t2 = holder.GetTask(2, newTaskCallback);

    EXPECT_FALSE(t1.get() == t2.get());
    EXPECT_EQ(2, timesCalled);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, GetTask_CalledWithSameId_ReturnsSameTaskWithoutCallingNewTaskCallbackSecondTime)
    {
    UniqueTasksHolder<int, int> holder;

    int timesCalled = 0;
    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        timesCalled++;
        return StubStartedAsyncTask<int>(1);
        };

    auto t1 = holder.GetTask(1, newTaskCallback);
    auto t2 = holder.GetTask(1, newTaskCallback);

    EXPECT_TRUE(t1.get() == t2.get());
    EXPECT_EQ(1, timesCalled);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, GetTask_CalledTwiceInSequence_ReturnsSameTaskAndParentTaskGetsBlockedByExistingTaskRetrieval)
    {
    UniqueTasksHolder<int, int> holder;

    auto thread = WorkerThread::Create();
    auto thread2 = WorkerThread::Create();

    AsyncTestCheckpoint checkpointA;
    AsyncTestCheckpoint checkpointB;

    int timesCalled = 0;
    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        timesCalled++;
        return thread2->ExecuteAsync<int>([&]
            {
            checkpointA.CheckinAndWait();
            return 42;
            });
        };

    auto t1 = holder.GetTask(1, newTaskCallback);

    bool thenOnParentTaskCalled = false;
    bool thenOnSubTaskCalled = false;

    auto t2 = thread->ExecuteAsync([&]
        {
        holder.GetTask(1, newTaskCallback)
            ->Then([&] (int)
            {
            thenOnSubTaskCalled = true;
            });
        checkpointB.CheckinAndWait();
        })
        ->Then([&]
            {
            thenOnParentTaskCalled = true;
            EXPECT_TRUE(thenOnSubTaskCalled);
            });

        checkpointB.WaitUntilReached();
        checkpointB.Continue();

        checkpointA.WaitUntilReached();
        checkpointA.Continue();

        AsyncTask::WhenAll(StubBSet<std::shared_ptr<AsyncTask>>({ t1, t2 }))->Wait();
        EXPECT_EQ(1, timesCalled);
        EXPECT_TRUE(thenOnSubTaskCalled);
        EXPECT_TRUE(thenOnParentTaskCalled);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, GetTask_CalledAfterTaskIsFinished_ReturnsDifferentTask)
    {
    UniqueTasksHolder<int, int> holder;

    int timesCalled = 0;
    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        timesCalled++;
        return StubStartedAsyncTask<int>(1);
        };

    auto t1 = holder.GetTask(1, newTaskCallback);
    t1->Execute();
    t1->Wait();

    auto t2 = holder.GetTask(1, newTaskCallback);

    EXPECT_FALSE(t1.get() == t2.get());
    EXPECT_EQ(2, timesCalled);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, GetTask_CalledAfterTaskIsCanceled_ReturnsDifferentTask)
    {
    UniqueTasksHolder<int, int> holder;

    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        return StubStartedAsyncTask<int>(1);
        };

    auto t1 = holder.GetTask(1, newTaskCallback);
    holder.CancelTask(1);
    auto t2 = holder.GetTask(1, newTaskCallback);

    EXPECT_FALSE(t1.get() == t2.get());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, GetTask_CalledAfterSameIdTaskIsCanceledButIsNotFinishedYet_SecondTaskCanAlsoBeCanceled)
    {
    auto thread1 = WorkerThread::Create();
    auto thread2 = WorkerThread::Create();
    EXPECT_FALSE(thread1->HasRunningTasks());
    UniqueTasksHolder<int, int> holder;
    AsyncTestCheckpoint a, b, c;

    auto t1 = holder.GetTask(42, [&] (ICancellationTokenPtr token)
        {
        EXPECT_FALSE(token->IsCanceled());
        return thread1->ExecuteAsync<int>([&, token]
            {
            a.CheckinAndWait();
            EXPECT_TRUE(token->IsCanceled());
            b.CheckinAndWait();
            return 0;
            });
        });
    EXPECT_TRUE(thread1->HasRunningTasks());
    a.WaitUntilReached();
    holder.CancelTask(42);
    a.Continue();

    auto t2 = holder.GetTask(42, [&] (ICancellationTokenPtr token)
        {
        EXPECT_FALSE(token->IsCanceled());
        return thread2->ExecuteAsync<int>([&, token]
            {
            c.CheckinAndWait();
            EXPECT_TRUE(token->IsCanceled());
            return 0;
            });
        });

    b.Continue();
    t1->Wait();

    holder.CancelTask(42);

    c.Continue();
    t2->Wait();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, CancelTask_CalledForExistingTask_TaskTokenIsSetToCanceled)
    {
    UniqueTasksHolder<int, int> holder;

    auto t1 = holder.GetTask(1, [&] (ICancellationTokenPtr token)
        {
        return std::make_shared<PackagedAsyncTask<int>>([=]
            {
            EXPECT_TRUE(token->IsCanceled());
            return 1;
            });
        });
    holder.CancelTask(1);

    t1->Execute();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, CancelTask_CalledForExistingTask_ReturnsTask)
    {
    UniqueTasksHolder<int, int> holder;

    auto t1 = holder.GetTask(1, [] (ICancellationTokenPtr token)
        {
        return std::make_shared<PackagedAsyncTask<int>>([] {return 1; });
        });
    auto t2 = holder.CancelTask(1);

    EXPECT_EQ(t2, t1);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, CancelTask_CalledForNotExistingTask_ReturnsNull)
    {
    UniqueTasksHolder<int, int> holder;

    auto t1 = holder.GetTask(1, [] (ICancellationTokenPtr token)
        {
        return std::make_shared<PackagedAsyncTask<int>>([] {return 1; });
        });
    auto t2 = holder.CancelTask(42);

    EXPECT_EQ(t2, nullptr);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, CancelTask_CalledForExistingTask_TaskTokenForNewTaskIsNotCanceled)
    {
    UniqueTasksHolder<int, int> holder;

    auto t1 = holder.GetTask(1, [&] (ICancellationTokenPtr token)
        {
        return std::make_shared<PackagedAsyncTask<int>>([=]
            {
            EXPECT_TRUE(token->IsCanceled());
            return 1;
            });
        });
    holder.CancelTask(1);

    auto t2 = holder.GetTask(1, [&] (ICancellationTokenPtr token)
        {
        return std::make_shared<PackagedAsyncTask<int>>([=]
            {
            EXPECT_FALSE(token->IsCanceled());
            return 1;
            });
        });

    t2->Execute();
    t1->Execute();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, IsTaskRunning_CalledForNotExistingTask_False)
    {
    UniqueTasksHolder<int, int> holder;

    EXPECT_FALSE(holder.IsTaskRunning(42));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, IsTaskRunning_FinishedTask_False)
    {
    auto thread = WorkerThread::Create();
    UniqueTasksHolder<int, int> holder;

    auto t1 = holder.GetTask(42, [&] (ICancellationTokenPtr token)
        {
        return thread->ExecuteAsync<int>([]
            {
            return 0;
            });
        });
    t1->Wait();

    EXPECT_FALSE(holder.IsTaskRunning(42));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, IsTaskRunning_TaskNotYetFinished_True)
    {
    auto thread = WorkerThread::Create();

    UniqueTasksHolder<int, int> holder;
    AsyncTestCheckpoint a;

    auto t1 = holder.GetTask(42, [&] (ICancellationTokenPtr token)
        {
        return thread->ExecuteAsync<int>([&]
            {
            a.CheckinAndWait();
            return 0;
            });
        });

    a.WaitUntilReached();
    EXPECT_TRUE(holder.IsTaskRunning(42));

    a.Continue();
    t1->Wait();

    EXPECT_FALSE(holder.IsTaskRunning(42));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, GetTask_MaxQueueIsOneAndCalledOnceWithDifferentIds_ReturnsDifferentTasks)
    {
    UniqueTasksHolder<int, int, 1> holder;

    int timesCalled = 0;
    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        timesCalled++;
        return StubStartedAsyncTask<int>(1);
        };

    auto t1 = holder.GetTask(1, newTaskCallback);
    auto t2 = holder.GetTask(2, newTaskCallback);

    EXPECT_FALSE(t1.get() == t2.get());
    EXPECT_EQ(2, timesCalled);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, GetTask_MaxQueueIsOneAndCalledWithSameId_CreatesNewTaskAfterFirstIsFinished)
    {
    UniqueTasksHolder<int, int, 1> holder;

    int timesCalled = 0;
    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        timesCalled++;
        return StubStartedAsyncTask<int>(1);
        };

    auto t1 = holder.GetTask(1, newTaskCallback);
    auto t2 = holder.GetTask(1, newTaskCallback);

    EXPECT_FALSE(t1.get() == t2.get());
    EXPECT_EQ(1, timesCalled);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, GetTask_MaxQueueIsOneAndCalledWithSameIdThreeTimes_ReturnsSecondTaskInThirdCall)
    {
    UniqueTasksHolder<int, int, 1> holder;

    int timesCalled = 0;
    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        timesCalled++;
        return StubStartedAsyncTask<int>(1);
        };

    auto t1 = holder.GetTask(1, newTaskCallback);
    auto t2 = holder.GetTask(1, newTaskCallback);
    auto t3 = holder.GetTask(1, newTaskCallback);

    EXPECT_FALSE(t1.get() == t2.get());
    EXPECT_TRUE(t2.get() == t3.get());
    EXPECT_EQ(1, timesCalled);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, GetTask_MaxQueueIsOneAndCalledWithSameId_StartsNewTaskAfterFirstTaskIsFinished)
    {
    UniqueTasksHolder<int, int, 1> holder;

    auto thread = WorkerThread::Create();

    bool called1 = false;
    bool called2 = false;
    bool finished1 = false;

    AsyncTestCheckpoint cp;
    auto newTaskCallback1 = [&] (ICancellationTokenPtr token)
        {
        called1 = true;
        return thread->ExecuteAsync<int>([&]
            {
            cp.CheckinAndWait();
            finished1 = true;
            return 1;
            });
        };

    auto t1 = holder.GetTask(1, newTaskCallback1);

    auto newTaskCallback2 = [&] (ICancellationTokenPtr token)
        {
        called2 = true;
        EXPECT_TRUE(finished1);
        return CreateCompletedAsyncTask<int>(2);
        };

    auto t2 = holder.GetTask(1, newTaskCallback2);

    cp.WaitUntilReached();
    cp.Continue();

    EXPECT_EQ(1, t1->GetResult());
    EXPECT_EQ(2, t2->GetResult());
    EXPECT_TRUE(called1);
    EXPECT_TRUE(called2);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(UniqueTasksHolderTests, GetTask_MaxQueueIsOneAndCalledTwiceInSequence_QueuesAndAddsQueuedTaskToParentTask)
    {
    UniqueTasksHolder<int, int, 1> holder;

    auto thread = WorkerThread::Create();
    auto thread2 = WorkerThread::Create();

    AsyncTestCheckpoint checkpointA;
    AsyncTestCheckpoint checkpointB;

    int timesCalled = 0;
    auto newTaskCallback = [&] (ICancellationTokenPtr token)
        {
        timesCalled++;
        return thread2->ExecuteAsync<int>([&]
            {
            checkpointA.CheckinAndWait();
            return 42;
            });
        };

    auto t1 = holder.GetTask(1, newTaskCallback);

    bool thenOnParentTaskCalled = false;
    bool thenOnSubTaskCalled = false;

    auto t2 = thread->ExecuteAsync([&]
        {
        holder.GetTask(1, newTaskCallback)
            ->Then([&] (int)
            {
            thenOnSubTaskCalled = true;
            });
        checkpointB.CheckinAndWait();
        })
        ->Then([&]
            {
            thenOnParentTaskCalled = true;
            EXPECT_TRUE(thenOnSubTaskCalled);
            });

        checkpointB.WaitUntilReached();
        checkpointB.Continue();

        checkpointA.WaitUntilReached();
        checkpointA.Continue();

        AsyncTask::WhenAll(StubBSet<std::shared_ptr<AsyncTask>>({ t1, t2 }))->Wait();
        EXPECT_EQ(2, timesCalled);
        EXPECT_TRUE(thenOnSubTaskCalled);
        EXPECT_TRUE(thenOnParentTaskCalled);
    }
