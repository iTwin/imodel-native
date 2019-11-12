/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>

#include <Bentley/BeThread.h>

USING_NAMESPACE_BENTLEY

class BeClockTests : public ::testing::Test {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Lukaosnok                  11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeClockTests, GetSteadyTime_CalledInBetweenTwoBeTimePointNowCalls_ValueResidesInBetween)
    {
    BeClock clock;

    BeTimePoint before = BeTimePoint::Now();
    BeThreadUtilities::BeSleep(1);
    BeTimePoint value = clock.GetSteadyTime();
    BeThreadUtilities::BeSleep(1);
    BeTimePoint after = BeTimePoint::Now();

    EXPECT_BETWEEN(before, value, after);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Lukaosnok                  11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeClockTests, GetSystemTime_CalledInBetweenTwoDateTimeGetCurrentTimeCalls_ValueResidesInBetween)
    {
    BeClock clock;

    DateTime before = DateTime::GetCurrentTime();
    BeThreadUtilities::BeSleep(1);
    DateTime value = clock.GetSystemTime();
    BeThreadUtilities::BeSleep(1);
    DateTime after = DateTime::GetCurrentTime();

    EXPECT_EQ(DateTime::CompareResult::EarlierThan, DateTime::Compare(before, value));
    EXPECT_EQ(DateTime::CompareResult::EarlierThan, DateTime::Compare(value, after));
    }
