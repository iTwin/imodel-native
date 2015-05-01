/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/DateTimeConverter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <Bentley/DateTime.h>
#include <Bentley/WString.h>
#include <Bentley/NonCopyableClass.h>
#include <regex>
#include <time.h>
#include <Bentley/bvector.h>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================    
//! Utility to convert between DateTime and representations like Julian Day or Unix milliseconds
//! @bsiclass                                                   Krischan.Eberle   02/2013
//=======================================================================================    
struct DateTimeConverter : NonCopyableClass
    {
private:
    //*** Consts and Fields ***
    static const uint16_t HECTONANOSECS_IN_MILLISEC;
    static const uint64_t HECTONANOSECS_IN_DAY;
    static const uint32_t SECS_IN_DAY;

    //! 2000-01-01 00:00:00 UTC
    //! @remarks Used for time zone offset computation if the actual date is outside the Unix epoch
    static const uint64_t TIMEZONEOFFSET_DEFAULT_DATE_IN_JULIANDAY_HNS;

    //! 1970-01-01 00:00:00 UTC
    static const uint64_t UNIXEPOCH_BASELINE_IN_JULIANDAY_HNS;
    //! 2038-01-19 03:14:07 UTC
    //! @remarks Actually, this is the end of the Unix epoch on 32bit machines only.
    //! On 64bit machines it is much later. As long as 32 bit is supported the 32 bit end
    //! is used.
    //! The value is relevant when computing the local time zone offset which is only
    //! possible for dates within the Unix epoch. For dates outside TIMEZONEOFFSET_DEFAULT_DATE_IN_JULIANDAY_HNS
    //! is used.
    static const uint64_t UNIXEPOCH_END_IN_JULIANDAY_HNS;

    //! Common Era epoch as Julian Day in hecto-nanoseconds.
    //! @remarks The Common Era epoch starts at 0001-01-01 00:00:00 UTC according
    //! to the proleptic Gregorian calendar.
    static const uint64_t CE_EPOCH_AS_JD_HNS;

    //static class -> not instantiable
    DateTimeConverter ();
    ~DateTimeConverter ();

    //*** Julian Day ***
    static BentleyStatus ComputeJulianDay (uint64_t& julianDayInHns, DateTimeCR dateTime);
    static BentleyStatus ComputeJulianDay (uint64_t& julianDayInHns, const tm& dateTimeTm);
    static BentleyStatus ComputeJulianDay (uint64_t& julianDayInHns, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint32_t hectoNanosecond);

    static BentleyStatus ParseJulianDay (DateTimeR dateTime, uint64_t jdInHns, DateTime::Info const& targetInfo);

    //*** Other conversions ***
    static bool IsInUnixEpoch (uint64_t julianDayInHns);

    //! Computes the offset in hecto-nanoseconds at the specified date time compared to UTC.
    //!
    //! If the date is outside the Unix epoch, it will be shifted to a date
    //! within the Unix epoch as the underlying system functions usually only work within the Unix epoch.
    //! Daylight saving times are recognized (per the underlying system function)
    //!
    //! The offset is negative if the local time zone is east of UTC and positive
    //! if it is west of UTC (offset = UTC - local time)
    //! @param [out] timezoneOffsetInHns resulting time zone offset in hecto-nanoseconds (offset = UTC - local time).
    //! @param [in] jdInHns Julian day number in hecto-nanoseconds interpreted as local time
    //! @return SUCCESS if the computation was successful, ERROR otherwise.
    static BentleyStatus ComputeLocalTimezoneOffsetFromLocalTime (int64_t& localTimezoneOffsetInHns, uint64_t rawJdInHns);
    static BentleyStatus ComputeLocalTimezoneOffsetFromUtcTime (int64_t& localTimezoneOffsetInHns, uint64_t utcJd);

    static int64_t RoundToMilliseconds (int64_t hns);

public:
    //! Number of hecto-nanoseconds (i.e. 100 nanoseconds) in a second (1E+07)
    static const uint32_t HECTONANOSECS_IN_SEC;

    //! @name Julian Day conversion
    //! @{

    //! Computes the \ref JulianDay number in hecto-nanoseconds from this DateTime instance.
    //! @remarks This DateTime must be on the proleptic Gregorian calendar.
    //! @param  [out] julianDayInHns Julian Day number in hecto-nanoseconds
    //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g.
    //!          if computation of local time zone offset failed.
    static BentleyStatus ToJulianDay (uint64_t& julianDayInHns, DateTimeCR dateTime);

    //! Computes the \ref JulianDay number from this DateTime instance.
    //! @remarks This DateTime must be on the proleptic Gregorian calendar.
    //! @param  [out] julianDay Julian Day number with the time component being the fractional component of the double value.
    //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g.
    //!          if computation of local time zone offset failed.
    static BentleyStatus ToJulianDay (double& julianDay, DateTimeCR dateTime);

    //! Computes the DateTime from the given \ref JulianDay number.
    //! @param  [out] dateTime Resulting DateTime object (on the proleptic Gregorian calendar)
    //! @param  [in] julianDayInHns Julian Day number in hecto-nanoseconds to convert to DateTime
    //! @param  [in] targetInfo DateTime::Info the resulting DateTime should have
    //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g.
    //!          if computation of local time zone offset failed.
    static BentleyStatus FromJulianDay (DateTimeR dateTime, uint64_t julianDayInHns, DateTime::Info const& targetInfo);

    //! Computes the DateTime from the given \ref JulianDay number.
    //! @param  [out] dateTime Resulting DateTime object (on the proleptic Gregorian calendar)
    //! @param  [in] julianDay Julian Day number with the time component being the fractional component of the double value.
    //! @param  [in] targetInfo DateTime::Info the resulting DateTime should have
    //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g.
    //!          if computation of local time zone offset failed.
    static BentleyStatus FromJulianDay (DateTimeR dateTime, double julianDay, DateTime::Info const& targetInfo);

    static BentleyStatus ConvertToJulianDay (uint64_t& julianDayInHns, DateTimeCR dateTime, bool applyTimezoneOffset);
    static BentleyStatus ConvertFromJulianDay (DateTimeR dateTime, uint64_t jdInHns, DateTime::Info const& targetInfo, bool applyTimezoneOffset);

    /// @}

    /// @name Conversions
    /// @{

    //! Converts this DateTime to Unix epoch milliseconds (UTC).
    //!
    //! This DateTime must be on the proleptic Gregorian calendar.
    //!
    //! The resulting Unix milliseconds are UTC based. If this DateTime object is in local time, the method internally
    //! accounts for converting to UTC first.
    //! @param [out] unixMilliseconds Unix epoch milliseconds. The value is negative for dates before the beginning
    //!         of the Unix epoch (1970-01-01 00:00:00 UTC)
    //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g.
    //!          if computation of local time zone offset failed.
    static BentleyStatus ToUnixMilliseconds (int64_t& unixMilliseconds, DateTimeCR dateTime);

    //! Creates a DateTime from the given Unix epoch milliseconds
    //! @param [out] dateTime The resulting DateTime object on the proleptic Gregorian calendar.
    //! @param [in] unixMilliseconds The Unix epoch milliseconds. Negative if they refer to a date before
    //!             the Unix epoch (1970-01-01 00:00:00 UTC)
    //! @return SUCCESS if conversion was successful. ERROR otherwise
    static BentleyStatus FromUnixMilliseconds (DateTimeR dateTime, uint64_t unixMilliseconds, DateTime::Info const& targetInfo);

    //! Computes the Unix milliseconds from the given \ref JulianDay number.
    //! @param  [in] julianDayInHectoNanoseconds Julian day in hecto-nanoseconds.unixMilliseconds Unix milliseconds.
    //! @return Unix milliseconds. Negative numbers indicate dates before the Unix epoch.
    static int64_t JulianDayToUnixMilliseconds (uint64_t julianDayInHectoNanoseconds);

    //! Computes the \ref JulianDay number from the given Unix milliseconds.
    //! @param  [in] unixMilliseconds Unix milliseconds. Negative numbers indicate dates before the Unix epoch.
    //! @return Julian Day number in hecto-nanoseconds
    static uint64_t UnixMillisecondsToJulianDay (uint64_t unixMilliseconds);

    //! Computes the Common Era ticks from this DateTime instance.
    //! This DateTime must be on the proleptic Gregorian calendar.
    //! The Common Era begins at 0001-01-01 00:00:00 UTC.
    //! @param  [out] commonEraTicks Common Era ticks in hecto-nanoseconds
    //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g.
    //!          if computation of local time zone offset failed.
    static BentleyStatus ToCommonEraTicks (int64_t& commonEraTicks, DateTimeCR dateTime);

    //! Computes the DateTime from the given Common Era ticks.
    //! The Common Era begins at 0001-01-01 00:00:00 UTC.
    //! @param  [out] dateTime Resulting DateTime object (on the proleptic Gregorian calendar)
    //! @param  [in] commonEraTicks Common Era ticks in hecto-nanoseconds
    //! @param  [in] targetInfo DateTime::Info the resulting DateTime should have
    //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g.
    //!          if computation of local time zone offset failed.
    static BentleyStatus FromCommonEraTicks (DateTimeR dateTime, int64_t commonEraTicks, DateTime::Info const& targetInfo);

    //! Computes Common Era ticks from the given the \ref JulianDay number.
    //! The Common Era begins at 0001-01-01 00:00:00 UTC.
    //! @param  [in] julianDayInHectoNanoseconds JulianDay number 
    //! @return Common Era ticks in hecto-nanoseconds
    static int64_t JulianDayToCommonEraTicks (uint64_t julianDayInHectoNanoseconds);

    //! Computes the \ref JulianDay number from the given Common Era ticks.
    //! The Common Era begins at 0001-01-01 00:00:00 UTC.
    //! @param  [in] commonEraTicks Common Era ticks in hecto-nanoseconds
    //! @return Julian Day number in hecto-nanoseconds
    static uint64_t CommonEraTicksToJulianDay (int64_t commonEraTicks);

    //! Converts a number in hecto-nanoseconds into a rational day number
    //! where the integral component represents the day numbers and the fractional component
    //! represents the time in the day.
    //! @param [in] hns The number in hecto-nanoseconds
    //! @return the rational day number
    static double HnsToRationalDay (uint64_t hns);

    //! Converts a rational day number to hecto-nanoseconds.
    //! @param [in] rationalDay The rational day number where the integral component 
    //! represents the day numbers and the fractional component represents the time in the day
    //! @return The rational day number in hecto-nanoseconds
    static uint64_t RationalDayToHns (double rationalDay);
    /// @}

    static uint16_t ToMillisecond (uint32_t hectoNanosecond);
    };


//=======================================================================================    
//! Regex that parses an ISO 8601 Date Time string
//! @remarks This encapsulates the non-portable regex code
//! @bsiclass                                                   Krischan.Eberle   05/2013
//=======================================================================================    
struct Iso8601Regex : NonCopyableClass
    {
public:
    typedef std::match_results<Utf8CP> Matches;

    static const size_t YEAR_GROUPINDEX = 1;
    static const size_t MONTH_GROUPINDEX = 2;
    static const size_t DAY_GROUPINDEX = 3;
    static const size_t HOUR_GROUPINDEX = 4;
    static const size_t MINUTE_GROUPINDEX = 5;
    static const size_t SECOND_GROUPINDEX = 6;
    static const size_t SECONDFRACTION_GROUPINDEX = 7;
    static const size_t TIMEZONE_GROUPINDEX = 8;

    static const size_t EXPECTED_MATCH_COUNT = 9;

private:
    static Utf8CP const PATTERN;

    std::regex m_regex;

public:
    Iso8601Regex ();
    ~Iso8601Regex ();

    //! Applies the ISO 8601 Regex against the specified date time string (@p iso8601DateTime).
    //! @remarks This method supports T and a space as delimiter of the date and time component, e.g.
    //! both <c>2013-09-15T12:05:39</> and <c>2013-09-15 12:05:39</> can be parsed correctly.
    //! This is a minor deviation from the ISO standard (specifies the T delimiter), but allows to parse 
    //! SQL-99 date time literals (specifies the space delimiter)
    //! @param[out] matches Resulting matches
    //! @param[in] iso8601DateTime ISO 8601 date time string to parse
    //! @return true if there are matches. false, if no matches were found
    bool Match (Matches& matches, Utf8CP iso8601DateTime) const;
    };


//=======================================================================================    
//! Utility to convert between DateTime and string representations
//! @bsiclass                                                   Krischan.Eberle   02/2013
//=======================================================================================    
struct DateTimeStringConverter : NonCopyableClass
    {
private:
    static const Utf8Char ISO8601_TIMEZONE_UTC;
    static const Utf8Char ISO8601_TIMEZONE_UTC_LOWERCASE;

    //static class -> not instantiable
    DateTimeStringConverter ();
    ~DateTimeStringConverter ();

    static bool TryRetrieveValueFromRegexMatch (int& matchValue, Iso8601Regex::Matches const& matches, size_t groupIndex);
    static bool TryRetrieveValueFromRegexMatch (uint8_t& matchValue, Iso8601Regex::Matches const& matches, size_t groupIndex);
    static bool TryRetrieveValueFromRegexMatch (double& matchValue, Iso8601Regex::Matches const& matches, size_t groupIndex);
    static bool TryRetrieveValueFromRegexMatch (Utf8String& matchValue, Iso8601Regex::Matches const& matches, size_t groupIndex);
    static bool RegexGroupMatched (Iso8601Regex::Matches const& matches, size_t groupIndex);

    //! @return Seconds with fractional component 
    static double ToRationalSeconds (uint8_t second, uint32_t hectoNanosecond);
    static uint32_t FractionToHectoNanosecond (double secondFraction);

public:
    static Utf8String ToIso8601 (DateTimeCR dateTime);
    static BentleyStatus FromIso8601 (DateTime& dateTime, Utf8CP iso8601DateTime);
    };

END_BENTLEY_NAMESPACE
