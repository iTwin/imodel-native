/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/DateTime.h>

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @bsimethod                                      Farhad.Kabir                    11/16
//---------------------------------------------------------------------------------------
TEST(DateTime_Tests, MillisecondsToRationalDays)
    {
    ASSERT_DOUBLE_EQ(5.7938, DateTime::MsecToRationalDay(500584320));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Farhad.Kabir                    11/16
//---------------------------------------------------------------------------------------
TEST(DateTime_Tests, RationalDaysToMilliseconds)
    {
    ASSERT_EQ(500584320, DateTime::RationalDayToMsec(5.7938));
    }
