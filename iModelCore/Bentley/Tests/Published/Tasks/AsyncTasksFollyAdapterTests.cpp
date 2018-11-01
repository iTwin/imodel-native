/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/AsyncTasksFollyAdapterTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>

#include <Bentley/Tasks/AsyncTaskFollyAdapter.h>
#include <Bentley/Tasks/WorkerThread.h>
#include <folly/Executor.h>
//#include <folly/futures/ManualExecutor.h>

#include "AsyncTestCheckpoint.h"

// Some of these tests randomly fail on Linux. Since we plan to remove folly
// on imodel02, where all future Linux builds will come from, it is not worth
// debugging here.
#if !defined(BENTLEYCONFIG_OS_LINUX)

USING_NAMESPACE_BENTLEY_TASKS

struct AsyncTasksFollyAdapterTests : public ::testing::Test {};

struct TestExecutor : folly::Executor
    {
    bool autoRun = false;
    std::vector<folly::Function<void()>> funcs;

    virtual void add(folly::Function<void()> func)
        {
        if (autoRun)
            func();
        else
            funcs.push_back(std::move(func));
        }

    virtual void addWithPriority(folly::Function<void()> func, int8_t priority)
        {
        add(std::move(func));
        }

    TestExecutor& runonce()
        {
        for (auto& func : funcs)
            func();
        return *this;
        }

    TestExecutor& start()
        {
        autoRun = true;
        runonce();
        return *this;
        }
    };

//---------------------------------------------------------------------------------------
// @betest                                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksFollyAdapterTests, ToFolly_CompletedTask_ReturnsCompletedFuture)
    {
    folly::Future<int> future = AsyncTaskFollyAdapter::ToFolly(CreateCompletedAsyncTask<int>(42));

    ASSERT_TRUE(future.isReady());
    EXPECT_EQ(42, future.get());
    }

//---------------------------------------------------------------------------------------
// @betest                                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksFollyAdapterTests, ToFolly_CompletedVoidTask_ReturnsCompletedUnitFuture)
    {
    folly::Future<folly::Unit> future = AsyncTaskFollyAdapter::ToFolly(CreateCompletedAsyncTask());

    ASSERT_TRUE(future.isReady());
    }

//---------------------------------------------------------------------------------------
// @betest                                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksFollyAdapterTests, ToFolly_TaskNotYetCompleted_CompletesFutureOnceTaskIsCompleted)
    {
    AsyncTestCheckpoint cp;

    WorkerThreadPtr thread = WorkerThread::Create();
    AsyncTaskPtr<int> task = thread->ExecuteAsync<int>([&]
        {
        cp.CheckinAndWait();
        return 999;
        });

    folly::Future<int> future = AsyncTaskFollyAdapter::ToFolly(task);

    EXPECT_FALSE(future.isReady());

    cp.Continue();
    thread->OnEmpty()->Wait();

    ASSERT_TRUE(future.isReady());
    EXPECT_EQ(999, future.get());
    }

//---------------------------------------------------------------------------------------
// @betest                                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksFollyAdapterTests, ToFolly_VoidTaskNotYetCompleted_CompletesUnitFutureOnceTaskIsCompleted)
    {
    AsyncTestCheckpoint cp;

    WorkerThreadPtr thread = WorkerThread::Create();
    AsyncTaskPtr<void> task = thread->ExecuteAsync([&]
        {
        cp.CheckinAndWait();
        });

    folly::Future<folly::Unit> future = AsyncTaskFollyAdapter::ToFolly(task);

    EXPECT_FALSE(future.isReady());

    cp.Continue();
    thread->OnEmpty()->Wait();

    ASSERT_TRUE(future.isReady());
    }

//---------------------------------------------------------------------------------------
// @betest                                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksFollyAdapterTests, FromFolly_CompletedFuture_ReturnsCompletedTask)
    {
    folly::Future<int> future = folly::makeFuture(123);
    AsyncTaskPtr<int> task = AsyncTaskFollyAdapter::FromFolly(future);

    ASSERT_TRUE(task->IsCompleted());
    EXPECT_EQ(123, task->GetResult());
    }

//---------------------------------------------------------------------------------------
// @betest                                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksFollyAdapterTests, FromFolly_CompletedUnitFuture_ReturnsCompletedVoidTask)
    {
    folly::Future<folly::Unit> future = folly::makeFuture<folly::Unit>(folly::Unit());
    AsyncTaskPtr<void> task = AsyncTaskFollyAdapter::FromFolly(future);

    ASSERT_TRUE(task->IsCompleted());
    }

//---------------------------------------------------------------------------------------
// @betest                                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksFollyAdapterTests, FromFolly_FutureNotYetCompleted_CompletesTaskOnceFutureIsCompleted)
    {
    TestExecutor executor;
    folly::Future<int> future = folly::via(&executor, []
        {
        return 789;
        });

    AsyncTaskPtr<int> task = AsyncTaskFollyAdapter::FromFolly(future);

    EXPECT_FALSE(task->IsCompleted());

    executor.start();

    ASSERT_TRUE(task->IsCompleted());
    EXPECT_EQ(789, task->GetResult());
    }

//---------------------------------------------------------------------------------------
// @betest                                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksFollyAdapterTests, FromFolly_UnitFutureNotYetCompleted_CompletesVoidTaskOnceFutureIsCompleted)
    {
    TestExecutor thread;
    folly::Future<folly::Unit> future = folly::via(&thread, [] {});

    AsyncTaskPtr<void> task = AsyncTaskFollyAdapter::FromFolly(future);

    EXPECT_FALSE(task->IsCompleted());

    thread.start();

    ASSERT_TRUE(task->IsCompleted());
    }

//---------------------------------------------------------------------------------------
// @betest                                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksFollyAdapterTests, FromFolly_FutureNotYetCompletedAndCalledFromWithinTask_ParentTaskIsCompletedOnlyWhenFutureIs)
    {
    TestExecutor thread;
    folly::Future<int> future = folly::via(&thread, []
        {
        return 42;
        });

    AsyncTaskPtr<int> convertedTask;

    WorkerThreadPtr workerThread = WorkerThread::Create();
    auto parentTask = workerThread->ExecuteAsync([&]
        {
        convertedTask = AsyncTaskFollyAdapter::FromFolly(future);
        });

    BeThreadUtilities::BeSleep(10);

    EXPECT_FALSE(convertedTask->IsCompleted());
    EXPECT_FALSE(parentTask->IsCompleted());

    thread.start();

    ASSERT_TRUE(convertedTask->IsCompleted());
    ASSERT_TRUE(parentTask->IsCompleted());
    EXPECT_EQ(42, convertedTask->GetResult());
    }

//---------------------------------------------------------------------------------------
// @betest                                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksFollyAdapterTests, FromFolly_UnitFutureNotYetCompletedAndCalledFromWithinTask_ParentTaskIsCompletedOnlyWhenFutureIs)
    {
    TestExecutor thread;
    folly::Future<folly::Unit> future = folly::via(&thread, [] {});

    AsyncTaskPtr<void> convertedTask;

    WorkerThreadPtr workerThread = WorkerThread::Create();
    auto parentTask = workerThread->ExecuteAsync([&]
        {
        convertedTask = AsyncTaskFollyAdapter::FromFolly(future);
        });

    BeThreadUtilities::BeSleep(10);

    EXPECT_FALSE(convertedTask->IsCompleted());
    EXPECT_FALSE(parentTask->IsCompleted());

    thread.start();

    ASSERT_TRUE(convertedTask->IsCompleted());
    ASSERT_TRUE(parentTask->IsCompleted());

    workerThread->OnEmpty()->Wait();
    }

//---------------------------------------------------------------------------------------
// @betest                                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksFollyAdapterTests, FromFolly_FutureThrowsCustomException_TaskReturnsDefaultValue)
    {
    struct TestType
        {
        Utf8String str = "Default";
        };

    TestExecutor thread;
    folly::Future<TestType> future = folly::via(&thread, []
        {
        if (true)
            throw 666;

        TestType value;
        value.str = "Foo";
        return value;
        });

    AsyncTaskPtr<TestType> task = AsyncTaskFollyAdapter::FromFolly(future);

    EXPECT_FALSE(task->IsCompleted());

    BeTest::SetFailOnAssert(false);
    thread.start();
    BeTest::SetFailOnAssert(true);

    ASSERT_TRUE(task->IsCompleted());
    EXPECT_EQ("Default", task->GetResult().str);
    }

//---------------------------------------------------------------------------------------
// @betest                                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(AsyncTasksFollyAdapterTests, FromFolly_UnitFutureThrowsCustomException_TaskStillCompletes)
    {
    TestExecutor thread;
    folly::Future<folly::Unit> future = folly::via(&thread, []
        {
        throw 666;
        });

    AsyncTaskPtr<void> task = AsyncTaskFollyAdapter::FromFolly(future);

    EXPECT_FALSE(task->IsCompleted());

    BeTest::SetFailOnAssert(false);
    thread.start();
    BeTest::SetFailOnAssert(true);

    ASSERT_TRUE(task->IsCompleted());
    }

#endif
