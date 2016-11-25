/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeTimeUtilities_test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)
#include <windows.h>
#endif
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/bmap.h>
#include <Bentley/DateTime.h>
USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST(BeTimeUtilitiesTests, QueryMillisecondsCounterUInt32)
    {
    uint32_t t1 = BeTimeUtilities::QueryMillisecondsCounterUInt32();
    uint32_t t2 = BeTimeUtilities::QueryMillisecondsCounterUInt32();

    ASSERT_GE(t2, t1);
    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST(BeTimeUtilitiesTests, QueryMillisecondsCounter)
    {
    uint64_t t1 = BeTimeUtilities::QueryMillisecondsCounter();
    uint64_t t2 = BeTimeUtilities::QueryMillisecondsCounter();

    ASSERT_GE(t2, t1);
    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                      09/13
//---------------------------------------------------------------------------------------
Utf8String unixMillisToString(uint64_t inputTime)
    {
    struct tm timeinfo;
    BeTimeUtilities::ConvertUnixMillisToTm(timeinfo, inputTime);   // GMT

    Utf8String fullDate;
    char buf[128];

    strftime(buf, sizeof(buf), "%Y/%m/%d", &timeinfo);
    fullDate = buf;

    fullDate.append(" ");

    strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
    fullDate.append(buf);

    return fullDate;
    }

//---------------------------------------------------------------------------------------
// @betest                                      Julija.Suboc                    09/13
//---------------------------------------------------------------------------------------
TEST(BeTimeUtilitiesTests, strftime)
    {
    bvector<uint64_t> testTime;
    bmap<uint64_t, Utf8String> testData;
    testData.Insert(1095379199000, "2004/09/16 23:59:59");  // GMT
    testData.Insert(1379469420000, "2013/09/18 01:57:00");
    /* Can't do this test. 32-bit mobile devices do not support time_t beyond 2038.
        testData.Insert(10953791988000, L"2317/02/10 23:59:48");
        */
    testData.begin();
    bmap<uint64_t, Utf8String>::iterator data = testData.begin();
    while (data != testData.end())
        {
        //find out how differs utc time from local time 
        uint64_t time = data->first;
        Utf8String fullDate = unixMillisToString(time);
        //Compare returned strings with string expected
        Utf8String stringExpected = data->second;
        EXPECT_TRUE(stringExpected.Equals(fullDate)) << "got:" << fullDate.c_str() << " expected:" << stringExpected.c_str();
        ++data;
        }
    }

#if defined (NOT_NOW)
// It's hopeless. The time spent in Sleep varies a great deal and is unpredictable
TEST(BeTimeUtilitiesTests, StopWatch)
    {
#if defined (_WIN32)
    StopWatch w(L"stopwatch");

    w.Start();
    ::Sleep(100);
    w.Stop();
    ASSERT_GE(w.GetElapsedSeconds(), 0.05);
    ASSERT_LE(w.GetElapsedSeconds(), 0.15);

    w.Start();
    ::Sleep(10);
    w.Stop();
    ASSERT_GE(w.GetElapsedSeconds(), 0.005);
    ASSERT_LE(w.GetElapsedSeconds(), 0.015);
#endif
    }
#endif

//     This method calculates time difference between local time zone of machine and
//     GMT/UTC time in milli seconds.

#if defined (NOT_NOW)
//---------------------------------------------------------------------------------------
// @bsimethod                                   Farhad.Kabir                       11/16
//---------------------------------------------------------------------------------------
TEST(BeTimeUtilitiesTests, AdjustUnixMillisForLocalTime)
    {
    DateTime dateTimeUtc = DateTime::GetCurrentTimeUtc();
    ASSERT_TRUE(dateTimeUtc.IsValid());
    //EXPECT_EQ(static_cast<int> (d1.GetYear()), 2013) << "Day 1 is  " << d1.ToString().c_str();
    DateTime dateTimeTemp;
    ASSERT_TRUE(!dateTimeTemp.IsValid());
    BentleyStatus stat = dateTimeUtc.ToLocalTime(dateTimeTemp);
    ASSERT_TRUE(dateTimeTemp.IsValid());
    ASSERT_EQ(BentleyStatus::SUCCESS, stat);
    //EXPECT_EQ(static_cast<int> (d2.GetYear()), 2013) << "Day 2 is  " << d2.ToString().c_str();
    int64_t retUtcMillis;
    int64_t retLocalMillis;
    DateTime dateTimeLocal(DateTime::Kind::Utc, dateTimeTemp.GetYear(), dateTimeTemp.GetMonth(), dateTimeTemp.GetDay(), dateTimeTemp.GetHour(),
                dateTimeTemp.GetMinute(), dateTimeTemp.GetSecond(), dateTimeTemp.GetHectoNanosecond());
    ASSERT_EQ(BentleyStatus::SUCCESS, dateTimeUtc.ToUnixMilliseconds(retUtcMillis));
    ASSERT_EQ(BentleyStatus::SUCCESS, dateTimeLocal.ToUnixMilliseconds(retLocalMillis));
    uint64_t localMillis = static_cast <uint64_t>(retLocalMillis - retUtcMillis);

    uint64_t actualMillis = 1095379199000;
    uint64_t expectedMillis = 1095379199000 + localMillis;
    stat = BeTimeUtilities::AdjustUnixMillisForLocalTime(actualMillis);
    ASSERT_TRUE(stat == BentleyStatus::SUCCESS);
    EXPECT_EQ(expectedMillis, actualMillis);
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                   Farhad.Kabir                       10/16
//---------------------------------------------------------------------------------------
TEST(BeTimeUtilitiesTests, GetCurrentTimeAsUnixMillisDoubleWithDelay)
    {
    double ts0 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDoubleWithDelay();
    double ts1 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDoubleWithDelay();
    EXPECT_NE(ts0, ts1) << "ts0 is " << ts0 << "ts1 is " << ts1;
    }
