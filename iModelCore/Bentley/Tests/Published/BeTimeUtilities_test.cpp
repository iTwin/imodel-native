/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeTimeUtilities_test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeTimeUtilitiesTests, BeTimePoint)
    {
    BeTimePoint invalid;
    ASSERT_TRUE(!invalid.IsValid());

    BeDuration zero;
    ASSERT_TRUE(zero == BeDuration::Seconds(0));
    ASSERT_TRUE(zero.IsZero());
    ASSERT_TRUE(!zero.IsTowardsFuture());
    ASSERT_TRUE(!zero.IsTowardsPast());

    BeDuration threeSeconds(BeDuration::FromSeconds(3)); // integer seconds
    BeDuration twoSeconds(BeDuration::FromSeconds(2.0)); // double seconds
    BeDuration negative1(BeDuration::FromSeconds(-1));
    ASSERT_TRUE(threeSeconds == BeDuration::Seconds(3));
    ASSERT_TRUE(twoSeconds == BeDuration::Seconds(2));
    ASSERT_TRUE(twoSeconds.IsTowardsFuture());
    ASSERT_TRUE(negative1 == BeDuration::Seconds(-1));
    ASSERT_TRUE(negative1.IsTowardsPast());

    double two = twoSeconds;
    ASSERT_TRUE(two == 2.0);
    
    BeDuration twoandhalf(BeDuration::FromSeconds(2.5));
    ASSERT_TRUE(twoandhalf == 2.5);
    ASSERT_TRUE(twoandhalf == BeDuration::FromMilliseconds(2500));
    ASSERT_TRUE(twoandhalf.ToSeconds() == 2.5);
 
    BeDuration::Milliseconds twoInMillis = twoSeconds;
    ASSERT_TRUE(twoInMillis == BeDuration::FromMilliseconds(2000));

    BeTimePoint t1 = BeTimePoint::Now();
    BeTimePoint t2 = BeTimePoint::Now();
    BeTimePoint t3 = BeTimePoint::FromNow(BeDuration::Seconds(3));
    ASSERT_TRUE(t3.IsInFuture());

    ASSERT_TRUE(t1.IsValid());
    ASSERT_TRUE(t2 >= t1);
    double diff = BeDuration(t3-t2);
    ASSERT_TRUE(diff>=3.0 && diff<4.0);

    BeDuration::FromMilliseconds(100).Sleep();
    ASSERT_TRUE(!t1.IsInFuture());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST(BeTimeUtilitiesTests, QueryMillisecondsCounter)
    {
    uint64_t t1 = BeTimeUtilities::QueryMillisecondsCounter();
    uint64_t t2 = BeTimeUtilities::QueryMillisecondsCounter();

    ASSERT_GE(t2, t1);
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
//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
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
                dateTimeTemp.GetMinute(), dateTimeTemp.GetSecond(), dateTimeTemp.GetMillisecond());
    ASSERT_EQ(BentleyStatus::SUCCESS, dateTimeUtc.ToUnixMilliseconds(retUtcMillis));
    ASSERT_EQ(BentleyStatus::SUCCESS, dateTimeLocal.ToUnixMilliseconds(retLocalMillis));
    int64_t localMillisAdjustment = retLocalMillis - retUtcMillis;

    int64_t localMillisAdjustmentAlt;
    ASSERT_EQ(BentleyStatus::SUCCESS, dateTimeTemp.ComputeOffsetToUtcInMsec(localMillisAdjustmentAlt));
    ASSERT_EQ(localMillisAdjustmentAlt, localMillisAdjustment);

    uint64_t actualMillis = retUtcMillis;
    uint64_t expectedMillis = actualMillis + localMillisAdjustment;
    stat = BeTimeUtilities::AdjustUnixMillisForLocalTime(actualMillis);
    ASSERT_TRUE(stat == BentleyStatus::SUCCESS);
    printf("%Id    %Id\n", expectedMillis, actualMillis);
    EXPECT_EQ(expectedMillis, actualMillis);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Farhad.Kabir                       10/16
//---------------------------------------------------------------------------------------
TEST(BeTimeUtilitiesTests, GetCurrentTimeAsUnixMillisDoubleWithDelay)
    {
    double ts0 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDoubleWithDelay();
    double ts1 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDoubleWithDelay();
    EXPECT_NE(ts0, ts1) << "ts0 is " << ts0 << "ts1 is " << ts1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Farhad.Kabir                       12/16
//---------------------------------------------------------------------------------------
TEST(BeTimeUtilitiesTests, ConvertTmToUnixMillis)
    {
    uint64_t unixMillisExpected = 1095379199000;
    tm expectedTm;
    ASSERT_EQ(BentleyStatus::SUCCESS, BeTimeUtilities::ConvertUnixMillisToTm(expectedTm, unixMillisExpected));
    uint64_t unixMillisReceived = BeTimeUtilities::ConvertTmToUnixMillis(expectedTm);
    EXPECT_EQ(unixMillisExpected, unixMillisReceived);
    }
