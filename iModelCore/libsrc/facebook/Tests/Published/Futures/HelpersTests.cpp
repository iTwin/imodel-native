/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <folly/futures/helpers.h>

struct HelpersTests : public ::testing::Test {};

TEST_F(HelpersTests, sleep_ZeroTime_ReturnsImmeadetely)
    {
    auto timeBefore = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    folly::futures::sleep(folly::Duration(0)).wait();
    auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    EXPECT_GE(timeAfter - timeBefore, 0);
    }

TEST_F(HelpersTests, sleep_SpecifiedTime_SleepsForTime)
    {
    auto timeBefore = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    folly::futures::sleep(folly::Duration(10)).wait();
    auto timeAfter = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    EXPECT_GE(timeAfter - timeBefore, 10);
    }
