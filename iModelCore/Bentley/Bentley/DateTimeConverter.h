/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <Bentley/DateTime.h>
#include <Bentley/WString.h>
#include <Bentley/NonCopyableClass.h>
#include <Bentley/bvector.h>
#include <mutex>
#include <regex>
#include <time.h>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================    
//! Utility to convert between DateTime and representations like Julian Day or Unix milliseconds
//! @bsiclass                                                   Krischan.Eberle   02/2013
//=======================================================================================    
struct DateTimeConverter : NonCopyableClass
    {
    private:
        //*** Consts and Fields ***
        static const uint32_t SECS_IN_DAY;
        static const uint32_t MSECS_IN_DAY;

        //! 2000-01-01 00:00:00 UTC
        //! @remarks Used for time zone offset computation if the actual date is outside the Unix epoch
        static const uint64_t TIMEZONEOFFSET_DEFAULT_DATE_IN_JULIANDAY_MSEC;

        //! 1970-01-01 00:00:00 UTC
        static const uint64_t UNIXEPOCH_BASELINE_IN_JULIANDAY_MSEC;
        //! 2038-01-19 03:14:07 UTC
        //! @remarks Actually, this is the end of the Unix epoch on 32bit machines only.
        //! On 64bit machines it is much later. As long as 32 bit is supported the 32 bit end
        //! is used.
        //! The value is relevant when computing the local time zone offset which is only
        //! possible for dates within the Unix epoch. For dates outside TIMEZONEOFFSET_DEFAULT_DATE_IN_JULIANDAY_MSEC
        //! is used.
        static const uint64_t UNIXEPOCH_END_IN_JULIANDAY_MSEC;

        //! Common Era epoch as Julian Day in millisecs.
        //! @remarks The Common Era epoch starts at 0001-01-01 00:00:00 UTC according
        //! to the proleptic Gregorian calendar.
        static const uint64_t CE_EPOCH_AS_JD_MSEC;

        //static class -> not instantiable
        DateTimeConverter();
        ~DateTimeConverter();

        //*** Julian Day ***
        static BentleyStatus ComputeJulianDay(uint64_t& julianDayInMsec, DateTimeCR dateTime);
        static BentleyStatus ComputeJulianDay(uint64_t& julianDayInMsec, const tm& dateTimeTm);
        static BentleyStatus ComputeJulianDay(uint64_t& julianDayInMsec, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond);

        static BentleyStatus ParseJulianDay(DateTimeR dateTime, uint64_t jdInMsec, DateTime::Info const& targetInfo);

        //*** Other conversions ***
        static bool IsInUnixEpoch(uint64_t julianDayInMsec) { return julianDayInMsec >= UNIXEPOCH_BASELINE_IN_JULIANDAY_MSEC && julianDayInMsec <= UNIXEPOCH_END_IN_JULIANDAY_MSEC; }

        //! Computes the offset in milliseconds at the specified date time compared to UTC.
        //!
        //! If the date is outside the Unix epoch, it will be shifted to a date
        //! within the Unix epoch as the underlying system functions usually only work within the Unix epoch.
        //! Daylight saving times are recognized (per the underlying system function)
        //!
        //! The offset is negative if the local time zone is east of UTC and positive
        //! if it is west of UTC (offset = UTC - local time)
        //! @param [out] timezoneOffsetInMsec resulting time zone offset in milliseconds (offset = UTC - local time).
        //! @param [in] jdInMsec Julian day number in milliseconds interpreted as local time
        //! @return SUCCESS if the computation was successful, ERROR otherwise.
        static BentleyStatus ComputeLocalTimezoneOffsetFromLocalTime(int64_t& localTimezoneOffsetInMsec, uint64_t rawJdInMsec);
        static BentleyStatus ComputeLocalTimezoneOffsetFromUtcTime(int64_t& localTimezoneOffsetInMsec, uint64_t utcJd);

    public:
        static BentleyStatus ConvertToJulianDay(uint64_t& julianDayInMsec, DateTimeCR dateTime, bool applyTimezoneOffset);
        static BentleyStatus ConvertFromJulianDay(DateTimeR dateTime, uint64_t jdInMsec, DateTime::Info const& targetInfo, bool applyTimezoneOffset);
        static int64_t JulianDayToUnixMilliseconds(uint64_t julianDayInMsec) { return ((int64_t) (julianDayInMsec)) - UNIXEPOCH_BASELINE_IN_JULIANDAY_MSEC; }
        static uint64_t UnixMillisecondsToJulianDay(uint64_t unixMilliseconds) { return unixMilliseconds + UNIXEPOCH_BASELINE_IN_JULIANDAY_MSEC; }

        // commonEraTicks is number of ticks since 0001-01-01 00:00:00
        static int64_t JulianDayToCommonEraMilliseconds(uint64_t julianDayInMsec) { return julianDayInMsec - (int64_t) CE_EPOCH_AS_JD_MSEC; }
        static uint64_t CommonEraMillisecondsToJulianDay(int64_t commonEraMsec) { return (uint64_t) (commonEraMsec + (int64_t) CE_EPOCH_AS_JD_MSEC); }
        static double MillisecsToRationalDay(uint64_t msecs) { return msecs / (1000.0 * SECS_IN_DAY); }
        static int64_t RationalDayToMillisecs(double rationalDay) { return (int64_t) (rationalDay * 1000 * SECS_IN_DAY + 0.5); }
    };

//=======================================================================================    
//! Utility to convert between DateTime and string representations
//! @bsiclass                                                   Krischan.Eberle   02/2013
//=======================================================================================    
struct DateTimeStringConverter final
    {
    private:
        static std::regex* s_dtRegex;
        static std::regex* s_todRegex;

        DateTimeStringConverter() = delete;
        ~DateTimeStringConverter() = delete;

        static bool TryRetrieveValueFromRegexMatch(int16_t& matchValue, std::match_results<Utf8CP> const& matches, size_t groupIndex);
        static bool TryRetrieveValueFromRegexMatch(uint8_t& matchValue, std::match_results<Utf8CP> const& matches, size_t groupIndex);
        static bool TryRetrieveValueFromRegexMatch(double& matchValue, std::match_results<Utf8CP> const& matches, size_t groupIndex);
        static bool TryRetrieveValueFromRegexMatch(Utf8String& matchValue, std::match_results<Utf8CP> const& matches, size_t groupIndex);
        static bool RegexGroupMatched(std::match_results<Utf8CP> const& matches, size_t groupIndex);

        static BentleyStatus FromIso8601TimeOfDay(DateTime& dateTime, Utf8CP iso8601TimeOfDay);

        static std::regex const& GetDateTimeRegex();
        static std::regex const& GetTimeOfDayRegex();

    public:
        static Utf8String ToIso8601(DateTimeCR dateTime);
        static BentleyStatus FromIso8601(DateTime& dateTime, Utf8CP iso8601DateTime);
    };

END_BENTLEY_NAMESPACE
