/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/DateTimeTestDataset.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/bvector.h>
#include "DateTimeTestDataset.h"

USING_NAMESPACE_BENTLEY
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
BentleyStatus DateTimeTestDataset::ComputeExpectedJD
(
DateTimeTestItem& testItem,
uint64_t expectedJDDateComponent
)
    {
    const DateTime& dateTime = testItem.GetDateTime ();
    int64_t localTimezoneOffsetInHns;
    BentleyStatus stat = ComputeExpectedLocalTimezoneOffset (localTimezoneOffsetInHns, dateTime);
    POSTCONDITION (stat == SUCCESS, ERROR);

    uint64_t jdUtc = expectedJDDateComponent * DateTimeTestHelper::HNS_IN_DAY + (((dateTime.GetHour () + 12) % 24) * 3600 + dateTime.GetMinute () * 60 + dateTime.GetSecond ()) * DateTimeTestHelper::HNS_IN_SEC + dateTime.GetHectoNanosecond ();
    uint64_t expectedJd = jdUtc + localTimezoneOffsetInHns;
    testItem.SetJdInHns (expectedJd);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
BentleyStatus DateTimeTestDataset::ComputeExpectedUnixMillisecs
(
DateTimeTestItem& testItem
)
    {
    const DateTime& dateTime = testItem.GetDateTime ();

    uint64_t unixMillisecsUtc;
    BentleyStatus stat = DateTimeTestHelper::DateTimeToUnixMillisecsCRT (unixMillisecsUtc, dateTime);
    POSTCONDITION (stat == SUCCESS, ERROR);

    return ComputeExpectedUnixMillisecs (testItem, unixMillisecsUtc);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
BentleyStatus DateTimeTestDataset::ComputeExpectedUnixMillisecs
(
DateTimeTestItem& testItem,
int64_t rawUnixMillisecs
)
    {
    const DateTime& dateTime = testItem.GetDateTime ();

    int64_t localTimezoneOffsetInHns;
    BentleyStatus stat = ComputeExpectedLocalTimezoneOffset (localTimezoneOffsetInHns, dateTime);
    POSTCONDITION (stat == SUCCESS, ERROR);

    int64_t expectedUnixMilliseconds = static_cast<int64_t> (rawUnixMillisecs) + localTimezoneOffsetInHns / DateTimeTestHelper::HNS_IN_MSEC;
    testItem.SetUnixMilliseconds (expectedUnixMilliseconds);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
BentleyStatus DateTimeTestDataset::ComputeExpectedLocalTimezoneOffset
(
int64_t& localTimezoneOffsetInHns,
const DateTime& dateTime
)
    {
    if (dateTime.GetInfo ().GetKind () != DateTime::Kind::Local)
        {
        localTimezoneOffsetInHns = 0LL;
        return SUCCESS;
        }

    //1. convert from local to UTC Unix secs
    DateTime utc;
    BentleyStatus stat = DateTimeTestHelper::LocalToUtcViaCRT (utc, dateTime);
    POSTCONDITION (stat == SUCCESS, ERROR);
    int64_t utcUnixMillisecs;
    stat = utc.ToUnixMilliseconds (utcUnixMillisecs);
    POSTCONDITION (stat == SUCCESS, ERROR);
    POSTCONDITION (utcUnixMillisecs >= 0LL, ERROR);

    //2. Compute Unix secs from local without time zone transformation
    uint64_t localUnixMillisecs = 0ULL;
    stat = DateTimeTestHelper::DateTimeToUnixMillisecsCRT (localUnixMillisecs, dateTime);
    POSTCONDITION (stat == SUCCESS, ERROR);

    localTimezoneOffsetInHns = (utcUnixMillisecs - localUnixMillisecs) * DateTimeTestHelper::HNS_IN_MSEC;
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeTestDataset::CreateBaseTestDataset
(
DateTimeTestItemList& testItemList,
DateTime::Kind targetKind
)
    {
    //date in Unix epoch
    DateTimeTestItem testItem (DateTime (DateTime::Kind::Utc, 1972, 10, 8, 13, 33, 24));
    BentleyStatus stat = ComputeExpectedJD (testItem, 2441599ULL);
    ASSERT_EQ (SUCCESS, stat);
    stat = ComputeExpectedUnixMillisecs (testItem, 87399204000LL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (1971, 4, 30));
    stat = ComputeExpectedJD (testItem, 2441071ULL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (targetKind, 1971, 4, 30, 12, 0, 0));
    stat = ComputeExpectedJD (testItem, 2441072ULL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);
    

    testItem = DateTimeTestItem (DateTime (targetKind, 1971, 4, 30, 20, 9, 0));
    stat = ComputeExpectedJD (testItem, 2441072ULL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    //around 12 noon which is where each julian day starts
    testItem = DateTimeTestItem (DateTime (targetKind, 1978, 11, 14, 11, 30, 0));
    stat = ComputeExpectedJD (testItem, 2443826ULL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (targetKind, 1978, 11, 14, 12, 30, 0));
    stat = ComputeExpectedJD (testItem, 2443827ULL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    //date before Unix epoch, but in FILETIME epoch
    testItem = DateTimeTestItem (DateTime (1814, 5, 13));
    stat = ComputeExpectedJD (testItem, 2383741ULL);
    ASSERT_EQ (SUCCESS, stat);
    stat = ComputeExpectedUnixMillisecs (testItem, -4911494400000LL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (targetKind, 1814, 5, 13, 12, 1, 59));
    stat = ComputeExpectedJD (testItem, 2383742ULL);
    ASSERT_EQ (SUCCESS, stat);
    stat = ComputeExpectedUnixMillisecs (testItem, -4911451081000LL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    //date before FILETIME epoch
    testItem = DateTimeTestItem (DateTime (213, 6, 13));
    stat = ComputeExpectedJD (testItem, 1799019ULL);
    ASSERT_EQ (SUCCESS, stat);
    stat = ComputeExpectedUnixMillisecs (testItem, -55431475200000LL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (targetKind, 213, 6, 13, 12, 1, 59));
    stat = ComputeExpectedJD (testItem, 1799020ULL);
    ASSERT_EQ (SUCCESS, stat);
    stat = ComputeExpectedUnixMillisecs (testItem, -55431431881000LL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (targetKind, -1000, 2, 28, 4, 4, 59));
    stat = ComputeExpectedJD (testItem, 1355875ULL);
    ASSERT_EQ (SUCCESS, stat);
    stat = ComputeExpectedUnixMillisecs (testItem, -93719102101000LL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    //roundtrip doesn't work with Meeus algorithm for dates before BCE which are Julian leap years (according to proleptic Julian calendar)
    //and which are no Gregorian leap years (according to proleptic Gregorian calendar).
    testItem = DateTimeTestItem (DateTime (targetKind, -1000, 3, 1, 4, 3, 12), DateTimeTestItem::FromDateTime);
    stat = ComputeExpectedJD (testItem, 1355877ULL);
    ASSERT_EQ (SUCCESS, stat);
    stat = ComputeExpectedUnixMillisecs (testItem, -93718929408000LL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    //dates after Unix epoch (32bit)
    testItem = DateTimeTestItem (DateTime (2100, 7, 18));
    stat = ComputeExpectedJD (testItem, 2488267ULL);
    ASSERT_EQ (SUCCESS, stat);
    stat = ComputeExpectedUnixMillisecs (testItem, 4119552000000LL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (targetKind, 2100, 7, 18, 13, 4, 0));
    stat = ComputeExpectedJD (testItem, 2488268ULL);
    ASSERT_EQ (SUCCESS, stat);
    stat = ComputeExpectedUnixMillisecs (testItem, 4119599040000LL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    //leap year tests
    //1900 is no leap year
    testItem = DateTimeTestItem (DateTime (targetKind, 1900, 3, 1, 14, 2, 0, (uint32_t)(250 * DateTimeTestHelper::HNS_IN_MSEC)));
    stat = ComputeExpectedJD (testItem, 2415080ULL);
    ASSERT_EQ (SUCCESS, stat);
    stat = ComputeExpectedUnixMillisecs (testItem, -2203840680000LL + 250LL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);

    //1904 is a leap year
    testItem = DateTimeTestItem (DateTime (targetKind, 1904, 2, 29, 11, 20, 1, (uint32_t)(333 * DateTimeTestHelper::HNS_IN_MSEC)));
    stat = ComputeExpectedJD (testItem, 2416539ULL);
    ASSERT_EQ (SUCCESS, stat);
    stat = ComputeExpectedUnixMillisecs (testItem, -2077706399000LL + 333LL);
    ASSERT_EQ (SUCCESS, stat);
    testItemList.push_back (testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeTestDataset::CreateBeforeGregorianCalendarReformTestDataset
(
DateTimeTestItemList& testDataset
)
    {
    DateTimeTestItem testItem (DateTime (DateTime::Kind::Utc, -4713, 11, 24,
        12, 0, 0), 0ULL);
    testItem.SetUnixMilliseconds (-210866760000000LL);
    testDataset.push_back (testItem);

    //0000-01-01 12:00:00 UTC
    //1BCE means 0 in astronomical year numbering
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Utc, 0, 1, 1,
        12, 0, 0), 1721060ULL * DateTimeTestHelper::HNS_IN_DAY);
    testItem.SetUnixMilliseconds (-62167176000000LL);
    testDataset.push_back (testItem);

    //0001-01-01 00:00:00 UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Utc, 1, 1, 1,
        0, 0, 0), 1721425ULL * DateTimeTestHelper::HNS_IN_DAY + 12LL * DateTimeTestHelper::HOUR_IN_HNS);
    testItem.SetUnixMilliseconds (-62135596800000LL);
    testDataset.push_back (testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeTestDataset::CreateAccuracyTestDataset
(
DateTimeTestItemList& testDataset
)
    {
    //milli sec accuracy
    DateTimeTestItem testItem (DateTime (DateTime::Kind::Utc, 2012, 10, 18,
                                16, 31, 25, (uint32_t)(123 * DateTimeTestHelper::HNS_IN_MSEC)));

    testItem.SetJdInHns (2122173378851230000ULL);
    testItem.SetUnixMilliseconds (1350577885000ULL + 123ULL);
    testDataset.push_back (testItem);


    //hns accuracy
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Utc, 
                                2012, 10, 18,
                                16, 31, 25, 1234567));

    testItem.SetJdInHns (2122173378851234567ULL);
    testItem.SetUnixMilliseconds (1350577885000ULL + 123ULL);
    testDataset.push_back (testItem);

    //now with rounding in effect
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Utc, 
                                2012, 10, 18,
                                16, 31, 25, 1239999));

    testItem.SetJdInHns (2122173378851239999ULL);
    testItem.SetUnixMilliseconds (1350577885000ULL + 124ULL);
    testDataset.push_back (testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeTestDataset::CreateBrimOfUnixEpochCETTestDataset
(
DateTimeTestItemList& testDataset
)
    {
    //test start of Unix epoch
    DateTimeTestItem testItem (DateTime (DateTime::Kind::Utc, 1970, 1, 1, 0, 0, 0),
                                2108667600000000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_START_MSEC);
    testDataset.push_back (testItem);

    //around beginning of Unix epoch
    //1969 CET always maps to 1969 UTC. So both are outside of Unix epoch!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 1969, 12, 31, 23, 59, 59), 
                                2108667563990000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_START_MSEC - DateTimeTestHelper::HOUR_IN_MSEC - 1000LL);
    testDataset.push_back (testItem);

    //1970-01-01 midnight CET maps to 1969-12-31 23:00:00 UTC -> local is not, but UTC is outside of Unix epoch!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 1970, 1, 1, 0, 0, 0),
                                2108667564000000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_START_MSEC - DateTimeTestHelper::HOUR_IN_MSEC);
    testDataset.push_back (testItem);

    //1970-01-01 00:30 CET maps to 1969-12-31 23:30:00 UTC -> local is not, but UTC is outside of Unix epoch!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 1970, 1, 1, 0, 30, 0),
                                2108667582000000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_START_MSEC - DateTimeTestHelper::HALFHOUR_IN_MSEC);

    testDataset.push_back (testItem);

    //1970-01-01 01:00 CET maps to 1970-01-01 00:00:00 UTC -> both are inside Unix epoch!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 1970, 1, 1, 1, 0, 0),
                                2108667600000000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_START_MSEC);
    testDataset.push_back (testItem);

    //1970-01-01 01:30 CET maps to 1970-01-01 00:30:00 UTC -> both are inside Unix epoch!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 1970, 1, 1, 1, 30, 0),
                                2108667618000000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_START_MSEC + DateTimeTestHelper::HALFHOUR_IN_MSEC);
    testDataset.push_back (testItem);

    //around end of (32bit) Unix epoch
    //2038-01-19 03:14:07 CET maps to 2038-01-19 02:14:07 UTC. Both are inside Unix epoch!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2038, 1, 19, 3, 14, 7),
                                2130142400470000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_END_MSEC - DateTimeTestHelper::HOUR_IN_MSEC);
    
    testDataset.push_back (testItem);

    //2038-01-19 04:14:07 CET maps to 2038-01-19 03:14:07 UTC. Local is outside, UTC still inside Unix epoch!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2038, 1, 19, 4, 14, 7),
                                2130142436470000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_END_MSEC);
    testDataset.push_back (testItem);

    //2038-01-19 04:14:08 CET maps to 2038-01-19 03:14:08 UTC. Both are outside Unix epoch!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2038, 1, 19, 4, 14, 8),
                                2130142436480000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_END_MSEC + 1000LL);
    testDataset.push_back (testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeTestDataset::CreateBrimOfUnixEpochESTTestDataset
(
DateTimeTestItemList& testDataset
)
    {
    //around beginning of Unix epoch
    //1969-12-31 18:00:00 EST maps to 1969-12-31 23:00 UTC -> both outside Unix epoch
    DateTimeTestItem testItem (DateTime (DateTime::Kind::Local, 1969, 12, 31, 18, 0, 0),
                            2108667564000000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_START_MSEC - DateTimeTestHelper::HOUR_IN_MSEC);
    testDataset.push_back (testItem);

    //1969-12-31 19:00:00 EST maps to 1970-01-01 00:00 UTC -> local is outside, UTC is just inside Unix epoch
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 1969, 12, 31, 19, 0, 0),
                                2108667600000000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_START_MSEC);
    testDataset.push_back (testItem);

    //1969-12-31 23:59:59 EST always maps to 1970 UTC -> local is outside, UTC is inside Unix epoch!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 1969, 12, 31, 23, 59, 59),
                                2108667779990000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_START_MSEC + 5LL * DateTimeTestHelper::HOUR_IN_MSEC - 1000LL);
    testDataset.push_back (testItem);

    //1970-01-01 midnight EST maps to 1970-01-01 05:00:00 UTC -> both are inside Unix epoch!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 1970, 1, 1, 0, 0, 0),
                                2108667780000000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_START_MSEC + 5LL * DateTimeTestHelper::HOUR_IN_MSEC);
    testDataset.push_back (testItem);

    //around end of (32bit) Unix epoch
    //2038-01-18 22:14:07 EST maps to 2038-01-19 03:14:07 UTC -> Both are inside!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2038, 1, 18, 22, 14, 7),
                                2130142436470000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_END_MSEC);
    testDataset.push_back (testItem);

    //2038-01-19 00:00 EST maps to 2038-01-19 05:00 UTC -> Local is inside, UTC is outside!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2038, 1, 19, 0, 0, 0),
                                2130142500000000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_END_MSEC + 6353 * 1000LL);
    testDataset.push_back (testItem);

    //2038-01-19 03:14:07 EST maps to 2038-01-19 08:14:07 UTC -> Local is inside, UTC is outside Unix epoch!
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2038, 1, 19, 3, 14, 7),
                                2130142616470000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_END_MSEC + 5LL * DateTimeTestHelper::HOUR_IN_MSEC);
    testDataset.push_back (testItem);

    //2038-01-19 03:14:08 EST maps to 2038-01-19 08:14:08 UTC -> Both are outside.
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2038, 1, 19, 3, 14, 8),
                                2130142616480000000ULL);
    testItem.SetUnixMilliseconds (DateTimeTestHelper::UNIXEPOCH_END_MSEC + 5LL * DateTimeTestHelper::HOUR_IN_MSEC + 1000LL);
    testDataset.push_back (testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeTestDataset::CreateDstUKTestDataset 
(
DateTimeTestItemList& testDataset
)
    {
    //DST in the UK does not start on March 11. The TZ approach falsely assumes though
    //that all DSTs start and end when the US DST starts and ends. As this test intends
    //to cover the changing of the time, rather than the actual dates, this issue is not
    //relevant for this test.

    //dates on brim of switching to DST. Officially DST starts at 2am local time. 
    //Clocks are then switched to 3am right away.

    //1:30am local -> 1:30 UTC -> 1:30 local
    DateTimeTestItem testItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 1, 30, 0),
                                2121981894000000000ULL);
    testItem.SetUnixMilliseconds (1331429400000LL);
    testDataset.push_back (testItem);

    //**Tests with times in critical interval between 2am local and just before 3am local
    //This critical interval usually yields inconsistent results by the underlying OS functions.
    //-> Roundtrip doesn't work here.

    //SQLite: 2am local -> 2am UTC -> 3am local
    //using time which is 1 sec off the actual switch to avoid rounding errors when testing with rational Julian Days
    uint64_t jdInHns = 2121981912010000000ULL;
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 2, 0, 1),
                                 DateTimeTestItem::FromDateTime);
    testItem.SetJdInHns (jdInHns);
    testItem.SetUnixMilliseconds (1331431201000LL);
    testDataset.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 3, 0, 1),
                                 DateTimeTestItem::ToDateTime);
    testItem.SetJdInHns (jdInHns);
    testItem.SetUnixMilliseconds (1331431201000LL);
    testDataset.push_back (testItem);


    //times between 2am and 3am don't make sense actually as time is switched from 2am to 3am at 2am.
    //SQLite: 2:30am local -> 2:30am UTC -> 3:30am local
    jdInHns = 2121981930000000000ULL;
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 2, 30, 0),
                                 DateTimeTestItem::FromDateTime);
    testItem.SetJdInHns (jdInHns);
    testItem.SetUnixMilliseconds (1331433000000LL);
    testDataset.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 3, 30, 0),
                                 DateTimeTestItem::ToDateTime);
    testItem.SetJdInHns (jdInHns);
    testItem.SetUnixMilliseconds (1331433000000LL);
    testDataset.push_back (testItem);

    //**now test times are outside uncritical interval again -> roundtrip works

    //3am local maps to 2am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 3, 00, 0),
                                    2121981912000000000ULL);
    testItem.SetUnixMilliseconds (1331431200000LL);
    testDataset.push_back (testItem);

    //3:30am local maps to 2:30 UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 3, 30, 0),
                                    2121981930000000000ULL);
    testItem.SetUnixMilliseconds (1331433000000LL);
    testDataset.push_back (testItem);

    //dates on brim of switching back from DST. Officially DST ends at 3am DST local time. 
    //Clocks are then switched back to 2am.

    //0:30am local maps to 23:30am UTC day before
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 11, 4, 0, 30, 0),
                                2122187454000000000ULL);
    testItem.SetUnixMilliseconds (1351985400000LL);
    testDataset.push_back (testItem);

    //1am local maps to 1am UTC
    //using time which is 1 sec off the actual switch to avoid rounding errors when testing with rational Julian Days
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 11, 4, 1, 0, 1),
                                2122187508010000000ULL);
    testItem.SetUnixMilliseconds (1351990801000LL);
    testDataset.push_back (testItem);

    //1:30am local maps to 1:30am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 11, 4, 1, 30, 0),
                                2122187526000000000ULL);
    testItem.SetUnixMilliseconds (1351992600000LL);
    testDataset.push_back (testItem);

    //2am local maps to 2am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 11, 4, 2, 00, 0),
                                2122187544000000000ULL);
    testItem.SetUnixMilliseconds (1351994400000LL);
    testDataset.push_back (testItem);
    
    //2:30am local maps to 2:30am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 11, 4, 2, 30, 0),
                                2122187562000000000ULL);
    testItem.SetUnixMilliseconds (1351996200000LL);
    testDataset.push_back (testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeTestDataset::CreateDstCETTestDataset
(
DateTimeTestItemList& testDataset
)
    {
    //dates on brim of switching to CET DST. Officially DST starts at 2am local time. 
    //Clocks are then switched to 3am right away.
    
    //1:30am local maps to 0:30am UTC
    DateTimeTestItem testItem (DateTime (DateTime::Kind::Local, 2012, 3, 25, 1, 30, 0),
                                2121993954000000000ULL);
    testDataset.push_back (testItem);

    //**Tests with times in critical interval between 2am local and just before 3am local
    //This critical interval usually yields inconsistent results by the underlying OS functions.
    //-> Roundtrip doesn't work here

    //SQLite: 2am local -> 1am UTC -> 3am local
    uint64_t jdInHns = 2121993972000000000ULL;
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 25, 2, 0, 0),
                                 DateTimeTestItem::FromDateTime);
    testItem.SetJdInHns (jdInHns);
    testDataset.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 25, 3, 0, 0),
                                 DateTimeTestItem::ToDateTime);
    testItem.SetJdInHns (jdInHns);
    testDataset.push_back (testItem);

    //times between 2am and 3am don't make sense actually as time is switched from 2am to 3am at 2am.
    //SQLite: 2:30am local -> 1:30am UTC -> 3:30am local
    jdInHns = 2121993990000000000ULL;
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 25, 2, 30, 0),
                                 DateTimeTestItem::FromDateTime);
    testItem.SetJdInHns (jdInHns);
    testDataset.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 25, 3, 30, 0),
                                 DateTimeTestItem::ToDateTime);
    testItem.SetJdInHns (jdInHns);
    testDataset.push_back (testItem);
    //**now test times are outside uncritical interval again -> roundtrip works
    
    //2am and 3am yield the same values as they are both mapped to 1am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 25, 3, 00, 0),
                                            2121993972000000000ULL);
    testDataset.push_back (testItem);

    //3:30am local maps to 1:30 UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 25, 3, 30, 0),
                                            2121993990000000000ULL);
    testDataset.push_back (testItem);

    //dates on brim of switching back from CET DST. Officially DST ends at 3am DST local time. 
    //Clocks are then switched back to 2am.
    //1:30am local maps to 11:30pm UTC (day before)
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 10, 28, 1, 30, 0),
                                 2122181406000000000ULL);
    testDataset.push_back (testItem);

    //2am local maps to 1am UTC
    //using time which is 1 sec off the actual switch to avoid rounding errors when testing with rational Julian Days
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 10, 28, 2, 0, 1),
                                2122181460010000000ULL);
    testDataset.push_back (testItem);

    //2:30am local maps to 1:30am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 10, 28, 2, 30, 0),
                                2122181478000000000ULL);
    testDataset.push_back (testItem);

    //3am local maps to 2am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 10, 28, 3, 00, 0),
                                2122181496000000000ULL);
    testDataset.push_back (testItem);

    //3:30am local maps to 2:30am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 10, 28, 3, 30, 0),
                                2122181514000000000ULL);
    testDataset.push_back (testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeTestDataset::CreateDstUSESTTestDataset 
(
DateTimeTestItemList& testDataset
)
    {
    //dates on brim of switching to EST DST. Officially DST starts at 2am local time. 
    //Clocks are then switched to 3am right away.

    //1:30am local maps to 6:30am UTC
    DateTimeTestItem testItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 1, 30, 0),
                                2121982074000000000ULL);
    testDataset.push_back (testItem);

    //**Tests with times in critical interval between 2am local and just before 3am local
    //This critical interval usually yields inconsistent results by the underlying OS functions.
    //-> Roundtrip doesn't work here.

    //SQLite: 2am local -> 6am UTC -> 1am local
    uint64_t jdInHns = 2121982056000000000ULL;
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 2, 0, 0),
                                DateTimeTestItem::FromDateTime);
    testItem.SetJdInHns (jdInHns);
    testDataset.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 1, 0, 0),
                                DateTimeTestItem::ToDateTime);
    testItem.SetJdInHns (jdInHns);
    testDataset.push_back (testItem);

    //times between 2am and 3am don't make sense actually as time is switched from 2am to 3am at 2am.
    //SQLite: 2:30am local -> 6:30am UTC -> 1:30am local
    jdInHns = 2121982074000000000ULL;
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 2, 30, 0),
                                DateTimeTestItem::FromDateTime);
    testItem.SetJdInHns (jdInHns);
    testDataset.push_back (testItem);

    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 1, 30, 0),
                                DateTimeTestItem::ToDateTime);
    testItem.SetJdInHns (jdInHns);
    testDataset.push_back (testItem);

    //**now test times are outside uncritical interval again -> roundtrip works

    //3am maps to 7am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 3, 00, 0),
                                2121982092000000000ULL);
    testDataset.push_back (testItem);

    //3:30am local maps to 7:30 UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 3, 11, 3, 30, 0),
                                2121982110000000000ULL);
    testDataset.push_back (testItem);

    //dates on brim of switching back from EST DST. Officially DST ends at 3am DST local time. 
    //Clocks are then switched back to 2am.

    //1:30am local maps to 5:30am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 11, 4, 1, 30, 0),
                                2122187670000000000ULL);
    testDataset.push_back (testItem);

    //2am local maps to 7am UTC
    //using time which is 1 sec off the actual switch to avoid rounding errors when testing with rational Julian Days
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 11, 4, 2, 0, 1),
                                2122187724010000000ULL);
    testDataset.push_back (testItem);
    
    //2:30am local maps to 7:30am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 11, 4, 2, 30, 0),
                                2122187742000000000ULL);
    testDataset.push_back (testItem);

    //3am local maps to 8am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 11, 4, 3, 00, 0),
                                2122187760000000000ULL);
    testDataset.push_back (testItem);

    //3:30am local maps to 8:30am UTC
    testItem = DateTimeTestItem (DateTime (DateTime::Kind::Local, 2012, 11, 4, 3, 30, 0),
                                2122187778000000000ULL);
    testDataset.push_back (testItem);
    }