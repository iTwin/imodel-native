/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeTimeUtilities.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Bentley.h"
#include "WString.h"
#include <time.h>

//__PUBLISH_SECTION_END__
#if defined (BENTLEY_WIN32) || defined(BENTLEY_WINRT)

    struct _FILETIME;

#endif
//__PUBLISH_SECTION_START__

struct tm;

BEGIN_BENTLEY_NAMESPACE

/** 
* @addtogroup BeTimeGroup 
* Cross-platform utilities for working with dates and times.
*/

//=======================================================================================    
//! Functions to get the current time and to convert time values between different bases and formats.
//! @section UnixTimeMillis
//! A "Unix time in milliseconds" is expressed as milliseconds since 1/1/1970 UTC
//! @ingroup BeTimeGroup
//=======================================================================================    
struct BeTimeUtilities
    {
    /// @name Elapsed time
    /// @{

    //! Get a value that represents elapsed time since some /i unspecified start time, expressed as a 1/1000th of a second.
    //! @return a number of clock ticks in 1/1000ths of a second
    BENTLEYDLL_EXPORT static uint64_t QueryMillisecondsCounter ();

    //! Get a value that represents elapsed time in seconds and fractions of seconds since some unspecified start time.
    //! @return a number of seconds.
    BENTLEYDLL_EXPORT static double QuerySecondsCounter () {return QueryMillisecondsCounter() / 1000.0;}

    //! Get a value that represents elapsed time since some unspecified start time, expressed as a 1/1000th of a second.
    //! @remarks This version of the function returns its result as a 32-bit integer. Therefore, if a session were to continue for 49 days,
    //! the tick count returned by this function would overflow and wrap around. This is only a problem if the program were to compare a
    //! tick count obtained before the values wrapped around to one obtained after.
    //! @return a number of ticks in 1/1000ths of a second
    BENTLEYDLL_EXPORT static uint32_t QueryMillisecondsCounterUInt32 () {return (uint32_t)((0xffffffff & QueryMillisecondsCounter()));}

    /// @}

    /// @name Current time
    /// @{

    //! Get the current time expressed as \ref UnixTimeMillis
    BENTLEYDLL_EXPORT static uint64_t GetCurrentTimeAsUnixMillis ();

    //! Get the current time expressed as \ref UnixTimeMillis, as a double
    static double GetCurrentTimeAsUnixMillisDouble () {return (double)(int64_t)GetCurrentTimeAsUnixMillis();}

    //! Get a timestamp that is guaranteed to be different from the timestamp retrieved
    //! by any previous or any subsequent caller of GetCurrentTimeAsUnixMillis[Double]
    BENTLEYDLL_EXPORT static double GetCurrentTimeAsUnixMillisDoubleWithDelay ();

    /// @}

    /// @name Conversions
    /// @{

//__PUBLISH_SECTION_END__

#if defined (BENTLEY_WIN32) || defined(BENTLEY_WINRT)
    //! Convert \ref FileTime to \ref UnixTimeMillis, as a double
    //! @param f    A \ref FileTime
    //! @return the equivalent \ref UnixTimeMillis
    BENTLEYDLL_EXPORT static double ConvertFiletimeToUnixMillisDouble (_FILETIME const& f);

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
    BENTLEYDLL_EXPORT static uint64_t ConvertTmToUnixMillis (tm const& t);

    //! Convert a \ref UnixTimeMillis to Unix tm
    //! @remarks The resulting tm is in UTC, i.e. no time zone transformations are done.
    //! @param[out] t       The time info returned
    //! @param[in]  umillis The time
    //! @return SUCCESS if conversion was successful. ERROR otherwise.
    BENTLEYDLL_EXPORT static BentleyStatus ConvertUnixMillisToTm (struct tm& t, uint64_t umillis);

    //! Convert a Unix tm to \ref UnixTimeMillis, as a double
    //! @remarks No time zone transformations are done.
    //! @param[in] t     The time
    //! @return the equivalent \ref UnixTimeMillis
    static double ConvertTmToUnixMillisDouble (tm const& t) {return (double)(int64_t)ConvertTmToUnixMillis(t);}

    //! Adjust \ref UnixTimeMillis (UTC) according to user's local time
    //! @remarks Implicitly does a conversion from UTC to local time
    //! @param  u The \ref UnixTimeMillis to adjust
    //! @return SUCCESS if the conversion was successful
    BENTLEYDLL_EXPORT static BentleyStatus AdjustUnixMillisForLocalTime (uint64_t& u);

    //! Converts \ref UnixTimeMillis (UTC) to the user's local time
    //! @param  [out] localTime the resulting local time
    //! @param  [in] unixMilliseconds The \ref UnixTimeMillis to convert
    //! @return SUCCESS if the conversion was successful
    BENTLEYDLL_EXPORT static BentleyStatus ConvertUnixMillisToLocalTime (struct tm& localTime, uint64_t unixMilliseconds);

    /// @}
    };


#if !defined (DOCUMENTATION_GENERATOR)
/*=================================================================================**//**
* Measures elapsed time
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct StopWatch : NonCopyableClass
    {
private:
    WString     m_description;
    uint64_t    m_start;
    uint64_t    m_stop;
    uint64_t    m_frequency;

protected:
    //! Sets the internal frequency to match the high-resolution performance counter, if one exists.
    BENTLEYDLL_EXPORT void SetFrequency();

public:
    //! Return the current time.  On Windows this is the current value of the high-resolution performance counter.  
    //! On Unix it is the current time in milliseconds.
    BENTLEYDLL_EXPORT static uint64_t Now();
    //! Initialize the stopwatch.  If true is passed as start then it will start it as well.  Called from the constructor.
    void Init(bool start) {m_start=m_stop=0; SetFrequency(); if (start) Start();}
    //! Create a named stopwatch and possibly start it.
    //! @param[in] description      A name for the stopwatch that can be retrieved later.  This is a convenience.
    //! @param[in] startImmediately Pass true to start the stopwatch on creation
    explicit StopWatch (WCharCP description = L"", bool startImmediately=false) : m_description (description)   {Init(startImmediately);}
    //! Create a named stopwatch and possibly start it.
    //! @param[in] description      A name for the stopwatch that can be retrieved later.  This is a convenience.
    //! @param[in] startImmediately Pass true to start the stopwatch on creation
    StopWatch (Utf8CP description, bool startImmediately) : m_description (description, true) {Init(startImmediately);}
    StopWatch (bool startImmediately) : m_description (L"") {Init(startImmediately);}
    //! Extract the description provided to the constructor
    WString GetDescription() {return m_description;}

    //! Start or restart the stopwatch.  Any future time measurements will be based on this new value.
    void Start(){m_start = Now();}
    //! Stop the stopwatch so that the duration can be viewed later.
    void Stop() {m_stop = Now();}

    //! Get the elapsed seconds since Start() on a running timer.
    double GetCurrentSeconds() {return (Now() - m_start) / (double)m_frequency;}

    //! Get the elapsed seconds between Start() and Stop() on this timer.
    double GetElapsedSeconds() {return (m_stop - m_start) / (double)m_frequency;}
    };
#endif // DOCUMENTATION_GENERATOR

END_BENTLEY_NAMESPACE
