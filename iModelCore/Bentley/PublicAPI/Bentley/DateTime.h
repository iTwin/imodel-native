/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/DateTime.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
//! A DateTime's accuracy is 100's of nanoseconds (i.e hecto-nanoseconds, i.e. 1e-7 seconds)
//! @ingroup BeTimeGroup
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
        //! The time represented is not specified as either local time or Coordinated Universal Time (UTC).
        //! Only use that for legacy data when the time zone information is not known.
        Unspecified = 0,

        //! The time represented is UTC.
        Utc = 1,

        //! The time represented is local time (in machine's time zone).
        Local = 2
        };

    //=======================================================================================    
    //! Specifies whether a DateTime represents a date (without time component) or a date time
    // @bsiclass                                                 Krischan.Eberle      02/2013
    //=======================================================================================    
    enum class Component
        {
        //! The DateTime represents a date only (without time component)
        Date = 0,

        //! The DateTime represents a date and a time
        DateAndTime = 1,
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
        Kind m_kind;
        Component m_component;

    public:
        //! Construct a new instance of the Info class.
        //! @param[in] kind DateTime kind
        //! @param[in] component DateTime component
        explicit Info(Kind kind=Kind::Unspecified, Component component=Component::DateAndTime) : m_kind(kind), m_component(component) {}

        //! Compares the given Info object with this Info object.
        //! @param [in] rhs Info object to compare with this Info object
        //! @return true if the two Info objects are equal, false otherwise
        bool operator== (Info const& rhs) const {return m_kind == rhs.m_kind && m_component == rhs.m_component;}

        //! Compares the given Info object with this Info object for inequality.
        //! @param [in] rhs Info object to compare with this Info object
        //! @return true if the two Info objects are not equal, false otherwise
        bool operator!= (Info const& rhs) const {return !(*this == rhs);}

        //! Gets the DateTime kind.
        //! @return DateTime kind.
        Kind GetKind() const {return m_kind;}

        //! Gets the DateTime component.
        //! @return DateTime component.
        Component GetComponent() const {return m_component;}

        //! Generates a text representation of the specified DateTime::Kind
        //! @param[in] kind DateTime::Kind to generate text representation for
        //! @return Text representation of a DateTime::Kind
        BENTLEYDLL_EXPORT static Utf8String KindToString(DateTime::Kind kind);
        BENTLEYDLL_EXPORT static WString KindToStringW(DateTime::Kind kind);

        //! Generates a text representation of the specified DateTime::Kind
        //! @param[in] component DateTime::Component to generate text representation for
        //! @return Text representation of a DateTime::Kind
        BENTLEYDLL_EXPORT static Utf8String ComponentToString(DateTime::Component component);
        BENTLEYDLL_EXPORT static WString ComponentToStringW(DateTime::Component component);
        };

private:
    Info m_info;
    int16_t m_year;
    uint8_t m_month;
    uint8_t m_day;
    uint8_t m_hour;
    uint8_t m_minute;
    uint8_t m_second;
    uint32_t m_hectoNanosecond;

public:
    //***** Construction ******
//__PUBLISH_SECTION_END__
    //Intentionally use the compiler-generated versions of copy constructor, assignment operator, and destructor
//__PUBLISH_SECTION_START__

    //! Initializes a new empty instance of the DateTime struct.
    //! @remarks DateTime::IsValid returns false in that case, as it doesn't represent any date time.
    BENTLEYDLL_EXPORT DateTime();

    //! Initializes a new instance of the DateTime struct.
    //! @remarks This creates a "date-only" DateTime, i.e.
    //!     the DateTime::Component of this DateTime amounts to DateTime::Component::Date.
    //!     DateTime::Kind is irrelevant for "date-only" DateTime;
    //! @param [in] year The year (-4713 through 9999) 
    //! @param [in] month The month (1 through 12) 
    //! @param [in] day The day in the month (1 through the number in @p month) 
    BENTLEYDLL_EXPORT DateTime(int16_t year, uint8_t month, uint8_t day);

    //! Initializes a new instance of the DateTime struct.
    //! @param [in] kind the Kind for this DateTime
    //! @param [in] year The year (-4713 through 9999) 
    //! @param [in] month The month (1 through 12) 
    //! @param [in] day The day in the month (1 through the number in @p month) 
    //! @param [in] hour The hours (0 through 23)
    //! @param [in] minute The minutes (0 through 59)
    //! @param [in] second The seconds (0 through 59)
    //! @param [in] hectoNanosecond The hecto-nanoseconds (0 through 9,999,999)
    BENTLEYDLL_EXPORT DateTime(Kind kind, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second = 0, uint32_t hectoNanosecond = 0);
    
    //! Indicates whether this DateTime instance is a valid date/time or not.
    //! @remarks Using the default constructor creates an invalid date time as none of the components have been set.
    //! @return true, if the DateTime instance is valid, false otherwise
    BENTLEYDLL_EXPORT bool IsValid() const;

//__PUBLISH_SECTION_END__
    //! Initializes a new instance of the DateTime struct.
    //! @param [in] info the DateTime::Info for this DateTime
    //! @param [in] year The year (-4713 through 9999) 
    //! @param [in] month The month (1 through 12) 
    //! @param [in] day The day in the month (1 through the number in \p month) 
    //! @param [in] hour The hours (0 through 23)
    //! @param [in] minute The minutes (0 through 59)
    //! @param [in] second The seconds (0 through 59)
    //! @param [in] hectoNanosecond The hecto-nanoseconds (0 through 9,999,999)
    DateTime(Info const& info, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second = 0, uint32_t hectoNanosecond = 0);
//__PUBLISH_SECTION_START__

    //! Compares the given DateTime with this DateTime.
    //! @param [in] rhs DateTime to compare with this DateTime
    //! @return true if the two DateTimes are equal, false otherwise
    bool operator== (DateTime const& rhs) const { return Equals(rhs, false); }

    //! Checks the given DateTime with this DateTime for inequality.
    //! @param [in] rhs DateTime to compare with this DateTime
    //! @return true if the two DateTimes are not equal, false if they are equal
    bool operator!= (DateTime const& rhs) const { return !(*this == rhs); }

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

    //! Computes the Julian Day number in hecto-nanoseconds from this DateTime instance.
    //! @remarks This DateTime must be on the proleptic Gregorian calendar.
    //! @param  [out] julianDayInHns Julian Day number in hecto-nanoseconds
    //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g. if date time instance is not valid (see DateTime::IsValid)
    //!         or if computation of local time zone offset failed.
    BENTLEYDLL_EXPORT BentleyStatus ToJulianDay(uint64_t& julianDayInHns) const;

    //! Computes the Julian Day number from this DateTime instance.
    //! @remarks This DateTime must be on the proleptic Gregorian calendar.
    //! @param  [out] julianDay Julian Day number with the time component being the fractional component of the double value.
    //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g. if date time instance is not valid (see DateTime::IsValid)
    //!         or if computation of local time zone offset failed.
    BENTLEYDLL_EXPORT BentleyStatus ToJulianDay(double& julianDay) const;

    //! Computes the DateTime from the given Julian Day number.
    //! @param  [out] dateTime Resulting DateTime object (on the proleptic Gregorian calendar)
    //! @param  [in] julianDayInHns Julian Day number in hecto-nanoseconds to convert to DateTime
    //! @param  [in] targetInfo DateTime::Info the resulting DateTime should have
    //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g.
    //!          if computation of local time zone offset failed.
    BENTLEYDLL_EXPORT static BentleyStatus FromJulianDay(DateTime& dateTime, uint64_t julianDayInHns, Info const& targetInfo);

    //! Computes the DateTime from the given Julian Day number.
    //! @param  [out] dateTime Resulting DateTime object (on the proleptic Gregorian calendar)
    //! @param  [in] julianDay Julian Day number with the time component being the fractional component of the double value.
    //! @param  [in] targetInfo DateTime::Info the resulting DateTime should have
    //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g.
    //!          if computation of local time zone offset failed.
BENTLEYDLL_EXPORT static BentleyStatus FromJulianDay(DateTime& dateTime, double julianDay, Info const& targetInfo);

    /// @}

    /// @name Conversions
    /// @{

    //! Converts this local DateTime to UTC
    //! @param  [out] utcDateTime Resulting DateTime object in UTC 
    //! @return SUCCESS if successful. ERROR in case of errors, e.g.
    //!         if this date time instance is invalid (see DateTime::IsValid) or if computation of local time zone offset failed.
    BENTLEYDLL_EXPORT BentleyStatus ToUtc(DateTime& utcDateTime) const;

    //! Converts this UTC DateTime to local time
    //! @param  [out] localDateTime Resulting DateTime object in local time
    //! @return SUCCESS if successful. ERROR in case of errors, e.g.
    //!         if this date time instance is invalid (see DateTime::IsValid) or if computation of local time zone offset failed.
    BENTLEYDLL_EXPORT BentleyStatus ToLocalTime(DateTime& localDateTime) const;

    //! Converts this DateTime to Unix epoch milliseconds (UTC).
    //!
    //! This DateTime must be on the proleptic Gregorian calendar.
    //!
    //! The resulting Unix milliseconds are UTC based. If this DateTime object is in local time, the method internally
    //! accounts for converting to UTC first.
    //! @param [out] unixMilliseconds Unix epoch milliseconds. The value is negative for dates before the beginning
    //!         of the Unix epoch (1970-01-01 00:00:00 UTC)
    //! @return SUCCESS if successful. ERROR in case of errors, e.g.
    //!         if this date time instance is invalid (see DateTime::IsValid) or if computation of local time zone offset failed.
    BENTLEYDLL_EXPORT BentleyStatus ToUnixMilliseconds(int64_t& unixMilliseconds) const;

    //! Creates a DateTime in UTC from the given Unix epoch milliseconds
    //! @param [out] dateTime The resulting DateTime object on the proleptic Gregorian calendar (in UTC).
    //! @param [in] unixMilliseconds The Unix epoch milliseconds. Negative if they refer to a date before
    //!             the Unix epoch (1970-01-01 00:00:00 UTC)
    //! @return SUCCESS if conversion was successful. ERROR otherwise
    BENTLEYDLL_EXPORT static BentleyStatus FromUnixMilliseconds(DateTime& dateTime, uint64_t unixMilliseconds);
    
    //! Computes the Unix milliseconds from the given Julian Day number.
    //! @param  [in] julianDayInHectoNanoseconds Julian day in hecto-nanoseconds.unixMilliseconds Unix milliseconds.
    //! @return Unix milliseconds. Negative numbers indicate dates before the Unix epoch.
    BENTLEYDLL_EXPORT static int64_t JulianDayToUnixMilliseconds(uint64_t julianDayInHectoNanoseconds);
    
    //! Computes the Julian Day number from the given Unix milliseconds.
    //! @param  [in] unixMilliseconds Unix milliseconds. Negative numbers indicate dates before the Unix epoch.
    //! @return Julian Day number in hecto-nanoseconds
    BENTLEYDLL_EXPORT static uint64_t UnixMillisecondsToJulianDay(uint64_t unixMilliseconds);

    //! Computes the Common Era ticks from this DateTime instance.
    //! This DateTime must be on the proleptic Gregorian calendar.
    //! The Common Era begins at 0001-01-01 00:00:00 UTC.
    //! @param  [out] commonEraTicks Common Era ticks in hecto-nanoseconds
    //! @return SUCCESS if successful. ERROR in case of errors, e.g.
    //!         if this date time instance is invalid (see DateTime::IsValid) or if computation of local time zone offset failed.
    BENTLEYDLL_EXPORT BentleyStatus ToCommonEraTicks(int64_t& commonEraTicks) const;

    //! Computes the DateTime from the given Common Era ticks.
    //! The Common Era begins at 0001-01-01 00:00:00 UTC.
    //! @param  [out] dateTime Resulting DateTime object (on the proleptic Gregorian calendar)
    //! @param  [in] commonEraTicks Common Era ticks in hecto-nanoseconds
    //! @param  [in] targetInfo DateTime::Info the resulting DateTime should have
    //! @return SUCCESS if computation was successful. ERROR in case of errors, e.g.
    //!          if computation of local time zone offset failed.
    BENTLEYDLL_EXPORT static BentleyStatus FromCommonEraTicks(DateTime& dateTime, int64_t commonEraTicks, Info const& targetInfo);

    //! Computes Common Era ticks from the given the Julian Day number.
    //! The Common Era begins at 0001-01-01 00:00:00 UTC.
    //! @param  [in] julianDayInHectoNanoseconds Julian Day number 
    //! @return Common Era ticks in hecto-nanoseconds
    BENTLEYDLL_EXPORT static int64_t JulianDayToCommonEraTicks(uint64_t julianDayInHectoNanoseconds);

    //! Computes the Julian Day number from the given Common Era ticks.
    //! The Common Era begins at 0001-01-01 00:00:00 UTC.
    //! @param  [in] commonEraTicks Common Era ticks in hecto-nanoseconds
    //! @return Julian Day number in hecto-nanoseconds
    BENTLEYDLL_EXPORT static uint64_t CommonEraTicksToJulianDay(int64_t commonEraTicks);

//__PUBLISH_SECTION_END__
    //! Converts a number in hecto-nanoseconds into a rational day number
    //! where the integral component represents the day numbers and the fractional component
    //! represents the time in the day.
    //! @param [in] hns The number in hecto-nanoseconds
    //! @return the rational day number
    BENTLEYDLL_EXPORT static double HnsToRationalDay(uint64_t hns);

    //! Converts a rational day number to hecto-nanoseconds.
    //! @param [in] rationalDay The rational day number where the integral component 
    //! represents the day numbers and the fractional component represents the time in the day
    //! @return The rational day number in hecto-nanoseconds
    BENTLEYDLL_EXPORT static uint64_t RationalDayToHns(double rationalDay);
    /// @}

//__PUBLISH_SECTION_START__

    /// @name ToString and FromString
    /// @{

    //! Converts the value of this DateTime into a string representation.
    //! @remarks The resulting string is formatted according to the ISO8601 standard.
    //!          Second fractions are rounded to milliseconds.
    //! @return String representation of this DateTime object. An empty string is returned
    //!         if this date time info object is not valid (see DateTime::IsValid )
    BENTLEYDLL_EXPORT Utf8String ToUtf8String() const;

    //! Converts the value of this DateTime into a string representation.
    //! @remarks The resulting string is formatted according to the ISO8601 standard.
    //!          Second fractions are rounded to milliseconds.
    //! @return String representation of this DateTime object. An empty string is returned
    //!         if this date time info object is not valid (see DateTime::IsValid )
    WString ToString() const {return WString(ToUtf8String().c_str(), BentleyCharEncoding::Utf8);}
    
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
    BENTLEYDLL_EXPORT static BentleyStatus FromString(DateTime& dateTime, WCharCP dateTimeIso8601);

    /// @name DateTime components
    /// @{

    //! Gets meta data about this DateTime object.
    //! @return DateTime::Info of this DateTime object.
    BENTLEYDLL_EXPORT Info const& GetInfo() const;

//__PUBLISH_SECTION_END__
    //! Gets the Kind of this DateTime object.
    //! @return Kind.
    Kind GetKind() const;
//__PUBLISH_SECTION_START__

    //! Gets the year component of this DateTime object.
    //! @return Year (negative if BCE, positive otherwise).
    BENTLEYDLL_EXPORT int16_t GetYear() const;

    //! Gets the month component of this DateTime object.
    //! @return Month (1 through 12).
    BENTLEYDLL_EXPORT uint8_t GetMonth() const;

    //! Gets the day component of this DateTime object.
    //! @return Day in the month (1 through the number in GetMonth) 
    BENTLEYDLL_EXPORT uint8_t GetDay() const;

    //! Gets the hour component of this DateTime object.
    //! @return Hours (0 through 59) 
    BENTLEYDLL_EXPORT uint8_t GetHour() const;

    //! Gets the minute component of this DateTime object.
    //! @return Minutes (0 through 59) 
    BENTLEYDLL_EXPORT uint8_t GetMinute() const;

    //! Gets the second component of this DateTime object.
    //! @return Seconds (0 through 59) 
    BENTLEYDLL_EXPORT uint8_t GetSecond() const;   

    //! Gets the milliseconds component of this DateTime object.
    //! @remarks The hecto-nanoseconds are truncated (not rounded) at the millisecond position.
    //! @return Milliseconds (0 through 999) 
    BENTLEYDLL_EXPORT uint16_t GetMillisecond() const;

    //! Gets the hecto-nanosecond component of this DateTime object.
    //! @remarks 1 hecto-nanosecond is 1e-7 seconds.
    //! @return Hecto-nanoseconds (0 through 9,999,999) 
    BENTLEYDLL_EXPORT uint32_t GetHectoNanosecond() const;

    //! Gets the day of the week of this DateTime object.
    //! @remarks Only call this method if the DateTime is valid (see DateTime::IsValid)
    //! @return Day of the week
    BENTLEYDLL_EXPORT DayOfWeek GetDayOfWeek() const;

    //! Gets the day of the year of this DateTime object.
    //! @return Day of the year or 0 if the DateTime object is not valid (see DateTime::IsValid)
    BENTLEYDLL_EXPORT uint16_t GetDayOfYear() const;

    //! Indicates whether the specified year is a leap year or not
    //! @return true, if @p year is a leap year. false, otherwise.
    BENTLEYDLL_EXPORT static bool IsLeapYear(uint16_t year);

    //! If this object is of kind Local, this computes the offset from local time to UTC in HNS (hectonanoseconds). Otherwise, it sets the parameter to 0.
    BENTLEYDLL_EXPORT BentleyStatus ComputeOffsetToUtcInHns(int64_t& offset) const;
    /// @}

//__PUBLISH_SECTION_END__
    //! Gets the maximum day number of the given month in the given year.
    //! @param[in] year Year number
    //! @param[in] month Month number
    //! @return Maximum day number of the given month
    static uint8_t GetMaxDay(int16_t year, uint8_t month);
//__PUBLISH_SECTION_START__
    };

//Convenience typedefs for the DateTime type
typedef DateTime& DateTimeR;
typedef DateTime const& DateTimeCR;
typedef DateTime* DateTimeP;
typedef DateTime const* DateTimeCP;

END_BENTLEY_NAMESPACE
