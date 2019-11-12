/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/Tasks/Tasks.h>

#include <Bentley/Tasks/UniqueTaskHolder.h>
#include <Bentley/Tasks/WorkerThread.h>
#include "AsyncTestCheckpoint.h"

USING_NAMESPACE_BENTLEY_TASKS

struct UniqueTaskHolderTests : ::testing::Test {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UniqueTaskHolderTests, GetRunningTask_CalledTwice_ExecutesJustFirstTask)
    {
    auto thread = WorkerThread::Create("TestThread");

    UniqueTaskHolder<int> taskHolder;

    bset<std::shared_ptr<AsyncTask>> tasks;
    BeAtomic<bool> block(true);

    tasks.insert(taskHolder.GetTask([&]
        {
        return
            thread->ExecuteAsync<int>([&]
            {
            while (block);
            return 0;
            });
        }));

    tasks.insert(taskHolder.GetTask([&]
        {
        BeAssert(false);
        return nullptr;
        }));

    block.store(false);

    AsyncTask::WhenAll(tasks)->Wait();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UniqueTaskHolderTests, GetRunningTask_CalledSecondTimeAfterFirstFinished_ExecutesSecondTime)
    {
    auto thread = WorkerThread::Create("TestThread");

    UniqueTaskHolder<int> taskHolder;

    BeAtomic<int> i(0);

    taskHolder.GetTask([&]
        {
        return thread->ExecuteAsync<int>([&]
            {
            EXPECT_EQ(1, ++i);
            return 0;
            });
        })->Wait();

        taskHolder.GetTask([&]
            {
            return thread->ExecuteAsync<int>([&]
                {
                EXPECT_EQ(2, ++i);
                return 0;
                });
            })->Wait();

            EXPECT_EQ(2, i);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UniqueTaskHolderTests, GetRunningTask_CalledTwiceWithThenTasks_ReturnsValueToBothThens)
    {
    auto thread = WorkerThread::Create("TestThread");

    UniqueTaskHolder<Utf8String> taskHolder;

    bset<std::shared_ptr<AsyncTask>> tasks;

    BeAtomic<int> i(0);
    BeAtomic<bool> block(true);

    tasks.insert(taskHolder.GetTask([&]
        {
        return
            thread->ExecuteAsync<Utf8String>([&]
            {
            while (block);
            return "UniqueTaskResult";
            });
        })
        ->Then([&] (Utf8String result)
            {
            EXPECT_EQ(1, ++i);
            EXPECT_EQ("UniqueTaskResult", result);
            }));

    tasks.insert(taskHolder.GetTask([=] { return nullptr; })
        ->Then([&] (Utf8String result)
        {
        EXPECT_EQ(2, ++i);
        EXPECT_EQ("UniqueTaskResult", result);
        }));

    block.store(false);

    AsyncTask::WhenAll(tasks)->Wait();
    EXPECT_EQ(2, i);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UniqueTaskHolderTests, GetRunningTask_CalledOnceAndExecutorDeleted_DoesNotCrashAndExecutesTask)
    {
    auto thread = WorkerThread::Create("TestThread");

    auto taskHolder = new UniqueTaskHolder<int>();

    BeAtomic<bool> block(true);
    auto task = taskHolder->GetTask([&]
        {
        return
            thread->ExecuteAsync<int>([&]
            {
            while (block);
            return 42;
            });
        });

    delete taskHolder;
    block.store(false);

    EXPECT_EQ(42, task->GetResult());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UniqueTaskHolderTests, GetRunningTask_ReturnsCompletedTask_ExecutesAndPropogatesBack)
    {
    auto thread = WorkerThread::Create("TestThread");
    auto taskHolder = new UniqueTaskHolder<int>();

    thread->ExecuteAsync([&]
        {
        taskHolder->GetTask([]
            {
            return CreateCompletedAsyncTask(5);
            });
        })
        ->Wait();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTaskHolderTests, GetTask_CalledAfterTaskIsCanceled_ReturnsDifferentTask)
    {
    UniqueTaskHolder<int> holder;

    auto newTaskCallback = [&] (ICancellationTokenPtr token) -> AsyncTaskPtr<int>
        {
        return std::make_shared<PackagedAsyncTask<int>>([=]
            {
            return 1;
            });
        };

    auto t1 = holder.GetTask(newTaskCallback);
    holder.CancelTask();
    auto t2 = holder.GetTask(newTaskCallback);

    EXPECT_FALSE(t1.get() == t2.get());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTaskHolderTests, GetTask_CalledAfterSameIdTaskIsCanceledButIsNotFinishedYet_SecondTaskCanAlsoBeCanceled)
    {
    auto thread1 = WorkerThread::Create();
    auto thread2 = WorkerThread::Create();
    EXPECT_FALSE(thread1->HasRunningTasks());
    UniqueTaskHolder<int> holder;
    AsyncTestCheckpoint a, b, c;

    auto t1 = holder.GetTask([&] (ICancellationTokenPtr token)
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
    holder.CancelTask();
    a.Continue();

    auto t2 = holder.GetTask([&] (ICancellationTokenPtr token)
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

    holder.CancelTask();

    c.Continue();
    t2->Wait();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTaskHolderTests, CancelTask_CalledForExistingTask_TaskTokenIsSetToCanceled)
    {
    UniqueTaskHolder<int> holder;

    auto t1 = holder.GetTask([&] (ICancellationTokenPtr token)
        {
        return std::make_shared<PackagedAsyncTask<int>>([=]
            {
            EXPECT_TRUE(token->IsCanceled());
            return 1;
            });
        });
    holder.CancelTask();

    t1->Execute();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(UniqueTaskHolderTests, CancelTask_CalledForExistingTask_ReturnsTask)
    {
    UniqueTaskHolder<int> holder;

    auto t1 = holder.GetTask([] (ICancellationTokenPtr token)
        {
        return std::make_shared<PackagedAsyncTask<int>>([] {return 1; });
        });
    auto t2 = holder.CancelTask();

    EXPECT_EQ(t2, t1);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(UniqueTaskHolderTests, CancelTask_CalledForNotExistingTask_ReturnsNull)
    {
    UniqueTaskHolder<int> holder;

    auto t1 = holder.CancelTask();

    EXPECT_EQ(t1, nullptr);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTaskHolderTests, CancelTask_CalledForExistingTask_TaskTokenForNewTaskIsNotCanceled)
    {
    UniqueTaskHolder<int> holder;

    auto t1 = holder.GetTask([&] (ICancellationTokenPtr token)
        {
        return std::make_shared<PackagedAsyncTask<int>>([=]
            {
            EXPECT_TRUE(token->IsCanceled());
            return 1;
            });
        });
    holder.CancelTask();

    auto t2 = holder.GetTask([&] (ICancellationTokenPtr token)
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
TEST_F(UniqueTaskHolderTests, IsTaskRunning_CalledForNotExistingTask_False)
    {
    UniqueTaskHolder<int> holder;

    EXPECT_FALSE(holder.IsTaskRunning());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTaskHolderTests, IsTaskRunning_FinishedTask_False)
    {
    auto thread = WorkerThread::Create();
    UniqueTaskHolder<int> holder;

    auto t1 = holder.GetTask([&] (ICancellationTokenPtr token)
        {
        return thread->ExecuteAsync<int>([]
            {
            return 0;
            });
        });
    t1->Wait();

    EXPECT_FALSE(holder.IsTaskRunning());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F(UniqueTaskHolderTests, IsTaskRunning_TaskNotYetFinished_True)
    {
    auto thread = WorkerThread::Create();

    UniqueTaskHolder<int> holder;
    AsyncTestCheckpoint a;

    auto t1 = holder.GetTask([&] (ICancellationTokenPtr token)
        {
        return thread->ExecuteAsync<int>([&]
            {
            a.CheckinAndWait();
            return 0;
            });
        });

    a.WaitUntilReached();
    EXPECT_TRUE(holder.IsTaskRunning());

    a.Continue();
    t1->Wait();

    EXPECT_FALSE(holder.IsTaskRunning());
    }