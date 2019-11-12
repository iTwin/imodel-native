/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    #include <windows.h>
    #include <objbase.h>
#elif defined (__unix__)
    #include <limits.h>
    #include <stdarg.h>
    #include <sys/time.h>
  #if defined (ANDROID) && !defined(__LP64__)
    #include <time64.h>
  #endif
  #if defined (__APPLE__)
    #include <mach/mach_time.h>
  #endif
    #include <Bentley/Bentley.h>
    typedef struct _FILETIME
    {
            uint32_t dwLowDateTime;
            uint32_t dwHighDateTime;
    } FILETIME;
#else
    #error unknown compiler
#endif

#include <stdlib.h>
#include <ctime>
#include <time.h>
#include <math.h>
#include "../BentleyInternal.h"
#include <Bentley/BeThread.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/WString.h>
#include <Bentley/BeDebugLog.h>

USING_NAMESPACE_BENTLEY

// Unix <-> Windows time conversions:
// * Windows "file time" is 100-nanosecond (1.0e-7) intervals since January 1, 1601 UTC. Stored in a FILETIME struct, which is equivalent to UInt64.
// * "Unix time" is seconds since January, 1, 1970 UTC. Usually stored in a time_t.
// * "Unix millis" is a Bentley concept. It's milliseconds (1.0e-3) since 1/1/1970. Sometimes stored in a double. Can be stored in an Int64 or UInt64.
// I call the Windows 100-nanosecond interval an FTI below.

#define FILETIME_1_1_1970  116444736000000000LL           // Win32 file time of midnight 1/1/70
#define UMILLIS_TO_FTI     10000LL                        // milliseconds -> 100-nanosecond interval

#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    struct QPC
        {
      private:
        uint64_t m_ticksPerMillisecond;

      public:
        QPC()
            {
            LARGE_INTEGER f;
            ::QueryPerformanceFrequency(&f);
            m_ticksPerMillisecond = f.QuadPart / 1000;  // ticks/second * seconds/millisecond = ticks/millisecond
            }

        uint64_t GetCountInMillis() const
            {
            LARGE_INTEGER tm;
            ::QueryPerformanceCounter(&tm);
            return tm.QuadPart / m_ticksPerMillisecond;
            }
        };

    static QPC s_qpc;   // grab frequency at program start-up time

#elif defined (__unix__)

    struct StartTime
        {
        uint64_t m_initializationTimeInMillis;
        StartTime() : m_initializationTimeInMillis(BeTimeUtilities::GetCurrentTimeAsUnixMillis()) {;}
        };

    static StartTime s_startTime;   // grab current time at program start-up

#else
#error unknown runtime
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/11
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t BeTimeUtilities::QueryMillisecondsCounter()
    {
#if defined (__APPLE__)
    const int64_t oneMillion = 1000 * 1000;
    static mach_timebase_info_data_t s_timebase_info;

    if (s_timebase_info.denom == 0)
        {
        (void) mach_timebase_info(&s_timebase_info);
        s_timebase_info.denom *= oneMillion;
        }

    // mach_absolute_time() returns billionth of seconds,
    // so divide by one million to get milliseconds
    return (int)((mach_absolute_time() * s_timebase_info.numer) / s_timebase_info.denom);

#elif defined (__unix__)

    return (uint32_t)(BeTimeUtilities::GetCurrentTimeAsUnixMillis() - s_startTime.m_initializationTimeInMillis);

#elif defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    return s_qpc.GetCountInMillis();

#else
#error unknown runtime
#endif
    }

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
static uint64_t fileTimeAsUInt64(FILETIME const& f)   {return *(uint64_t*)  &f;}
static void     uInt64AsFileTime(FILETIME& f, uint64_t i) {f = *(FILETIME*)&i;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static uint64_t convertFiletimeToUnixMillis(FILETIME const& ft)
    {
    uint64_t umillis = fileTimeAsUInt64(ft);
    //TODO: this can result in negative number if FILETIME is before Unix epoch!
    umillis -= FILETIME_1_1_1970;   // re-base
    umillis /= UMILLIS_TO_FTI;      // 100-nanosecond interval -> millisecond
    return umillis;
    }

void BeTimeUtilities::ConvertUnixTimeToFiletime(FILETIME& f, time_t t)
{
    uint64_t ll = (uint64_t)t * 1000LL; // Second --> millisecond
    ll *= UMILLIS_TO_FTI;           // millisecond -> 100-nanosecond interval
    ll += FILETIME_1_1_1970;        // re-base
    uInt64AsFileTime(f, ll);
}

double BeTimeUtilities::ConvertFiletimeToUnixMillisDouble(_FILETIME const& f) {return (double)(int64_t)convertFiletimeToUnixMillis(f);}

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t BeTimeUtilities::GetCurrentTimeAsUnixMillis()
    {
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    SYSTEMTIME st0;
    FILETIME   ft0;

    GetSystemTime(&st0);
    SystemTimeToFileTime(&st0, &ft0);

    return convertFiletimeToUnixMillis(ft0);
#elif defined (__unix__)

    struct timeval tv;
    gettimeofday(&tv, NULL);    // GMT
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec/1000;  // => milliseconds

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2007
+---------------+---------------+---------------+---------------+---------------+------*/
double  BeTimeUtilities::GetCurrentTimeAsUnixMillisDoubleWithDelay()
    {
    double ts0 = GetCurrentTimeAsUnixMillisDouble();
    double ts1;
    while ((ts1 = GetCurrentTimeAsUnixMillisDouble()) == ts0)
        BeThreadUtilities::BeSleep(0);

    double ts2;
    while ((ts2 = GetCurrentTimeAsUnixMillisDouble()) == ts1)
        BeThreadUtilities::BeSleep(0);

    return ts1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeTimeUtilities::AdjustUnixMillisForLocalTime(uint64_t& millis)
    {
    //milliseconds not supported by tm -> extract and add separately
    uint16_t millisecondComponent = millis % 1000LL;

    struct tm localTime;
    BentleyStatus stat = ConvertUnixMillisToLocalTime(localTime, millis);
    if (stat == ERROR)
        {
        return ERROR;
        }

    uint64_t localMillis = ConvertTmToUnixMillis(localTime);
    localMillis += millisecondComponent;

    millis = localMillis;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Krischan.Eberle   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
BentleyStatus BeTimeUtilities::ConvertUnixMillisToLocalTime
(
struct tm& localTime,
uint64_t unixMilliseconds
)
    {
    time_t t = unixMilliseconds / 1000LL;

    struct tm local;
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    errno_t stat = localtime_s(&local, &t);
    if (stat != 0)
        {
        return ERROR;
        }

#elif defined (__unix__)
    //localtime_r returns NULL on failure
    if (NULL == localtime_r(&t, &local))
        {
        return ERROR;
        }
#endif

    localTime = local;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeTimeUtilities::ConvertUnixMillisToTm(struct tm& stm, uint64_t umillis)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    __time64_t t = umillis/1000LL;
    //resulting tm is in UTC (no time zone transformation)
    errno_t stat = _gmtime64_s(&stm, &t);
    if (stat != 0)
        {
        return ERROR;
        }

    return SUCCESS;

#elif defined (__unix__)
    time_t t = umillis/1000LL;
    //resulting tm is in UTC (no time zone transformation)
    if (NULL == gmtime_r(&t, &stm))
        {
        return ERROR;
        }

    return SUCCESS;
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/03
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t BeTimeUtilities::ConvertTmToUnixMillis(tm const& timeStructIn)
    {
    tm timeStruct(timeStructIn);
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)
    __time64_t time = _mkgmtime64(&timeStruct);
#elif defined (__unix__)
    time_t time;
    #if defined (ANDROID) && !defined(__LP64__)
        time = timegm64(&timeStruct);
    #else
        time = timegm(&timeStruct);
    #endif
#else
#error unknown runtime
#endif
    return time*1000LL;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Robert.Lukasonok    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime BeTimeUtilities::ConvertBeTimePointToDateTime(BeTimePoint timePoint, BeClock const* clock)
    {
    if (!timePoint.IsValid())
        return DateTime();

    if (nullptr == clock)
        clock = &BeClock::Get();

    int64_t systemTime;
    if (SUCCESS != clock->GetSystemTime().ToUnixMilliseconds(systemTime))
        return DateTime();

    using namespace std::chrono;
    int64_t dateTimeUnixMilliseconds = systemTime + duration_cast<milliseconds>(timePoint - clock->GetSteadyTime()).count();
    DateTime result;
    if (SUCCESS != DateTime::FromUnixMilliseconds(result, dateTimeUnixMilliseconds))
        return DateTime();

    return result;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Robert.Lukasonok    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BeTimePoint BeTimeUtilities::ConvertDateTimeToBeTimePoint(DateTimeCR dateTime, BeClock const* clock)
    {
    if (nullptr == clock)
        clock = &BeClock::Get();

    int64_t dateTimeUnixMilliseconds;
    if (SUCCESS != dateTime.ToUnixMilliseconds(dateTimeUnixMilliseconds))
        return BeTimePoint();

    int64_t currentUnixMilliseconds;
    if (SUCCESS != clock->GetSystemTime().ToUnixMilliseconds(currentUnixMilliseconds))
        return BeTimePoint();

    using namespace std::chrono;
    return clock->GetSteadyTime() + milliseconds(dateTimeUnixMilliseconds - currentUnixMilliseconds);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BeClock& BeClock::Get()
    {
    static BeClock s_clock; // POD type, carries no data
    return s_clock;
    }
