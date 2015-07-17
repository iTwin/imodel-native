/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/DateTime.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/DateTime.h>
#include "DateTimeConverter.h"
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
WString DateTime::Info::KindToStringW (DateTime::Kind kind)
    {
    switch (kind)
        {
    case Kind::Local:
        return L"Local";

    case Kind::Utc:
        return L"Utc";

    default:
    case Kind::Unspecified:
        return L"Unspecified";
        }
    }
//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>07/2015</date>
//---------------+---------------+---------------+---------------+---------------+-----
//static
Utf8String DateTime::Info::KindToString(DateTime::Kind kind)
    {
    switch (kind)
        {
        case Kind::Local:
            return "Local";

        case Kind::Utc:
            return "Utc";

        default:
        case Kind::Unspecified:
            return "Unspecified";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
WString DateTime::Info::ComponentToStringW (DateTime::Component component)
    {
    switch (component)
        {
    case Component::Date:
        return L"Date";

    default:
    case Component::DateAndTime:
        return L"DateTime";
        }
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>07/2015</date>
//---------------+---------------+---------------+---------------+---------------+-----
//static
Utf8String DateTime::Info::ComponentToString(DateTime::Component component)
    {
    switch (component)
        {
        case Component::Date:
            return "Date";

        default:
        case Component::DateAndTime:
            return "DateTime";
        }
    }

//*********************** DateTime *****************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::DateTime()
: m_info (Kind::Unspecified, Component::DateAndTime),
  m_year (0),
  m_month (0),
  m_day (0),
  m_hour (0),
  m_minute (0),
  m_second (0),
  m_hectoNanosecond (0)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::DateTime (int16_t year, uint8_t month, uint8_t day)
    : m_info (Kind::Unspecified, Component::Date),
    m_year ((year >= -4713 && year <= 9999) ? year : 1),
    m_month ((month >= 1 && month <= 12)? month : 1),
    m_day ((day >= 1 && day <= GetMaxDay(m_year, m_month)) ? day : 1),
    m_hour (0),
    m_minute (0),
    m_second (0),
    m_hectoNanosecond (0)
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::DateTime (Kind kind, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint32_t hectoNanosecond)
: m_info (kind, Component::DateAndTime),
  m_year ((year >= -4713 && year <= 9999) ? year : 1),
  m_month ((month >= 1 && month <= 12)? month : 1),
  m_day ((day >= 1 && day <= GetMaxDay(m_year, m_month)) ? day : 1),
  m_hour ((hour >= 0 && hour < 24) ? hour : 0),
  m_minute ((minute >= 0 && minute < 60) ? minute : 0),
  m_second ((second >= 0 && second < 60) ? second : 0),
  m_hectoNanosecond ((hectoNanosecond < DateTimeConverter::HECTONANOSECS_IN_SEC) ? hectoNanosecond : 0)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::DateTime (Info const& info, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint32_t hectoNanosecond)
    : m_info (info),
    m_year ((year >= -4713 && year <= 9999) ? year : 1),
    m_month ((month >= 1 && month <= 12)? month : 1),
    m_day ((day >= 1 && day <= GetMaxDay(m_year, m_month)) ? day : 1),
    m_hour ((hour >= 0 && hour < 24) ? hour : 0),
    m_minute ((minute >= 0 && minute < 60) ? minute : 0),
    m_second ((second >= 0 && second < 60) ? second : 0),
    m_hectoNanosecond ((hectoNanosecond < DateTimeConverter::HECTONANOSECS_IN_SEC) ? hectoNanosecond : 0)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTime::IsValid () const
    {
    //the default constructor initializes m_month to 0 which is used to indicate that the date time is not valid,i.e. not set.
    return m_month > 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
DateTime::CompareResult DateTime::Compare (DateTime const& lhs, DateTime const& rhs)
    {
    if (!lhs.IsValid () || !rhs.IsValid ())
        //if only one of them is invalid, return error; if both them are invalid, return equals
        return lhs.IsValid () == rhs.IsValid () ? CompareResult::Equals : CompareResult::Error;

    uint64_t lhsJd = 0ULL;
    uint64_t rhsJd = 0ULL;
    if (lhs.ToJulianDay (lhsJd) != SUCCESS ||
        rhs.ToJulianDay (rhsJd) != SUCCESS)
        return CompareResult::Error;

    const int64_t diff = lhsJd - rhsJd;
    if (diff < 0LL)
        return CompareResult::EarlierThan;
    else if (diff == 0LL)
        return CompareResult::Equals;
    else
        return CompareResult::LaterThan;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTime::Equals (DateTime const& rhs, bool ignoreDateTimeInfo) const
     {
     if (!IsValid () || !rhs.IsValid ())
         //if only one of them is invalid, return false; if both them are invalid, return true
         return IsValid () == rhs.IsValid ();

     return (ignoreDateTimeInfo || GetInfo () == rhs.GetInfo ()) &&  
         GetYear() == rhs.GetYear() && GetMonth() == rhs.GetMonth() && GetDay() == rhs.GetDay() && 
         GetHour() == rhs.GetHour() && GetMinute() == rhs.GetMinute() && GetSecond() == rhs.GetSecond() &&
         GetHectoNanosecond() == rhs.GetHectoNanosecond();
     }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
DateTime DateTime::GetCurrentTime ()
    {
    DateTime dateTime;
    DateTimeConverter::FromUnixMilliseconds (dateTime, BeTimeUtilities::GetCurrentTimeAsUnixMillis(), Info (Kind::Local, Component::DateAndTime));
    return dateTime;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
DateTime DateTime::GetCurrentTimeUtc ()
    {
    DateTime dateTime;
    DateTimeConverter::FromUnixMilliseconds (dateTime, BeTimeUtilities::GetCurrentTimeAsUnixMillis(), Info (Kind::Utc, Component::DateAndTime));
    return dateTime;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTime::ToUtc (DateTime& utcDateTime) const
    {
    if (!IsValid () || GetKind () != Kind::Local)
        return ERROR;

    uint64_t jd = 0ULL;
    BentleyStatus stat = DateTimeConverter::ConvertToJulianDay (jd, *this, true);
    if (stat != SUCCESS)
        return ERROR;

    //as target is UTC, no need to apply any time zone offsets
    return DateTimeConverter::ConvertFromJulianDay (utcDateTime, jd, Info (Kind::Utc, GetInfo ().GetComponent ()), false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTime::ToLocalTime (DateTime& localDateTime) const
    {
    if (!IsValid () || GetKind () != Kind::Utc)
        return ERROR;

    //as we are in UTC, no need to apply any time zone offsets
    uint64_t jd = 0ULL;
    BentleyStatus stat = DateTimeConverter::ConvertToJulianDay (jd, *this, false);
    if (stat != SUCCESS)
        return ERROR;

    return DateTimeConverter::ConvertFromJulianDay (localDateTime, jd, Info (Kind::Local, GetInfo ().GetComponent ()), true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTime::ToJulianDay (uint64_t& julianDayInHns) const
    {   
    return DateTimeConverter::ToJulianDay (julianDayInHns, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTime::ToJulianDay (double& julianDay) const
    {
    return DateTimeConverter::ToJulianDay (julianDay, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTime::FromJulianDay (DateTimeR dateTime, uint64_t julianDayInHns, DateTime::Info const& targetInfo)
    {
    return DateTimeConverter::FromJulianDay (dateTime, julianDayInHns, targetInfo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTime::FromJulianDay (DateTimeR dateTime, double julianDay, DateTime::Info const& targetInfo)
    {
    return DateTimeConverter::FromJulianDay (dateTime, julianDay, targetInfo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTime::ToUnixMilliseconds (int64_t& unixMilliseconds) const
    {
    return DateTimeConverter::ToUnixMilliseconds (unixMilliseconds, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus DateTime::FromUnixMilliseconds (DateTimeR dateTime, uint64_t unixMilliseconds)
    {
    return DateTimeConverter::FromUnixMilliseconds (dateTime, unixMilliseconds, Info (Kind::Utc, Component::DateAndTime));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
int64_t DateTime::JulianDayToUnixMilliseconds (uint64_t julianDayInHectoNanoseconds)
    {
    return DateTimeConverter::JulianDayToUnixMilliseconds (julianDayInHectoNanoseconds);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint64_t DateTime::UnixMillisecondsToJulianDay (uint64_t unixMilliseconds)
    {
    return DateTimeConverter::UnixMillisecondsToJulianDay (unixMilliseconds);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTime::ToCommonEraTicks (int64_t& commonEraTicks) const
    {
    return DateTimeConverter::ToCommonEraTicks (commonEraTicks, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTime::FromCommonEraTicks (DateTimeR dateTime, int64_t commonEraTicks, DateTime::Info const& targetInfo)
    {
    return DateTimeConverter::FromCommonEraTicks (dateTime, commonEraTicks, targetInfo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
int64_t DateTime::JulianDayToCommonEraTicks (uint64_t julianDayInHectoNanoseconds)
    {
    return DateTimeConverter::JulianDayToCommonEraTicks (julianDayInHectoNanoseconds);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint64_t DateTime::CommonEraTicksToJulianDay (int64_t commonEraTicks)
    {
    return DateTimeConverter::CommonEraTicksToJulianDay (commonEraTicks);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
double DateTime::HnsToRationalDay (uint64_t hns)
    {
    return DateTimeConverter::HnsToRationalDay (hns);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint64_t DateTime::RationalDayToHns (double rationalDay)
    {
    return DateTimeConverter::RationalDayToHns (rationalDay);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::Info const& DateTime::GetInfo () const
    {
    BeAssert (IsValid ());
    return m_info;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::Kind DateTime::GetKind () const
    {
    return GetInfo ().GetKind ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
int16_t DateTime::GetYear() const
    {
    BeAssert (IsValid ());
    return m_year;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
uint8_t DateTime::GetMonth() const
    {
    BeAssert (IsValid ());
    return m_month;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
uint8_t DateTime::GetDay() const
    {
    BeAssert (IsValid ());
    return m_day;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
uint8_t DateTime::GetHour() const
    {
    BeAssert (IsValid ());
    return m_hour;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
uint8_t DateTime::GetMinute() const
    {
    BeAssert (IsValid ());
    return m_minute;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
uint8_t DateTime::GetSecond() const
    {
    BeAssert (IsValid ());
    return m_second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t DateTime::GetHectoNanosecond() const
    {
    BeAssert (IsValid ());
    return m_hectoNanosecond;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
uint16_t DateTime::GetMillisecond() const
    {
    BeAssert (IsValid ());
    //only millisecond component is returned (not rounded)
    return DateTimeConverter::ToMillisecond (m_hectoNanosecond);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2014
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::DayOfWeek DateTime::GetDayOfWeek () const
    {
    if (!IsValid ())
        {
        BeAssert (false && "Cannot call DateTime::GetDayOfWeek on invalid DateTime.");
        return DayOfWeek::Sunday;
        }

    //algorithm taken from: http://jonathan.rawle.org/hyperpedia/day_calculation.php
    const uint8_t monthTable[] = { 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };

    const auto year = GetYear ();
    auto dayOfWeekNumber = year + (year / 4) - (year / 100) + (year / 400) + GetDay ();
    const auto month = GetMonth ();
    dayOfWeekNumber += monthTable[month - 1] - 1;
    //leap year correction (only for positive years. Leap years BC are not well defined, if at all)
    if (year > 0 && IsLeapYear (year))
        {
        int8_t leapYearCorrection = 0;
        if (month == 1)
            leapYearCorrection = 6;
        else if (month == 2)
            leapYearCorrection = -1;

        dayOfWeekNumber += leapYearCorrection;
        }

    dayOfWeekNumber = dayOfWeekNumber % 7;

    return static_cast<DayOfWeek> (dayOfWeekNumber);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2014
//+---------------+---------------+---------------+---------------+---------------+------
uint16_t DateTime::GetDayOfYear () const
    {
    if (!IsValid ())
        {
        BeAssert (false && "Cannot call DateTime::GetDayOfYear on invalid DateTime.");
        return 0;
        }

    const uint8_t month = GetMonth ();
    const int16_t year = GetYear ();

    uint16_t dayOfYear = GetDay ();
    //iterate from January till month before current month
    for (uint8_t i = 1; i < month; i++)
        {
        dayOfYear += GetMaxDay (year, i);
        }

    return dayOfYear;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTime::IsLeapYear (uint16_t year)
    {
    return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint8_t DateTime::GetMaxDay (int16_t year, uint8_t month)
    {
    //special case for February if leap year
    if (month == 2 && IsLeapYear (year))
        return 29;

    const uint8_t monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    return monthDays[month - 1];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DateTime::ToUtf8String() const
    {
    return DateTimeStringConverter::ToIso8601 (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTime::FromString (DateTimeR dateTime, Utf8CP iso8601DateTime)
    {
    return DateTimeStringConverter::FromIso8601 (dateTime, iso8601DateTime);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BillSteinbock   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
//static
BentleyStatus DateTime::FromString (DateTimeR dateTime, WCharCP iso8601DateTime)
    {
    Utf8String iso8601DateTimeUtf8;
    BeStringUtilities::WCharToUtf8 (iso8601DateTimeUtf8, iso8601DateTime);

    return DateTimeStringConverter::FromIso8601 (dateTime, iso8601DateTimeUtf8.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2014
//---------------------------------------------------------------------------------------
BentleyStatus DateTime::ComputeOffsetToUtcInHns(int64_t& offset) const
    {
    offset = 0;
    
    if (Kind::Local != m_info.GetKind())
        return SUCCESS;
    
    uint64_t localJD;
    if (SUCCESS != DateTimeConverter::ConvertToJulianDay (localJD, *this, false))
        return ERROR;

    uint64_t utcJD;
    if (SUCCESS == DateTimeConverter::ConvertToJulianDay (utcJD, *this, true))
        return ERROR;

    if (localJD < utcJD)
        offset = -(int64_t)(utcJD - localJD);
    else
        offset = (int64_t)(localJD - utcJD);
    
    return SUCCESS;
    }
