/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DateTimeConverter.h"

#include <Bentley/BeTimeUtilities.h>
#include <Bentley/ScopedArray.h>
#include <Bentley/Nullable.h>
#include <Bentley/Logging.h>
#include <limits.h>
#include <stdlib.h>

#define LOG NativeLogging::CategoryLogger("DateTime")

BEGIN_BENTLEY_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static member initialization
const uint32_t DateTimeConverter::SECS_IN_DAY = 86400;
const uint32_t DateTimeConverter::MSECS_IN_DAY = SECS_IN_DAY * 1000;

//2000-01-01 00:00:00 UTC
const uint64_t DateTimeConverter::TIMEZONEOFFSET_DEFAULT_DATE_IN_JULIANDAY_MSEC = INT64_C(211813444800000);
//1970-01-01 00:00:00 UTC
const uint64_t DateTimeConverter::UNIXEPOCH_BASELINE_IN_JULIANDAY_MSEC = INT64_C(210866760000000);
//2018-01-19 03:14:07 UTC
const uint64_t DateTimeConverter::UNIXEPOCH_END_IN_JULIANDAY_MSEC = INT64_C(213014265247000);

//01-01-01 00:00:00 UTC as JulianDay number in millisecs
//(according to http://quasar.as.utexas.edu/BillInfo/JulianDateCalc.html)
const uint64_t DateTimeConverter::CE_EPOCH_AS_JD_MSEC = INT64_C(148731163200000);

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTimeConverter::ConvertToJulianDay(uint64_t& julianDayInMsec, DateTime const& dateTime, bool applyTimezoneOffset)
    {
    if (!dateTime.IsValid())
        return ERROR;

    //compute raw JD (without time zone offset)
    uint64_t rawJdInMsec = INT64_C(0);
    if (SUCCESS != ComputeJulianDay(rawJdInMsec, dateTime))
        return ERROR;

    if (!applyTimezoneOffset)
        {
        julianDayInMsec = rawJdInMsec;
        return SUCCESS;
        }

    //compute local time zone offset if date time is in local time
    int64_t localTimezoneOffsetInMsec = INT64_C(0);
    if (dateTime.GetInfo().GetKind() == DateTime::Kind::Local)
        {
        if (SUCCESS != ComputeLocalTimezoneOffsetFromLocalTime(localTimezoneOffsetInMsec, rawJdInMsec))
            return ERROR;
        }

    julianDayInMsec = rawJdInMsec + localTimezoneOffsetInMsec;
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//does the raw computation without considering time zones (i.e. treats the given
//date as if it were in UTC)
BentleyStatus DateTimeConverter::ComputeJulianDay(uint64_t& julianDayInMsec, DateTime const& dateTime) {
    return ComputeJulianDay(julianDayInMsec, dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay(),
                            dateTime.GetHour(), dateTime.GetMinute(), dateTime.GetSecond(), dateTime.GetMillisecond());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//does the raw computation without considering time zones (i.e. treats the given
//date as if it were in UTC)
BentleyStatus DateTimeConverter::ComputeJulianDay(uint64_t& julianDayInMsec, tm const& dateTimeTm)
    {
    return ComputeJulianDay(julianDayInMsec, (int16_t) (dateTimeTm.tm_year + 1900), //tm year is based on 1900
                            (uint8_t) (dateTimeTm.tm_mon + 1), //tm month is 0-based
                            (uint8_t) dateTimeTm.tm_mday,
                            (uint8_t) dateTimeTm.tm_hour,
                            (uint8_t) dateTimeTm.tm_min,
                            (uint8_t) dateTimeTm.tm_sec, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//does the raw computation without considering time zones (i.e. treats the given
//date as if it were in UTC)
BentleyStatus DateTimeConverter::ComputeJulianDay(uint64_t& julianDayInMsec, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond)
    {
    julianDayInMsec = INT64_C(0);

    //ensure that year is within Julian epoch (according to proleptic Gregorian calendar).
    if (!
        (year > -4713 ||
        (year == -4713 && (month > 11 ||
         (month == 11 && (day > 24 ||
         (day == 24 && hour >= 12))))))
        )
        return ERROR;

    //Algorithm adopted from Meeus�page�61 (as in SQLite)

    //*** compute date component

    //think of Jan and Feb as being month 13 and 14 of the previous year
    if (month <= 2)
        {
        --year;
        month += 12;
        }

    const int64_t a = year / 100;
    const int64_t b = 2 - a + (a / 4);
    const int64_t x1 = (int64_t) (365.25 * (year + 4716));
    const int64_t x2 = (int64_t) (30.6001 * (month + 1));

    const int64_t dateComponentInSecs = (int64_t) ((x1 + x2 + (int64_t) (day) + b - 1524.5) * SECS_IN_DAY);

    //the minimum value is -0.5 (in fractional notation) which maps to the beginning of the epoch.
    //values below are wrong as the precondition checks that the input is not before the epoch.
    BeAssert(dateComponentInSecs >= INT64_C(-43200));

    //internally compute the JulianDays in msecs since start of JulianDay epoch
    //(no loss of accuracy here)

    //*** compute time component
    const uint64_t timeComponentInSecs = hour * UINT64_C(3600) + minute * UINT64_C(60) + second;

    const uint64_t julianDayInSecs = dateComponentInSecs + timeComponentInSecs;

    //convert to millisecs and add the input millisec component
    julianDayInMsec = julianDayInSecs * 1000 + millisecond;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DateTime DateTimeConverter::ConvertFromJulianDay(uint64_t julianDayInMsec, DateTime::Info const& targetInfo, bool applyTimezoneOffset)  {
    if (!targetInfo.IsValid())
        return DateTime();

    if (applyTimezoneOffset && targetInfo.GetKind() == DateTime::Kind::Local)
        {
        int64_t localTimezoneOffsetInMsec = INT64_C(0);
        BentleyStatus stat = ComputeLocalTimezoneOffsetFromUtcTime(localTimezoneOffsetInMsec, julianDayInMsec);
        if (stat != SUCCESS)
            return DateTime();

        julianDayInMsec -= localTimezoneOffsetInMsec;
        }

    return ParseJulianDay(julianDayInMsec, targetInfo);
}

/** does the raw computation without considering time zones (i.e. expects the givendate to be in UTC */
DateTime DateTimeConverter::ParseJulianDay(uint64_t julianDayInMsec, DateTime::Info const& targetInfo) {
    const uint64_t HALFDAY_IN_MSECS = MSECS_IN_DAY / 2;

    uint8_t day = 0;
    uint8_t month = 0;
    int16_t year = 0;
    if (targetInfo.GetComponent() != DateTime::Component::TimeOfDay) {
        // date fraction of julian day
        // Strip time fraction. As JD is based on noon, the JD value is shifted by half a day so that z represents an unambiguous day.
        //(technically this is the same as rounding the JD value to the next full day number)
        const uint64_t zRaw = (julianDayInMsec + HALFDAY_IN_MSECS) / MSECS_IN_DAY;

        // out of bounds check for this algo. local operands would have to be adjusted to int64_t.
        // but int32 suffices for the next 1,000,000 years
        if (zRaw > std::numeric_limits<int32_t>::max())
            return DateTime();

        const int32_t z = (int32_t)zRaw;
        BeAssert(z >= 0);
        const int32_t g = (int32_t)((z - 1867216.25) / 36524.25);
        const int32_t a = z + 1 + g - g / 4;
        const int32_t b = a + 1524;
        const int32_t c = (int32_t)((b - 122.1) / 365.25);
        const int32_t d = (int32_t)(c * 365.25);
        const int32_t e = (int32_t)((b - d) / 30.6001);
        const int32_t f = (int32_t)(30.6001 * e);

        const int32_t dayRaw = b - d - f;
        const int32_t monthRaw = e < 14 ? e - 1 : e - 13;
        const int32_t yearRaw = monthRaw > 2 ? c - 4716 : c - 4715;

        if (dayRaw < 0 || dayRaw > std::numeric_limits<uint8_t>::max() ||
            monthRaw < 0 || monthRaw > std::numeric_limits<uint8_t>::max() ||
            yearRaw < std::numeric_limits<int16_t>::min() || yearRaw > std::numeric_limits<int16_t>::max())
            return DateTime();

        day = (uint8_t)dayRaw;
        month = (uint8_t)monthRaw;
        year = (int16_t)yearRaw;
    }

    if (targetInfo.GetComponent() == DateTime::Component::Date)
        return DateTime(year, month, day);

    //time fraction

    //Rebase time fraction to start at 0 instead of 12 and strip date component
    uint64_t timeFraction = (julianDayInMsec + HALFDAY_IN_MSECS) % MSECS_IN_DAY;

    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    uint16_t msec = 0;
    //if time fraction is 0, all components are 0
    if (timeFraction != UINT64_C(0))
        {
        msec = timeFraction % 1000;

        //now remaining time fraction is in secs
        timeFraction = (timeFraction - msec) / 1000;

        second = timeFraction % 60;
        //now remaining time fraction is in minutes
        timeFraction = (timeFraction - second) / 60;

        minute = timeFraction % 60;

        //now remaining time fraction is the hours
        hour = (uint8_t) ((timeFraction - minute) / 60);
        BeAssert(hour < 24);
        }

    if (targetInfo.GetComponent() == DateTime::Component::TimeOfDay)
        return  DateTime::CreateTimeOfDay(hour, minute, second, msec);

    return DateTime(targetInfo.GetKind(), year, month, day, hour, minute, second, msec);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTimeConverter::ComputeLocalTimezoneOffsetFromLocalTime(int64_t& timezoneOffsetInMsec, uint64_t rawJdInMsec)
    {
    //rawJdInMsec does not yet include time zone transformation.
    //In this algo we first treat it as if it was UTC based and compute an approximate offset from it.
    //Offset is only approximate as local date time might just be in the area of switching
    //to / from DST. In that case treating local time as UTC yields the wrong offset.
    //This algo computes stuff based on the wrong offset which finally yields the right offset.
    int64_t approximateOffset = INT64_C(0);
    if (SUCCESS != ComputeLocalTimezoneOffsetFromUtcTime(approximateOffset, rawJdInMsec))
        return ERROR;

    //take JD and add approximate offset, so we are in approximate utc
    uint64_t approximateUtcJD = rawJdInMsec + approximateOffset;

    int64_t correctOffset = INT64_C(0);
    if (SUCCESS != ComputeLocalTimezoneOffsetFromUtcTime(correctOffset, approximateUtcJD))
        return ERROR;

    timezoneOffsetInMsec = correctOffset;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DateTimeConverter::ComputeLocalTimezoneOffsetFromUtcTime(int64_t& timezoneOffsetInMsec, uint64_t jdInMsec)
    {
    if (!IsInUnixEpoch(jdInMsec))
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
            {
            double jd = MillisecsToRationalDay(jdInMsec);
            LOG.debugv("Date [Julian Day %f] is outside Unix epoch. Using default date in Unix epoch (2000-01-01 00:00:00 UTC) for computing the local time zone offset.",
                       jd);
            }

        //use a default date time within the epoch (2000-1-1 midnight) outside DST.
        jdInMsec = TIMEZONEOFFSET_DEFAULT_DATE_IN_JULIANDAY_MSEC;
        }
    else
        {
        //strip the below second portion away as the conversion to local time ignores them anyways (as it uses a tm)
        //As time zone offsets are never more fine-grained than minutes, stripping the millisecs isn't a problem
        //(If outside the Unix epoch, this is not necessary, as the default date chosen doesn't have below-sec accuracy)
        jdInMsec -= jdInMsec % 1000;
        }

    //assert that the below seconds components are gone
    BeAssert(jdInMsec % 1000 == 0);

    int64_t utcUnixMillis = JulianDayToUnixMilliseconds(jdInMsec);
    BeAssert(utcUnixMillis >= 0);

    uint64_t utcUnixMillisUnsigned = (uint64_t) (utcUnixMillis);
    struct tm localTime;
    if (SUCCESS != BeTimeUtilities::ConvertUnixMillisToLocalTime(localTime, utcUnixMillisUnsigned))
        return ERROR;

    uint64_t localJdInMsec = INT64_C(0);
    if (SUCCESS != ComputeJulianDay(localJdInMsec, localTime))
        return ERROR;

    BeAssert(localJdInMsec % 1000 == 0);

    //cast both operands to int64 before substracting them from each other to avoid implicit conversions
    timezoneOffsetInMsec = (int64_t)(jdInMsec) -(int64_t) (localJdInMsec);
    return SUCCESS;
    }


//************************ DateTimeStringConverter ********************************

//---------------------------------------------------------------------------------------
//! @remarks This pattern supports T and a space as delimiter of the date and time component, e.g.
//! both <c>2013-09-15T12:05:39</> and <c>2013-09-15 12:05:39</> can be parsed correctly.
//! This is a minor deviation from the ISO standard (specifies the T delimiter), but allows to parse
//! SQL-99 date time literals (specifies the space delimiter)
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//Group 1: Year (Group 0 is entire match) - Pattern starts with ^ as no leading stuff is supported
//Group 2: Month
//Group 3: Day
//Non-capture group: Entire time component
//both T and space are supported as delimiters so that SQL-99 date time literals can be parsed, too. (see comment above)
//Group 4: Hour, can also be 24 as 24:00 (if used as end time)
//time component delimiter ':' is optional
//Group 5: Minute
//non-capture group: Second component is optional
//time component delimiter ':' is optional
//Group 6: Second
//Group 7: Second fraction
//Group 8: time zone (time zone delimiter ':' is optional)
// string has to end here. This is necessary as the time component is optional and therefore invalid time components would match the regex which they must not however.

#define DATETIME_REGEXPATTERN "^([+-]?[\\d]{4})" \
                            "-?" \
                            "(1[0-2]|0[1-9])" \
                            "-?" \
                            "(3[0-1]|0[1-9]|[1-2][\\d])" \
                            "(?:"  \
                            "[T ]" \
                            "(2[0-4]|[0-1][\\d])" \
                            ":?" \
                            "([0-5][\\d])" \
                            "(?:" \
                            ":?" \
                            "([0-5][\\d])" \
                            "(\\.[\\d]+)?" \
                            ")?" \
                            "(Z|[+-](?:2[0-3]|[0-1][\\d]):?[0-5][\\d])?" \
                            ")?$"

#define TIMEOFDAY_REGEXPATTERN "^(2[0-4]|[0-1][\\d]):?([0-5][\\d])(?::?([0-5][\\d])(\\.[\\d]+)?)?$"

#define ISO8601_TIMEZONE_UTC "Z"

#define ISO8601_DATE_FORMAT "%.4" PRIi16 "-%.2" PRIu8 "-%.2" PRIu8
#define ISO8601_TIME_FORMAT "%.2" PRIu8 ":%.2" PRIu8 ":%06.3lf"

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// Bentley guideline: Do not release non-POD static variables
std::regex* DateTimeStringConverter::s_dtRegex = nullptr;
std::regex* DateTimeStringConverter::s_todRegex = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static Utf8String ToIsoDateTimeString(DateTimeCR dateTime, bool useSubSec, bool appendUtcTimeZone) {
    char zBuf[32];
    int Y = dateTime.GetYear();
    int M = dateTime.GetMonth();
    int D = dateTime.GetDay();
    int h = dateTime.GetHour();
    int m = dateTime.GetMinute();
    int s = dateTime.GetSecond();
    int ms = dateTime.GetMillisecond();
    int n;
    if (Y < 0) Y = -Y;
    zBuf[1] = '0' + (Y / 1000) % 10;
    zBuf[2] = '0' + (Y / 100) % 10;
    zBuf[3] = '0' + (Y / 10) % 10;
    zBuf[4] = '0' + (Y) % 10;
    zBuf[5] = '-';
    zBuf[6] = '0' + (M / 10) % 10;
    zBuf[7] = '0' + (M) % 10;
    zBuf[8] = '-';
    zBuf[9] = '0' + (D / 10) % 10;
    zBuf[10] = '0' + (D) % 10;
    zBuf[11] = 'T';
    zBuf[12] = '0' + (h / 10) % 10;
    zBuf[13] = '0' + (h) % 10;
    zBuf[14] = ':';
    zBuf[15] = '0' + (m / 10) % 10;
    zBuf[16] = '0' + (m) % 10;
    zBuf[17] = ':';
    if (useSubSec) {
        s = (int)(1000.0 * s + ms + 0.5);
        zBuf[18] = '0' + (s / 10000) % 10;
        zBuf[19] = '0' + (s / 1000) % 10;
        zBuf[20] = '.';
        zBuf[21] = '0' + (s / 100) % 10;
        zBuf[22] = '0' + (s / 10) % 10;
        zBuf[23] = '0' + (s) % 10;
        if (appendUtcTimeZone && dateTime.GetInfo().GetKind() == DateTime::Kind::Utc){
            zBuf[24] = 'Z';
            zBuf[25] = 0;
            n = 25;
        } else {
            zBuf[24] = 0;
            n = 24;
        }
    } else {
        zBuf[18] = '0' + (s / 10) % 10;
        zBuf[19] = '0' + (s) % 10;
        if (appendUtcTimeZone && dateTime.GetInfo().GetKind() == DateTime::Kind::Utc){
            zBuf[20] = 'Z';
            zBuf[21] = 0;
            n = 21;
        } else {
            zBuf[20] = 0;
            n = 20;
        }
    }
    if (dateTime.GetYear() < 0) {
        zBuf[0] = '-';
        return Utf8String(zBuf, n);
    }
    return Utf8String(&zBuf[1], n - 1);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static Utf8String ToIsoDateString(DateTimeCR dateTime) {
    char zBuf[16];
    int Y = dateTime.GetYear();
    int M = dateTime.GetMonth();
    int D = dateTime.GetDay();

    if (Y < 0) Y = -Y;
    zBuf[1] = '0' + (Y / 1000) % 10;
    zBuf[2] = '0' + (Y / 100) % 10;
    zBuf[3] = '0' + (Y / 10) % 10;
    zBuf[4] = '0' + (Y) % 10;
    zBuf[5] = '-';
    zBuf[6] = '0' + (M / 10) % 10;
    zBuf[7] = '0' + (M) % 10;
    zBuf[8] = '-';
    zBuf[9] = '0' + (D / 10) % 10;
    zBuf[10] = '0' + (D) % 10;
    zBuf[11] = 0;

    if (dateTime.GetYear() < 0) {
        zBuf[0] = '-';
        return Utf8String(zBuf, 11);
    }
    return Utf8String(&zBuf[1], 10);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static Utf8String ToIsoTimeString(DateTimeCR dateTime, bool useSubSec) {
    char zBuf[16];
    int h = dateTime.GetHour();
    int m = dateTime.GetMinute();
    int s = dateTime.GetSecond();
    int ms = dateTime.GetMillisecond();
    int n;
    zBuf[0] = '0' + (h / 10) % 10;
    zBuf[1] = '0' + (h) % 10;
    zBuf[2] = ':';
    zBuf[3] = '0' + (m / 10) % 10;
    zBuf[4] = '0' + (m) % 10;
    zBuf[5] = ':';
    if (useSubSec) {
        s = (int)(1000.0 * s + ms + 0.5);
        zBuf[6] = '0' + (s / 10000) % 10;
        zBuf[7] = '0' + (s / 1000) % 10;
        zBuf[8] = '.';
        zBuf[9] = '0' + (s / 100) % 10;
        zBuf[10] = '0' + (s / 10) % 10;
        zBuf[11] = '0' + (s) % 10;
        zBuf[12] = 0;
        n = 12;
    } else {
        zBuf[6] = '0' + (s / 10) % 10;
        zBuf[7] = '0' + (s) % 10;
        zBuf[8] = 0;
        n = 8;
    }
    return Utf8String(zBuf, n);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DateTimeStringConverter::ToIso8601(DateTimeCR dateTime)
    {
    if (!dateTime.IsValid())
        return Utf8String();

    if (dateTime.GetInfo().GetComponent() == DateTime::Component::Date)
        return ToIsoDateString(dateTime);

    if (dateTime.IsTimeOfDay())
        return ToIsoTimeString(dateTime, true);

    BeAssert(dateTime.GetInfo().GetComponent() == DateTime::Component::DateAndTime);
    return ToIsoDateTimeString(dateTime, true, true);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DateTime DateTimeStringConverter::FromIso8601(Utf8CP iso8601DateTime) {
    if (Utf8String::IsNullOrEmpty(iso8601DateTime))
        return DateTime();

    std::match_results<Utf8CP> matches;
    const bool hasMatches = regex_search(iso8601DateTime, matches, GetDateTimeRegex());
    if (!hasMatches || matches.size() == 0)
        return FromIso8601TimeOfDay(iso8601DateTime);

    //Parse date component
    const size_t yearGroupIndex = 1;
    const size_t monthGroupIndex = 2;
    const size_t dayGroupIndex = 3;
    const size_t hourGroupIndex = 4;
    const size_t minGroupIndex = 5;
    const size_t secGroupIndex = 6;
    const size_t secFractionGroupIndex = 7;
    const size_t timezoneGroupIndex = 8;

    int16_t year = 0;
    uint8_t month = 0, day = 0;
    if (!TryRetrieveValueFromRegexMatch(year, matches, yearGroupIndex)) // year is required
        return DateTime();

    if (!TryRetrieveValueFromRegexMatch(month, matches, monthGroupIndex))
        return DateTime();

    if (!TryRetrieveValueFromRegexMatch(day, matches, dayGroupIndex))
        return DateTime();

    //Parse time component (if specified in input string)
    uint8_t hour = 0;
    if (!TryRetrieveValueFromRegexMatch(hour, matches, hourGroupIndex))
        {
        //no time component specified -> error out, if no date component specified either or if other time components are specified
        if (RegexGroupMatched(matches, minGroupIndex) ||
            RegexGroupMatched(matches, secGroupIndex) || RegexGroupMatched(matches, secFractionGroupIndex) ||
            RegexGroupMatched(matches, timezoneGroupIndex))
            return DateTime();

        BeAssert(month > 0 && day > 0);
        return DateTime(year, month, day);
        }

    uint8_t minute = 0;
    if (!TryRetrieveValueFromRegexMatch(minute, matches, minGroupIndex))
        return DateTime();  // minute must be given too (all the rest is optional)

    uint8_t second = 0;
    uint16_t msecond = 0;
    if (TryRetrieveValueFromRegexMatch(second, matches, secGroupIndex))
        {
        double secondFraction = 0.0;
        if (TryRetrieveValueFromRegexMatch(secondFraction, matches, secFractionGroupIndex))
            {
            BeAssert(secondFraction >= 0.0 && secondFraction < 1.0);
            msecond = (uint16_t) (secondFraction * 1000 + 0.5);
            // Due to rounding up of some microsecond value (0.99956 for example ... yes it happened) we may end up with 1000 as value
            // Which should be set to 0 and increase the seconds by one which may result in 60 seconds that would
            // require augmenting the minutes and so on ... till we may end up changing the year.
            // This would be a pain for a mere half of a millisecond so we will simple downgrade 1000 to 999 in this improbable case
            if (1000 == msecond)
                msecond = 999;
            }
        }
    else
        {
        //if seconds are not specified, second fraction must not exist either
        if (RegexGroupMatched(matches, secFractionGroupIndex))
            return DateTime();
        }

    if (hour == 24 && (minute != 0 || second != 0 || msecond != 0))
        return DateTime();

    // now we know that this is a date and time.
    DateTime::Kind kind = DateTime::Kind::Unspecified;
    Utf8String timezoneIndicator;
    if (TryRetrieveValueFromRegexMatch(timezoneIndicator, matches, timezoneGroupIndex))
        {
        BeAssert(!timezoneIndicator.empty());
        if (timezoneIndicator.StartsWithIAscii(ISO8601_TIMEZONE_UTC))
            kind = DateTime::Kind::Utc;
        else
            kind = DateTime::Kind::Local;
        }

    return DateTime(kind, year, month, day, hour, minute, second, msecond);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DateTime DateTimeStringConverter::FromIso8601TimeOfDay(Utf8CP iso8601TimeOfDay) {
    if (Utf8String::IsNullOrEmpty(iso8601TimeOfDay))
        return DateTime();

    std::match_results<Utf8CP> matches;
    const bool hasMatches = regex_search(iso8601TimeOfDay, matches, GetTimeOfDayRegex());
    if (!hasMatches || matches.size() == 0)
        return DateTime();

    const size_t hourGroupIndex = 1;
    const size_t minGroupIndex = 2;
    const size_t secGroupIndex = 3;
    const size_t secFractionGroupIndex = 4;

    uint8_t hour = 0;
    if (!TryRetrieveValueFromRegexMatch(hour, matches, hourGroupIndex))
        return DateTime(); // hour is required

    uint8_t minute = 0;
    if (!TryRetrieveValueFromRegexMatch(minute, matches, minGroupIndex))
        return DateTime(); // minute must be given too (all the rest is optional)

    uint8_t second = 0;
    uint16_t msecond = 0;
    if (TryRetrieveValueFromRegexMatch(second, matches, secGroupIndex)) {
        double secondFraction = 0.0;
        if (TryRetrieveValueFromRegexMatch(secondFraction, matches, secFractionGroupIndex)) {
            BeAssert(secondFraction >= 0.0 && secondFraction < 1.0);
            msecond = (uint16_t)(secondFraction * 1000 + 0.5);
            // Due to rounding up of some microsecond value (0.99956 for example ... yes it happened) we may end up with 1000 as value
            // Which should be set to 0 and increase the seconds by one which may result in 60 seconds that would
            // require augmenting the minutes and so on ... till we may end up changing the year.
            // This would be a pain for a mere half of a millisecond so we will simple downgrade 1000 to 999 in this improbable case
            if (1000 == msecond)
                msecond = 999;
        }
    } else {
        // if seconds are not specified, second fraction must not exist either
        if (RegexGroupMatched(matches, secFractionGroupIndex))
            return DateTime();
    }

    if (hour == 24 && (minute != 0 || second != 0 || msecond != 0))
        return DateTime();

    return DateTime::CreateTimeOfDay(hour, minute, second, msecond);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeStringConverter::RegexGroupMatched(std::match_results<Utf8CP> const& matches, size_t groupIndex)
    {
    return matches[groupIndex].length() > 0 && matches[groupIndex].matched;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeStringConverter::TryRetrieveValueFromRegexMatch(int16_t& matchValue, std::match_results<Utf8CP> const& matches, size_t groupIndex)
    {
    if (!RegexGroupMatched(matches, groupIndex))
        return false;

    int matchValueInt = atoi(matches[groupIndex].str().c_str());
    matchValue = (int16_t) matchValueInt;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeStringConverter::TryRetrieveValueFromRegexMatch(uint8_t& matchValue, std::match_results<Utf8CP> const& matches, size_t groupIndex)
    {
    if (!RegexGroupMatched(matches, groupIndex))
        {
        return false;
        }

    int matchValueInt = atoi(matches[groupIndex].str().c_str());
    matchValue = (uint8_t) (matchValueInt);
    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeStringConverter::TryRetrieveValueFromRegexMatch(double& matchValue, std::match_results<Utf8CP> const& matches, size_t groupIndex)
    {
    if (!RegexGroupMatched(matches, groupIndex))
        {
        return false;
        }

    matchValue = atof(matches[groupIndex].str().c_str());
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeStringConverter::TryRetrieveValueFromRegexMatch(Utf8StringR matchValue, std::match_results<Utf8CP> const& matches, size_t groupIndex)
    {
    if (!RegexGroupMatched(matches, groupIndex))
        {
        return false;
        }

    matchValue.assign(matches[groupIndex].str().c_str());
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::regex const& DateTimeStringConverter::GetDateTimeRegex()
    {
    static std::once_flag s_onceFlag;
    std::call_once(s_onceFlag, [] () { s_dtRegex = new std::regex(DATETIME_REGEXPATTERN, std::regex_constants::optimize); });
    return *s_dtRegex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::regex const& DateTimeStringConverter::GetTimeOfDayRegex()
    {
    static std::once_flag s_onceFlag;
    std::call_once(s_onceFlag, [] () { s_todRegex = new std::regex(TIMEOFDAY_REGEXPATTERN, std::regex_constants::optimize); });
    return *s_todRegex;
    }

END_BENTLEY_NAMESPACE
