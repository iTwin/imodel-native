/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/DateTimeTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/DateTime.h>

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @bsimethod                                      Farhad.Kabir                    11/16
//---------------------------------------------------------------------------------------
TEST(DateTime_Tests, HnsToRationalDays)
    {
    uint64_t hns = 38189990000247;
    double expectedRationals = 44.201377315100694;
    double gotRationals = DateTime::HnsToRationalDay(hns);
    EXPECT_DOUBLE_EQ(expectedRationals, gotRationals);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Farhad.Kabir                    11/16
//---------------------------------------------------------------------------------------
TEST(DateTime_Tests, RationalDaysToHns)
    {
    double rationalDays = 5.7938;
    uint64_t expectedHns = 5005843200000;
    uint64_t receivedHns = DateTime::RationalDayToHns(rationalDays);
    EXPECT_EQ(expectedHns, receivedHns);
    }
