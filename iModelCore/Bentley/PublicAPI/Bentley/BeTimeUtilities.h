/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeTimeUtilities.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Bentley.h"
#include "DateTime.h"
#include "WString.h"

#include <time.h>
#include <chrono>

//__PUBLISH_SECTION_END__
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    struct _FILETIME;
#endif
//__PUBLISH_SECTION_START__

struct tm;

BEGIN_BENTLEY_NAMESPACE

struct BeTimePoint;
struct BeClock;

/**
* @addtogroup GROUP_Time Dates and Time Module
* Cross-platform utilities for working with dates and times.
*/

//=======================================================================================
//! Functions to get the current time and to convert time values between different bases and formats.
//! @section UnixTimeMillis
//! A "Unix time in milliseconds" is expressed as milliseconds since 1/1/1970 UTC
//! @ingroup GROUP_Time
//=======================================================================================
struct BeTimeUtilities
    {
    //! @name Elapsed time
    //! @{

    //! Get a value that represents elapsed time since some /i unspecified start time, expressed as a 1/1000th of a second.
    //! @return a number of clock ticks in 1/1000ths of a second
    //! @note this method is deprecated, use BeTimePoint::Now() instead.
    BENTLEYDLL_EXPORT static uint64_t QueryMillisecondsCounter();

    //! Get a value that represents elapsed time in seconds and fractions of seconds since some unspecified start time.
    //! @return a number of seconds.
    //! @note this method is deprecated, use BeTimePoint::Now() instead.
    static double QuerySecondsCounter() {return QueryMillisecondsCounter() / 1000.0;}

    //! Get a value that represents elapsed time since some unspecified start time, expressed as a 1/1000th of a second.
    //! @remarks This version of the function returns its result as a 32-bit integer. Therefore, if a session were to continue for 49 days,
    //! the tick count returned by this function would overflow and wrap around. This is only a problem if the program were to compare a
    //! tick count obtained before the values wrapped around to one obtained after.
    //! @return a number of ticks in 1/1000ths of a second
    //! @note this method is deprecated, use BeTimePoint::Now() instead.
    static uint32_t QueryMillisecondsCounterUInt32() {return (uint32_t)((0xffffffff & QueryMillisecondsCounter()));}

    /// @}

    /// @name Current time
    /// @{

    //! Get the current time expressed as \ref UnixTimeMillis
    BENTLEYDLL_EXPORT static uint64_t GetCurrentTimeAsUnixMillis();

    //! Get the current time expressed as \ref UnixTimeMillis, as a double
    static double GetCurrentTimeAsUnixMillisDouble() {return (double)(int64_t)GetCurrentTimeAsUnixMillis();}

    //! Get a timestamp that is guaranteed to be different from the timestamp retrieved
    //! by any previous or any subsequent caller of GetCurrentTimeAsUnixMillis[Double]
    BENTLEYDLL_EXPORT static double GetCurrentTimeAsUnixMillisDoubleWithDelay();

    /// @}

    /// @name Conversions
    /// @{

//__PUBLISH_SECTION_END__

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    //! Convert \ref FileTime to \ref UnixTimeMillis, as a double
    //! @param f    A \ref FileTime
    //! @return the equivalent \ref UnixTimeMillis
    BENTLEYDLL_EXPORT static double ConvertFiletimeToUnixMillisDouble(_FILETIME const& f);

    //! Convert UnixTime to FileTime
    //! @param[out] f   A time expresses in _FILETIME
    //! @param[in]  t   A time expresses in time_t
    BENTLEYDLL_EXPORT static void ConvertUnixTimeToFiletime(_FILETIME& f, time_t t);
#endif

//__PUBLISH_SECTION_START__

    //! Convert a tm to \ref UnixTimeMillis (UTC)
    //! @remarks No time zone transformations are done.
    //! @param[in] t     The time
    //! @return the equivalent \ref UnixTimeMillis
    BENTLEYDLL_EXPORT static uint64_t ConvertTmToUnixMillis(tm const& t);

    //! Convert a \ref UnixTimeMillis to Unix tm
    //! @remarks The resulting tm is in UTC, i.e. no time zone transformations are done.
    //! @param[out] t       The time info returned
    //! @param[in]  umillis The time
    //! @return SUCCESS if conversion was successful. ERROR otherwise.
    BENTLEYDLL_EXPORT static BentleyStatus ConvertUnixMillisToTm(struct tm& t, uint64_t umillis);

    //! Convert a Unix tm to \ref UnixTimeMillis, as a double
    //! @remarks No time zone transformations are done.
    //! @param[in] t     The time
    //! @return the equivalent \ref UnixTimeMillis
    static double ConvertTmToUnixMillisDouble(tm const& t) {return (double)(int64_t)ConvertTmToUnixMillis(t);}

    //! Adjust \ref UnixTimeMillis (UTC) according to user's local time
    //! @remarks Implicitly does a conversion from UTC to local time
    //! @param  u The \ref UnixTimeMillis to adjust
    //! @return SUCCESS if the conversion was successful
    BENTLEYDLL_EXPORT static BentleyStatus AdjustUnixMillisForLocalTime(uint64_t& u);

    //! Converts \ref UnixTimeMillis (UTC) to the user's local time
    //! @param  [out] localTime the resulting local time
    //! @param  [in] unixMilliseconds The \ref UnixTimeMillis to convert
    //! @return SUCCESS if the conversion was successful
    BENTLEYDLL_EXPORT static BentleyStatus ConvertUnixMillisToLocalTime(struct tm& localTime, uint64_t unixMilliseconds);

    //! Approximates system clock time point from steady clock time point using the supplied BeClock instance.
    //! @remarks The conversion is lossy due to Unix time not accounting for leap seconds. Also, the implementation
    //! calls BeClock::GetSteadyTime and BeClock::GetSystemTime in quick succession, which introduces some
    //! additional time drift.
    //! @param[in] timePoint steady clock time point to convert.
    //! @param[in] clock BeClock to use for obtaining reference time points. If nullptr, BeClock::Get() is used.
    //! @returns @p timePoint represented as DateTime if conversion was successful, invalid DateTime otherwise.
    BENTLEYDLL_EXPORT static DateTime ConvertBeTimePointToDateTime(BeTimePoint timePoint, BeClock const* clock = nullptr);

    //! Approximates steady clock time point from system clock time point using the supplied BeClock instance.
    //! @remarks The conversion is lossy due to Unix time not accounting for leap seconds. Also, the implementation
    //! calls BeClock::GetSteadyTime and BeClock::GetSystemTime in quick succession, which introduces some
    //! additional time drift.
    //! @param[in] dateTime system clock time point to convert.
    //! @param[in] clock BeClock to use for obtaining reference time points. If nullptr, BeClock::Get() is used.
    //! @returns @p dateTime represented as BeTimePoint if conversion was successful, invalid BeTimePoint otherwise.
    BENTLEYDLL_EXPORT static BeTimePoint ConvertDateTimeToBeTimePoint(DateTimeCR dateTime, BeClock const* clock = nullptr);
    /// @}
    };

//=======================================================================================
//! A duration, in steady_clock units (usually nanoseconds), int64. This is a std::chrono::steady_clock::duration with a
//! static function for constructing from double seconds and a few other convenience methods.
// @bsiclass                                                    Keith.Bentley   01/17
//=======================================================================================
struct BeDuration : std::chrono::steady_clock::duration
    {
    DEFINE_T_SUPER(std::chrono::steady_clock::duration)
    typedef std::chrono::nanoseconds Nanoseconds;
    typedef std::chrono::milliseconds Milliseconds;
    typedef std::chrono::seconds Seconds;
    typedef std::chrono::minutes Minutes;
    typedef std::chrono::hours Hours;

    constexpr BeDuration() : T_Super(0) {}    //!< construct a BeDuration with 0 seconds
    constexpr BeDuration(Hours val) : T_Super(val) {} // allow implicit conversion
    constexpr BeDuration(Minutes val) : T_Super(val) {} // allow implicit conversion
    constexpr BeDuration(Seconds val) : T_Super(val) {} // allow implicit conversion
    constexpr BeDuration(Milliseconds val) : T_Super(val) {} // allow implicit conversion
    constexpr BeDuration(Nanoseconds val) : T_Super(val) {} // allow implicit conversion
    constexpr BeDuration(std::chrono::duration<double,BeDuration::period> val) : T_Super(std::chrono::duration_cast<T_Super>(val)) {}
    BeDuration(int) = delete; // Note: you must specifiy units!
    BeDuration(double) = delete; // Note: you must specifiy units!

    //! construct a BeDuration from double seconds
    constexpr static BeDuration FromSeconds(double val) {return BeDuration(std::chrono::duration_cast<T_Super>(std::chrono::duration<double>(val)));}

    //! construct a BeDuration from int milliseconds
    constexpr static BeDuration FromMilliseconds(int64_t val) {return BeDuration(Milliseconds(val));}

    //! cast this BeDuration to a double number of *seconds* (not nanoseconds!)
    constexpr operator double() const {return std::chrono::duration_cast<std::chrono::duration<double>>(*this).count();}

    //! get this duration in seconds
    constexpr double ToSeconds() const {return (double) *this;}

    constexpr operator Milliseconds() const {return std::chrono::duration_cast<Milliseconds>(*this);}
    constexpr operator Seconds() const {return std::chrono::duration_cast<Seconds>(*this);}

    //! Determine whether this BeDuration is zero seconds
    bool IsZero() const {return 0 == count();}

    //! Determine whether this BeDuration is a positive (i.e. future) duration
    bool IsTowardsFuture() const {return 0 < count();}

    //! Determine whether this BeDuration is a negative (i.e. past) duration
    bool IsTowardsPast() const {return 0 > count();}

    //! Suspend the current thread for this duration
    BENTLEYDLL_EXPORT void Sleep();
    };

//=======================================================================================
//! A time point in steady_clock units (usually nanoseconds). This is a std::chrono::steady_clock::time_point with a few
//! convenience methods added.
// @bsiclass                                                    Keith.Bentley   01/17
//=======================================================================================
struct BeTimePoint : std::chrono::steady_clock::time_point
    {
    DEFINE_T_SUPER(std::chrono::steady_clock::time_point)
    using T_Super::time_point;

// WIP_XCODE9
#if (defined (__clang__) && defined (__APPLE__) && ((__clang_major__ * 1000 + __clang_minor__ * 10) >= 9000))
    BeTimePoint(T_Super const& val) : T_Super(val) {}
#endif

    //! get the current time point from the steady_clock
    static BeTimePoint Now() {return std::chrono::steady_clock::now();}

    //! Get a BeTimePoint at a specified duration in the future from now
    //! @param[in] val the duration from now
    static BeTimePoint FromNow(BeDuration val) {return Now()+val;}

    //! Get a BeTimePoint at a specified duration in the past before now
    //! @param[in] val the duration before now
    static BeTimePoint BeforeNow(BeDuration val) {return Now()-val;}

    BeTimePoint::rep GetTicks() const {return time_since_epoch().count();}

    //! Determine whether this BeTimePoint is valid (non-zero)
    bool IsValid() const {return 0 != GetTicks();}

    //! return true if this BeTimePoint is a valid time in the future from the time this method is called (it calls Now()!)
    //! @note always returns false and does not call Now() if this is not a valid BeTimePoint
    bool IsInFuture() const {return IsValid() && (Now() < *this);}

    //! return true if this BeTimePoint was a valid time that has past before the time this method is called (it calls Now()!)
    //! @note always returns false and does not call Now() if this is not a valid BeTimePoint
    bool IsInPast() const {return IsValid() && (Now() > *this);}
    };

/*=================================================================================**//**
* Measures elapsed time using std::chrono::steady_clock (not std::chrono::high_resolution_clock).
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct StopWatch : NonCopyableClass
    {
private:
    Utf8CP m_description = nullptr;
    BeTimePoint m_start;
    BeTimePoint m_stop;

public:
    static BeTimePoint Now () {return BeTimePoint::Now();}

    //! Initialize the stopwatch.
    //! @param[in] start If true, start StopWatch now.
    void Init(bool start) {if (start) Start();}

    //! Create a named stopwatch and possibly start it.
    //! @param[in] description A name for the stopwatch that can be retrieved later
    //! @param[in] startImmediately Pass true to start the stopwatch on creation
    explicit StopWatch(Utf8CP description=nullptr, bool startImmediately=false) : m_description(description) {Init(startImmediately);}

    //! Create an unnamed stopwatch and possibly start it.
    explicit StopWatch(bool startImmediately) {Init(startImmediately);}

    //! Get the description provided to the constructor
    Utf8CP GetDescription() {return m_description;}

    //! Start or restart the stopwatch. Any future time measurements will be based on this new value.
    void Start() {m_start = Now();}

    //! Stop the stopwatch so that the duration can be viewed later.
    void Stop() {m_stop = Now();}

    //! Get the elapsed time since Start() on a running timer.
    BeDuration GetCurrent() {return Now() - m_start;}
    double GetCurrentSeconds() {return GetCurrent();}

    //! Get the elapsed time between Start() and Stop() on this timer.
    BeDuration GetElapsed() {return m_stop - m_start;}
    double GetElapsedSeconds() {return GetElapsed();}
    };

//=======================================================================================
//! This class serves as an extension point for code that deals with current time. It
//! makes the code that receives a BeClock pointer easily testable, as you can control
//! what the virtual methods of this class will return at test time.
// @bsiclass                                                     Vincas.Razma    10/2018
//=======================================================================================
struct BeClock
    {
    //! Easily accessible singleton that holds the default implementation.
    BENTLEYDLL_EXPORT static BeClock& Get();

    //! DEPRECATED, superseded by GetSteadyTime().
    //! Returns current time point.
    virtual BeTimePoint Now() const { return GetSteadyTime(); }

    //! Returns the current time point on a monotonic clock. Most suitable for measuring time intervals.
    //! @remarks The returned BeTimePoint is guaranteed to be valid only in the process that created it.
    //! If you are interested in persistently storing the current time point, use GetSystemTime() instead.
    virtual BeTimePoint GetSteadyTime() const { return BeTimePoint::Now(); }

    //! Returns the current time point in UTC on the system-wide clock.
    //! @remarks Always assume that the system clock is not monotonic and can be manipulated by the user.
    //! @note The returned DateTime is suitable for use in persisted storage.
    virtual DateTime GetSystemTime() const { return DateTime::GetCurrentTimeUtc(); }

    virtual ~BeClock() {};
    };

END_BENTLEY_NAMESPACE
