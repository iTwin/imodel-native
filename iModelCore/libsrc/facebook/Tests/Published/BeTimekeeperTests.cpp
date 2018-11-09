/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/BeTimekeeperTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/DateTime.h>
#include <folly/BeTimekeeper.h>
#include <folly/futures/InlineExecutor.h>
#include "AsyncTestCheckpoint.h"

struct BeTimekeeperTests : public ::testing::Test {};

using namespace BeFolly;

struct TestBeTimekeeper : BeTimekeeper
    {
    TestBeTimekeeper() : BeTimekeeper() {};
    };

TEST_F(BeTimekeeperTests, Get_CalledMultipleTimes_AlwaysReturnsSameInstance)
    {
    auto executor1 = &BeTimekeeper::Get();
    auto executor2 = &BeTimekeeper::Get();

    EXPECT_EQ(executor1, executor2);
    }

TEST_F(BeTimekeeperTests, after_ZeroTime_ExecuteCallbackImmeadetely)
    {
    for (int i = 0; i < 10; i++)
        {
        std::atomic_bool executed1 = false;

        auto t1 = BeTimekeeper::Get().after(folly::Duration(0)).then([&]
            {
            executed1 = true;
            });

        t1.wait();

        EXPECT_TRUE(executed1);
        }
    }

TEST_F(BeTimekeeperTests, after_MultipleCallsWithSameDuration_ExecuteCallbacksAfterSpecifiedTimeInSequence)
    {
    auto timeBefore = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    std::atomic_bool executed1 = false;
    std::atomic_bool executed2 = false;
    std::atomic_bool executed3 = false;

    auto t1 = BeTimekeeper::Get().after(folly::Duration(10)).then([&]
        {
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 10);
        EXPECT_FALSE(executed2);
        EXPECT_FALSE(executed3);
        executed1 = true;
        });

    auto t2 = BeTimekeeper::Get().after(folly::Duration(10)).then([&]
        {
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 10);
        EXPECT_TRUE(executed1);
        EXPECT_FALSE(executed3);
        executed2 = true;
        });

    auto t3 = BeTimekeeper::Get().after(folly::Duration(10)).then([&]
        {
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 10);
        EXPECT_TRUE(executed1);
        EXPECT_TRUE(executed2);
        executed3 = true;
        });

    folly::collectAll(t1, t2, t3).wait();

    EXPECT_TRUE(executed1);
    EXPECT_TRUE(executed2);
    EXPECT_TRUE(executed3);
    }

TEST_F(BeTimekeeperTests, after_FromLongestToShortest_ExecuteCallbacksAtAppropriateTimes)
    {
    auto timeBefore = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    std::atomic_bool executed1 = false;
    std::atomic_bool executed2 = false;
    std::atomic_bool executed3 = false;

    auto t1 = BeTimekeeper::Get().after(folly::Duration(30)).then([&]
        {
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 30);
        EXPECT_TRUE(executed2);
        EXPECT_TRUE(executed3);
        executed1 = true;
        });

    auto t2 = BeTimekeeper::Get().after(folly::Duration(20)).then([&]
        {
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 20);
        EXPECT_FALSE(executed1);
        EXPECT_TRUE(executed3);
        executed2 = true;
        });

    auto t3 = BeTimekeeper::Get().after(folly::Duration(10)).then([&]
        {
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 10);
        EXPECT_FALSE(executed1);
        EXPECT_FALSE(executed2);
        executed3 = true;
        });

    folly::collectAll(t1, t2, t3).wait();

    EXPECT_TRUE(executed1);
    EXPECT_TRUE(executed2);
    EXPECT_TRUE(executed3);
    }

TEST_F(BeTimekeeperTests, after_FromShortestToLongest_ExecuteCallbacksAtAppropriateTimes)
    {
    auto timeBefore = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    std::atomic_bool executed1 = false;
    std::atomic_bool executed2 = false;
    std::atomic_bool executed3 = false;

    auto t1 = BeTimekeeper::Get().after(folly::Duration(10)).then([&]
        {
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 10);
        EXPECT_FALSE(executed2);
        EXPECT_FALSE(executed3);
        executed1 = true;
        });

    auto t2 = BeTimekeeper::Get().after(folly::Duration(20)).then([&]
        {
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 20);
        EXPECT_TRUE(executed1);
        EXPECT_FALSE(executed3);
        executed2 = true;
        });

    auto t3 = BeTimekeeper::Get().after(folly::Duration(30)).then([&]
        {
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 30);
        EXPECT_TRUE(executed1);
        EXPECT_TRUE(executed2);
        executed3 = true;
        });

    folly::collectAll(t1, t2, t3).wait();

    EXPECT_TRUE(executed1);
    EXPECT_TRUE(executed2);
    EXPECT_TRUE(executed3);
    }

TEST_F(BeTimekeeperTests, after_CalledFromWithinCallback_ExecuteCallbacksAtAppropriateTimes)
    {
    auto timeBefore = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    std::atomic_bool executed1 = false;
    std::atomic_bool executed2 = false;

    folly::Future<folly::Unit> t2 = folly::makeFuture();

    auto t1 = BeTimekeeper::Get().after(folly::Duration(10)).then([&]
        {
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 10);
        EXPECT_FALSE(executed2);
        executed1 = true;

        t2 = BeTimekeeper::Get().after(folly::Duration(10)).then([&]
            {
            auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            EXPECT_GE(timeAfter - timeBefore, 20);
            EXPECT_TRUE(executed1);
            executed2 = true;
            });
        });

    t1.wait();
    t2.wait();

    EXPECT_TRUE(executed1);
    EXPECT_TRUE(executed2);
    }

TEST_F(BeTimekeeperTests, Dtor_DestroyedWhileExecutingFunc_DestroysWithoutCrashing)
    {
    for (int i = 0; i < 10; i++)
        {
        std::unique_ptr<TestBeTimekeeper> delayedExecutor(new TestBeTimekeeper());
        delayedExecutor->after(folly::Duration(0)).then([] {});
        delayedExecutor->after(folly::Duration(10)).then([] {});
        delayedExecutor = nullptr;
        }
    }

TEST_F(BeTimekeeperTests, Dtor_DestroyedEmpty_DestroysWithoutCrashing)
    {
    for (int i = 0; i < 10; i++)
        {
        std::unique_ptr<TestBeTimekeeper> delayedExecutor(new TestBeTimekeeper());
        delayedExecutor = nullptr;
        }
    }

TEST_F(BeTimekeeperTests, Dtor_ExecutingLongRunningTaskBeforeProgramShutdown_DestroysWithoutCrashingOrHanging)
    {
    BeTimekeeper::Get().after(std::chrono::hours(24)).then([&]
        {
        ADD_FAILURE() << "This should not be called";
        });
    }

TEST_F(BeTimekeeperTests, Dtor_ExecutingLongRunningTaskBeforeProgramShutdownViaCpuExecutor_DestroysWithoutCrashingOrHanging)
    {
    BeTimekeeper::Get().after(std::chrono::hours(24)).then(&BeFolly::ThreadPool::GetCpuPool(), [&]
        {
        ADD_FAILURE() << "This should not be called";
        });
    }
