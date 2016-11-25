/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/DateTime_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/DateTime.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <vector>

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, DateTimeConstructorTests)
    {
    //default constructor
    DateTime dateTime;
    ASSERT_TRUE(!dateTime.IsValid());

    //with positive year
    DateTime::Kind expectedKind = DateTime::Kind::Local;
    int16_t expectedYear = 2012;
    uint8_t expectedMonth = 10;
    uint8_t expectedDay = 17;
    uint8_t expectedHour = 18;
    uint8_t expectedMinute = 3;
    uint8_t expectedSecond = 18;
    uint16_t expectedMillisecond = 100;

    dateTime = DateTime(expectedKind, expectedYear, expectedMonth, expectedDay, expectedHour, expectedMinute, expectedSecond, expectedMillisecond);
    ASSERT_TRUE(dateTime.IsValid());
    ASSERT_EQ((int) expectedKind, (int) dateTime.GetInfo().GetKind());
    ASSERT_EQ((int) DateTime::Component::DateAndTime, (int) dateTime.GetInfo().GetComponent());
    ASSERT_EQ(expectedYear, dateTime.GetYear());
    ASSERT_EQ(expectedMonth, dateTime.GetMonth());
    ASSERT_EQ(expectedDay, dateTime.GetDay());
    ASSERT_EQ(expectedHour, dateTime.GetHour());
    ASSERT_EQ(expectedMinute, dateTime.GetMinute());
    ASSERT_EQ(expectedSecond, dateTime.GetSecond());
    ASSERT_EQ(expectedMillisecond, dateTime.GetMillisecond());

    //with negative year
    expectedKind = DateTime::Kind::Local;
    expectedYear = -3123;
    expectedMonth = 10;
    expectedDay = 17;
    expectedHour = 18;
    expectedMinute = 3;
    expectedSecond = 18;
    expectedMillisecond = 100;

    dateTime = DateTime(expectedKind, expectedYear, expectedMonth, expectedDay, expectedHour, expectedMinute, expectedSecond, expectedMillisecond);
    ASSERT_TRUE(dateTime.IsValid());
    ASSERT_EQ((int) expectedKind, (int) dateTime.GetInfo().GetKind());
    ASSERT_EQ((int) DateTime::Component::DateAndTime, (int) dateTime.GetInfo().GetComponent());
    ASSERT_EQ(expectedYear, dateTime.GetYear());
    ASSERT_EQ(expectedMonth, dateTime.GetMonth());
    ASSERT_EQ(expectedDay, dateTime.GetDay());
    ASSERT_EQ(expectedHour, dateTime.GetHour());
    ASSERT_EQ(expectedMinute, dateTime.GetMinute());
    ASSERT_EQ(expectedSecond, dateTime.GetSecond());
    ASSERT_EQ(expectedMillisecond, dateTime.GetMillisecond());

    //with date only
    DateTime dateOnly(expectedYear, expectedMonth, expectedDay);
    ASSERT_TRUE(dateOnly.IsValid());
    ASSERT_EQ((int) DateTime::Kind::Unspecified, (int) dateOnly.GetInfo().GetKind());
    ASSERT_EQ((int) DateTime::Component::Date, (int) dateOnly.GetInfo().GetComponent());
    ASSERT_EQ(expectedYear, dateOnly.GetYear());
    ASSERT_EQ(expectedMonth, dateOnly.GetMonth());
    ASSERT_EQ(expectedDay, dateOnly.GetDay());
    ASSERT_EQ(0, dateOnly.GetHour());
    ASSERT_EQ(0, dateOnly.GetMinute());
    ASSERT_EQ(0, dateOnly.GetSecond());
    ASSERT_EQ(0, dateOnly.GetMillisecond());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, DateTimeSpecialMemberTests)
    {
    DateTime d1(DateTime::Kind::Utc, 2012, 10, 18, 8, 30, 0, 0);
    ASSERT_TRUE(d1.IsValid());
    //test copy constructor
    DateTime d2(d1);
    ASSERT_EQ(d1, d2);

    //test copy assignment operator
    DateTime d3;
    ASSERT_TRUE(!d3.IsValid());
    d3 = d1;
    ASSERT_EQ(d1, d3);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/14
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, DateTimeIsValid)
    {
    //tests with invalid date times (created with default ctor)
    DateTime dt;
    ASSERT_FALSE(dt.IsValid());

    BeTest::SetFailOnAssert(false);
    {
    ASSERT_EQ(DateTime::DayOfWeek::Sunday, dt.GetDayOfWeek());
    ASSERT_EQ(0, dt.GetDayOfYear());
    }
    BeTest::SetFailOnAssert(true);

    DateTime newDt;
    ASSERT_EQ(ERROR, dt.ToLocalTime(newDt));
    ASSERT_EQ(ERROR, dt.ToUtc(newDt));

    double jd = 0.0;
    ASSERT_EQ(ERROR, dt.ToJulianDay(jd));
    uint64_t jdHns = 0ULL;
    ASSERT_EQ(ERROR, dt.ToJulianDay(jdHns));

    int64_t millisecs = -1LL;
    ASSERT_EQ(ERROR, dt.ToUnixMilliseconds(millisecs));

    Utf8String actualStr = dt.ToString();
    ASSERT_TRUE(actualStr.empty());

    WString actualStrW = dt.ToWString();
    ASSERT_TRUE(actualStrW.empty());

    //tests validity of DateTime objects created by conversion
    DateTime now = DateTime::GetCurrentTime();
    ASSERT_TRUE(now.IsValid());
    now = DateTime::GetCurrentTimeUtc();
    ASSERT_TRUE(now.IsValid());

    ASSERT_EQ(SUCCESS, now.ToJulianDay(jd));
    DateTime resultDt;
    ASSERT_FALSE(resultDt.IsValid());
    ASSERT_EQ(SUCCESS, DateTime::FromJulianDay(resultDt, jd, DateTime::Info::CreateForDateTime(DateTime::Kind::Utc)));
    ASSERT_TRUE(resultDt.IsValid());

    ASSERT_EQ(SUCCESS, now.ToUnixMilliseconds(millisecs));
    resultDt = DateTime();
    ASSERT_FALSE(resultDt.IsValid());
    ASSERT_EQ(SUCCESS, DateTime::FromUnixMilliseconds(resultDt, millisecs));
    ASSERT_TRUE(resultDt.IsValid());

    resultDt = DateTime();
    ASSERT_FALSE(resultDt.IsValid());
    ASSERT_EQ(SUCCESS, DateTime::FromString(resultDt, now.ToString().c_str()));
    ASSERT_TRUE(resultDt.IsValid());

    resultDt = DateTime();
    ASSERT_FALSE(resultDt.IsValid());
    ASSERT_EQ(SUCCESS, DateTime::FromString(resultDt, now.ToWString().c_str()));
    ASSERT_TRUE(resultDt.IsValid());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, DateTimeEqualityTests)
    {
    DateTime dt1, dt2;
    ASSERT_TRUE(!dt1.IsValid());
    ASSERT_TRUE(!dt2.IsValid());
    ASSERT_TRUE(dt1 == dt2) << "Two invalid date times are expected to be interpreted as equal";

    DateTime utc(DateTime::Kind::Utc, 2012, 10, 18, 8, 30, 0, 0);
    DateTime utc2 = utc;
    DateTime utc3;
    utc3 = utc;
    DateTime local(DateTime::Kind::Local, 2012, 10, 18, 8, 30, 0, 0);
    DateTime unspec(DateTime::Kind::Unspecified, 2012, 10, 18, 8, 30, 0, 0);

    DateTime dateOnly(2012, 10, 18);
    DateTime dateOnly2 = dateOnly;
    DateTime utcDateOnly(DateTime::Kind::Utc, 2012, 10, 18, 0, 0);
    DateTime unspecifiedDateOnly(DateTime::Kind::Unspecified, 2012, 10, 18, 0, 0);

    //test comparison operator
    EXPECT_TRUE(utc == utc2);
    EXPECT_TRUE(utc == utc3);

    EXPECT_FALSE(utc != utc2);
    EXPECT_FALSE(utc != utc3);

    EXPECT_TRUE(dateOnly == dateOnly2);
    EXPECT_FALSE(dateOnly != dateOnly2);

    EXPECT_FALSE(dateOnly == utcDateOnly);
    EXPECT_FALSE(dateOnly == unspecifiedDateOnly);

    DateTime empty1;
    DateTime empty2;
    EXPECT_TRUE(empty1 == empty2);
    EXPECT_FALSE(empty1 != empty2);

    EXPECT_FALSE(utc == local);
    EXPECT_TRUE(utc != local);

    EXPECT_FALSE(utc == unspec);
    EXPECT_TRUE(utc != unspec);

    EXPECT_FALSE(local == unspec);
    EXPECT_TRUE(local != unspec);

    //test Compare method
    EXPECT_TRUE(utc.Equals(utc2, true));
    EXPECT_TRUE(utc.Equals(utc2, false));
    EXPECT_EQ((int) DateTime::CompareResult::Equals, (int) DateTime::Compare(utc, utc2));

    EXPECT_TRUE(utc.Equals(utc3, true));
    EXPECT_TRUE(utc.Equals(utc3, false));
    EXPECT_EQ((int) DateTime::CompareResult::Equals, (int) DateTime::Compare(utc, utc3));

    EXPECT_TRUE(utc.Equals(local, true));
    EXPECT_FALSE(utc.Equals(local, false));
    EXPECT_TRUE(local.Equals(utc, true));
    EXPECT_FALSE(local.Equals(utc, false));

    EXPECT_TRUE(utc.Equals(unspec, true));
    EXPECT_FALSE(utc.Equals(unspec, false));
    EXPECT_TRUE(unspec.Equals(utc, true));
    EXPECT_FALSE(unspec.Equals(utc, false));

    EXPECT_TRUE(local.Equals(unspec, true));
    EXPECT_FALSE(local.Equals(unspec, false));
    EXPECT_TRUE(unspec.Equals(local, true));
    EXPECT_FALSE(unspec.Equals(local, false));

    EXPECT_TRUE(dateOnly.Equals(dateOnly2, true));
    EXPECT_TRUE(dateOnly.Equals(dateOnly2, false));

    EXPECT_TRUE(dateOnly.Equals(utcDateOnly, true));
    EXPECT_FALSE(dateOnly.Equals(utcDateOnly, false));

    EXPECT_TRUE(dateOnly.Equals(unspecifiedDateOnly, true));
    EXPECT_FALSE(dateOnly.Equals(unspecifiedDateOnly, false));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Julija.Suboc                    09/13
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, DateTimeInfoEqualityTests)
    {
    DateTime utc(DateTime::Kind::Utc, 2012, 10, 18, 8, 30, 0, 0);
    DateTime utc2 = utc;
    DateTime utc3;
    utc3 = utc;
    DateTime local(DateTime::Kind::Local, 2012, 10, 18, 8, 30, 0, 0);
    DateTime unspec(DateTime::Kind::Unspecified, 2012, 10, 18, 8, 30, 0, 0);
    utc = local;
    DateTime utcDateOnly(DateTime::Kind::Utc, 2012, 10, 18, 0, 0);
    DateTime dateOnly(2012, 10, 18);

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
TEST(DateTimeTests, DateTimeComparisonTests)
    {
    std::vector<DateTime> dateTimeDataset = {DateTime(-10, 3, 4), DateTime(2000, 3, 18), DateTime(2014, 6, 30), DateTime(DateTime::Kind::Utc, 2014, 6, 30, 0, 0, 1),
        DateTime(DateTime::Kind::Utc, 2014, 6, 30, 0, 0, 1, 1)};

    const size_t datasetSize = dateTimeDataset.size();
    for (size_t lhsIx = 0; lhsIx < datasetSize; lhsIx++)
        for (size_t rhsIx = 0; rhsIx < datasetSize; rhsIx++)
            {
            DateTimeCR lhs = dateTimeDataset[lhsIx];
            DateTimeCR rhs = dateTimeDataset[rhsIx];
            const auto res = DateTime::Compare(lhs, rhs);
            if (lhsIx < rhsIx)
                ASSERT_EQ((int) DateTime::CompareResult::EarlierThan, (int) res) << "Unexpected comparison result. LHS: " << lhs.ToString().c_str() << " RHS: " << rhs.ToString().c_str();
            else if (lhsIx == rhsIx)
                ASSERT_EQ((int) DateTime::CompareResult::Equals, (int) res) << "Unexpected comparison result. LHS: " << lhs.ToString().c_str() << " RHS: " << rhs.ToString().c_str();
            else
                ASSERT_EQ((int) DateTime::CompareResult::LaterThan, (int) res) << "Unexpected comparison result. LHS: " << lhs.ToString().c_str() << " RHS: " << rhs.ToString().c_str();
            }

    ASSERT_EQ((int) DateTime::CompareResult::Equals, (int) DateTime::Compare(DateTime(2014, 6, 30), DateTime(DateTime::Kind::Utc, 2014, 6, 30, 0, 0)));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    02/14
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, GetDayOfWeek)
    {
    const DateTime baselineYear(2014, 2, 15);
    const int baselineDayOfWeekNr = static_cast<int> (DateTime::DayOfWeek::Saturday);

    DateTime testDt(baselineYear);
    int i = 0;
    do
        {
        int expectedDayOfWeekNr = baselineDayOfWeekNr - (i % 7);
        const auto actualDayOfWeek = testDt.GetDayOfWeek();
        ASSERT_EQ(expectedDayOfWeekNr, static_cast<int> (actualDayOfWeek)) << "GetDayOfWeek failed for test date " << testDt.ToString().c_str();

        //decrement test date by one day
        double testJd = 0.0;
        ASSERT_EQ(SUCCESS, testDt.ToJulianDay(testJd));
        testJd--;
        ASSERT_EQ(SUCCESS, DateTime::FromJulianDay(testDt, testJd, testDt.GetInfo()));

        i++;
        } while (testDt.GetYear() > 1); //dates before 1 AD are not supported
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    02/14
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, GetDayOfYear)
    {
    auto runTest = [] (int16_t year)
        {
        DateTime testDt(year, 1, 1);
        const int dayCount = DateTime::IsLeapYear(year) ? 366 : 365;
        for (int i = 0; i < dayCount; i++)
            {
            int expectedDayOfYear = i + 1;
            int actualDayOfYear = testDt.GetDayOfYear();
            ASSERT_EQ(expectedDayOfYear, actualDayOfYear) << "GetDayOfYear failed for test date " << testDt.ToString().c_str();

            //incrememt test date by one day
            double testJd = 0.0;
            ASSERT_EQ(SUCCESS, testDt.ToJulianDay(testJd));
            testJd++;
            ASSERT_EQ(SUCCESS, DateTime::FromJulianDay(testDt, testJd, testDt.GetInfo()));
            }
        };

    runTest(1); //no leap year
    runTest(1900); //no leap year
    runTest(1904); //leap year
    runTest(2000); //leap year
    runTest(2001); //no leap year
    runTest(2004); //leap year
    runTest(2014); //no leap year
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    02/13
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, StringConversionRoundtripTests)
    {
    auto assertStringConversion = [] (DateTimeCR date, Utf8CP dateStr)
        {
        Utf8String actualStr = date.ToString();
        ASSERT_STREQ(dateStr, actualStr.c_str());

        DateTime actualDt;
        ASSERT_EQ(SUCCESS, DateTime::FromString(actualDt, dateStr));
        ASSERT_EQ(date, actualDt);
        };

    DateTime testDate(2012, 3, 1);
    Utf8CP isoDate = "2012-03-01";
    assertStringConversion(testDate, isoDate);

    testDate = DateTime(-342, 4, 5);
    isoDate = "-0342-04-05";
    assertStringConversion(testDate, isoDate);

    testDate = DateTime(1, 10, 5);
    isoDate = "0001-10-05";
    assertStringConversion(testDate, isoDate);

    testDate = DateTime(DateTime::Kind::Unspecified, 2012, 3, 1, 2, 3, 0);
    isoDate = "2012-03-01T02:03:00.000";
    assertStringConversion(testDate, isoDate);

    testDate = DateTime(DateTime::Kind::Utc, 2012, 12, 5, 23, 44, 59, 500);
    isoDate = "2012-12-05T23:44:59.500Z";
    assertStringConversion(testDate, isoDate);

    testDate = DateTime(DateTime::Kind::Utc, 2012, 12, 5, 23, 44, 59, 111);
    isoDate = "2012-12-05T23:44:59.111Z";
    assertStringConversion(testDate, isoDate);

    testDate = DateTime(DateTime::Kind::Utc, 2012, 12, 5, 23, 44, 59, 112);
    isoDate = "2012-12-05T23:44:59.112Z";
    assertStringConversion(testDate, isoDate);

    testDate = DateTime(DateTime::Kind::Utc, 2012, 12, 5, 23, 44, 59, 999);
    isoDate = "2012-12-05T23:44:59.999Z";
    assertStringConversion(testDate, isoDate);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    02/13
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, FromStringTests)
    {
    auto assertFromString = [] (Utf8CP dateStr, DateTime const& expectedDt, bool expectedSuccess)
        {
        DateTime actualDt;
        if (expectedSuccess)
            {
            ASSERT_EQ(SUCCESS, DateTime::FromString(actualDt, dateStr));
            ASSERT_EQ(expectedDt, actualDt);
            }
        else
            ASSERT_EQ(ERROR, DateTime::FromString(actualDt, dateStr));
        };

    //supported ISO strings
    Utf8CP isoDateTime = "2012-12-05";
    DateTime expectedDate(2012, 12, 5);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-2012-12-05";
    expectedDate = DateTime(-2012, 12, 5);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05";
    expectedDate = DateTime(-1, 12, 5);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-00011205";
    expectedDate = DateTime(-1, 12, 5);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05T13:11:22Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05 13:11:22Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-00011205T13:11:22.44Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22, 440);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-00011205 13:11:22.44Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22, 440);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05T131122Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05 131122Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-00011205T131122.44Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22, 440);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-00011205 131122.44Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22, 440);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-00011205T131122";
    expectedDate = DateTime(DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-00011205 131122";
    expectedDate = DateTime(DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-00011205T1311";
    expectedDate = DateTime(DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-00011205 1311";
    expectedDate = DateTime(DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "20131205T131122-01:45";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "20131205 131122-01:45";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "20131205T131122-0145";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "20131205 131122-0145";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "2013-12-05T13:11:22-0145";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "2013-12-05 13:11:22-0145";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "2013-12-05T131122-0145";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "2013-12-05 131122-0145";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "20131205T13:11-0145";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "20131205 13:11-0145";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "20131205T1311-0145";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "20131205 1311-0145";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05T13:11";
    expectedDate = DateTime(DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05 13:11";
    expectedDate = DateTime(DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-00011205T1311";
    expectedDate = DateTime(DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-00011205 1311";
    expectedDate = DateTime(DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05T13:11Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05 13:11Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05T13:11+01:11";
    expectedDate = DateTime(DateTime::Kind::Local, -1, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "-0001-12-05 13:11+01:11";
    expectedDate = DateTime(DateTime::Kind::Local, -1, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05 13:11:00.123Z";
    expectedDate = DateTime(DateTime::Kind::Utc, 2012, 12, 5, 13, 11, 0, 123);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05T13:11:00.123";
    expectedDate = DateTime(DateTime::Kind::Unspecified, 2012, 12, 5, 13, 11, 0, 123);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05T13:11:00.123+00:00";
    expectedDate = DateTime(DateTime::Kind::Local, 2012, 12, 5, 13, 11, 0, 123);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05 13:11:00.123+00:00";
    expectedDate = DateTime(DateTime::Kind::Local, 2012, 12, 5, 13, 11, 0, 123);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05T13:11:00.123-01:34";
    expectedDate = DateTime(DateTime::Kind::Local, 2012, 12, 5, 13, 11, 0, 123);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = "2012-12-05 13:11:00.123-01:34";
    expectedDate = DateTime(DateTime::Kind::Local, 2012, 12, 5, 13, 11, 0, 123);
    assertFromString(isoDateTime, expectedDate, true);

    //Invalid ISO strings for which the parser still returns something.
    isoDateTime = "2011-02-29";
    expectedDate = DateTime(2011, 2, 29);
    assertFromString(isoDateTime, expectedDate, true);


    //unsupported ISO strings
    //The following are ISO strings which are compliant to ISO but are alternative representations. DateTime::FromString
    //doesn't support them (yet?).
    bvector<Utf8String> unsupportedIsoDateTimeStrings;
    //leaving out month or day or minute
    unsupportedIsoDateTimeStrings.push_back("2012");
    unsupportedIsoDateTimeStrings.push_back("-2012");
    unsupportedIsoDateTimeStrings.push_back("2012-09");
    unsupportedIsoDateTimeStrings.push_back("2012-09-31T13");
    //omit T or space delimiter
    unsupportedIsoDateTimeStrings.push_back("2012-09-3013:03:48");
    unsupportedIsoDateTimeStrings.push_back("20120930130348.123");
    unsupportedIsoDateTimeStrings.push_back("2012093013:03:48.123Z");
    //leaving out the minutes in the time zone indicator
    unsupportedIsoDateTimeStrings.push_back("2012-04-29T12:33:11+01");

    //leading / trailing blanks are not supported
    unsupportedIsoDateTimeStrings.push_back(" 2012-12-05 13:11:00.1234567-01:34");
    unsupportedIsoDateTimeStrings.push_back("        2012-12-05 13:11:00.1234567-01:34");
    unsupportedIsoDateTimeStrings.push_back("2012-12-05 13:11:00.1234567-01:34 ");
    unsupportedIsoDateTimeStrings.push_back("2012-12-05 13:11:00.1234567-01:34       ");
    unsupportedIsoDateTimeStrings.push_back(" 2012-12-05 13:11:00.1234567-01:34 ");
    unsupportedIsoDateTimeStrings.push_back("   2012-12-05 13:11:00.1234567-01:34     ");
    unsupportedIsoDateTimeStrings.push_back(" 2012-12-05");
    unsupportedIsoDateTimeStrings.push_back("    2012-12-05");
    unsupportedIsoDateTimeStrings.push_back("2012-12-05 ");
    unsupportedIsoDateTimeStrings.push_back("2012-12-05    ");
    unsupportedIsoDateTimeStrings.push_back("  2012-12-05    ");

    //dummy date
    DateTime dummyDate;
    for(Utf8StringCR unsupportedIsoDateTimeString : unsupportedIsoDateTimeStrings)
        {
        assertFromString(unsupportedIsoDateTimeString.c_str(), dummyDate, false);
        }

    //invalid ISO strings
    bvector<Utf8String> invalidIsoDateTimeStrings;
    invalidIsoDateTimeStrings.push_back("012-12-05");
    invalidIsoDateTimeStrings.push_back("12-12-05");
    invalidIsoDateTimeStrings.push_back("1-12-05");
    invalidIsoDateTimeStrings.push_back("2012-13-05");
    invalidIsoDateTimeStrings.push_back("2012-20-05");
    invalidIsoDateTimeStrings.push_back("2012-1-05");
    invalidIsoDateTimeStrings.push_back("2012-5-05");
    invalidIsoDateTimeStrings.push_back("2012-03-1");
    invalidIsoDateTimeStrings.push_back("2012-03-32");
    invalidIsoDateTimeStrings.push_back("2012-04-29T");
    invalidIsoDateTimeStrings.push_back("2012-04-29T1");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:");
    invalidIsoDateTimeStrings.push_back("2012-04-29T25:00:12");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:2");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:60:12");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:.12");
    invalidIsoDateTimeStrings.push_back("2012-04-29T123:12:11");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:331:11");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:1");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:60");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:100");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:11Y");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:11+1");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:11-100:30");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:11-24:30");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:11-01.00");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:11-1:00");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:11-01:0");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:11-01:60");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:11-01:100");

    //double blanks as time comp delimiter or blanks around T delimiter is invalid
    invalidIsoDateTimeStrings.push_back("2012-09-30  13:03:48.123Z");
    invalidIsoDateTimeStrings.push_back("2012-09-30T 13:03:48.123Z");
    invalidIsoDateTimeStrings.push_back("2012-09-30 T13:03:48.123Z");
    invalidIsoDateTimeStrings.push_back("2012-09-30 T 13:03:48.123Z");

    //valid date time strings which are embedded in a string is not supported.
    invalidIsoDateTimeStrings.push_back("BlaBla2012-04-29T12:33:11");
    invalidIsoDateTimeStrings.push_back("BlaBla2012-04-29 12:33:11");
    invalidIsoDateTimeStrings.push_back("2012-04-29T12:33:11BlaBla");
    invalidIsoDateTimeStrings.push_back("2012-04-29 12:33:11BlaBla");
    invalidIsoDateTimeStrings.push_back("BlaBla2012-04-29T12:33:11BlaBla");
    invalidIsoDateTimeStrings.push_back("BlaBla2012-04-29 12:33:11BlaBla");

    for(Utf8StringCR invalidIsoDateTimeString : invalidIsoDateTimeStrings)
        {
        assertFromString(invalidIsoDateTimeString.c_str(), dummyDate, false);
        }
    }
//---------------------------------------------------------------------------------------
// @betest                                      Julija.Suboc                    09/13
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, FromStringWCharCPTests)
    {
    auto assertFromString = [] (WCharCP dateStr, DateTime const& expectedDt, bool expectedSuccess)
        {
        DateTime actualDt;
        if (expectedSuccess)
            {
            ASSERT_EQ(SUCCESS, DateTime::FromString(actualDt, dateStr));
            ASSERT_EQ(expectedDt, actualDt);
            }
        else
            ASSERT_EQ(ERROR, DateTime::FromString(actualDt, dateStr));
        };

    //supported ISO strings
    WCharCP isoDateTime = L"2012-12-05";
    DateTime expectedDate(2012, 12, 5);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = L"-0001-12-05";
    expectedDate = DateTime(-1, 12, 5);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = L"-00011205";
    expectedDate = DateTime(-1, 12, 5);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = L"-0001-12-05T13:11:22Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = L"-0001-12-05 13:11:22Z";
    expectedDate = DateTime(DateTime::Kind::Utc, -1, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = L"-00011205T1311";
    expectedDate = DateTime(DateTime::Kind::Unspecified, -1, 12, 5, 13, 11, 0);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = L"20131205T131122-01:45";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = L"20131205 131122-01:45";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = L"2013-12-05 13:11:22-0145";
    expectedDate = DateTime(DateTime::Kind::Local, 2013, 12, 5, 13, 11, 22);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = L"2012-12-05T13:11:00.123-01:34";
    expectedDate = DateTime(DateTime::Kind::Local, 2012, 12, 5, 13, 11, 0, 123);
    assertFromString(isoDateTime, expectedDate, true);

    isoDateTime = L"2012-12-05 13:11:00.123-01:34";
    expectedDate = DateTime(DateTime::Kind::Local, 2012, 12, 5, 13, 11, 0, 123);
    assertFromString(isoDateTime, expectedDate, true);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, GetCurrentTimeTests)
    {
    DateTime currentTimeLocal = DateTime::GetCurrentTime();
    EXPECT_EQ((int) DateTime::Kind::Local, (int) currentTimeLocal.GetInfo().GetKind());
    EXPECT_EQ((int) DateTime::Component::DateAndTime, (int) currentTimeLocal.GetInfo().GetComponent());
    //Simple check only to avoid drasticly wrong implementations (as were seen on iOS)
    EXPECT_GE(static_cast<int> (currentTimeLocal.GetYear()), 2013) << Utf8PrintfString("CurrentTime is %s", currentTimeLocal.ToString().c_str()).c_str();

    DateTime currentTimeUtc = DateTime::GetCurrentTimeUtc();
    EXPECT_EQ((int) DateTime::Kind::Utc, (int) currentTimeUtc.GetInfo().GetKind());
    EXPECT_EQ((int) DateTime::Component::DateAndTime, (int) currentTimeUtc.GetInfo().GetComponent());
    //Simple check only to avoid drasticly wrong implementations (as were seen on iOS)
    EXPECT_GE(static_cast<int> (currentTimeUtc.GetYear()), 2013) << Utf8PrintfString("currentTimeUtc is %s", currentTimeUtc.ToString().c_str()).c_str();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, ToUtcAndToLocalTests)
    {
    DateTime localTime = DateTime::GetCurrentTime();

    DateTime utc;
    BentleyStatus stat = localTime.ToUtc(utc);
    EXPECT_EQ(SUCCESS, stat);
    EXPECT_EQ((int) DateTime::Kind::Utc, (int) utc.GetInfo().GetKind());
    EXPECT_EQ((int) DateTime::Component::DateAndTime, (int) utc.GetInfo().GetComponent());

    DateTime localTime2;
    stat = utc.ToLocalTime(localTime2);
    EXPECT_EQ(SUCCESS, stat);
    EXPECT_TRUE(localTime == localTime2);

    DateTime res;
    stat = localTime.ToLocalTime(res);
    EXPECT_EQ(ERROR, stat);

    stat = utc.ToUtc(res);
    EXPECT_EQ(ERROR, stat);

    DateTime unspec(2012, 10, 10);
    stat = unspec.ToLocalTime(res);
    EXPECT_EQ(ERROR, stat);

    stat = unspec.ToUtc(res);
    EXPECT_EQ(ERROR, stat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Farhad.Kabir                    11/16
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, JulianDayToCommonEra)
    {
    uint64_t expec_ticks = 63612172800000;
    uint64_t ticks = DateTime::JulianDayToCommonEraMilliseconds(212343336000000);
    EXPECT_EQ(expec_ticks, ticks);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Farhad.Kabir                    11/16
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, CommonEraToJulianDay)
    {
    uint64_t expec_jday = 212343336000000;
    uint64_t jday = DateTime::CommonEraMillisecondsToJulianDay(63612172800000);
    EXPECT_EQ(expec_jday, jday);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Farhad.Kabir                    11/16
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, CompareDates)
    {
    //Date Only
    DateTime date1(2014, 02, 13);
    DateTime date2(2014, 04, 13);

    ASSERT_TRUE(date1.IsValid());
    ASSERT_TRUE(date2.IsValid());
    EXPECT_EQ((int) DateTime::CompareResult::EarlierThan, (int) DateTime::Compare(date1, date2));
    EXPECT_EQ((int) DateTime::CompareResult::LaterThan, (int) DateTime::Compare(date2, date1));
    date1 = date2;
    EXPECT_EQ((int) DateTime::CompareResult::Equals, (int) DateTime::Compare(date2, date1));

    //Date and Date Time Objects
    DateTime date3(DateTime::Kind::Utc, 2014, 13, 13, 22, 3, 12, 888);
    DateTime date4(2014, 04, 13);

    ASSERT_TRUE(date3.IsValid());
    ASSERT_TRUE(date4.IsValid());
    EXPECT_EQ((int) DateTime::CompareResult::EarlierThan, (int) DateTime::Compare(date3, date4));
    EXPECT_EQ((int) DateTime::CompareResult::LaterThan, (int) DateTime::Compare(date4, date3));
    date3 = date4;
    EXPECT_EQ((int) DateTime::CompareResult::Equals, (int) DateTime::Compare(date4, date3));

    //DateTime Utc Objects
    DateTime date5(DateTime::Kind::Utc, 2014, 02, 13, 22, 3, 12, 888);
    DateTime date6(DateTime::Kind::Utc, 2015, 10, 13, 22, 3, 12, 889);

    ASSERT_TRUE(date5.IsValid());
    ASSERT_TRUE(date6.IsValid());
    EXPECT_EQ((int) DateTime::CompareResult::EarlierThan, (int) DateTime::Compare(date5, date6));
    EXPECT_EQ((int) DateTime::CompareResult::LaterThan, (int) DateTime::Compare(date6, date5));
    date5 = date6;
    EXPECT_EQ((int) DateTime::CompareResult::Equals, (int) DateTime::Compare(date6, date5));

    //DateTime Local Objects
    DateTime date7(DateTime::Kind::Local, 2014, 02, 13, 22, 3, 12, 888);
    DateTime date8(DateTime::Kind::Local, 2014, 02, 13, 22, 3, 12, 889);

    ASSERT_TRUE(date7.IsValid());
    ASSERT_TRUE(date8.IsValid());
    EXPECT_EQ((int) DateTime::CompareResult::EarlierThan, (int) DateTime::Compare(date7, date8));
    EXPECT_EQ((int) DateTime::CompareResult::LaterThan, (int) DateTime::Compare(date8, date7));
    date7 = date8;
    EXPECT_EQ((int) DateTime::CompareResult::Equals, (int) DateTime::Compare(date8, date7));

    //DateTime Unspecified Objects
    DateTime date9(DateTime::Kind::Unspecified, 2014, 01, 13, 22, 3, 12, 888);
    DateTime date10(DateTime::Kind::Unspecified, 2014, 02, 13, 22, 3, 12, 888);
    ASSERT_TRUE(date9.IsValid());
    ASSERT_TRUE(date10.IsValid());
    EXPECT_EQ((int) DateTime::CompareResult::EarlierThan, (int) DateTime::Compare(date9, date10));
    EXPECT_EQ((int) DateTime::CompareResult::LaterThan, (int) DateTime::Compare(date10, date9));
    date9 = date10;
    EXPECT_EQ((int) DateTime::CompareResult::Equals, (int) DateTime::Compare(date10, date9));

    //Invalid DateTime Objects
    DateTime date11;
    DateTime date12;
    ASSERT_TRUE(!date11.IsValid());
    ASSERT_EQ((int) DateTime::CompareResult::Equals, (int) DateTime::Compare(date11, date12));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Farhad.Kabir                    11/16
//---------------------------------------------------------------------------------------
TEST(DateTimeTests, ComputeOffsetToUtcInMsec)
    {
    DateTime date(2013, 02, 14);
    ASSERT_TRUE(date.IsValid());
    int64_t offset = 0;

    int64_t offset_expected = 0;
    ASSERT_EQ(SUCCESS, date.ComputeOffsetToUtcInMsec(offset));
    EXPECT_EQ(offset_expected, offset);

    DateTime date1(DateTime::Kind::Unspecified, 2013, 02, 14, 23, 23, 56, 888);
    ASSERT_TRUE(date1.IsValid());
    offset = 0;

    offset_expected = 0;
    ASSERT_EQ(SUCCESS, date1.ComputeOffsetToUtcInMsec(offset));
    EXPECT_EQ(offset_expected, offset);

    DateTime date2(DateTime::Kind::Utc, 2013, 02, 14, 23, 23, 56, 888);
    ASSERT_TRUE(date2.IsValid());
    offset = 0;

    offset_expected = 0;
    ASSERT_EQ(SUCCESS, date2.ComputeOffsetToUtcInMsec(offset));
    EXPECT_EQ(offset_expected, offset);

    DateTime local(DateTime::Kind::Local, 2012, 02, 14, 23, 23, 56, 888);
    ASSERT_TRUE(local.IsValid());
    offset = 0;

    offset_expected = 18000000;
    ASSERT_EQ(SUCCESS, local.ComputeOffsetToUtcInMsec(offset)) << "This is Offset " << offset;
    ASSERT_EQ(offset_expected, offset) << "This is Offset " << offset;
    }