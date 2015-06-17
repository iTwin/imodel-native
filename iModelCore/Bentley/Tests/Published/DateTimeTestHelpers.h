/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/DateTimeTestHelpers.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <time.h>

#include <Bentley/bvector.h>
#include <Logging/bentleylogging.h>
#include <Bentley/DateTime.h>

//=======================================================================================    
//! @bsiclass                                           Krischan.Eberle             03/14
//=======================================================================================    
struct DateTimeLogger
    {
private:
    static BentleyApi::NativeLogging::ILogger* s_logger;

    DateTimeLogger ();
    ~DateTimeLogger ();

public:
    static BentleyApi::NativeLogging::ILogger& Get ();
    };

#define LOG (DateTimeLogger::Get ())

#define POSTCONDITION(_Expression, _ErrorStatus)  \
    if (!(_Expression))                          \
    {                                        \
    LOG.errorv (L"Postcondition failed:\n  postcondition: %hs\n  method: %hs\n  file: %hs\n  line: %i\n", \
#_Expression, __FUNCTION__, __FILE__, __LINE__); \
    return _ErrorStatus;                     \
        }

//=======================================================================================    
//! @bsiclass                                           Krischan.Eberle             10/12
//=======================================================================================    
struct DateTimeTestItem
    {
public:
    //=======================================================================================    
    //! @bsiclass                                           Krischan.Eberle             10/12
    //=======================================================================================    
    enum TestMode
        {
        FromDateTime = 1,
        ToDateTime = 2,
        Roundtrip = FromDateTime | ToDateTime
        };

private:
    DateTime m_dateTime;
    uint64_t m_jdInHns;
    int64_t m_unixMilliseconds;
    bool m_isUnixMillisecondsNull;
    TestMode m_mode;

    TestMode GetMode () const;

public:
    DateTimeTestItem ();
    DateTimeTestItem (const DateTime& dateTime, TestMode testMode);
    DateTimeTestItem (const DateTime& dateTime, uint64_t jdInHns = 0ULL);
    ~DateTimeTestItem ();
    //use compiler generated copy constructor and assignment operator

    void SetDateTime (const DateTime& dateTime);
    const DateTime& GetDateTime () const;
    
    void SetJdInHns (uint64_t jdInHns);
    const uint64_t& GetJdInHns () const;

    void SetUnixMilliseconds (int64_t unixMilliseconds);
    const int64_t& GetUnixMilliseconds () const;
    bool IsUnixMillisecondsNull () const;

    bool IsToDateTimeTestMode () const;
    bool IsFromDateTimeTestMode () const;
    };

typedef bvector<DateTimeTestItem> DateTimeTestItemList;

//=======================================================================================    
//! @bsiclass                                           Krischan.Eberle            10/12
//=======================================================================================    
struct DateTimeAsserter
    {
private:
    DateTimeAsserter ();
    ~DateTimeAsserter ();

    static void AssertToJulianDay (const DateTimeTestItem& testItem);
    static void AssertFromJulianDay (const DateTimeTestItem& testItem);
    static void AssertToUnixMilliseconds (const DateTimeTestItem& testItem);
    static void AssertFromUnixMilliseconds (const DateTimeTestItem& testItem);

public:
    static void Assert (const DateTimeTestItemList& testItemList);
    static void Assert (const DateTimeTestItem& testItem);

    static void Assert (const BentleyApi::DateTime& expected, const BentleyApi::DateTime& actual, WCharCP assertMessage = L"");
    static void Assert (const BentleyApi::DateTime& expected, const BentleyApi::DateTime& actual, bool millisecondsAccuracyOnly, WCharCP assertMessage);
    static void Assert (const BentleyApi::DateTime& expected, const BentleyApi::DateTime& actual, bool ignoreDateTimeComponent, bool millisecondsAccuracyOnly, WCharCP assertMessage);

    static void AssertStringConversion (DateTimeCR expectedDateTime, Utf8CP expectedIsoDate);
    static void AssertFromString (Utf8CP iso8601DateTime, DateTimeCR expectedDateTime, bool expectedIsEqual);
    static void AssertFromString (WCharCP iso8601DateTime, DateTimeCR expectedDateTime, bool expectedIsEqual);
    };

//********************* DateTimeTestHelper *************************************
//=======================================================================================    
//! Provides several helper methods for the DateTime ATPs
//! @bsiclass                                           Krischan.Eberle                    10/12
//=======================================================================================    
struct DateTimeTestHelper
    {
private:
    DateTimeTestHelper ();
    ~DateTimeTestHelper ();

    static bool IsInUnixEpoch (const BentleyApi::DateTime& dateTime);

    static void DateTimeToTm (struct tm& out, const BentleyApi::DateTime& in);
    static void TmToDateTime (BentleyApi::DateTime& out, const tm& in, uint32_t hectoNanosecond, BentleyApi::DateTime::Kind targetKind);

public:
    static const int64_t HNS_IN_MSEC;
    static const int64_t HNS_IN_SEC;
    static const int64_t HNS_IN_DAY;
    static const int64_t HALFHOUR_IN_MSEC;
    static const int64_t HOUR_IN_MSEC;
    static const int64_t HOUR_IN_HNS;
    static const int64_t UNIXEPOCH_START_MSEC;
    static const int64_t UNIXEPOCH_END_MSEC;


    static BentleyStatus LocalToUtcViaCRT (BentleyApi::DateTime& utc, const BentleyApi::DateTime& local);
    static BentleyStatus UtcToLocalViaCRT (BentleyApi::DateTime& local, const BentleyApi::DateTime& utc);
    static BentleyStatus DateTimeToUnixMillisecsCRT (uint64_t& unixMillisecs, const DateTime& dateTime);
    };


//********************* TzSetter *************************************

#if defined (BENTLEY_WIN32)
//=======================================================================================    
//! TzSetter allows to set a timezone using the TZ env var approach.
//! When the TimezoneSetter goes out of scope, the timezone is reset (the TZ env var is deleted).
//! @bsiclass                                           Krischan.Eberle                    10/12
//=======================================================================================    
struct TzSetter
    {
private:
    static const char* TZ_ENVVAR_NAME;

    //making class non copyable
    TzSetter (const TzSetter& rhs);
    TzSetter& operator= (const TzSetter&);

 public:
    //! @bsimethod                                    Krischan.Eberle                    10/12
    TzSetter ();

    //! Resets the time zone setting again.
    //! @bsimethod                                    Krischan.Eberle                    10/12
    ~TzSetter ();

    //! Sets the current time zone to the specified value.
    //! @remarks The format of the value must comply to the TZ env var specification.
    //! @param tzEnvVarValue Time zone to set as current in format as specified by TZ env var spec.
    //! @bsimethod                                    Krischan.Eberle                    10/12
     void Set (const char* tzEnvVarValue) const;

     //! Indicates whether the current machine time zone observes DST or not.
     //! @remarks Even if TZ is set to a time zone that observes DST calling the CRT functions
     //! (e.g. localtime_s) return strange results regarding DST if the current machine's time zone does not 
     //! observe DST. ATPs can therefore check for this via this method and skip its execution.
     //! @return true if the current machine time zone observes DST. false otherwise.
     static bool CurrentMachineTimezoneObservesDst ();
    };
#endif
