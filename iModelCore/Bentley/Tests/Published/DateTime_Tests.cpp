/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/DateTime_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <vector>
#include "DateTimeTestDataset.h"

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, DateTimeConstructorTests)
    {
    //default constructor
    DateTime dateTime;
    ASSERT_TRUE (!dateTime.IsValid ());

    //with positive year
    DateTime::Kind expectedKind = DateTime::Kind::Local;
    int16_t expectedYear = 2012;
    uint8_t expectedMonth = 10;
    uint8_t expectedDay = 17;
    uint8_t expectedHour = 18;
    uint8_t expectedMinute = 3;
    uint8_t expectedSecond = 18;
    uint32_t expectedHectoNanosecond = 1000000;
    
    dateTime = DateTime (expectedKind, expectedYear, expectedMonth, expectedDay, expectedHour, expectedMinute, expectedSecond, expectedHectoNanosecond);
    ASSERT_TRUE (dateTime.IsValid ());
    ASSERT_EQ ((int) expectedKind, (int) dateTime.GetInfo ().GetKind ());
    ASSERT_EQ ((int) DateTime::Component::DateAndTime, (int) dateTime.GetInfo ().GetComponent ());
    ASSERT_EQ (expectedYear, dateTime.GetYear ());
    ASSERT_EQ (expectedMonth, dateTime.GetMonth ());
    ASSERT_EQ (expectedDay, dateTime.GetDay ());
    ASSERT_EQ (expectedHour, dateTime.GetHour ());
    ASSERT_EQ (expectedMinute, dateTime.GetMinute ());
    ASSERT_EQ (expectedSecond, dateTime.GetSecond ());
    ASSERT_EQ (expectedHectoNanosecond, dateTime.GetHectoNanosecond ());

    //with negative year
    expectedKind = DateTime::Kind::Local;
    expectedYear = -3123;
    expectedMonth = 10;
    expectedDay = 17;
    expectedHour = 18;
    expectedMinute = 3;
    expectedSecond = 18;
    expectedHectoNanosecond = 1000000;
    
    dateTime = DateTime (expectedKind, expectedYear, expectedMonth, expectedDay, expectedHour, expectedMinute, expectedSecond, expectedHectoNanosecond);
    ASSERT_TRUE (dateTime.IsValid ());
    ASSERT_EQ ((int) expectedKind, (int) dateTime.GetInfo ().GetKind ());
    ASSERT_EQ ((int) DateTime::Component::DateAndTime, (int) dateTime.GetInfo ().GetComponent ());
    ASSERT_EQ (expectedYear, dateTime.GetYear ());
    ASSERT_EQ (expectedMonth, dateTime.GetMonth ());
    ASSERT_EQ (expectedDay, dateTime.GetDay ());
    ASSERT_EQ (expectedHour, dateTime.GetHour ());
    ASSERT_EQ (expectedMinute, dateTime.GetMinute ());
    ASSERT_EQ (expectedSecond, dateTime.GetSecond ());
    ASSERT_EQ (expectedHectoNanosecond, dateTime.GetHectoNanosecond ());

    //with date only
    DateTime dateOnly (expectedYear, expectedMonth, expectedDay);
    ASSERT_TRUE (dateOnly.IsValid ());
    ASSERT_EQ ((int) DateTime::Kind::Unspecified, (int) dateOnly.GetInfo ().GetKind ());
    ASSERT_EQ ((int) DateTime::Component::Date, (int) dateOnly.GetInfo ().GetComponent ());
    ASSERT_EQ (expectedYear, dateOnly.GetYear ());
    ASSERT_EQ (expectedMonth, dateOnly.GetMonth ());
    ASSERT_EQ (expectedDay, dateOnly.GetDay ());
    ASSERT_EQ (0, dateOnly.GetHour ());
    ASSERT_EQ (0, dateOnly.GetMinute ());
    ASSERT_EQ (0, dateOnly.GetSecond ());
    ASSERT_EQ (0, dateOnly.GetHectoNanosecond ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, DateTimeSpecialMemberTests)
    {
    DateTime d1 (DateTime::Kind::Utc, 2012, 10, 18, 8, 30, 0, 0);
    ASSERT_TRUE (d1.IsValid ());
    //test copy constructor
    DateTime d2 = d1;
    DateTimeAsserter::Assert (d1, d2);

    //test copy assignment operator
    DateTime d3;
    ASSERT_TRUE (!d3.IsValid ());
    d3 = d1;
    DateTimeAsserter::Assert (d1, d3);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/14
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, DateTimeIsValid)
    {
    //tests with invalid date times (created with default ctor)
    DateTime dt;
    ASSERT_FALSE (dt.IsValid ());

    BeTest::SetFailOnAssert (false);
        {
        ASSERT_EQ (DateTime::DayOfWeek::Sunday, dt.GetDayOfWeek ());
        ASSERT_EQ (0, dt.GetDayOfYear ());
        }
    BeTest::SetFailOnAssert (true);

    DateTime newDt;
    ASSERT_EQ (ERROR, dt.ToLocalTime (newDt));
    ASSERT_EQ (ERROR, dt.ToUtc (newDt));

    double jd = 0.0;
    ASSERT_EQ (ERROR, dt.ToJulianDay (jd));
    uint64_t jdHns = 0ULL;
    ASSERT_EQ (ERROR, dt.ToJulianDay (jdHns));

    int64_t millisecs = -1LL;
    ASSERT_EQ (ERROR, dt.ToUnixMilliseconds (millisecs));
    ASSERT_EQ (ERROR, dt.ToCommonEraTicks (millisecs));

    Utf8String actualStr = dt.ToUtf8String ();
    ASSERT_TRUE (actualStr.empty ());

    WString actualStrW = dt.ToString ();
    ASSERT_TRUE (actualStrW.empty ());

    //tests validity of DateTime objects created by conversion
    DateTime now = DateTime::GetCurrentTime ();
    ASSERT_TRUE (now.IsValid ());
    now = DateTime::GetCurrentTimeUtc ();
    ASSERT_TRUE (now.IsValid ());

    ASSERT_EQ (SUCCESS, now.ToJulianDay (jd));
    DateTime resultDt;
    ASSERT_FALSE (resultDt.IsValid ());
    ASSERT_EQ (SUCCESS, DateTime::FromJulianDay (resultDt, jd, DateTime::Info (DateTime::Kind::Utc, DateTime::Component::DateAndTime)));
    ASSERT_TRUE (resultDt.IsValid ());

    ASSERT_EQ (SUCCESS, now.ToUnixMilliseconds (millisecs));
    resultDt = DateTime ();
    ASSERT_FALSE (resultDt.IsValid ());
    ASSERT_EQ (SUCCESS, DateTime::FromUnixMilliseconds (resultDt, millisecs));
    ASSERT_TRUE (resultDt.IsValid ());

    ASSERT_EQ (SUCCESS, now.ToCommonEraTicks (millisecs));
    resultDt = DateTime ();
    ASSERT_FALSE (resultDt.IsValid ());
    ASSERT_EQ (SUCCESS, DateTime::FromCommonEraTicks (resultDt, millisecs, DateTime::Info (DateTime::Kind::Utc, DateTime::Component::DateAndTime)));
    ASSERT_TRUE (resultDt.IsValid ());

    resultDt = DateTime ();
    ASSERT_FALSE (resultDt.IsValid ());
    ASSERT_EQ (SUCCESS, DateTime::FromString (resultDt, now.ToUtf8String ().c_str ()));
    ASSERT_TRUE (resultDt.IsValid ());

    resultDt = DateTime ();
    ASSERT_FALSE (resultDt.IsValid ());
    ASSERT_EQ (SUCCESS, DateTime::FromString (resultDt, now.ToString ().c_str ()));
    ASSERT_TRUE (resultDt.IsValid ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, DateTimeEqualityTests)
    {
    DateTime dt1, dt2;
    ASSERT_TRUE (!dt1.IsValid ());
    ASSERT_TRUE (!dt2.IsValid ());
    ASSERT_TRUE (dt1 == dt2) << "Two invalid date times are expected to be interpreted as equal";

    DateTime utc (DateTime::Kind::Utc, 2012, 10, 18, 8, 30, 0, 0);
    DateTime utc2 = utc;
    DateTime utc3;
    utc3 = utc;
    DateTime local (DateTime::Kind::Local, 2012, 10, 18, 8, 30, 0, 0);
    DateTime unspec (DateTime::Kind::Unspecified, 2012, 10, 18, 8, 30, 0, 0);

    DateTime dateOnly (2012, 10, 18);
    DateTime dateOnly2 = dateOnly;
    DateTime utcDateOnly (DateTime::Kind::Utc, 2012, 10, 18, 0, 0);
    DateTime unspecifiedDateOnly (DateTime::Kind::Unspecified, 2012, 10, 18, 0, 0);

    //test comparison operator
    EXPECT_TRUE (utc == utc2);
    EXPECT_TRUE (utc == utc3);

    EXPECT_FALSE (utc != utc2);
    EXPECT_FALSE (utc != utc3);

    EXPECT_TRUE (dateOnly == dateOnly2);
    EXPECT_FALSE (dateOnly != dateOnly2);

    EXPECT_FALSE (dateOnly == utcDateOnly);
    EXPECT_FALSE (dateOnly == unspecifiedDateOnly);

    DateTime empty1;
    DateTime empty2;
    EXPECT_TRUE (empty1 == empty2);
    EXPECT_FALSE (empty1 != empty2);

    EXPECT_FALSE (utc == local);
    EXPECT_TRUE (utc != local);

    EXPECT_FALSE (utc == unspec);
    EXPECT_TRUE (utc != unspec);

    EXPECT_FALSE (local == unspec);
    EXPECT_TRUE (local != unspec);

    //test Compare method
    EXPECT_TRUE (utc.Equals (utc2, true));
    EXPECT_TRUE (utc.Equals (utc2, false));
    EXPECT_EQ ((int) DateTime::CompareResult::Equals, (int) DateTime::Compare (utc, utc2));

    EXPECT_TRUE (utc.Equals (utc3, true));
    EXPECT_TRUE (utc.Equals (utc3, false));
    EXPECT_EQ ((int) DateTime::CompareResult::Equals, (int) DateTime::Compare (utc, utc3));

    EXPECT_TRUE (utc.Equals (local, true));
    EXPECT_FALSE (utc.Equals (local, false));
    EXPECT_TRUE (local.Equals (utc, true));
    EXPECT_FALSE (local.Equals (utc, false));

    EXPECT_TRUE (utc.Equals (unspec, true));
    EXPECT_FALSE (utc.Equals (unspec, false));
    EXPECT_TRUE (unspec.Equals (utc, true));
    EXPECT_FALSE (unspec.Equals (utc, false));

    EXPECT_TRUE (local.Equals (unspec, true));
    EXPECT_FALSE (local.Equals (unspec, false));
    EXPECT_TRUE (unspec.Equals (local, true));
    EXPECT_FALSE (unspec.Equals (local, false));

    EXPECT_TRUE (dateOnly.Equals (dateOnly2, true));
    EXPECT_TRUE (dateOnly.Equals (dateOnly2, false));

    EXPECT_TRUE (dateOnly.Equals (utcDateOnly, true));
    EXPECT_FALSE (dateOnly.Equals (utcDateOnly, false));

    EXPECT_TRUE (dateOnly.Equals (unspecifiedDateOnly, true));
    EXPECT_FALSE (dateOnly.Equals (unspecifiedDateOnly, false));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Julija.Suboc                    09/13
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, DateTimeInfoEqualityTests)
    {
    DateTime utc (DateTime::Kind::Utc, 2012, 10, 18, 8, 30, 0, 0);
    DateTime utc2 = utc;
    DateTime utc3;
    utc3 = utc;
    DateTime local (DateTime::Kind::Local, 2012, 10, 18, 8, 30, 0, 0);
    DateTime unspec (DateTime::Kind::Unspecified, 2012, 10, 18, 8, 30, 0, 0);
    utc = local;
    DateTime utcDateOnly (DateTime::Kind::Utc, 2012, 10, 18, 0, 0);
    DateTime dateOnly (2012, 10, 18);
    
    EXPECT_TRUE(utc2.GetInfo() == utc3.GetInfo());
    EXPECT_FALSE(utc2.GetInfo() != utc3.GetInfo());

    EXPECT_FALSE(utc2 == utc);
    EXPECT_FALSE(utc2.GetInfo() == utc.GetInfo());
    EXPECT_TRUE(utc2.GetInfo() != utc.GetInfo());

    EXPECT_FALSE(utc2.GetInfo() != utcDateOnly.GetInfo());
    EXPECT_TRUE(utc2.GetInfo() == utcDateOnly.GetInfo());

    EXPECT_FALSE(dateOnly.GetInfo() == utcDateOnly.GetInfo());
    EXPECT_TRUE(dateOnly.GetInfo() != utcDateOnly.GetInfo());

    EXPECT_FALSE(unspec.GetInfo() == local.GetInfo());
    EXPECT_TRUE(unspec.GetInfo() != local.GetInfo());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    06/14
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, DateTimeComparisonTests)
    {
    std::vector<DateTime> dateTimeDataset = { DateTime (-10, 3, 4), DateTime (2000, 3, 18), DateTime (2014, 6, 30), DateTime (DateTime::Kind::Utc, 2014, 6, 30, 0, 0, 1),
        DateTime (DateTime::Kind::Utc, 2014, 6, 30, 0, 0, 1, 1) };

    const size_t datasetSize = dateTimeDataset.size ();
    for (size_t lhsIx = 0; lhsIx < datasetSize; lhsIx++)
        for (size_t rhsIx = 0; rhsIx < datasetSize; rhsIx++)
            {
            DateTimeCR lhs = dateTimeDataset[lhsIx];
            DateTimeCR rhs = dateTimeDataset[rhsIx];
            const auto res = DateTime::Compare (lhs, rhs);
            if (lhsIx < rhsIx)
                ASSERT_EQ ((int) DateTime::CompareResult::EarlierThan, (int) res) << "Unexpected comparison result. LHS: " << lhs.ToUtf8String ().c_str () << " RHS: " << rhs.ToUtf8String ().c_str ();
            else if (lhsIx == rhsIx)
                ASSERT_EQ ((int) DateTime::CompareResult::Equals, (int) res) << "Unexpected comparison result. LHS: " << lhs.ToUtf8String ().c_str () << " RHS: " << rhs.ToUtf8String ().c_str ();
            else 
                ASSERT_EQ ((int) DateTime::CompareResult::LaterThan, (int) res) << "Unexpected comparison result. LHS: " << lhs.ToUtf8String ().c_str () << " RHS: " << rhs.ToUtf8String ().c_str ();
            }

    ASSERT_EQ ((int) DateTime::CompareResult::Equals, (int) DateTime::Compare (DateTime (2014, 6, 30), DateTime (DateTime::Kind::Utc, 2014, 6, 30, 0, 0)));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Julija.Suboc                    09/13
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, DateTimeComponentToString)
    {
    Utf8String date = DateTime::Info::ComponentToString(DateTime::Component::Date);
    EXPECT_EQ(0, date.CompareTo("Date"));
    Utf8String dateTime = DateTime::Info::ComponentToString(DateTime::Component::DateAndTime);
    EXPECT_EQ(0, dateTime.CompareTo("DateTime"));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, GetMillisecondTests)
    {
    DateTime testDate (DateTime::Kind::Local, 2012, 3, 4, 5, 6, 7, 8888000);
    uint16_t millisecs = testDate.GetMillisecond ();
    EXPECT_EQ (888, millisecs);

    testDate = DateTime (DateTime::Kind::Local, 2012, 3, 4, 5, 6, 7, 8885000);
    millisecs = testDate.GetMillisecond ();
    EXPECT_EQ (888, millisecs);

    testDate = DateTime (DateTime::Kind::Local, 2012, 3, 4, 5, 6, 7, 8880000);
    millisecs = testDate.GetMillisecond ();
    EXPECT_EQ (888, millisecs);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    02/14
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, GetDayOfWeek)
    {
    const DateTime baselineYear (2014, 2, 15);
    const int baselineDayOfWeekNr = static_cast<int> (DateTime::DayOfWeek::Saturday);

    DateTime testDt (baselineYear);
    int i = 0;
    do 
        {
        int expectedDayOfWeekNr = baselineDayOfWeekNr - (i % 7);
        const auto actualDayOfWeek = testDt.GetDayOfWeek ();
        ASSERT_EQ (expectedDayOfWeekNr, static_cast<int> (actualDayOfWeek)) << "GetDayOfWeek failed for test date " << testDt.ToUtf8String ().c_str ();

        //decrement test date by one day
        double testJd = 0.0;
        ASSERT_EQ (SUCCESS, testDt.ToJulianDay (testJd));
        testJd--;
        ASSERT_EQ (SUCCESS, DateTime::FromJulianDay (testDt, testJd, testDt.GetInfo ()));

        i++;
        } while (testDt.GetYear () > 1); //dates before 1 AD are not supported
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    02/14
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, GetDayOfYear)
    {
    auto runTest = [] (int16_t year)
        {
        DateTime testDt (year, 1, 1);
        const int dayCount = DateTime::IsLeapYear (year) ? 366 : 365;
        for (int i = 0; i < dayCount; i++)
            {
            int expectedDayOfYear = i + 1;
            int actualDayOfYear = testDt.GetDayOfYear ();
            ASSERT_EQ (expectedDayOfYear, actualDayOfYear) << "GetDayOfYear failed for test date " << testDt.ToUtf8String ().c_str ();

            //incrememt test date by one day
            double testJd = 0.0;
            ASSERT_EQ (SUCCESS, testDt.ToJulianDay (testJd));
            testJd++;
            ASSERT_EQ (SUCCESS, DateTime::FromJulianDay (testDt, testJd, testDt.GetInfo ()));
            }
        };

    runTest (1); //no leap year
    runTest (1900); //no leap year
    runTest (1904); //leap year
    runTest (2000); //leap year
    runTest (2001); //no leap year
    runTest (2004); //leap year
    runTest (2014); //no leap year
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    02/13
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, StringConversionRoundtripTests)
    {
    DateTime testDate (2012, 3, 1);
    Utf8CP isoDate = "2012-03-01";
    DateTimeAsserter::AssertStringConversion (testDate, isoDate);
    
    testDate = DateTime (-342, 4, 5);
    isoDate = "-0342-04-05";
    DateTimeAsserter::AssertStringConversion (testDate, isoDate);

    testDate = DateTime (1, 10, 5);
    isoDate = "0001-10-05";
    DateTimeAsserter::AssertStringConversion (testDate, isoDate);

    testDate = DateTime (DateTime::Kind::Unspecified, 2012, 3, 1, 2, 3, 0);
    isoDate = "2012-03-01T02:03:00.000";
    DateTimeAsserter::AssertStringConversion (testDate, isoDate);

    testDate = DateTime (DateTime::Kind::Utc, 2012, 12, 5, 23, 44, 59, 5000000);
    isoDate = "2012-12-05T23:44:59.500Z";
    DateTimeAsserter::AssertStringConversion (testDate, isoDate);

    testDate = DateTime (DateTime::Kind::Utc, 2012, 12, 5, 23, 44, 59, 1111999);
    isoDate = "2012-12-05T23:44:59.111Z";
    DateTimeAsserter::AssertStringConversion (testDate, isoDate);

    testDate = DateTime (DateTime::Kind::Utc, 2012, 12, 5, 23, 44, 59, 1114000);
    isoDate = "2012-12-05T23:44:59.111Z";
    DateTimeAsserter::AssertStringConversion (testDate, isoDate);

    testDate = DateTime (DateTime::Kind::Utc, 2012, 12, 5, 23, 44, 59, 1115000);
    isoDate = "2012-12-05T23:44:59.112Z";
    DateTimeAsserter::AssertStringConversion (testDate, isoDate);

    testDate = DateTime (DateTime::Kind::Utc, 2012, 12, 5, 23, 44, 59, 1116000);
    isoDate = "2012-12-05T23:44:59.112Z";
    DateTimeAsserter::AssertStringConversion (testDate, isoDate);

    testDate = DateTime (DateTime::Kind::Utc, 2012, 12, 5, 23, 44, 59, 1119999);
    isoDate = "2012-12-05T23:44:59.112Z";
    DateTimeAsserter::AssertStringConversion (testDate, isoDate);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    03/14
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, ToStringWithMillisecRounding)
    {
    DateTime date (DateTime::Kind::Utc, 2012, 11, 5, 14, 33, 59, 9999444);
    Utf8CP expectedIso = "2012-11-05T14:34:00.000Z";
    ASSERT_STREQ (expectedIso, date.ToUtf8String ().c_str ());

    date = DateTime (DateTime::Kind::Utc, 2012, 11, 5, 14, 59, 59, 9999444);
    expectedIso = "2012-11-05T15:00:00.000Z";
    ASSERT_STREQ (expectedIso, date.ToUtf8String ().c_str ());

    date = DateTime (DateTime::Kind::Utc, 2012, 11, 5, 23, 59, 59, 9999444);
    expectedIso = "2012-11-06T00:00:00.000Z";
    ASSERT_STREQ (expectedIso, date.ToUtf8String ().c_str ());

    date = DateTime (DateTime::Kind::Utc, 2012, 11, 30, 23, 59, 59, 9999444);
    expectedIso = "2012-12-01T00:00:00.000Z";
    ASSERT_STREQ (expectedIso, date.ToUtf8String ().c_str ());

    date = DateTime (DateTime::Kind::Utc, 2012, 1, 30, 23, 59, 59, 9999444);
    expectedIso = "2012-01-31T00:00:00.000Z";
    ASSERT_STREQ (expectedIso, date.ToUtf8String ().c_str ());

    date = DateTime (DateTime::Kind::Utc, 2012, 1, 31, 23, 59, 59, 9999444);
    expectedIso = "2012-02-01T00:00:00.000Z";
    ASSERT_STREQ (expectedIso, date.ToUtf8String ().c_str ());

    date = DateTime (DateTime::Kind::Utc, 2013, 2, 28, 23, 59, 59, 9999444);
    expectedIso = "2013-03-01T00:00:00.000Z";
    ASSERT_STREQ (expectedIso, date.ToUtf8String ().c_str ());

    //leap year
    date = DateTime (DateTime::Kind::Utc, 2012, 2, 28, 23, 59, 59, 9999444);
    expectedIso = "2012-02-29T00:00:00.000Z";
    ASSERT_STREQ (expectedIso, date.ToUtf8String ().c_str ());

    date = DateTime (DateTime::Kind::Utc, 2012, 2, 29, 23, 59, 59, 9999444);
    expectedIso = "2012-03-01T00:00:00.000Z";
    ASSERT_STREQ (expectedIso, date.ToUtf8String ().c_str ());

    date = DateTime (DateTime::Kind::Utc, 2012, 4, 30, 23, 59, 59, 9999444);
    expectedIso = "2012-05-01T00:00:00.000Z";
    ASSERT_STREQ (expectedIso, date.ToUtf8String ().c_str ());

    date = DateTime (DateTime::Kind::Utc, 2012, 12, 31, 23, 59, 59, 9999444);
    expectedIso = "2013-01-01T00:00:00.000Z";
    ASSERT_STREQ (expectedIso, date.ToUtf8String ().c_str ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    02/13
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, FromStringTests)
    {
    //supported ISO strings
    Utf8CP isoDateTime = "2012-12-05";
    DateTime expectedDate (2012, 12, 5);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-2012-12-05";
    expectedDate = DateTime (-2012, 12, 5);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05";
    expectedDate = DateTime (-1, 12, 5);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-00011205";
    expectedDate = DateTime (-1, 12, 5);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05T13:11:22Z";
    expectedDate = DateTime (DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05 13:11:22Z";
    expectedDate = DateTime (DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-00011205T13:11:22.44Z";
    expectedDate = DateTime (DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22, 4400000);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-00011205 13:11:22.44Z";
    expectedDate = DateTime (DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22, 4400000);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05T131122Z";
    expectedDate = DateTime (DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05 131122Z";
    expectedDate = DateTime (DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-00011205T131122.44Z";
    expectedDate = DateTime (DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22, 4400000);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-00011205 131122.44Z";
    expectedDate = DateTime (DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22, 4400000);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-00011205T131122";
    expectedDate = DateTime (DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-00011205 131122";
    expectedDate = DateTime (DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-00011205T1311";
    expectedDate = DateTime (DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-00011205 1311";
    expectedDate = DateTime (DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "20131205T131122-01:45";
    expectedDate = DateTime (DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "20131205 131122-01:45";
    expectedDate = DateTime (DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "20131205T131122-0145";
    expectedDate = DateTime (DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "20131205 131122-0145";
    expectedDate = DateTime (DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "2013-12-05T13:11:22-0145";
    expectedDate = DateTime (DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "2013-12-05 13:11:22-0145";
    expectedDate = DateTime (DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "2013-12-05T131122-0145";
    expectedDate = DateTime (DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "2013-12-05 131122-0145";
    expectedDate = DateTime (DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "20131205T13:11-0145";
    expectedDate = DateTime (DateTime::Kind::Local, 2013, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "20131205 13:11-0145";
    expectedDate = DateTime (DateTime::Kind::Local, 2013, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "20131205T1311-0145";
    expectedDate = DateTime (DateTime::Kind::Local, 2013, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "20131205 1311-0145";
    expectedDate = DateTime (DateTime::Kind::Local, 2013, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05T13:11";
    expectedDate = DateTime (DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05 13:11";
    expectedDate = DateTime (DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-00011205T1311";
    expectedDate = DateTime (DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-00011205 1311";
    expectedDate = DateTime (DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05T13:11Z";
    expectedDate = DateTime (DateTime::Kind::Utc, -1, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05 13:11Z";
    expectedDate = DateTime (DateTime::Kind::Utc, -1, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05T13:11+01:11";
    expectedDate = DateTime (DateTime::Kind::Local, -1, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05 13:11+01:11";
    expectedDate = DateTime (DateTime::Kind::Local, -1, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05T13:11:00.1234567Z";
    expectedDate = DateTime (DateTime::Kind::Utc, 2012, 12, 5, 13, 11, 0, 1234567);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05 13:11:00.1234567Z";
    expectedDate = DateTime (DateTime::Kind::Utc, 2012, 12, 5, 13, 11, 0, 1234567);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05T13:11:00.1234567";
    expectedDate = DateTime (DateTime::Kind::Unspecified, 2012, 12, 5, 13, 11, 0, 1234567);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05T13:11:00.1234567+00:00";
    expectedDate = DateTime (DateTime::Kind::Local, 2012, 12, 5, 13, 11, 0, 1234567);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05 13:11:00.1234567+00:00";
    expectedDate = DateTime (DateTime::Kind::Local, 2012, 12, 5, 13, 11, 0, 1234567);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05T13:11:00.1234567-01:34";
    expectedDate = DateTime (DateTime::Kind::Local, 2012, 12, 5, 13, 11, 0, 1234567);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05 13:11:00.1234567-01:34";
    expectedDate = DateTime (DateTime::Kind::Local, 2012, 12, 5, 13, 11, 0, 1234567);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    //Invalid ISO strings for which the parser still returns something.
    isoDateTime = "2011-02-29";
    expectedDate = DateTime (2011, 2, 29);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);


    //unsupported ISO strings
    //The following are ISO strings which are compliant to ISO but are alternative representations. DateTime::FromString
    //doesn't support them (yet?).
    bvector<Utf8String> unsupportedIsoDateTimeStrings;
    //leaving out month or day or minute
    unsupportedIsoDateTimeStrings.push_back ("2012");
    unsupportedIsoDateTimeStrings.push_back ("-2012");
    unsupportedIsoDateTimeStrings.push_back ("2012-09");
    unsupportedIsoDateTimeStrings.push_back ("2012-09-31T13");
    //omit T or space delimiter
    unsupportedIsoDateTimeStrings.push_back ("2012-09-3013:03:48");
    unsupportedIsoDateTimeStrings.push_back ("20120930130348.123");
    unsupportedIsoDateTimeStrings.push_back ("2012093013:03:48.123Z");
    //leaving out the minutes in the time zone indicator
    unsupportedIsoDateTimeStrings.push_back ("2012-04-29T12:33:11+01");

    //leading / trailing blanks are not supported
    unsupportedIsoDateTimeStrings.push_back (" 2012-12-05 13:11:00.1234567-01:34");
    unsupportedIsoDateTimeStrings.push_back ("        2012-12-05 13:11:00.1234567-01:34");
    unsupportedIsoDateTimeStrings.push_back ("2012-12-05 13:11:00.1234567-01:34 ");
    unsupportedIsoDateTimeStrings.push_back ("2012-12-05 13:11:00.1234567-01:34       ");
    unsupportedIsoDateTimeStrings.push_back (" 2012-12-05 13:11:00.1234567-01:34 ");
    unsupportedIsoDateTimeStrings.push_back ("   2012-12-05 13:11:00.1234567-01:34     ");
    unsupportedIsoDateTimeStrings.push_back (" 2012-12-05");
    unsupportedIsoDateTimeStrings.push_back ("    2012-12-05");
    unsupportedIsoDateTimeStrings.push_back ("2012-12-05 ");
    unsupportedIsoDateTimeStrings.push_back ("2012-12-05    ");
    unsupportedIsoDateTimeStrings.push_back ("  2012-12-05    ");

    //dummy date
    DateTime dummyDate;
    FOR_EACH (Utf8StringCR unsupportedIsoDateTimeString, unsupportedIsoDateTimeStrings)
        {
        DateTimeAsserter::AssertFromString (unsupportedIsoDateTimeString.c_str (), dummyDate, false);
        }

    //invalid ISO strings
    bvector<Utf8String> invalidIsoDateTimeStrings;
    invalidIsoDateTimeStrings.push_back ("012-12-05");
    invalidIsoDateTimeStrings.push_back ("12-12-05");
    invalidIsoDateTimeStrings.push_back ("1-12-05");
    invalidIsoDateTimeStrings.push_back ("2012-13-05");
    invalidIsoDateTimeStrings.push_back ("2012-20-05");
    invalidIsoDateTimeStrings.push_back ("2012-1-05");
    invalidIsoDateTimeStrings.push_back ("2012-5-05");
    invalidIsoDateTimeStrings.push_back ("2012-03-1");
    invalidIsoDateTimeStrings.push_back ("2012-03-32");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T1");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T25:00:12");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:2");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:60:12");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:.12");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T123:12:11");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:331:11");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:1");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:60");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:100");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:11Y");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:11+1");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:11-100:30");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:11-24:30");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:11-01.00");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:11-1:00");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:11-01:0");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:11-01:60");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:11-01:100");

    //double blanks as time comp delimiter or blanks around T delimiter is invalid
    invalidIsoDateTimeStrings.push_back ("2012-09-30  13:03:48.123Z");
    invalidIsoDateTimeStrings.push_back ("2012-09-30T 13:03:48.123Z");
    invalidIsoDateTimeStrings.push_back ("2012-09-30 T13:03:48.123Z");
    invalidIsoDateTimeStrings.push_back ("2012-09-30 T 13:03:48.123Z");

    //valid date time strings which are embedded in a string is not supported.
    invalidIsoDateTimeStrings.push_back ("BlaBla2012-04-29T12:33:11");
    invalidIsoDateTimeStrings.push_back ("BlaBla2012-04-29 12:33:11");
    invalidIsoDateTimeStrings.push_back ("2012-04-29T12:33:11BlaBla");
    invalidIsoDateTimeStrings.push_back ("2012-04-29 12:33:11BlaBla");
    invalidIsoDateTimeStrings.push_back ("BlaBla2012-04-29T12:33:11BlaBla");
    invalidIsoDateTimeStrings.push_back ("BlaBla2012-04-29 12:33:11BlaBla");

    FOR_EACH (Utf8StringCR invalidIsoDateTimeString, invalidIsoDateTimeStrings)
        {
        DateTimeAsserter::AssertFromString (invalidIsoDateTimeString.c_str (), dummyDate, false);
        }
    }
//---------------------------------------------------------------------------------------
// @betest                                      Julija.Suboc                    09/13
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, FromStringWCharCPTests)
    {
    //supported ISO strings
    WCharCP isoDateTime = L"2012-12-05";
    DateTime expectedDate (2012, 12, 5);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = L"-0001-12-05";
    expectedDate = DateTime(-1, 12, 5);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = L"-00011205";
    expectedDate = DateTime(-1, 12, 5);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = L"-0001-12-05T13:11:22Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = L"-0001-12-05 13:11:22Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = L"-00011205T1311";
    expectedDate = DateTime(DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = L"20131205T131122-01:45";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = L"20131205 131122-01:45";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = L"2013-12-05 13:11:22-0145";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = L"2012-12-05T13:11:00.1234567-01:34";
    expectedDate = DateTime(DateTime::Kind::Local, 2012, 12, 5, 13, 11, 0, 1234567);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);

    isoDateTime = L"2012-12-05 13:11:00.1234567-01:34";
    expectedDate = DateTime(DateTime::Kind::Local, 2012, 12, 5, 13, 11, 0, 1234567);
    DateTimeAsserter::AssertFromString (isoDateTime, expectedDate, true);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    02/13
//---------------------------------------------------------------------------------------
TEST (Performance_DateTimeTests, FromString)
    {
    bvector<Utf8String> testDates;
    testDates.push_back ("2012-12-05");
    testDates.push_back ("2013-12-05T13:11:22");
    testDates.push_back ("2013-12-05T13:11:22Z");
    testDates.push_back ("2013-12-05T13:11:22+01:45");
    testDates.push_back ("2013-01-09T12:30:00.0000000Z");

    const auto repetitionCount = 10000;
    for (auto const& testDate : testDates)
        {
        StopWatch timer (true);
        for (auto i = 0; i < repetitionCount; i++)
            {
            DateTime dt;
            ASSERT_EQ (SUCCESS, DateTime::FromString (dt, testDate.c_str ()));
            }

        timer.Stop ();
        LOG.infov ("DateTime::FromString (\"%hs\"): %.4f msecs [%d repetitions].", testDate.c_str (), timer.GetElapsedSeconds () * 1000.0, repetitionCount);
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, GetCurrentTimeTests)
    {
    DateTime currentTimeLocal = DateTime::GetCurrentTime ();
    EXPECT_EQ ((int) DateTime::Kind::Local, (int) currentTimeLocal.GetInfo ().GetKind ());
    EXPECT_EQ ((int) DateTime::Component::DateAndTime, (int) currentTimeLocal.GetInfo ().GetComponent ());
    //Simple check only to avoid drasticly wrong implementations (as were seen on iOS)
    EXPECT_GE (static_cast<int> (currentTimeLocal.GetYear ()), 2013);

    DateTime currentTimeUtc = DateTime::GetCurrentTimeUtc ();
    EXPECT_EQ ((int) DateTime::Kind::Utc, (int) currentTimeUtc.GetInfo ().GetKind ());
    EXPECT_EQ ((int) DateTime::Component::DateAndTime, (int) currentTimeUtc.GetInfo ().GetComponent ());
    //Simple check only to avoid drasticly wrong implementations (as were seen on iOS)
    EXPECT_GE (static_cast<int> (currentTimeUtc.GetYear ()), 2013);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, ToUtcAndToLocalTests)
    {
    DateTime localTime = DateTime::GetCurrentTime ();
    
    DateTime utc;
    BentleyStatus stat = localTime.ToUtc (utc);
    EXPECT_EQ (SUCCESS, stat);
    EXPECT_EQ ((int) DateTime::Kind::Utc, (int) utc.GetInfo ().GetKind ());
    EXPECT_EQ ((int) DateTime::Component::DateAndTime, (int) utc.GetInfo ().GetComponent ());

    DateTime localTime2;
    stat = utc.ToLocalTime (localTime2);
    EXPECT_EQ (SUCCESS, stat);
    EXPECT_TRUE (localTime == localTime2);

    DateTime res;
    stat = localTime.ToLocalTime (res);
    EXPECT_EQ (ERROR, stat);

    stat = utc.ToUtc (res);
    EXPECT_EQ (ERROR, stat);

    DateTime unspec (2012, 10, 10);
    stat = unspec.ToLocalTime (res);
    EXPECT_EQ (ERROR, stat);

    stat = unspec.ToUtc (res);
    EXPECT_EQ (ERROR, stat);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, TestsWithDatesBeforeGregorianCalendarReform)
    {
    DateTimeTestItemList testItemList;
    DateTimeTestDataset::CreateBeforeGregorianCalendarReformTestDataset (testItemList);

    DateTimeAsserter::Assert (testItemList);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
void RunTestsOnBaseTestDataset
(
)
    {
    DateTimeTestItemList testItemUtcList;
    DateTimeTestDataset::CreateBaseTestDataset (testItemUtcList, DateTime::Kind::Utc);

    DateTimeTestItemList testItemLocalList;
    DateTimeTestDataset::CreateBaseTestDataset (testItemLocalList, DateTime::Kind::Local);
    //local date tests can only be run if the current local time zone offset could be computed.
    //if that failed (e.g. because the tests run on a platform not supported by these tests)
    //the test date vector is empty.
    const bool runLocalDateTimeTests = testItemLocalList.size () > 0;

    DateTimeTestItemList testItemUnspecifiedList;
    DateTimeTestDataset:: CreateBaseTestDataset (testItemUnspecifiedList, DateTime::Kind::Unspecified);

    DateTimeAsserter::Assert (testItemUtcList);
    DateTimeAsserter::Assert (testItemUnspecifiedList);
    
    if (runLocalDateTimeTests)
        {
        DateTimeAsserter::Assert (testItemLocalList);
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, ConversionsTestsInCurrentTimezone)
    {
    LOG.info (L"Tests use the machine's current time zone setting.");
    //not setting TZ here which means that the current time zone from the system settings is used instead
    RunTestsOnBaseTestDataset ();
    }


#if defined(BENTLEY_WIN32)
//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, ConversionTestsInCET)
    {
    //Central European Time UTC+01
    TzSetter tz;
    tz.Set ("MEZ-1MESZ");

    RunTestsOnBaseTestDataset ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, ConversionTestsInUSEST)
    {
    //US Eastern Standard Time UTC-05
    TzSetter tz;
    tz.Set ("EST5EDT");

    RunTestsOnBaseTestDataset ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, ConversionTestsInAustralianEST)
    {
    //Australian Eastern Standard Time UTC+10
    TzSetter tz;
    tz.Set ("EST-10EDT");

    RunTestsOnBaseTestDataset ();
    }

#endif


//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, ConversionAccuracyTests)
    {
    DateTimeTestItemList testDataset;
    DateTimeTestDataset::CreateAccuracyTestDataset (testDataset);
    DateTimeAsserter::Assert (testDataset);
    }

#if defined(BENTLEY_WIN32) 

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, ConversionTestsAtBrimOfUnixEpochInCET)
    {
    //Central European Time UTC+01
    TzSetter tz;
    tz.Set ("MEZ-1MESZ");

    DateTimeTestItemList testDataset;
    DateTimeTestDataset::CreateBrimOfUnixEpochCETTestDataset (testDataset);
    DateTimeAsserter::Assert (testDataset);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, ConversionTestsAtBrimOfUnixEpochInEST)
    {
    //US Eastern Standard Time UTC-05
    TzSetter tz;
    tz.Set ("EST5EDT");

    DateTimeTestItemList testDataset;
    DateTimeTestDataset::CreateBrimOfUnixEpochESTTestDataset (testDataset);
    DateTimeAsserter::Assert (testDataset);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, ConversionTestDSTUK)
    {
    if (!TzSetter::CurrentMachineTimezoneObservesDst ())
        {
        LOG.info (L"Current machine's time zone does not observe DST. CRT functions on Windows return strange results for dates in DST. Therefore skipping DST tests.");
        return;
        }

    // Greenwich mean time (UK) (UTC+00)
    TzSetter tz;
    tz.Set ("GMT0BST");

    //Transition to and from DST is a bit undefined as more than one local time maps to one UTC point in time.
    //The algo used in DateTime differs is in-synch with the SQLite algo.
    //So these tests here use the values SQLite computes as expected values.

    DateTimeTestItemList testDataset;
    DateTimeTestDataset::CreateDstUKTestDataset (testDataset);
    DateTimeAsserter::Assert (testDataset);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTests, ConversionTestsDSTEST)
    {
    if (!TzSetter::CurrentMachineTimezoneObservesDst ())
        {
        LOG.info (L"Current machine's time zone does not observe DST. CRT functions on Windows return strange results for dates in DST. Therefore skipping DST tests.");
        return;
        }

    //US Eastern Standard Time UTC-05
    TzSetter tz;
    tz.Set ("EST5EDT");

    //Transition to and from DST is a bit undefined as more than one local time maps to one UTC point in time.
    //The algo used in DateTime differs is in-synch with the SQLite algo.
    //So these tests here use the values SQLite computes as expected values.

    DateTimeTestItemList testDataset;
    DateTimeTestDataset::CreateDstUSESTTestDataset (testDataset);
    DateTimeAsserter::Assert (testDataset);
    }

//***************************************************************************************
//Test fixture which can only be run when the machine's time zone is CET
//***************************************************************************************

#if defined(CET_IS_SYSTEMTIMEZONE) 
//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeTestsWhenCETIsSystemTimezone, ConversionTestsDSTCET)
    {
    //these tests do not use the TZ env var to force the current time zone to be CET because 
    //when using TZ the US DST rules are applied instead of the European ones. These tests want
    //to cover the actual (i.e. European) rules though and therefore can only be run
    //when the machine's current time zone is set to CET.

    //Transition to and from DST is a bit undefined as more than one local time maps to one UTC point in time.
    //The algo used in DateTime differs is in-synch with the SQLite algo.
    //So these tests here use the values SQLite computes as expected values.
    DateTimeTestItemList testDataset;
    DateTimeTestDataset::CreateDstCETTestDataset (testDataset);
    DateTimeAsserter::Assert (testDataset);
    }

#endif

#endif
