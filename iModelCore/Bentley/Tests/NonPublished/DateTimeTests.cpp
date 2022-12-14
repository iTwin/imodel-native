/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/DateTime.h>

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DateTime_Tests, MillisecondsToRationalDays)
    {
    ASSERT_DOUBLE_EQ(5.7938, DateTime::MsecToRationalDay(500584320));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DateTime_Tests, RationalDaysToMilliseconds)
    {
    ASSERT_EQ(500584320, DateTime::RationalDayToMsec(5.7938));
    }
