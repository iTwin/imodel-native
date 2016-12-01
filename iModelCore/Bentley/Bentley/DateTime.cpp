/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/DateTime.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/DateTime.h>
#include "DateTimeConverter.h"
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY

//*********************** DateTime *****************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
DateTime::CompareResult DateTime::Compare(DateTime const& lhs, DateTime const& rhs)
    {
    if (!lhs.IsValid() || !rhs.IsValid())
        //if only one of them is invalid, return error; if both them are invalid, return equals
        return lhs.IsValid() == rhs.IsValid() ? CompareResult::Equals : CompareResult::Error;

    uint64_t lhsJd = INT64_C(0);
    uint64_t rhsJd = INT64_C(0);
    if (lhs.ToJulianDay(lhsJd) != SUCCESS ||
        rhs.ToJulianDay(rhsJd) != SUCCESS)
        return CompareResult::Error;

    const int64_t diff = lhsJd - rhsJd;
    if (diff < INT64_C(0))
        return CompareResult::EarlierThan;
    else if (diff == INT64_C(0))
        return CompareResult::Equals;
    else
        return CompareResult::LaterThan;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTime::Equals(DateTime const& rhs, bool ignoreDateTimeInfo) const
    {
    if (!IsValid() || !rhs.IsValid())
        //if only one of them is invalid, return false; if both them are invalid, return true
        return IsValid() == rhs.IsValid();

    return (ignoreDateTimeInfo || GetInfo() == rhs.GetInfo()) &&
        GetYear() == rhs.GetYear() && GetMonth() == rhs.GetMonth() && GetDay() == rhs.GetDay() &&
        GetHour() == rhs.GetHour() && GetMinute() == rhs.GetMinute() && GetSecond() == rhs.GetSecond() &&
        GetMillisecond() == rhs.GetMillisecond();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
DateTime DateTime::GetCurrentTime()
    {
    const uint64_t jd = DateTimeConverter::UnixMillisecondsToJulianDay(BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    DateTime dateTime;
    FromJulianDay(dateTime, jd, Info::CreateForDateTime(Kind::Local));
    return dateTime;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
DateTime DateTime::GetCurrentTimeUtc()
    {
    const uint64_t jd = DateTimeConverter::UnixMillisecondsToJulianDay(BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    DateTime dateTime;
    FromJulianDay(dateTime, jd, Info::CreateForDateTime(Kind::Utc));
    return dateTime;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTime::ToUtc(DateTime& utcDateTime) const
    {
    if (!IsValid() || m_info.GetKind() != Kind::Local)
        return ERROR;

    uint64_t jd = INT64_C(0);
    BentleyStatus stat = DateTimeConverter::ConvertToJulianDay(jd, *this, true);
    if (stat != SUCCESS)
        return ERROR;

    //as target is UTC, no need to apply any time zone offsets
    return DateTimeConverter::ConvertFromJulianDay(utcDateTime, jd, Info::CreateForDateTime(Kind::Utc), false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTime::ToLocalTime(DateTime& localDateTime) const
    {
    if (!IsValid() || m_info.GetKind() != Kind::Utc)
        return ERROR;

    //as we are in UTC, no need to apply any time zone offsets
    uint64_t jd = INT64_C(0);
    BentleyStatus stat = DateTimeConverter::ConvertToJulianDay(jd, *this, false);
    if (stat != SUCCESS)
        return ERROR;

    return DateTimeConverter::ConvertFromJulianDay(localDateTime, jd, Info::CreateForDateTime(Kind::Local), true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTime::ToJulianDay(uint64_t& julianDay) const
    {
    return DateTimeConverter::ConvertToJulianDay(julianDay, *this, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTime::ToJulianDay(double& julianDay) const
    {
    uint64_t jdMsec = INT64_C(0);
    if (SUCCESS != ToJulianDay(jdMsec))
        return ERROR;

    julianDay = MsecToRationalDay(jdMsec);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTime::FromJulianDay(DateTimeR dateTime, uint64_t julianDayMsec, DateTime::Info const& targetInfo)
    {
    return DateTimeConverter::ConvertFromJulianDay(dateTime, julianDayMsec, targetInfo, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTime::FromJulianDay(DateTimeR dateTime, double julianDay, DateTime::Info const& targetInfo)
    {
    uint64_t jdMsec = RationalDayToMsec(julianDay);
    return FromJulianDay(dateTime, jdMsec, targetInfo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTime::ToUnixMilliseconds(int64_t& unixMilliseconds) const
    {
    uint64_t jd = INT64_C(0);
    if (SUCCESS != ToJulianDay(jd))
        return ERROR;

    unixMilliseconds = JulianDayToUnixMilliseconds(jd);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTime::FromUnixMilliseconds(DateTime& dateTime, uint64_t unixMilliseconds)
    {
    uint64_t jd = UnixMillisecondsToJulianDay(unixMilliseconds);
    return FromJulianDay(dateTime, jd, Info::CreateForDateTime(Kind::Utc));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
int64_t DateTime::JulianDayToUnixMilliseconds(uint64_t julianDayMsec)
    {
    return DateTimeConverter::JulianDayToUnixMilliseconds(julianDayMsec);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint64_t DateTime::UnixMillisecondsToJulianDay(uint64_t unixMilliseconds)
    {
    return DateTimeConverter::UnixMillisecondsToJulianDay(unixMilliseconds);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
int64_t DateTime::JulianDayToCommonEraMilliseconds(uint64_t julianDay)
    {
    return DateTimeConverter::JulianDayToCommonEraMilliseconds(julianDay);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint64_t DateTime::CommonEraMillisecondsToJulianDay(int64_t commonEraMsec)
    {
    return DateTimeConverter::CommonEraMillisecondsToJulianDay(commonEraMsec);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
double DateTime::MsecToRationalDay(uint64_t msec)
    {
    return DateTimeConverter::MillisecsToRationalDay(msec);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint64_t DateTime::RationalDayToMsec(double rationalDay)
    {
    return DateTimeConverter::RationalDayToMillisecs(rationalDay);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2014
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::DayOfWeek DateTime::GetDayOfWeek() const
    {
    if (!IsValid())
        {
        BeAssert(false && "Cannot call DateTime::GetDayOfWeek on invalid DateTime.");
        return DayOfWeek::Sunday;
        }

    //algorithm taken from: http://jonathan.rawle.org/hyperpedia/day_calculation.php
    const uint8_t monthTable[] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};

    const int16_t year = GetYear();
    int dayOfWeekNumber = year + (year / 4) - (year / 100) + (year / 400) + GetDay();
    const int8_t month = GetMonth();
    dayOfWeekNumber += monthTable[month - 1] - 1;
    //leap year correction (only for positive years. Leap years BC are not well defined, if at all)
    if (year > 0 && IsLeapYear(year))
        {
        int8_t leapYearCorrection = 0;
        if (month == 1)
            leapYearCorrection = 6;
        else if (month == 2)
            leapYearCorrection = -1;

        dayOfWeekNumber += leapYearCorrection;
        }

    dayOfWeekNumber = dayOfWeekNumber % 7;
    return (DayOfWeek) (dayOfWeekNumber);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2014
//+---------------+---------------+---------------+---------------+---------------+------
uint16_t DateTime::GetDayOfYear() const
    {
    if (!IsValid())
        {
        BeAssert(false && "Cannot call DateTime::GetDayOfYear on invalid DateTime.");
        return 0;
        }

    const uint8_t month = GetMonth();
    const int16_t year = GetYear();

    uint16_t dayOfYear = GetDay();
    //iterate from January till month before current month
    for (uint8_t i = 1; i < month; i++)
        {
        dayOfYear += GetMaxDay(year, i);
        }

    return dayOfYear;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint8_t DateTime::GetMaxDay(int16_t year, uint8_t month)
    {
    //special case for February if leap year
    if (month == 2 && IsLeapYear(year))
        return 29;

    const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return monthDays[month - 1];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DateTime::ToString() const
    {
    if (!IsValid())
        return Utf8String();

    return DateTimeStringConverter::ToIso8601(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTime::FromString(DateTimeR dateTime, Utf8CP iso8601DateTime)
    {
    return DateTimeStringConverter::FromIso8601(dateTime, iso8601DateTime);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2014
//---------------------------------------------------------------------------------------
BentleyStatus DateTime::ComputeOffsetToUtcInMsec(int64_t& offset) const
    {
    if (!IsValid())
        return ERROR;

    offset = 0;

    if (Kind::Local != m_info.GetKind())
        return SUCCESS;

    uint64_t localJD;
    if (SUCCESS != DateTimeConverter::ConvertToJulianDay(localJD, *this, false))
        return ERROR;

    uint64_t utcJD;
    if (SUCCESS != DateTimeConverter::ConvertToJulianDay(utcJD, *this, true))
        return ERROR;

    if (localJD < utcJD)
        offset = -(int64_t) (utcJD - localJD);
    else
        offset = (int64_t) (localJD - utcJD);

    return SUCCESS;
    }
