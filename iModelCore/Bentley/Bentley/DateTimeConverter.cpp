/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/DateTimeConverter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DateTimeConverter.h"

#include <Bentley/BeTimeUtilities.h>
#include <Bentley/ScopedArray.h>
#include <Logging/bentleylogging.h>
#include <limits.h>
#include <stdlib.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger(L"DateTime"))

BEGIN_BENTLEY_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
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
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
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
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//does the raw computation without considering time zones (i.e. treats the given
//date as if it were in UTC)
//static
BentleyStatus DateTimeConverter::ComputeJulianDay(uint64_t& julianDayInMsec, DateTime const& dateTime)
    {
    return ComputeJulianDay(julianDayInMsec, dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay(),
                            dateTime.GetHour(), dateTime.GetMinute(), dateTime.GetSecond(), dateTime.GetMillisecond());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//does the raw computation without considering time zones (i.e. treats the given
//date as if it were in UTC)
//static
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
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//does the raw computation without considering time zones (i.e. treats the given
//date as if it were in UTC)
//static 
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

    //Algorithm adopted from Meeus page 61 (as in SQLite)

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
    const uint64_t timeComponentInSecs = hour * INT64_C(3600) + minute * INT64_C(60) + second;

    const uint64_t julianDayInSecs = dateComponentInSecs + timeComponentInSecs;

    //convert to millisecs and add the input millisec component
    julianDayInMsec = julianDayInSecs * 1000 + millisecond;
    return SUCCESS;

    //ARCHIVE: Alternate algorithm from http://www.tondering.dk/claus/cal/julperiod.php
    //for which roundtrip also works for leap years BC. (e.g -1000-03-01)
    //See also http://www.cs.utsa.edu/~cs1063/projects/Spring2011/Project1/jdn-explanation.html
    /*
    //rebase to March 1st -4800  (4801BC).
    //-> think of Jan and Feb as being month 13 and 14 of the previous year
    //a = 1 for Jan / Feb. a = 0 for rest
    const int a = (14 - month) / 12;
    const int m = month + 12*a - 3;
    //years since 4800
    const int y = year + 4800 - a;
    const Int64 jdd = day + (153*m + 2) / 5 + 365 * y + y/4 - y/100 + y/400 - 32045;

    const Int64 dateComponentInSecs = jdd * SECS_IN_DAY - 43200LL;
    */
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus DateTimeConverter::ConvertFromJulianDay(DateTime& dateTime, uint64_t julianDayInMsec, DateTime::Info const& targetInfo, bool applyTimezoneOffset)
    {
    if (!targetInfo.IsValid())
        return ERROR;

    if (applyTimezoneOffset && targetInfo.GetKind() == DateTime::Kind::Local)
        {
        int64_t localTimezoneOffsetInMsec = INT64_C(0);
        BentleyStatus stat = ComputeLocalTimezoneOffsetFromUtcTime(localTimezoneOffsetInMsec, julianDayInMsec);
        if (stat != SUCCESS)
            return ERROR;

        julianDayInMsec -= localTimezoneOffsetInMsec;
        }

    return ParseJulianDay(dateTime, julianDayInMsec, targetInfo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//does the raw computation without considering time zones (i.e. expects the given
//date to be in UTC)
//static 
BentleyStatus DateTimeConverter::ParseJulianDay(DateTimeR dateTime, uint64_t julianDayInMsec, DateTime::Info const& targetInfo)
    {
    const uint64_t HALFDAY_IN_MSECS = MSECS_IN_DAY / 2;
    //date fraction of julian day
    //Strip time fraction. As JD is based on noon, the JD value is shifted by half a day so that z represents an unambiguous day.
    //(technically this is the same as rounding the JD value to the next full day number)
    const uint64_t zRaw = (julianDayInMsec + HALFDAY_IN_MSECS) / MSECS_IN_DAY;
    
    //out of bounds check for this algo. local operands would have to be adjusted to int64_t.
    //but int32 suffices for the next 1,000,000 years
    if (zRaw > std::numeric_limits<int32_t>::max())
        return ERROR; 

    const int32_t z = (int32_t) zRaw;
    BeAssert(z >= 0);
    const int32_t g = (int32_t) ((z - 1867216.25) / 36524.25);
    const int32_t a = z + 1 + g - g / 4;
    const int32_t b = a + 1524;
    const int32_t c = (int32_t) ((b - 122.1) / 365.25);
    const int32_t d = (int32_t) (c * 365.25);
    const int32_t e = (int32_t) ((b - d) / 30.6001);
    const int32_t f = (int32_t) (30.6001 * e);

    const int32_t dayRaw = b - d - f;
    const int32_t monthRaw = e < 14 ? e - 1 : e - 13;
    const int32_t yearRaw = monthRaw > 2 ? c - 4716 : c - 4715;

    if (dayRaw < 0 || dayRaw > std::numeric_limits<uint8_t>::max() || 
        monthRaw < 0 || monthRaw > std::numeric_limits<uint8_t>::max() ||
        yearRaw < std::numeric_limits<int16_t>::min() || yearRaw > std::numeric_limits<int16_t>::max())
        return ERROR;

    const uint8_t day = (uint8_t) dayRaw;
    const uint8_t month = (uint8_t) monthRaw;
    const int16_t year = (int16_t) yearRaw;

    if (targetInfo.GetComponent() == DateTime::Component::Date)
        {
        dateTime = DateTime(year, month, day);
        return dateTime.IsValid() ? SUCCESS : ERROR;
        }

    //time fraction

    //Rebase time fraction to start at 0 instead of 12 and strip date component
    uint64_t timeFraction = (julianDayInMsec + HALFDAY_IN_MSECS) % MSECS_IN_DAY;

    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    uint16_t msec = 0;
    //if time fraction is 0, all components are 0
    if (timeFraction != INT64_C(0))
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

    dateTime = DateTime(targetInfo.GetKind(), year, month, day, hour, minute, second, msec);
    return dateTime.IsValid() ? SUCCESS : ERROR;

    //ARCHIVE: Alternate algorithm from http://www.tondering.dk/claus/cal/julperiod.php
    //for which roundtrip also works for leap years BC. (e.g -1000-03-01)
        /*
    const double jd = 1.0 * julianDayInHns / HECTONANOSECS_IN_DAY;
    const int j = static_cast<int> (jd + 0.5) + 32044; // days since -4800
    const int g = j / 146097; //Gregorian quadricentennial cycles since epoch (146097 days per cycle)
    const int dg = j % 146097; //days since current cycle
    const int c = (dg / 36524 + 1) * 3 / 4; //number of Gregorian centennial cycles since current cycle (0 to 3) (36524 days per centennial cycle)
    const int dc =  dg - c * 36524; //days since current cycle
    BeAssert (dc >= 0);
    const int b = dc / 1461;//Julian quadrennials since Gregorian century (0 to 24) (1461 days in 4 years)
    const int db = dc % 1461; //days in the Gregorian century
    const int a = (db / 365 + 1) * 3 / 4;
    const int da = db - a * 365;
    const int y = g * 400 + c* 100 + b * 4 + a; // full years since March 1st, 4801BC 00:00 UTC
    const int m = (da * 5 + 308) / 153 - 2; // full months since last March 1st 00:00 UTC
    const int d = da - (m + 4) * 153 / 5 + 122; //days since first of month 00:00 UTC incl fractions

    const Int16 year = y - 4800 + (m + 2) / 12;
    const UInt16 month = (m + 2) % 12 + 1;
    const UInt16 day = d + 1;
    */
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
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
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
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


//************************ Iso8601Regex ********************************
//Regex taken from and modified: http://my.safaribooksonline.com/book/programming/regular-expressions/9780596802837/4dot-validation-and-formatting/id2983571
//---------------------------------------------------------------------------------------
//! @remarks This pattern supports T and a space as delimiter of the date and time component, e.g.
//! both <c>2013-09-15T12:05:39</> and <c>2013-09-15 12:05:39</> can be parsed correctly.
//! This is a minor deviation from the ISO standard (specifies the T delimiter), but allows to parse 
//! SQL-99 date time literals (specifies the space delimiter)
// @bsimethod                                    Krischan.Eberle                  05/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const Iso8601Regex::PATTERN =
"^([+-]?[\\d]{4})" //Group 1: Year (Group 0 is entire match) - Pattern starts with ^ as no leading stuff is supported
"-?" //date component delimiter '-' is optional
"(1[0-2]|0[1-9])" //Group 2: Month
"-?" //date component delimiter '-' is optional
"(3[0-1]|0[1-9]|[1-2][\\d])" //Group 3: Day
"(?:"                          //Non-capture group: Entire time component
"[T ]" //both T and space are supported as delimiters so that SQL-99 date time literals can be parsed, too. (see comment above)
"(2[0-3]|[0-1][\\d])" //Group 4: Hour
":?" //time component delimiter ':' is optional
"([0-5][\\d])" //Group 5: Minute
"(?:" //non-capture group: Second component is optional
":?" //time component delimiter ':' is optional
"([0-5][\\d])" //Group 6: Second
"(\\.[\\d]+)?" //Group 7: Second fraction
")?"
"(Z|[+-](?:2[0-3]|[0-1][\\d]):?[0-5][\\d])?" //Group 8: time zone (time zone delimiter ':' is optional)
")?$"; // string has to end here. This is necessary as the time component is optional and therefore invalid time components would match the regex which they must not however.

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  05/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool Iso8601Regex::Match(Matches& matches, Utf8CP iso8601DateTime) const
    {
    return std::regex_search(iso8601DateTime, matches, m_regex);
    }

//************************ DateTimeStringConverter ********************************
#define ISO8601_TIMEZONE_UTC "Z"

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String DateTimeStringConverter::ToIso8601(DateTimeCR dateTime)
    {
    if (!dateTime.IsValid())
        return Utf8String();

    if (dateTime.GetInfo().GetComponent() == DateTime::Component::Date)
        {
        Utf8CP isoFormat = "%.4d-%.2d-%.2d";
        size_t approxSize = 11;
        Utf8String str;
        str.reserve(approxSize);
        str.Sprintf(isoFormat, dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay());
        return str;
        }

    BeAssert(dateTime.GetInfo().GetComponent() == DateTime::Component::DateAndTime);

    double effectiveSeconds = dateTime.GetSecond() + dateTime.GetMillisecond() / 1000.0;
    BeAssert(effectiveSeconds < (dateTime.GetSecond() + 1.0));
    
    Utf8CP isoFormat = "%.4d-%.2d-%.2dT%.2d:%.2d:%06.3lf%s";
    size_t approxSize = 23;

    Utf8CP timeZoneIndicator = "";
    //if date is in UTC suffix the string with Z according to ISO
    //for local and unspecified dates don't suffix anything (compliant with ISO, too)
    if (dateTime.GetInfo().GetKind() == DateTime::Kind::Utc)
        {
        timeZoneIndicator = ISO8601_TIMEZONE_UTC;
        approxSize = 24;
        }

    Utf8String str;
    str.reserve(approxSize);
    str.Sprintf(isoFormat, dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay(),
                dateTime.GetHour(), dateTime.GetMinute(), effectiveSeconds, timeZoneIndicator);

    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTimeStringConverter::FromIso8601(DateTimeR dateTime, Utf8CP iso8601DateTime)
    {
    if (Utf8String::IsNullOrEmpty(iso8601DateTime))
        return ERROR;

    Iso8601Regex::Matches matches;
    static const Iso8601Regex regex;
    const bool hasMatches = regex.Match(matches, iso8601DateTime);
    size_t matchCount = 0;
    if (!hasMatches || (matchCount = matches.size()) == 0 || matchCount != Iso8601Regex::EXPECTED_MATCH_COUNT)
        return ERROR;

    //Parse date component
    BeAssert(RegexGroupMatched(matches, Iso8601Regex::YEAR_GROUPINDEX) && RegexGroupMatched(matches, Iso8601Regex::MONTH_GROUPINDEX) && RegexGroupMatched(matches, Iso8601Regex::DAY_GROUPINDEX) && "All three date components are mandatory");
    int intValue = 0;
    TryRetrieveValueFromRegexMatch(intValue, matches, Iso8601Regex::YEAR_GROUPINDEX);
    int16_t year = (int16_t) intValue;

    uint8_t month = 0;
    TryRetrieveValueFromRegexMatch(month, matches, Iso8601Regex::MONTH_GROUPINDEX);

    uint8_t day = 0;
    TryRetrieveValueFromRegexMatch(day, matches, Iso8601Regex::DAY_GROUPINDEX);

    //Parse time component (if specified in input string)
    uint8_t hour = 0;
    if (!TryRetrieveValueFromRegexMatch(hour, matches, Iso8601Regex::HOUR_GROUPINDEX))
        {
        //no time component specified.
        BeAssert(!RegexGroupMatched(matches, Iso8601Regex::MINUTE_GROUPINDEX) && !RegexGroupMatched(matches, Iso8601Regex::SECOND_GROUPINDEX) && !RegexGroupMatched(matches, Iso8601Regex::SECONDFRACTION_GROUPINDEX) && !RegexGroupMatched(matches, Iso8601Regex::TIMEZONE_GROUPINDEX));
        dateTime = DateTime(year, month, day);
        return dateTime.IsValid() ? SUCCESS : ERROR;
        }

    //if hour component is given, minute must be given too (all the rest is optional)
    BeAssert(RegexGroupMatched(matches, Iso8601Regex::MINUTE_GROUPINDEX));

    uint8_t minute = 0;
    TryRetrieveValueFromRegexMatch(minute, matches, Iso8601Regex::MINUTE_GROUPINDEX);

    uint8_t second = 0;
    uint16_t msecond = 0;
    if (TryRetrieveValueFromRegexMatch(second, matches, Iso8601Regex::SECOND_GROUPINDEX))
        {
        double secondFraction = 0.0;
        if (TryRetrieveValueFromRegexMatch(secondFraction, matches, Iso8601Regex::SECONDFRACTION_GROUPINDEX))
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
        BeAssert(!RegexGroupMatched(matches, Iso8601Regex::SECONDFRACTION_GROUPINDEX));
        }

    DateTime::Kind kind = DateTime::Kind::Unspecified;
    Utf8String timezoneIndicator;
    if (TryRetrieveValueFromRegexMatch(timezoneIndicator, matches, Iso8601Regex::TIMEZONE_GROUPINDEX))
        {
        BeAssert(!timezoneIndicator.empty());
        if (timezoneIndicator.StartsWithIAscii(ISO8601_TIMEZONE_UTC))
            kind = DateTime::Kind::Utc;
        else
            kind = DateTime::Kind::Local;
        }

    dateTime = DateTime(kind, year, month, day, hour, minute, second, msecond);
    return dateTime.IsValid() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeStringConverter::RegexGroupMatched(Iso8601Regex::Matches const& matches, size_t groupIndex)
    {
    return matches[groupIndex].length() > 0 && matches[groupIndex].matched;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeStringConverter::TryRetrieveValueFromRegexMatch(int& matchValue, Iso8601Regex::Matches const& matches, size_t groupIndex)
    {
    if (!RegexGroupMatched(matches, groupIndex))
        {
        return false;
        }

    matchValue = atoi(matches[groupIndex].str().c_str());
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeStringConverter::TryRetrieveValueFromRegexMatch(uint8_t& matchValue, Iso8601Regex::Matches const& matches, size_t groupIndex)
    {
    if (!RegexGroupMatched(matches, groupIndex))
        {
        return false;
        }

    int matchValueInt = atoi(matches[groupIndex].str().c_str());
    matchValue = static_cast<uint8_t> (matchValueInt);
    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeStringConverter::TryRetrieveValueFromRegexMatch(double& matchValue, Iso8601Regex::Matches const& matches, size_t groupIndex)
    {
    if (!RegexGroupMatched(matches, groupIndex))
        {
        return false;
        }

    matchValue = atof(matches[groupIndex].str().c_str());
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeStringConverter::TryRetrieveValueFromRegexMatch(Utf8StringR matchValue, Iso8601Regex::Matches const& matches, size_t groupIndex)
    {
    if (!RegexGroupMatched(matches, groupIndex))
        {
        return false;
        }

    matchValue.assign(matches[groupIndex].str().c_str());
    return true;
    }

END_BENTLEY_NAMESPACE
