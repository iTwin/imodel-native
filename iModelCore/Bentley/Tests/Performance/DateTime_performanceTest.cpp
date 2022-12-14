/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include <Bentley/DateTime.h>
#include <vector>

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Performance_DateTimeTests, FromString)
{
    bvector<Utf8String> testDates;
    testDates.push_back("2012-12-05");
    testDates.push_back("2013-12-05T13:11:22");
    testDates.push_back("2013-12-05T13:11:22Z");
    testDates.push_back("2013-12-05T13:11:22+01:45");
    testDates.push_back("2013-01-09T12:30:00.0000000Z");

    const auto repetitionCount = 10000;
    for (auto const& testDate : testDates)
    {
        StopWatch timer(true);
        for (auto i = 0; i < repetitionCount; i++)
        {
            DateTime dt;
            ASSERT_EQ(SUCCESS, DateTime::FromString(dt, testDate.c_str()));
        }

        timer.Stop();
        PERFORMANCELOG.infov("DateTime::FromString (\"%s\"): %.4f msecs [%d repetitions].", testDate.c_str(), timer.GetElapsedSeconds() * 1000.0, repetitionCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), repetitionCount, testDate.c_str());
    }
}
