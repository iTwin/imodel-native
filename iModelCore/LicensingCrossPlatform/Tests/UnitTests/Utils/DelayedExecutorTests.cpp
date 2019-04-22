/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "DelayedExecutorTests.h"

#include <Bentley/DateTime.h>

#include <Licensing/Utils/DelayedExecutor.h>

USING_NAMESPACE_BENTLEY_LICENSING

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

TEST_F(DelayedExecutorTests, GetExecutor_CalledMultipleTimes_AlwaysReturnsSameInstance)
    {
    auto executor1 = DelayedExecutor::Get();
    auto executor2 = DelayedExecutor::Get();

    EXPECT_EQ(&*executor1, &*executor2);
    }

TEST_F(DelayedExecutorTests, Delayed_FromLongestToShortest_ExecuteCallbacksAtAppropriateTimes)
    {
    auto timeBefore = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    bool executed1 = false;
    bool executed2 = false;
    bool executed3 = false;

    auto t1 = DelayedExecutor::Get()->Delayed(150).then([&]
        {
        executed1 = true;
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter-timeBefore, 150);
        });

    auto t2 = DelayedExecutor::Get()->Delayed(100).then([&]
        {
        executed2 = true;
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 100);
        });

    auto t3 = DelayedExecutor::Get()->Delayed(50).then([&]
        {
        executed3 = true;
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 50);
        });

    folly::collectAll(t1, t2, t3).wait();

    EXPECT_TRUE(executed1);
    EXPECT_TRUE(executed2);
    EXPECT_TRUE(executed3);
    }

TEST_F(DelayedExecutorTests, Delayed_FromShortestToLongest_ExecuteCallbacksAtAppropriateTimes)
    {
    auto timeBefore = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    bool executed1 = false;
    bool executed2 = false;
    bool executed3 = false;

    auto t1 = DelayedExecutor::Get()->Delayed(50).then([&]
        {
        executed1 = true;
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 50);
        });

    auto t2 = DelayedExecutor::Get()->Delayed(100).then([&]
        {
        executed2 = true;
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 100);
        });

    auto t3 = DelayedExecutor::Get()->Delayed(150).then([&]
        {
        executed3 = true;
        auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        EXPECT_GE(timeAfter - timeBefore, 150);
        });

    folly::collectAll(t1, t2, t3).wait();

    EXPECT_TRUE(executed1);
    EXPECT_TRUE(executed2);
    EXPECT_TRUE(executed3);
    }
