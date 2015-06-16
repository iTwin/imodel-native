/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/DateTimeTestHelpers.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include "DateTimeTestHelpers.h"

#if defined (_WIN32)
  #include <Windows.h>

#elif defined (__unix__)
  #include <sys/time.h>
  
  #if defined (ANDROID)
    #include <time64.h>
  #endif
  
  #if defined (__APPLE__)
    #include <mach/mach_time.h>
  #endif
#endif

//************ DateTimeLogger ********************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    03/2014
//---------------------------------------------------------------------------------------
//static member initialization
BentleyApi::NativeLogging::ILogger* DateTimeLogger::s_logger = nullptr;


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    03/2014
//---------------------------------------------------------------------------------------
//static
BentleyApi::NativeLogging::ILogger& DateTimeLogger::Get ()
    {
    if (s_logger == nullptr)
        s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger (L"DateTimeTests");

    return *s_logger;
    }

//************ DateTimeTestItem ********************

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
DateTimeTestItem::DateTimeTestItem 
(
)
    : m_isUnixMillisecondsNull (true), m_mode (Roundtrip)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
DateTimeTestItem::DateTimeTestItem 
(
const DateTime& dateTime, 
uint64_t jdInHns
)
: m_dateTime (dateTime), m_jdInHns (jdInHns), m_isUnixMillisecondsNull (true), m_mode (Roundtrip)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
DateTimeTestItem::DateTimeTestItem 
(
const DateTime& dateTime, 
TestMode testMode
)
: m_dateTime (dateTime), m_isUnixMillisecondsNull (true), m_mode (testMode)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
DateTimeTestItem::~DateTimeTestItem () {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
void DateTimeTestItem::SetDateTime 
(
const DateTime& dateTime
)
    {
    m_dateTime = dateTime;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
const DateTime& DateTimeTestItem::GetDateTime 
(
) const
    {
    return m_dateTime;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
void DateTimeTestItem::SetJdInHns 
(
uint64_t jdInHns
)
    {
    m_jdInHns = jdInHns;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
const uint64_t& DateTimeTestItem::GetJdInHns 
(
) const
    {
    return m_jdInHns;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
void DateTimeTestItem::SetUnixMilliseconds 
(
int64_t unixMilliseconds
)
    {
    m_unixMilliseconds = unixMilliseconds;
    m_isUnixMillisecondsNull = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
bool DateTimeTestItem::IsUnixMillisecondsNull
(
) const
    {
    return m_isUnixMillisecondsNull;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
const int64_t& DateTimeTestItem::GetUnixMilliseconds 
(
) const
    {
    return m_unixMilliseconds;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
DateTimeTestItem::TestMode DateTimeTestItem::GetMode 
(
) const
    {
    return m_mode;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
bool DateTimeTestItem::IsToDateTimeTestMode 
(
) const
    {
    return (GetMode () & ToDateTime) == ToDateTime;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
bool DateTimeTestItem::IsFromDateTimeTestMode 
(
) const
    {
    return (GetMode () & FromDateTime) == FromDateTime;
    }


//******************** DateTimeTestHelper ****************************************************
//static member init
const int64_t DateTimeTestHelper::HNS_IN_DAY = 864000000000LL;
const int64_t DateTimeTestHelper::HNS_IN_MSEC = 10000LL;
const int64_t DateTimeTestHelper::HNS_IN_SEC = 1000LL * HNS_IN_MSEC;
const int64_t DateTimeTestHelper::HALFHOUR_IN_MSEC = 1800000LL; 
const int64_t DateTimeTestHelper::HOUR_IN_MSEC = HALFHOUR_IN_MSEC * 2LL;
const int64_t DateTimeTestHelper::HOUR_IN_HNS = HOUR_IN_MSEC * HNS_IN_MSEC;

const int64_t DateTimeTestHelper::UNIXEPOCH_START_MSEC = 0LL;
const int64_t DateTimeTestHelper::UNIXEPOCH_END_MSEC = 2147483647000LL;


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
bool DateTimeTestHelper::IsInUnixEpoch
(
const DateTime& date
)
    {
    int16_t year = date.GetYear ();
    if (year != 2038)
        {
        return year >= 1970 && year <= 2037;
        }

    //year is 2038
    if (date.GetMonth () > 1)
        {
        return false;
        }

    uint8_t day = date.GetDay ();
    if (day != 19)
        {
        return day <= 18;
        }

    //day is 19
    const uint64_t actualHns = (date.GetHour () * 3600 + date.GetMinute () * 60 + date.GetSecond ()) * HNS_IN_SEC + date.GetHectoNanosecond ();
    const uint64_t endHns = (3 * 3600 + 14 * 60 + 7) * HNS_IN_SEC;
    return actualHns <= endHns;
    }


//************ DateTimeAsserter ********************
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeAsserter::Assert
(
const DateTimeTestItemList& testItemList
)
    {
    DateTimeTestItemList::const_iterator it;
    for (it = testItemList.begin (); it != testItemList.end (); ++it)
        {
        DateTimeTestItem testItem = *it;
        Assert (testItem);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeAsserter::Assert 
(
const DateTimeTestItem& testItem
)
    {
    AssertToJulianDay (testItem);
    AssertFromJulianDay (testItem);
    AssertToUnixMilliseconds (testItem);
    AssertFromUnixMilliseconds (testItem);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static 
void DateTimeAsserter::AssertToJulianDay 
(
const DateTimeTestItem& testItem
)
    {
    if (!testItem.IsFromDateTimeTestMode ())
        {
        return;
        }

    const DateTime& dateTime = testItem.GetDateTime ();
    uint64_t actualJdInHns;
    BentleyStatus stat = dateTime.ToJulianDay (actualJdInHns);
    EXPECT_EQ (SUCCESS, stat) << L"DateTime::ToJulianDay failed for '" << dateTime.ToString ().c_str () << "'";

    const uint64_t expectedJdInHns = testItem.GetJdInHns ();
    EXPECT_EQ (expectedJdInHns, actualJdInHns) << L"DateTime::ToJulianDay (uint64_t&): Unexpected Julian day number for '" << dateTime.ToString ().c_str () << "'";

    const double expectedJd = expectedJdInHns / 864000000000.0;
    double actualJd;
    stat = dateTime.ToJulianDay (actualJd);

    EXPECT_DOUBLE_EQ (expectedJd, actualJd) << L"DateTime::ToJulianDay (double&): Unexpected Julian day number for '" << dateTime.ToString ().c_str () << "'";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeAsserter::AssertFromJulianDay 
(
const DateTimeTestItem& testItem
)
    {
    if (!testItem.IsToDateTimeTestMode ())
        {
        return;
        }

    const DateTime& expectedDateTime = testItem.GetDateTime ();
    const uint64_t& jdInHns = testItem.GetJdInHns ();

    //test FromJulianDay (UInt64)
    DateTime actualDateTime;
    BentleyStatus stat = DateTime::FromJulianDay (actualDateTime, jdInHns, expectedDateTime.GetInfo ());
    EXPECT_EQ (SUCCESS, stat) << L"DateTime::FromJulianDay(uint64_t) failed for Julian Day '" << jdInHns << "'";

    WString assertMessage;
    assertMessage.Sprintf (L"DateTime::FromJulianDay (uint64_t): Julian Day '%lld'. Expected: %ls - Actual: %ls", jdInHns, 
                            expectedDateTime.ToString ().c_str (), actualDateTime.ToString ().c_str ());
    Assert (expectedDateTime, actualDateTime, true, false, assertMessage.c_str ());


    //now test FromJulianDay (double)
    const double jd = jdInHns / 864000000000.0;
    stat = DateTime::FromJulianDay (actualDateTime, jd, expectedDateTime.GetInfo ());
    assertMessage = WString ();
    assertMessage.Sprintf (L"DateTime::FromJulianDay (double): failed for Julian Day '%lf'", jd);
    EXPECT_EQ (SUCCESS, stat) << assertMessage.c_str ();

    double actualJd;
    stat = actualDateTime.ToJulianDay (actualJd);
    EXPECT_EQ (SUCCESS, stat) << L"AssertFromJulianDay failed";

    assertMessage = WString ();
    assertMessage.Sprintf (L"DateTime::FromJulianDay (double): Julian Day '%lf'. Expected: %ls - Actual: %ls", jd, 
        expectedDateTime.ToString ().c_str (), actualDateTime.ToString ().c_str ());

    EXPECT_DOUBLE_EQ (jd, actualJd) << assertMessage.c_str ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeAsserter::AssertToUnixMilliseconds
(
const DateTimeTestItem& testItem
)
    {
    //only run the tests if unix millisecs are defined in the test item
    //and if the requested testing mode matches
    if (testItem.IsUnixMillisecondsNull () ||
        !testItem.IsFromDateTimeTestMode ())
        {
        return;
        }

    const DateTime& testDate = testItem.GetDateTime ();

    WString assertMessage;
    assertMessage.Sprintf (L"DateTime::ToUnixMilliseconds failed for date: %ls", testDate.ToString ().c_str ());

    const int64_t& expectedUnixMilliseconds = testItem.GetUnixMilliseconds ();

    //now actual method call to test
    int64_t actualUnixMilliseconds;
    BentleyStatus stat = testDate.ToUnixMilliseconds (actualUnixMilliseconds);
    EXPECT_EQ (SUCCESS, stat) << assertMessage.c_str ();

    EXPECT_EQ (expectedUnixMilliseconds, actualUnixMilliseconds) << assertMessage.c_str ();

    //now test DateTime::JulianDayToUnixMilliseconds
    actualUnixMilliseconds = DateTime::JulianDayToUnixMilliseconds (testItem.GetJdInHns ());
    assertMessage.Sprintf (L"DateTime::JulianDayToUnixMilliseconds failed for date: %lld", testItem.GetJdInHns ());
    EXPECT_EQ (expectedUnixMilliseconds, actualUnixMilliseconds) << assertMessage.c_str ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeAsserter::AssertFromUnixMilliseconds
(
const DateTimeTestItem& testItem
)
    {
    const DateTime& expectedDate = testItem.GetDateTime ();
    //only run the tests if date time is not local and if unix millisecs are defined in the test item
    //and if the requested testing mode matches
    if (testItem.IsUnixMillisecondsNull () || expectedDate.GetInfo ().GetKind () != DateTime::Kind::Utc ||
        !testItem.IsToDateTimeTestMode ())
        {
        return;
        }

    const int64_t& testUnixMilliseconds = testItem.GetUnixMilliseconds ();
    WString assertMessage;
    assertMessage.Sprintf (L"DateTime::FromUnixMilliseconds failed for Unix milliseconds: %lld", testUnixMilliseconds);

    DateTime actualDateTime;
    BentleyStatus stat = DateTime::FromUnixMilliseconds (actualDateTime, testUnixMilliseconds);
    EXPECT_EQ (SUCCESS, stat) << assertMessage.c_str ();

    Assert (expectedDate, actualDateTime, true, true, assertMessage.c_str ());

    //now test DateTime::UnixMillisecondsToJulianDay
    const uint64_t actualJdInHns = DateTime::UnixMillisecondsToJulianDay (testUnixMilliseconds);
    assertMessage.Sprintf (L"DateTime::UnixMillisecondsToJulianDay failed for date: %lld", testUnixMilliseconds);
    //Round expected value to the millisecond because Unix milliseconds are involved here.
    const uint64_t expectedJdInHns = static_cast<uint64_t> (testItem.GetJdInHns () / (1.0 * DateTimeTestHelper::HNS_IN_MSEC) + 0.5) * DateTimeTestHelper::HNS_IN_MSEC;
    EXPECT_EQ (expectedJdInHns, actualJdInHns) << assertMessage.c_str ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/13
//---------------------------------------------------------------------------------------
//static
void DateTimeAsserter::AssertStringConversion (DateTimeCR expectedDateTime, Utf8CP expectedIsoDate)
    {
    Utf8String actualIsoDate = expectedDateTime.ToUtf8String ();
    EXPECT_STREQ (expectedIsoDate, actualIsoDate.c_str ()) << "DateTime::ToUtf8String";

    WString expectedIsoDateW (expectedIsoDate, BentleyCharEncoding::Utf8);
    WString actualIsoDateW = expectedDateTime.ToString ();
    EXPECT_STREQ (expectedIsoDateW.c_str (), actualIsoDateW.c_str ()) << "DateTime::ToString";

    DateTime actualDateTime;
    BentleyStatus stat = DateTime::FromString (actualDateTime, expectedIsoDate);
    ASSERT_EQ (SUCCESS, stat) << "Return value of DateTime::FromString (" << expectedIsoDate << ")";

    //Roundtrip doesn't work for Local times as string format for local and unspecified is the same.
    //FromString returns unspecified. Therefore for assertion take a copy of the returned DateTime, but exchange the kind
    if (expectedDateTime.GetInfo ().GetKind () == DateTime::Kind::Local)
        {
        EXPECT_TRUE (DateTime::Kind::Unspecified == actualDateTime.GetInfo ().GetKind ());
        actualDateTime = DateTime (DateTime::Kind::Local, expectedDateTime.GetYear (), expectedDateTime.GetMonth (), expectedDateTime.GetDay (),
            expectedDateTime.GetHour (), expectedDateTime.GetMinute (), expectedDateTime.GetSecond (), expectedDateTime.GetHectoNanosecond ());
        }

    DateTimeAsserter::Assert (expectedDateTime, actualDateTime, true, L"DateTime::FromString");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/13
//---------------------------------------------------------------------------------------
//static
void DateTimeAsserter::AssertFromString (Utf8CP iso8601DateTime, DateTimeCR expectedDateTime, 
                                        bool expectedSuccess)
    {
    DateTime actualDateTime;
    BentleyStatus stat = DateTime::FromString (actualDateTime, iso8601DateTime);

    BentleyStatus expectedStat = expectedSuccess ? SUCCESS : ERROR;
    ASSERT_EQ (expectedStat, stat) << "Return value of DateTime::FromString (" << iso8601DateTime << ")";

    if (expectedSuccess)
        {
        DateTimeAsserter::Assert (expectedDateTime, actualDateTime, true, L"DateTime::FromString (Utf8CP)");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/13
//---------------------------------------------------------------------------------------
//static
void DateTimeAsserter::AssertFromString (WCharCP iso8601DateTime, DateTimeCR expectedDateTime, 
                                         bool expectedSuccess)
    {
    DateTime actualDateTime;
    BentleyStatus stat = DateTime::FromString (actualDateTime, iso8601DateTime);

    BentleyStatus expectedStat = expectedSuccess ? SUCCESS : ERROR;
    ASSERT_EQ (expectedStat, stat) << L"Return value of DateTime::FromString (" << iso8601DateTime << L")";

    if (expectedSuccess)
        {
        DateTimeAsserter::Assert (expectedDateTime, actualDateTime, true, L"DateTime::FromString (WCharCP)");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeAsserter::Assert
(
const DateTime& expected,
const DateTime& actual,
WCharCP assertMessage
)
    {
    Assert (expected, actual, false, assertMessage);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeAsserter::Assert
(
const DateTime& expected,
const DateTime& actual,
bool millisecAccuracyOnly,
WCharCP assertMessage
)
    {
    Assert (expected, actual, false, millisecAccuracyOnly, assertMessage);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/13
//---------------------------------------------------------------------------------------
//static
void DateTimeAsserter::Assert
(
const DateTime& expected,
const DateTime& actual,
bool ignoreDateTimeComponent,
bool millisecAccuracyOnly,
WCharCP assertMessage
)
    {
    ASSERT_EQ (expected.IsValid (), actual.IsValid ());

    Utf8String expectedDateStr = expected.ToUtf8String ();
    EXPECT_TRUE (expected.GetInfo ().GetKind () == actual.GetInfo ().GetKind ()) << expectedDateStr.c_str () << " - " << assertMessage;

    if (!ignoreDateTimeComponent)
        {
        EXPECT_TRUE (expected.GetInfo ().GetComponent () == actual.GetInfo ().GetComponent ()) << expectedDateStr.c_str () << " - " << assertMessage;
        }

    EXPECT_EQ (expected.GetYear (), actual.GetYear ()) << expectedDateStr.c_str () << " - " << assertMessage;
    EXPECT_EQ (expected.GetMonth (), actual.GetMonth ()) << expectedDateStr.c_str () << " - " << assertMessage;
    EXPECT_EQ (expected.GetDay (), actual.GetDay ()) << expectedDateStr.c_str () << " - " << assertMessage;
    EXPECT_EQ (expected.GetHour (), actual.GetHour ()) << expectedDateStr.c_str () << " - " << assertMessage;
    EXPECT_EQ (expected.GetMinute (), actual.GetMinute ()) << expectedDateStr.c_str () << " - " << assertMessage;
    EXPECT_EQ (expected.GetSecond (), actual.GetSecond ()) << expectedDateStr.c_str () << " - " << assertMessage;

    uint64_t expectedHns = expected.GetHectoNanosecond ();
    uint64_t actualHns = actual.GetHectoNanosecond ();
    if (millisecAccuracyOnly)
        {
        EXPECT_NEAR ((double)expectedHns, (double)actualHns, (double)9999) << expectedDateStr.c_str () << " - " << assertMessage;
        }
    else
        {
        EXPECT_EQ (expectedHns, actualHns) << expectedDateStr.c_str () << " - " << assertMessage;
        }
    }


//************ DateTimeTestHelper ********************

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//status
BentleyStatus DateTimeTestHelper::LocalToUtcViaCRT 
(
DateTime& utc, 
const DateTime& local
)
    {
    struct tm localTm;
    DateTimeTestHelper::DateTimeToTm (localTm, local);

#if defined (_WIN32)
    time_t utcSecs = _mktime64 (&localTm);

#elif defined (__unix__)
    time_t utcSecs = mktime (&localTm);
#else
    return ERROR;
#endif
    if (utcSecs == (time_t) -1)
        {
        return ERROR;
        }

    uint64_t utcMillisecs = static_cast<uint64_t> (utcSecs) * 1000ULL;
    
    struct tm utcTm;
    BentleyStatus stat = BeTimeUtilities::ConvertUnixMillisToTm (utcTm, utcMillisecs);
    POSTCONDITION (stat == SUCCESS, ERROR);

    //tm doesn't support below-sec accuracy, so we just add that fraction manually
    DateTimeTestHelper::TmToDateTime (utc, utcTm, local.GetHectoNanosecond (), DateTime::Kind::Utc);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//status
BentleyStatus DateTimeTestHelper::UtcToLocalViaCRT 
(
DateTime& local, 
const DateTime& utc
)
    {
    int64_t utcMillisecs = 0LL;
    BentleyStatus stat = utc.ToUnixMilliseconds (utcMillisecs);
    POSTCONDITION (stat == SUCCESS, ERROR);

    uint64_t utcMillisecsUnsigned = static_cast<uint64_t> (utcMillisecs);
    struct tm localTm;
    stat = BeTimeUtilities::ConvertUnixMillisToLocalTime (localTm, utcMillisecsUnsigned);
    POSTCONDITION (stat == SUCCESS, ERROR);

    //tm doesn't support below-sec accuracy, so we just add that fraction manually
    DateTimeTestHelper::TmToDateTime (local, localTm, utc.GetHectoNanosecond (), DateTime::Kind::Local);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//status
BentleyStatus DateTimeTestHelper::DateTimeToUnixMillisecsCRT
(
uint64_t& unixMillisecs, 
const DateTime& dateTime
)
    {
    struct tm dateTimeTm;
    DateTimeToTm (dateTimeTm, dateTime);

    unixMillisecs = BeTimeUtilities::ConvertTmToUnixMillis (dateTimeTm);
    //tm does not support millisecs, so adding them afterwards (hectonanosecs get truncated)
    unixMillisecs += dateTime.GetMillisecond ();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeTestHelper::DateTimeToTm
(
struct tm& out,
const DateTime& in
)
    {
    if (!IsInUnixEpoch (in))
        {
        //use 2000-01-01 00:00:00 UTC
        out.tm_year = 100;
        out.tm_mon = 0;
        out.tm_mday = 1;
        out.tm_hour = 0;
        out.tm_min = 0;
        out.tm_sec = 0;
        out.tm_isdst = -1;
        return;
        }

    out.tm_year = in.GetYear () - 1900;
    out.tm_mon = in.GetMonth () - 1;
    out.tm_mday = in.GetDay ();
    out.tm_hour = in.GetHour ();
    out.tm_min = in.GetMinute ();
    out.tm_sec = in.GetSecond ();
    out.tm_isdst = -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
//static
void DateTimeTestHelper::TmToDateTime
(
DateTime& out,
const tm& in,
uint32_t hectoNanosecond,
DateTime::Kind targetKind
)
    {
    out = DateTime (targetKind, static_cast <int16_t> (in.tm_year + 1900),
                                static_cast <uint8_t> (in.tm_mon + 1),
                                static_cast <uint8_t> (in.tm_mday),
                                static_cast <uint8_t> (in.tm_hour),
                                static_cast <uint8_t> (in.tm_min),
                                static_cast <uint8_t> (in.tm_sec),
                                hectoNanosecond);
    }

#if defined (BENTLEY_WIN32)
//static member variable initialization
//static
const char* TzSetter::TZ_ENVVAR_NAME = "TZ";

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TzSetter::TzSetter 
(
)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TzSetter::~TzSetter 
(
)
    {
    //remove env var again
    _putenv_s (TZ_ENVVAR_NAME, "");

    _tzset ();

    LOG.info ("Time zone reset to machine setting.");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
void TzSetter::Set 
(
const char* tzEnvVarValue
) const
    {
    errno_t stat = _putenv_s (TZ_ENVVAR_NAME, tzEnvVarValue);
    ASSERT_EQ (0, stat) << "Setting current time zone failed: Environment variable TZ could not be set.";

    _tzset ();

    size_t tznameLength;
    char tzname[100];
    stat =_get_tzname (&tznameLength, tzname, sizeof(tzname), 0);
    ASSERT_EQ (0, stat) << "Setting current time zone failed: _tzset () failed.";
    //tznameLength includes 0 terminator -> empty string is at least 1 char long.
    ASSERT_GT (tznameLength, static_cast<size_t> (1)) << "Setting current time zone failed: _tzset () failed.";

    long offset;
    stat = _get_timezone (&offset);
    ASSERT_EQ (0, stat) << "Setting current time zone failed: _tzset () failed.";

    int dstObserved;
    stat = _get_daylight (&dstObserved);
    ASSERT_EQ (0, stat) << "Setting current time zone failed: _tzset () failed.";

    LOG.infov ("Current time zone set to %s. Offset from UTC [s]: %ld. DST observed: %d", tzEnvVarValue, offset, dstObserved != 0);
    }

//static 
bool TzSetter::CurrentMachineTimezoneObservesDst 
(
)
    {
    TIME_ZONE_INFORMATION tzInfo;
    DWORD stat = GetTimeZoneInformation (&tzInfo);
    EXPECT_NE (TIME_ZONE_ID_INVALID, stat) << L"Retrieving time zone information from machine failed.";
    return stat != TIME_ZONE_ID_UNKNOWN;
    }

#endif
