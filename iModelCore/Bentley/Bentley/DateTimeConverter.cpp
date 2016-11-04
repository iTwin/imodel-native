/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/DateTimeConverter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DateTimeConverter.h"

#include <Bentley/BeTimeUtilities.h>
#include <Bentley/ScopedArray.h>
#include <Logging/bentleylogging.h>
#include <limits.h>
#include <stdlib.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"DateTime"))

BEGIN_BENTLEY_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static member initialization
const uint16_t DateTimeConverter::HECTONANOSECS_IN_MILLISEC = 10000;
const uint32_t DateTimeConverter::SECS_IN_DAY = 86400;

const uint32_t DateTimeConverter::HECTONANOSECS_IN_SEC = 1000 * HECTONANOSECS_IN_MILLISEC;
const uint64_t DateTimeConverter::HECTONANOSECS_IN_DAY = 1ULL * SECS_IN_DAY * HECTONANOSECS_IN_SEC;

//2000-01-01 00:00:00 UTC
const uint64_t DateTimeConverter::TIMEZONEOFFSET_DEFAULT_DATE_IN_JULIANDAY_HNS = 211813444800ULL * HECTONANOSECS_IN_SEC;
//1970-01-01 00:00:00 UTC
const uint64_t DateTimeConverter::UNIXEPOCH_BASELINE_IN_JULIANDAY_HNS = 210866760000ULL * HECTONANOSECS_IN_SEC;
//2018-01-19 03:14:07 UTC
const uint64_t DateTimeConverter::UNIXEPOCH_END_IN_JULIANDAY_HNS = 213014265247ULL * HECTONANOSECS_IN_SEC;

//01-01-01 00:00:00 UTC as JulianDay number in hecto-nanoseconds
//(according to http://quasar.as.utexas.edu/BillInfo/JulianDateCalc.html)
const uint64_t DateTimeConverter::CE_EPOCH_AS_JD_HNS = 1487311632000000000ULL;


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTimeConverter::ToJulianDay (uint64_t& julianDayInHns, DateTimeCR dateTime)
    {   
    return ConvertToJulianDay (julianDayInHns, dateTime, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTimeConverter::ToJulianDay (double& julianDay, DateTimeCR dateTime)
    {
    uint64_t jdInHns = 0ULL;
    BentleyStatus stat = ToJulianDay (jdInHns, dateTime);
    if (SUCCESS != stat)
        return ERROR;
    
    julianDay = HnsToRationalDay (jdInHns);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTimeConverter::FromJulianDay (DateTime& dateTime, uint64_t julianDayInHns, DateTime::Info const& targetInfo)
    {
    return ConvertFromJulianDay (dateTime, julianDayInHns, targetInfo, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTimeConverter::FromJulianDay (DateTime& dateTime, double julianDay, DateTime::Info const& targetInfo)
    {
    const uint64_t jdInHns = RationalDayToHns (julianDay);
    return FromJulianDay (dateTime, jdInHns, targetInfo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTimeConverter::ConvertToJulianDay (uint64_t& julianDayInHns, DateTime const& dateTime, bool applyTimezoneOffset)
    {
    if (!dateTime.IsValid ())
        return ERROR;

    //compute raw JD (without time zone offset)
    uint64_t rawJd = 0ULL;
    BentleyStatus stat = ComputeJulianDay (rawJd, dateTime);
    if (SUCCESS != stat)
        return ERROR;

    if (!applyTimezoneOffset)
        {
        julianDayInHns = rawJd;
        return SUCCESS;
        }

    //compute local time zone offset if date time is in local time
    int64_t localTimezoneOffsetInHns = 0LL;
    if (dateTime.GetKind() == DateTime::Kind::Local)
        {
        stat = ComputeLocalTimezoneOffsetFromLocalTime (localTimezoneOffsetInHns, rawJd);
        if (SUCCESS != stat)
            return ERROR;
        }

    julianDayInHns = rawJd + localTimezoneOffsetInHns;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//does the raw computation without considering time zones (i.e. treats the given
//date as if it were in UTC)
//static
BentleyStatus DateTimeConverter::ComputeJulianDay (uint64_t& julianDayInHns, DateTime const& dateTime)
    {
    return ComputeJulianDay (julianDayInHns, dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay(),
                                            dateTime.GetHour(), dateTime.GetMinute(), dateTime.GetSecond(), dateTime.GetHectoNanosecond());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//does the raw computation without considering time zones (i.e. treats the given
//date as if it were in UTC)
//static
BentleyStatus DateTimeConverter::ComputeJulianDay (uint64_t& julianDayInHns, tm const& dateTimeTm)
    {
    return ComputeJulianDay (julianDayInHns, static_cast <int16_t> (dateTimeTm.tm_year + 1900), //tm year is based on 1900
                                             static_cast <uint8_t> (dateTimeTm.tm_mon + 1), //tm month is 0-based
                                             static_cast <uint8_t> (dateTimeTm.tm_mday),
                                             static_cast <uint8_t> (dateTimeTm.tm_hour),
                                             static_cast <uint8_t> (dateTimeTm.tm_min),
                                             static_cast <uint8_t> (dateTimeTm.tm_sec), 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//does the raw computation without considering time zones (i.e. treats the given
//date as if it were in UTC)
//static 
BentleyStatus DateTimeConverter::ComputeJulianDay (uint64_t& julianDayInHns, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint32_t hectoNanosecond)
    {
    julianDayInHns = 0ULL;

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

    const int a = year / 100;
    const int b = 2 - a + (a/4);
    const int x1 = static_cast<int> (365.25 * (year + 4716));
    const int x2 = static_cast<int> (30.6001 * (month + 1));

    const int64_t dateComponentInSecs = static_cast<int64_t> ((x1 + x2 + static_cast<int> (day) + b - 1524.5) * SECS_IN_DAY);

    //the minimum value is -0.5 (in fractional notation) which maps to the beginning of the epoch.
    //values below are wrong as the precondition checks that the input is not before the epoch.
    BeAssert (dateComponentInSecs >= -43200LL);

    //internally compute the JulianDays in hecto-nanosecs since start of JulianDay epoch
    //(no loss of accuracy here)

    //*** compute time component
    const uint64_t timeComponentInSecs = hour * 3600ULL + minute * 60ULL + second;
    
    const uint64_t julianDayInSecs = dateComponentInSecs + timeComponentInSecs;
    //check for UInt64 overflow 
    const uint64_t maxAllowedInSecs = ULLONG_MAX / HECTONANOSECS_IN_SEC;
    BeAssert (julianDayInSecs < maxAllowedInSecs);

    //convert to hecto nanosecs and add the input hecto-nanosecond component
    julianDayInHns = julianDayInSecs * HECTONANOSECS_IN_SEC + hectoNanosecond;
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
BentleyStatus DateTimeConverter::ConvertFromJulianDay (DateTime& dateTime, uint64_t julianDayInHns, DateTime::Info const& targetInfo, bool applyTimezoneOffset)
    {
    if (applyTimezoneOffset && targetInfo.GetKind () == DateTime::Kind::Local)
        {
        int64_t localTimezoneOffsetInHns = 0LL;
        BentleyStatus stat = ComputeLocalTimezoneOffsetFromUtcTime (localTimezoneOffsetInHns, julianDayInHns);
        if (stat != SUCCESS)
            return ERROR;

        julianDayInHns -= localTimezoneOffsetInHns;
        }

    return ParseJulianDay (dateTime, julianDayInHns, targetInfo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//does the raw computation without considering time zones (i.e. expects the given
//date to be in UTC)
//static 
BentleyStatus DateTimeConverter::ParseJulianDay (DateTimeR dateTime, uint64_t julianDayInHns, DateTime::Info const& targetInfo)
    {
    const uint64_t HALFDAY_IN_HECTONANOSECS = HECTONANOSECS_IN_DAY / 2;

    //date fraction of julian day
    //Strip time fraction. As JD is based on noon, the JD value is shifted by half a day so that z represents an unambiguous day.
    //(technically this is the same as rounding the JD value to the next full day number)
    const int z = static_cast<int> ((julianDayInHns + HALFDAY_IN_HECTONANOSECS) / HECTONANOSECS_IN_DAY);

    const int g = static_cast<int> ((z - 1867216.25) / 36524.25);
    const int a = z + 1 + g - g / 4;
    const int b = a + 1524;
    const int c = static_cast<int> ((b - 122.1) / 365.25);
    const int d = static_cast<int> (c * 365.25);
    const int e = static_cast<int> ((b - d) / 30.6001);
    const int f = static_cast<int> (30.6001 * e);

    const uint8_t day = static_cast <uint8_t> (b - d - f);
    const uint8_t month = static_cast <uint8_t> (e < 14 ? e - 1 : e - 13);
    const int16_t year = static_cast <int16_t> (month > 2 ? c - 4716 : c - 4715);

    //time fraction

    //Rebase time fraction to start at 0 instead of 12 and strip date component
    uint64_t timeFraction = (julianDayInHns + HALFDAY_IN_HECTONANOSECS) % HECTONANOSECS_IN_DAY;

    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    uint32_t hectoNanosecond = 0;
    //if time fraction is 0, all components are 0
    if (timeFraction != 0ULL)
        {
        hectoNanosecond = timeFraction % HECTONANOSECS_IN_SEC;
    
        //now remaining time fraction is in secs
        timeFraction = (timeFraction - hectoNanosecond) / HECTONANOSECS_IN_SEC;

        second = timeFraction % 60;
        //now remaining time fraction is in minutes
        timeFraction = (timeFraction - second) / 60;

        minute = timeFraction % 60;
    
        //now remaining time fraction is the hours
        hour = (uint8_t) ((timeFraction - minute) / 60);
        BeAssert (hour < 24);
        }

    dateTime = DateTime (targetInfo, year, month, day, hour, minute, second, hectoNanosecond);

    return SUCCESS;

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
BentleyStatus DateTimeConverter::ComputeLocalTimezoneOffsetFromLocalTime (int64_t& timezoneOffsetInHns, uint64_t rawJdInHns)
    {
    //rawJdInHns does not yet include time zone transformation.
    //In this algo we first treat it as if it was UTC based and compute an approximate offset from it.
    //Offset is only approximate as local date time might just be in the area of switching
    //to / from DST. In that case treating local time as UTC yields the wrong offset.
    //This algo computes stuff based on the wrong offset which finally yields the right offset.
    int64_t approximateOffset = 0LL;
    BentleyStatus stat = ComputeLocalTimezoneOffsetFromUtcTime (approximateOffset, rawJdInHns);
    if (SUCCESS != stat)
        return ERROR;

    //take JD and add approximate offset, so we are in approximate utc
    uint64_t approximateUtcJD = rawJdInHns + approximateOffset;

    int64_t correctOffset = 0LL;
    stat = ComputeLocalTimezoneOffsetFromUtcTime (correctOffset, approximateUtcJD);
    if (SUCCESS != stat)
        return ERROR;

    timezoneOffsetInHns = correctOffset;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTimeConverter::ComputeLocalTimezoneOffsetFromUtcTime (int64_t& timezoneOffsetInHns, uint64_t jdInHns)
    {
    if (!IsInUnixEpoch (jdInHns))
        {
        if (LOG.isSeverityEnabled (NativeLogging::LOG_DEBUG))
            {
            double jd = HnsToRationalDay (jdInHns);
            LOG.debugv (L"Date [Julian Day %lf] is outside Unix epoch. Using default date in Unix epoch (2000-01-01 00:00:00 UTC) for computing the local time zone offset.",
                            jd);
            }

        //use a default date time within the epoch (2000-1-1 midnight) outside DST.
        jdInHns = TIMEZONEOFFSET_DEFAULT_DATE_IN_JULIANDAY_HNS;
        }
    else
        {
        //strip the below second portion away as the conversion to local time ignores them anyways (as it uses a tm)
        //As time zone offsets are never more fine-grained than minutes, stripping the millisecs isn't a problem
        //(If outside the Unix epoch, this is not necessary, as the default date chosen doesn't have below-sec accuracy)
        jdInHns -= jdInHns % HECTONANOSECS_IN_SEC;
        }

    //assert that the below seconds components are gone
    BeAssert (jdInHns % HECTONANOSECS_IN_SEC == 0);

    int64_t utcUnixMillis = JulianDayToUnixMilliseconds (jdInHns);
    BeAssert (utcUnixMillis >= 0);

    uint64_t utcUnixMillisUnsigned = static_cast<uint64_t> (utcUnixMillis);
    struct tm localTime;
    BentleyStatus stat = BeTimeUtilities::ConvertUnixMillisToLocalTime (localTime, utcUnixMillisUnsigned);
    if (SUCCESS != stat)
        return stat;
    
    uint64_t localJdInHns = 0ULL;
    stat = ComputeJulianDay (localJdInHns, localTime);
    if (SUCCESS != stat)
        return stat;

    BeAssert (localJdInHns % HECTONANOSECS_IN_SEC == 0);

    //cast both operands to int64 before substracting them from each other to avoid implicit conversions
    timezoneOffsetInHns = static_cast<int64_t>(jdInHns) - static_cast<int64_t> (localJdInHns);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeConverter::IsInUnixEpoch (uint64_t jdInHns)
    {
    return jdInHns >= UNIXEPOCH_BASELINE_IN_JULIANDAY_HNS && jdInHns <= UNIXEPOCH_END_IN_JULIANDAY_HNS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTimeConverter::ToUnixMilliseconds (int64_t& unixMilliseconds, DateTimeCR dateTime)
    {
    uint64_t jdInHns = 0ULL;
    BentleyStatus stat = ToJulianDay (jdInHns, dateTime);
    if (SUCCESS != stat)
        return stat;

    unixMilliseconds = JulianDayToUnixMilliseconds (jdInHns);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus DateTimeConverter::FromUnixMilliseconds (DateTimeR dateTime, uint64_t unixMilliseconds, DateTime::Info const& targetInfo)
    {
    const uint64_t jd = UnixMillisecondsToJulianDay (unixMilliseconds);
    FromJulianDay (dateTime, jd, targetInfo);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
int64_t DateTimeConverter::JulianDayToUnixMilliseconds (uint64_t julianDayInHectoNanoseconds)
    {
    //cast to Int64 so that difference will not get stuffed in UInt64
    const int64_t unixHns = static_cast<int64_t> (julianDayInHectoNanoseconds) - UNIXEPOCH_BASELINE_IN_JULIANDAY_HNS;
    return RoundToMilliseconds (unixHns);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint64_t DateTimeConverter::UnixMillisecondsToJulianDay (uint64_t unixMilliseconds)
    {
    return unixMilliseconds * HECTONANOSECS_IN_MILLISEC + UNIXEPOCH_BASELINE_IN_JULIANDAY_HNS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTimeConverter::ToCommonEraTicks (int64_t& commonEraTicks, DateTimeCR dateTime)
    {
    uint64_t jdInHns = 0ULL;

    BentleyStatus stat = ToJulianDay (jdInHns, dateTime);
    if (SUCCESS != stat)
        return stat;

    commonEraTicks = JulianDayToCommonEraTicks (jdInHns);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTimeConverter::FromCommonEraTicks (DateTime& dateTime, int64_t commonEraTicks, DateTime::Info const& targetInfo)
    {
    const uint64_t jdInHns = CommonEraTicksToJulianDay (commonEraTicks);

    return FromJulianDay (dateTime, jdInHns, targetInfo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
int64_t DateTimeConverter::JulianDayToCommonEraTicks (uint64_t julianDayInHectoNanoseconds)
    {
    // commonEraTicks is number of ticks since 0001-01-01 00:00:00
    return julianDayInHectoNanoseconds - static_cast<int64_t> (CE_EPOCH_AS_JD_HNS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint64_t DateTimeConverter::CommonEraTicksToJulianDay (int64_t commonEraTicks)
    {
    // commonEraTicks is number of ticks since 0001-01-01 00:00:00
    const int64_t jdInHnsSigned = commonEraTicks + static_cast<int64_t> (CE_EPOCH_AS_JD_HNS);
    BeAssert (jdInHnsSigned >= 0);
    return static_cast<uint64_t> (jdInHnsSigned);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
double DateTimeConverter::HnsToRationalDay (uint64_t hns)
    {
    return hns / (1.0 * HECTONANOSECS_IN_DAY);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint64_t DateTimeConverter::RationalDayToHns (double rationalDay)
    {
    if (rationalDay <= 0.0)
        return 0ULL;

    //return rounded value
    return static_cast<uint64_t> (rationalDay * HECTONANOSECS_IN_DAY + 0.5);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
int64_t DateTimeConverter::RoundToMilliseconds (int64_t hns)
    {
    const double hnsInMillisecs = hns / (1.0 * HECTONANOSECS_IN_MILLISEC);
    double roundOffset = 0.5;
    if (hnsInMillisecs < 0)
        {
        roundOffset = -0.5;
        }

    return static_cast<int64_t> (hnsInMillisecs + roundOffset);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint16_t DateTimeConverter::ToMillisecond (uint32_t hectoNanosecond)
    {
    //only millisecond component is returned (not rounded)
    return static_cast <uint16_t> (hectoNanosecond / HECTONANOSECS_IN_MILLISEC);
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
//Characters in the ISO string have to be upper case, so not using case-insensitivity flag with regex
Iso8601Regex::Iso8601Regex () : m_regex (PATTERN) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Iso8601Regex::~Iso8601Regex () {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  05/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool Iso8601Regex::Match (Matches& matches, Utf8CP iso8601DateTime) const
    {
    return std::regex_search (iso8601DateTime, matches, m_regex);
    }

//************************ DateTimeStringConverter ********************************
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
const Utf8Char DateTimeStringConverter::ISO8601_TIMEZONE_UTC = 'Z';
const Utf8Char DateTimeStringConverter::ISO8601_TIMEZONE_UTC_LOWERCASE = 'z';

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String DateTimeStringConverter::ToIso8601 (DateTimeCR dateTime)
    {
    if (!dateTime.IsValid ())
        return Utf8String ();

    if (dateTime.GetInfo ().GetComponent () == DateTime::Component::Date)
        {
        Utf8CP isoFormat = "%.4d-%.2d-%.2d";
        size_t approxSize = 11;
        Utf8String str;
        str.reserve (approxSize);
        str.Sprintf (isoFormat, dateTime.GetYear (), dateTime.GetMonth (), dateTime.GetDay ());
        return str;
        }

    BeAssert (dateTime.GetInfo ().GetComponent () == DateTime::Component::DateAndTime);

    double effectiveSeconds = ToRationalSeconds (dateTime.GetSecond (), dateTime.GetHectoNanosecond ());
    //round to the milliseconds
    uint32_t millisecs = static_cast<uint32_t> (effectiveSeconds * 1000.0 + 0.5);
    effectiveSeconds = millisecs / 1000.0;

    uint8_t effectiveMinute = dateTime.GetMinute ();
    uint8_t effectiveHour = dateTime.GetHour ();
    uint8_t effectiveDay = dateTime.GetDay ();
    uint8_t effectiveMonth = dateTime.GetMonth ();
    int16_t effectiveYear = dateTime.GetYear ();
    if (millisecs >= 60000)
        {
        effectiveSeconds = 0.0;
        effectiveMinute++;
        }

    if (effectiveMinute >= 60)
        {
        effectiveMinute = 0;
        effectiveHour++;
        }

    if (effectiveHour >= 24)
        {
        effectiveHour = 0;
        effectiveDay++;
        }

    if (effectiveDay > DateTime::GetMaxDay (effectiveYear, effectiveMonth))
        {
        effectiveDay = 1;
        effectiveMonth++;
        }

    if (effectiveMonth > 12)
        {
        effectiveMonth = 1;
        effectiveYear++;
        }

    Utf8CP isoFormat = "%.4d-%.2d-%.2dT%.2d:%.2d:%06.3lf%hc";
    size_t approxSize = 23;
    
    Utf8Char timeZoneIndicator = '\0';
    //if date is in UTC suffix the string with Z according to ISO
    //for local and unspecified dates don't suffix anything (compliant with ISO, too)
    if (dateTime.GetKind () == DateTime::Kind::Utc)
        {
        timeZoneIndicator = ISO8601_TIMEZONE_UTC;
        approxSize = 24;
        }

    Utf8String str;
    str.reserve (approxSize);
    str.Sprintf (isoFormat, effectiveYear, effectiveMonth, effectiveDay,
        effectiveHour, effectiveMinute, effectiveSeconds, timeZoneIndicator);

    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DateTimeStringConverter::FromIso8601 (DateTimeR dateTime, Utf8CP iso8601DateTime)
    {
    if (Utf8String::IsNullOrEmpty(iso8601DateTime))
        return ERROR;

    Iso8601Regex::Matches matches;
    static const Iso8601Regex regex;
    const bool hasMatches = regex.Match (matches, iso8601DateTime);
    size_t matchCount = 0;
    if (!hasMatches || (matchCount = matches.size ()) == 0 || matchCount != Iso8601Regex::EXPECTED_MATCH_COUNT)
        return ERROR;

    //Parse date component
    BeAssert (RegexGroupMatched (matches, Iso8601Regex::YEAR_GROUPINDEX) && RegexGroupMatched (matches, Iso8601Regex::MONTH_GROUPINDEX) && RegexGroupMatched (matches, Iso8601Regex::DAY_GROUPINDEX) && "All three date components are mandatory");
    int intValue = 0;
    TryRetrieveValueFromRegexMatch (intValue, matches, Iso8601Regex::YEAR_GROUPINDEX);
    int16_t year = static_cast<int16_t> (intValue);

    uint8_t month = 0;
    TryRetrieveValueFromRegexMatch (month, matches, Iso8601Regex::MONTH_GROUPINDEX);

    uint8_t day = 0;
    TryRetrieveValueFromRegexMatch (day, matches, Iso8601Regex::DAY_GROUPINDEX);

    //Parse time component (if specified in input string)
    uint8_t hour = 0;
    if (!TryRetrieveValueFromRegexMatch (hour, matches, Iso8601Regex::HOUR_GROUPINDEX))
        {
        //no time component specified.
        BeAssert (!RegexGroupMatched (matches, Iso8601Regex::MINUTE_GROUPINDEX) && !RegexGroupMatched (matches, Iso8601Regex::SECOND_GROUPINDEX) && !RegexGroupMatched (matches, Iso8601Regex::SECONDFRACTION_GROUPINDEX) && !RegexGroupMatched (matches, Iso8601Regex::TIMEZONE_GROUPINDEX));
        dateTime = DateTime (year, month, day);
        return SUCCESS;
        }

    //if hour component is given, minute must be given too (all the rest is optional)
    BeAssert (RegexGroupMatched (matches, Iso8601Regex::MINUTE_GROUPINDEX));

    uint8_t minute = 0;
    TryRetrieveValueFromRegexMatch (minute, matches, Iso8601Regex::MINUTE_GROUPINDEX);

    uint8_t second = 0;
    uint32_t hns = 0;
    if (TryRetrieveValueFromRegexMatch (second, matches, Iso8601Regex::SECOND_GROUPINDEX))
        {
        double secondFraction = 0.0;
        if (TryRetrieveValueFromRegexMatch (secondFraction, matches, Iso8601Regex::SECONDFRACTION_GROUPINDEX))
            {
            hns = FractionToHectoNanosecond (secondFraction);
            }
        }
    else
        {
        //if seconds are not specified, second fraction must not exist either
        BeAssert (!RegexGroupMatched (matches, Iso8601Regex::SECONDFRACTION_GROUPINDEX));
        }

    DateTime::Kind kind = DateTime::Kind::Unspecified;
    Utf8String timezoneIndicator;
    if (TryRetrieveValueFromRegexMatch (timezoneIndicator, matches, Iso8601Regex::TIMEZONE_GROUPINDEX))
        {
        BeAssert (!timezoneIndicator.empty ());
        const Utf8Char firstChar = timezoneIndicator[0];
        if (firstChar == ISO8601_TIMEZONE_UTC || firstChar == ISO8601_TIMEZONE_UTC_LOWERCASE)
            kind = DateTime::Kind::Utc;
        else
            kind = DateTime::Kind::Local;
        }

    dateTime = DateTime (kind, year, month, day, hour, minute, second, hns);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeStringConverter::RegexGroupMatched (Iso8601Regex::Matches const& matches, size_t groupIndex)
    {
    return matches[groupIndex].length () > 0 && matches[groupIndex].matched;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeStringConverter::TryRetrieveValueFromRegexMatch (int& matchValue, Iso8601Regex::Matches const& matches, size_t groupIndex)
    {
    if (!RegexGroupMatched (matches, groupIndex))
        {
        return false;
        }

    matchValue = atoi (matches[groupIndex].str ().c_str ());
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeStringConverter::TryRetrieveValueFromRegexMatch (uint8_t& matchValue, Iso8601Regex::Matches const& matches, size_t groupIndex)
    {
    if (!RegexGroupMatched (matches, groupIndex))
        {
        return false;
        }

    int matchValueInt = atoi (matches[groupIndex].str ().c_str ());
    matchValue = static_cast<uint8_t> (matchValueInt);
    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeStringConverter::TryRetrieveValueFromRegexMatch (double& matchValue, Iso8601Regex::Matches const& matches, size_t groupIndex)
    {
    if (!RegexGroupMatched (matches, groupIndex))
        {
        return false;
        }

    matchValue = atof (matches[groupIndex].str ().c_str ());
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeStringConverter::TryRetrieveValueFromRegexMatch (Utf8StringR matchValue, Iso8601Regex::Matches const& matches, size_t groupIndex)
    {
    if (!RegexGroupMatched (matches, groupIndex))
        {
        return false;
        }

    matchValue.assign (matches[groupIndex].str ().c_str ());
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
double DateTimeStringConverter::ToRationalSeconds (uint8_t second, uint32_t hectoNanosecond)
    {
    return second + (hectoNanosecond / (1.0 * DateTimeConverter::HECTONANOSECS_IN_SEC));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
uint32_t DateTimeStringConverter::FractionToHectoNanosecond (double secondFraction)
    {
    BeAssert (secondFraction >= 0.0 && secondFraction < 1.0);
    return static_cast<uint32_t> (secondFraction * DateTimeConverter::HECTONANOSECS_IN_SEC + 0.5);
    }

END_BENTLEY_NAMESPACE
