/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/DateTime.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "WString.h"

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================    
//! Represents an instant in time, typically expressed as a date and time of day
//! 
//! A DateTime also holds additional metadata about the actual date time in its DateTime::Info member.
//!
//! This class can be used for dates (without time component), timestamps / date times, and
//! times (without date component), also known as time of day. DateTime::Component indicates what kind of DateTime the object represents.
//! @ingroup GROUP_Time
//! @nosubgrouping
//=======================================================================================    
struct DateTime
    {
    public:
        //=======================================================================================    
        //! Specifies whether a DateTime represents a local time, a Coordinated Universal Time (UTC), 
        //! or is not specified as either local time or UTC.
        // @bsiclass                                                 Krischan.Eberle      10/2012
        //=======================================================================================    
        enum class Kind
            {
            Unspecified = 0,//!< The time represented is not specified as either local time or Coordinated Universal Time (UTC)
            Utc = 1,//!< The time represented is UTC.
            Local = 2 //!<The time represented is local time (in machine's time zone).
            };

        //=======================================================================================    
        //! Specifies whether a DateTime represents a date (without time component), a time
        //! (without date component) or a date time
        // @bsiclass                                                 Krischan.Eberle      02/2013
        //=======================================================================================    
        enum class Component
            {
            Date = 0, //!< The DateTime represents a date only (without time component)
            DateAndTime = 1,//!< The DateTime represents a date and a time
            TimeOfDay = 2, //!< The DateTime represents a time only (without date component)
            };

        //=======================================================================================    
        //! Specifies the days of the week.
        // @bsiclass                                                 Krischan.Eberle      02/2014
        //=======================================================================================    
        enum class DayOfWeek
            {
            Sunday = 0, //!<Sunday
            Monday = 1, //!<Monday
            Tuesday = 2, //!<Tuesday
            Wednesday = 3, //!<Wednesday
            Thursday = 4, //!<Thursday
            Friday = 5, //!<Friday
            Saturday = 6 //!<Saturday
            };

        //=======================================================================================    
        //! Result of comparing two DateTimes
        //! @see DateTime::Compare
        // @bsiclass                                                 Krischan.Eberle      06/2014
        //=======================================================================================    
        enum class CompareResult
            {
            EarlierThan, //!<Left hand side operand is earlier than right hand side
            Equals, //!<Left hand side operand is same as right hand side
            LaterThan, //!<Left hand side operand is later than right hand side
            Error //!< Comparison failed
            };

        //=======================================================================================    
        //! Provides the metadata portion of a DateTime object.
        // @bsiclass                                                 Krischan.Eberle      02/2013
        //=======================================================================================    
        struct Info
            {
            private:
                Kind m_kind = Kind::Unspecified;
                Component m_component = Component::DateAndTime;
                bool m_isValid = false;

                Info(Kind kind, Component component) : m_isValid(true), m_kind(kind), m_component(component) {}

            public:
                //! Creates an uninitialized Info instance
                Info() {}

                //! Creates an Info object for a DateTime
                //! @param[in] kind DateTime kind
                static Info CreateForDateTime(Kind kind) { return Info(kind, Component::DateAndTime); }
                //! Creates an Info object for a Date
                static Info CreateForDate() { return Info(Kind::Unspecified, Component::Date); }
                //! Creates an Info object for a TimeOfDay
                static Info CreateForTimeOfDay() { return Info(Kind::Unspecified, Component::TimeOfDay); }

                //! Compares the given Info object with this Info object.
                //! @param [in] rhs Info object to compare with this Info object
                //! @return true if the two Info objects are equal, false otherwise
                bool operator==(Info const& rhs) const { return m_isValid == rhs.m_isValid && m_kind == rhs.m_kind && m_component == rhs.m_component; }

                //! Compares the given Info object with this Info object for inequality.
                //! @param [in] rhs Info object to compare with this Info object
                //! @return true if the two Info objects are not equal, false otherwise
                bool operator!=(Info const& rhs) const { return !(*this == rhs); }

                bool IsValid() const { return m_isValid; }

                //! Gets the DateTime kind.
                //! @return DateTime kind.
                Kind GetKind() const { return m_kind; }

                //! Gets the DateTime component.
                //! @return DateTime component.
                Component GetComponent() const { return m_component; }
            };

    private:
        Info m_info;
        int16_t m_year = 0;
        uint8_t m_month = 0;
        uint8_t m_day = 0;
        uint8_t m_hour = 0;
        uint8_t m_minute = 0;
        uint8_t m_second = 0;
        uint16_t m_millisecond = 0;

        // a time of day internally is a full-blown DateTime object, with the date
        // component set to a default date (2000-01-01), which is ignored for all TimeOfDay related matters.
        static constexpr uint16_t s_timeOfDayDummyYear = 2000;
        static constexpr uint8_t s_timeOfDayDummyMonth= 1;
        static constexpr uint8_t s_timeOfDayDummyDay = 1;

        //! Initializes a new instance of the DateTime struct.
        //! @param [in] info the DateTime::Info for this DateTime
        //! @param [in] year The year
        //! @param [in] month The month (1 through 12) 
        //! @param [in] day The day in the month (1 through the number of days in @p month) 
        //! @param [in] hour The hours (0 through 23, or 24 as 24:00 if end of day)
        //! @param [in] minute The minutes (0 through 59)
        //! @param [in] second The seconds (0 through 59)
        //! @param [in] millisecond The milliseconds (0 through 999)
        DateTime(Info const& info, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond)
            : m_info(info), m_year(year), m_month(month), m_day(day), m_hour(hour), m_minute(minute), m_second(second), m_millisecond(millisecond)
            {
            if (!IsValidDateTime(info, year, month, day, hour, minute, second, millisecond))
                {
                BeAssert(false && "Invalid parameters to DateTime ctor");
                m_info = Info(); //make DateTime object invalid
                }
            }

        static bool IsValidDateTime(Info const& info, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond)
            {
            if (!info.IsValid() || month < 1 || month > 12 || day < 1 || day > GetMaxDay(year, month) ||
                hour > 24 || minute > 59 || second > 59 || millisecond > 999)
                return false;

            return hour < 24 || (minute == 0 && second == 0 && millisecond == 0);
            }

    public:
        //! Initializes a new empty instance of the DateTime struct.
        //! @remarks DateTime::IsValid returns false in that case, as it doesn't represent any date time.
        DateTime() {}

        //! Initializes a new instance of the DateTime struct.
        //! @remarks This creates a "date-only" DateTime, i.e.
        //!     the DateTime::Component of this DateTime amounts to DateTime::Component::Date.
        //!     DateTime::Kind is irrelevant for "date-only" DateTime;
        //! @param [in] year The year (-4713 through 9999) 
        //! @param [in] month The month (1 through 12) 
        //! @param [in] day The day in the month (1 through the number in @p month) 
        DateTime(int16_t year, uint8_t month, uint8_t day) : DateTime(Info::CreateForDate(), year, month, day, 0, 0, 0, 0) {}

        //! Initializes a new instance of the DateTime struct.
        //! @param [in] kind the Kind for this DateTime
        //! @param [in] year The year
        //! @param [in] month The month (1 through 12) 
        //! @param [in] day The day in the month (1 through the number of days in @p month) 
        //! @param [in] hour The hours (0 through 23 or 24 as 24:00 if end of day)
        //! @param [in] minute The minutes (0 through 59)
        //! @param [in] second The seconds (0 through 59)
        //! @param [in] millisecond The milliseconds (0 through 999)
        DateTime(Kind kind, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second = 0, uint16_t millisecond = 0)
            : DateTime(Info::CreateForDateTime(kind), year, month, day, hour, minute, second, millisecond) {}

        //! Initializes a new "time of day" DateTime
        //! @param [in] hour The hours (0 through 23 or 24 as 24:00 if end of day)
        //! @param [in] minute The minutes (0 through 59)
        //! @param [in] second The seconds (0 through 59)
        //! @param [in] millisecond The milliseconds (0 through 999)
        //! @return "time of day" DateTime object
        static DateTime CreateTimeOfDay(uint8_t hour, uint8_t minute, uint8_t second = 0, uint16_t millisecond = 0) { return DateTime(Info::CreateForTimeOfDay(), s_timeOfDayDummyYear, s_timeOfDayDummyMonth, s_timeOfDayDummyDay, hour, minute, second, millisecond); }

        //! Indicates whether this DateTime instance is a valid date/time or not.
        //! @remarks Using the default constructor creates an invalid date time as none of the components have been set.
        //! @return true, if the DateTime instance is valid, false otherwise
        bool IsValid() const { return m_info.IsValid(); }

        //! Indicates whether this DateTime instance represents a "time of day", i.e. a time without date component.
        //! @return true, if this DateTime is a "time of day", false otherwise.
        bool IsTimeOfDay() const { return IsValid() && m_info.GetComponent() == Component::TimeOfDay; }

        //! Compares the given DateTime with this DateTime.
        //! @param [in] rhs DateTime to compare with this DateTime
        //! @return true if the two DateTimes are equal, false otherwise
        bool operator==(DateTime const& rhs) const { return Equals(rhs, false); }

        //! Checks the given DateTime with this DateTime for inequality.
        //! @param [in] rhs DateTime to compare with this DateTime
        //! @return true if the two DateTimes are not equal, false if they are equal
        bool operator!=(DateTime const& rhs) const { return !(*this == rhs); }

        //! Compares the given DateTime with this DateTime.
        //! @param [in] rhs DateTime to compare with this DateTime
        //! @param [in] ignoreDateTimeInfo true if the date time info should be ignored when comparing. false otherwise.
        //! @return true, if the two DateTimes are equal, false otherwise
        BENTLEYDLL_EXPORT bool Equals(DateTime const& rhs, bool ignoreDateTimeInfo = false) const;

        //! Compares two DateTimes.
        //! This method internally converts the DateTime objects to Julian Days and compares
        //! those. As the conversion can fail, this method can return an error (see DateTime::CompareResult).
        //! This is also the reason why the class does not provide comparison operators other than equality
        //! and inequality.
        //! @param [in] lhs Left-hand side DateTime
        //! @param [in] rhs Right-hand side DateTime
        //! @return A CompareResult value.
        BENTLEYDLL_EXPORT static CompareResult Compare(DateTime const& lhs, DateTime const& rhs);

        /// @name DateTime components
        /// @{

        //! Gets meta data about this DateTime object.
        //! @return DateTime::Info of this DateTime object.
        Info const& GetInfo() const { BeAssert(IsValid()); return m_info; }

        //! Gets the year component of this DateTime object (not applicable for DateTime::Component::TimeOfDay).
        //! @return Year (negative if BCE, positive otherwise).
        int16_t GetYear() const { BeAssert(IsValid()); return m_year; }

        //! Gets the month component of this DateTime object (not applicable for DateTime::Component::TimeOfDay).
        //! @return Month (1 through 12).
        uint8_t GetMonth() const { BeAssert(IsValid()); return m_month; }

        //! Gets the day component of this DateTime object (not applicable for DateTime::Component::TimeOfDay).
        //! @return Day in the month (1 through the number in GetMonth) 
        uint8_t GetDay() const { BeAssert(IsValid()); return m_day; }

        //! Gets the hour component of this DateTime object (not applicable for DateTime::Component::Date).
        //! @return Hours (0 through 59) 
        uint8_t GetHour() const { BeAssert(IsValid()); return m_hour; }

        //! Gets the minute component of this DateTime object (not applicable for DateTime::Component::Date).
        //! @return Minutes (0 through 59) 
        uint8_t GetMinute() const { BeAssert(IsValid()); return m_minute; }

        //! Gets the second component of this DateTime object (not applicable for DateTime::Component::Date).
        //! @return Seconds (0 through 59) 
        uint8_t GetSecond() const { BeAssert(IsValid()); return m_second; }

        //! Gets the millisecond component of this DateTime object.
        //! @return Milliseconds (0 through 999) 
        uint16_t GetMillisecond() const { BeAssert(IsValid()); return m_millisecond; }

        //! Gets the day of the week of this DateTime object (not applicable for DateTime::Component::TimeOfDay).
        //! @remarks Only call this method if the DateTime is valid (see DateTime::IsValid)
        //! @return Day of the week
        BENTLEYDLL_EXPORT DayOfWeek GetDayOfWeek() const;

        //! Gets the day of the year of this DateTime object (not applicable for DateTime::Component::TimeOfDay).
        //! @return Day of the year or 0 if the DateTime object is not valid (see DateTime::IsValid)
        BENTLEYDLL_EXPORT uint16_t GetDayOfYear() const;

        //! Extracts the time of day from this DateTime object (not applicable for DateTime::Component::Date)
        //! @return TimeOfDay or an invalid DateTime object, if this DateTime is invalid or has
        //! DateTime::Component::Date
        DateTime GetTimeOfDay() const { return (IsValid() && m_info.GetComponent() != Component::Date) ? CreateTimeOfDay(m_hour, m_minute, m_second, m_millisecond) : DateTime(); }

        //! Indicates whether the specified year is a leap year or not
        //! @return true, if @p year is a leap year. false, otherwise.
        static bool IsLeapYear(uint16_t year) { return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)); }

        /// @}
        //! @name Current time
        //! @{

        //! Gets the current system time in local time.
        //! @return Current system time in local time
        BENTLEYDLL_EXPORT static DateTime GetCurrentTime();

        //! Gets the current system time in UTC.
        //! @return Current system time in UTC
        BENTLEYDLL_EXPORT static DateTime GetCurrentTimeUtc();

        //! @}

        //! @name Julian Day Conversion
        //! @{
        //! The Julian Day is the count of days and fractions since @b noon on November 24, 4714 BCE UTC proleptic Gregorian
        //! calendar (-4713-11-24 12:00:00 UTC). See <a href="http://en.wikipedia.org/wiki/Julian_day" target="_blank">Julian Day</a>
        //! for details.
        //! If the date to convert to Julian Day is in local time, the DateTime is converted from local time to UTC.
        //! The time zone conversion works fine on most of the platforms within the Unix epoch only
        //! (1970-01-01 00:00:00 UTC through 2038-01-19 03:14:07 UTC). For dates outside this epoch
        //! the offset computation is performed by using a default date within the epoch (2000-01-01 00:00:00 UTC).
        //! In some cases this can lead to wrong offset computations and consequently to wrong Julian Day numbers.
        //! Clients can avoid this by using UTC date times for dates outside this epoch.

        //! Computes the Julian Day number in milliseconds from this DateTime instance.
        //! @remarks This DateTime must be on the proleptic Gregorian calendar.
        //! @param  [out] julianDay Julian Day number in milliseconds
        //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g. if date time instance is not valid (see DateTime::IsValid)
        //!         or if the date is before the beginning of the proleptic Gregorian calendar (-4713-11-24 12:00:00 UTC)
        //!         or if computation of local time zone offset failed.
        BENTLEYDLL_EXPORT BentleyStatus ToJulianDay(uint64_t& julianDay) const;

        //! Computes the Julian Day number from this DateTime instance.
        //! @remarks This DateTime must be on the proleptic Gregorian calendar.
        //! @param  [out] julianDay Julian Day number with the time component being the fractional component of the double value.
        //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g. if date time instance is not valid (see DateTime::IsValid)
        //!         or if the date is before the beginning of the proleptic Gregorian calendar (-4713-11-24 12:00:00 UTC)
        //!         or if computation of local time zone offset failed.
        BENTLEYDLL_EXPORT BentleyStatus ToJulianDay(double& julianDay) const;

        //! Computes the DateTime from the given Julian Day number.
        //! @param  [out] dateTime Resulting DateTime object (on the proleptic Gregorian calendar)
        //! @param  [in] julianDayMsec Julian Day number in milliseconds to convert to DateTime
        //! @param  [in] targetInfo DateTime::Info the resulting DateTime should have
        //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g.
        //!          if computation of local time zone offset failed.
        BENTLEYDLL_EXPORT static BentleyStatus FromJulianDay(DateTime& dateTime, uint64_t julianDayMsec, Info const& targetInfo);

        //! Computes the DateTime from the given Julian Day number.
        //! @param  [out] dateTime Resulting DateTime object (on the proleptic Gregorian calendar)
        //! @param  [in] julianDay Julian Day number with the time component being the fractional component of the double value.
        //! @param  [in] targetInfo DateTime::Info the resulting DateTime should have
        //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g.
        //!          if computation of local time zone offset failed.
        BENTLEYDLL_EXPORT static BentleyStatus FromJulianDay(DateTime& dateTime, double julianDay, Info const& targetInfo);

        /// @}

        /// @name Other Conversions
        /// @{

        //! This DateTime must be on the proleptic Gregorian calendar.
        //!
        //! The resulting Unix milliseconds are UTC based. If this DateTime object is in local time, the method internally
        //! accounts for converting to UTC first.
        //! @param [out] unixMilliseconds Unix epoch milliseconds. The value is negative for dates before the beginning
        //!         of the Unix epoch (1970-01-01 00:00:00 UTC)
        //! @return SUCCESS if successful. ERROR in case of errors, e.g.
        //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g. if date time instance is not valid (see DateTime::IsValid)
        //!         or if the date is before the beginning of the proleptic Gregorian calendar (-4713-11-24 12:00:00 UTC)
        //!         or if computation of local time zone offset failed.
        BENTLEYDLL_EXPORT BentleyStatus ToUnixMilliseconds(int64_t& unixMilliseconds) const;

        //! Creates a DateTime in UTC from the given Unix epoch milliseconds
        //! @param [out] dateTime The resulting DateTime object on the proleptic Gregorian calendar (in UTC).
        //! @param [in] unixMilliseconds The Unix epoch milliseconds. Negative if they refer to a date before
        //!             the Unix epoch (1970-01-01 00:00:00 UTC)
        //! @return SUCCESS if conversion was successful. ERROR otherwise
        BENTLEYDLL_EXPORT static BentleyStatus FromUnixMilliseconds(DateTime& dateTime, uint64_t unixMilliseconds);

        //! Computes the Unix milliseconds from the given Julian Day number.
        //! @param  [in] julianDayMsec Julian day in milliseconds
        //! @return Unix milliseconds. Negative numbers indicate dates before the Unix epoch (1970-01-01 00:00:00 UTC).
        BENTLEYDLL_EXPORT static int64_t JulianDayToUnixMilliseconds(uint64_t julianDayMsec);

        //! Computes the Julian Day number from the given Unix milliseconds.
        //! @param [in] unixMilliseconds The Unix epoch milliseconds. Negative if they refer to a date before
        //!             the Unix epoch (1970-01-01 00:00:00 UTC)
        //! @return Julian Day number in milliseconds
        BENTLEYDLL_EXPORT static uint64_t UnixMillisecondsToJulianDay(uint64_t unixMilliseconds);

        //! Computes Common Era milliseconds from the given the Julian Day number.
        //! The Common Era begins at 0001-01-01 00:00:00 UTC.
        //! @param  [in] julianDayMsec Julian Day 
        //! @return Time in milliseconds since beginning of Common Era
        BENTLEYDLL_EXPORT static int64_t JulianDayToCommonEraMilliseconds(uint64_t julianDayMsec);

        //! Computes the Julian Day number from the given Common Era milliseconds.
        //! The Common Era begins at 0001-01-01 00:00:00 UTC.
        //! @param  [in] commonEraMsec Common Era milliseconds
        //! @return Julian Day number in milliseconds
        BENTLEYDLL_EXPORT static uint64_t CommonEraMillisecondsToJulianDay(int64_t commonEraMsec);

        //! Converts this local DateTime to UTC
        //! @param  [out] utcDateTime Resulting DateTime object in UTC 
        //! @return SUCCESS if successful. ERROR in case of errors, e.g.
        //!         if this date time instance is invalid (see DateTime::IsValid) or
        //!         does not have DateTime::Component::DateTime
        //!         or if computation of local time zone offset failed.
        BENTLEYDLL_EXPORT BentleyStatus ToUtc(DateTime& utcDateTime) const;

        //! Converts this UTC DateTime to local time
        //! @param  [out] localDateTime Resulting DateTime object in local time
        //! @return SUCCESS if successful. ERROR in case of errors, e.g.
        //!         if this date time instance is invalid (see DateTime::IsValid) or
        //!         does not have DateTime::Component::DateTime or if computation of local time zone offset failed.
        BENTLEYDLL_EXPORT BentleyStatus ToLocalTime(DateTime& localDateTime) const;

        //! The method computes the milliseconds since midnight for this DateTime (not applicable for DateTime::Component::Date).
        //! @param [out] msecs The milliseconds since midnight.
        //! @return SUCCESS if successful. ERROR if the DateTime has DateTime::Component::Date.
        BENTLEYDLL_EXPORT BentleyStatus ToMillisecondsSinceMidnight(uint32_t& msecs) const;

#if !defined (DOCUMENTATION_GENERATOR)
        //! Converts a number in msecs into a rational day number
        //! where the integral component represents the day numbers and the fractional component
        //! represents the time in the day.
        //! @param [in] msecs The number in milliseconds
        //! @return the rational day number
        BENTLEYDLL_EXPORT static double MsecToRationalDay(uint64_t msecs);

        //! Converts a rational day number to milliseconds.
        //! @param [in] rationalDay The rational day number where the integral component 
        //! represents the day numbers and the fractional component represents the time in the day
        //! @return The rational day number in milliseconds
        BENTLEYDLL_EXPORT static uint64_t RationalDayToMsec(double rationalDay);

        //! If this object is of kind Local, this computes the offset from local time to UTC in milliseconds. Otherwise, it sets the parameter to 0.
        BENTLEYDLL_EXPORT BentleyStatus ComputeOffsetToUtcInMsec(int64_t& offset) const;
#endif

        /// @}

        /// @name ToString and FromString
        /// @{

        //! Converts the value of this DateTime into a string representation.
        //! @remarks The resulting string is formatted according to the ISO8601 standard.
        //!          Second fractions are rounded to milliseconds.
        //! @return String representation of this DateTime object. An empty string is returned
        //!         if this date time info object is not valid (see DateTime::IsValid )
        BENTLEYDLL_EXPORT Utf8String ToString() const;

        //! Converts the value of this DateTime into a string representation.
        //! @remarks The resulting string is formatted according to the ISO8601 standard.
        //!          Second fractions are rounded to milliseconds.
        //! @return String representation of this DateTime object. An empty string is returned
        //!         if this date time info object is not valid (see DateTime::IsValid )
        WString ToWString() const { return WString(ToString().c_str(), BentleyCharEncoding::Utf8); }

        //! Parses an ISO 8601 date time string into a DateTime instance.
        //! @remarks This method supports T and a space as delimiter of the date and time component, e.g.
        //! both <c>2013-09-15T12:05:39</c> and <c>2013-09-15 12:05:39</c> can be parsed into a DateTime object.
        //! This is a minor deviation from the ISO standard (specifies the T delimiter), which allows to parse 
        //! SQL-99 date time literals (specifies the space delimiter)
        //! @param[out] dateTime the resulting DateTime instance
        //! @param[in] dateTimeIso8601 the ISO 8601 date time string to parse
        //! @return SUCCESS, if parsing was successful. ERROR, otherwise
        BENTLEYDLL_EXPORT static BentleyStatus FromString(DateTime& dateTime, Utf8CP dateTimeIso8601);

        //! Parses an ISO 8601 date time string into a DateTime instance.
        //! @remarks This method supports T and a space as delimiter of the date and time component, e.g.
        //! both <c>2013-09-15T12:05:39</c> and <c>2013-09-15 12:05:39</c> can be parsed into a DateTime object.
        //! This is a minor deviation from the ISO standard (specifies the T delimiter), which allows to parse 
        //! SQL-99 date time literals (specifies the space delimiter)
        //! @param[out] dateTime the resulting DateTime instance
        //! @param[in] dateTimeIso8601 the ISO 8601 date time string to parse
        //! @return SUCCESS, if parsing was successful. ERROR, otherwise
        static BentleyStatus FromString(DateTime& dateTime, WCharCP dateTimeIso8601) { return FromString(dateTime, Utf8String(dateTimeIso8601).c_str()); }

        //! Gets the maximum day number of the given month in the given year.
        //! @param[in] year Year number
        //! @param[in] month Month number
        //! @return Maximum day number of the given month
        BENTLEYDLL_EXPORT static uint8_t GetMaxDay(int16_t year, uint8_t month);
    };

//Convenience typedefs for the DateTime type
typedef DateTime& DateTimeR;
typedef DateTime const& DateTimeCR;
typedef DateTime* DateTimeP;
typedef DateTime const* DateTimeCP;

END_BENTLEY_NAMESPACE
