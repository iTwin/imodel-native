/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/DateTime_DstAnalysis.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/bvector.h>

#include "DateTimeTestHelpers.h"

USING_NAMESPACE_BENTLEY

#if defined(BENTLEY_WIN32) && defined(CET_IS_SYSTEMTIMEZONE) 


typedef bvector<DateTime> DateTimeList;

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
void AddToTestSeries
(
DateTimeList& testSeries,
int16_t year,
uint16_t month,
uint16_t day
)
    {
    for (int hour = 12; hour <= 36; hour++)
        {
        //start at noon day before start
        int hourOfDay = hour % 24;
        int dayOffset = (hour / 24) - 1;
        int correctedDay = day + dayOffset;

        DateTime testDate (DateTime::Kind::Local, static_cast <int16_t> (year),
                                                         static_cast <uint8_t> (month),
                                                         static_cast <uint8_t> (correctedDay),
                                                         static_cast <uint8_t> (hourOfDay), 0, 0);
        testSeries.push_back (testDate);

        testDate = DateTime (DateTime::Kind::Local, static_cast <int16_t> (year),
                                                           static_cast <uint8_t> (month),
                                                           static_cast <uint8_t> (correctedDay),
                                                           static_cast <uint8_t> (hourOfDay), 30, 0);
        testSeries.push_back (testDate);
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
void CreateDstTestSeries
(
DateTimeList& testSeries,
int16_t year,
uint16_t dstStartMonth,
uint16_t dstStartDay,
uint16_t dstEndMonth,
uint16_t dstEndDay
)
    {
    testSeries.clear ();
    AddToTestSeries (testSeries, year, dstStartMonth, dstStartDay);
    AddToTestSeries (testSeries, year, dstEndMonth, dstEndDay);
    }



//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
void AnalyzeLocalTimeRoundtrip
(
const DateTime& local
)
    {
    ASSERT_EQ ((int) DateTime::Kind::Local, (int) local.GetInfo ().GetKind ()) << "Analysis can only be run with a local date time.";

    //DateTime round trip
    uint64_t jdInHns;
    BentleyStatus stat = local.ToJulianDay (jdInHns);
    ASSERT_EQ (SUCCESS, stat) << "Analysis> DateTime::ToJulianDay failed";

    DateTime utc;
    stat = DateTime::FromJulianDay (utc, jdInHns, DateTime::Info (DateTime::Kind::Utc, DateTime::Component::DateAndTime));
    ASSERT_EQ (SUCCESS, stat) << "Analysis> DateTime::FromJulianDay with UTC target failed";

    DateTime localRoundtripped;
    stat = DateTime::FromJulianDay (localRoundtripped, jdInHns, DateTime::Info (DateTime::Kind::Local, DateTime::Component::DateAndTime));
    ASSERT_EQ (SUCCESS, stat) << "Analysis> DateTime::FromJulianDay with local time target failed";

    //C Runtime round trip
    DateTime utcCrt;
    bool crtRoundTripSucceeded = DateTimeTestHelper::LocalToUtcViaCRT (utcCrt, local) == SUCCESS;
    DateTime localCrtRoundtripped;
    if (crtRoundTripSucceeded)
        {
        crtRoundTripSucceeded = DateTimeTestHelper::UtcToLocalViaCRT (localCrtRoundtripped, utcCrt) == SUCCESS;
        }

    if (crtRoundTripSucceeded)
        {
        LOG.tracev (L"%ls\t%ls\t%ls\t \t%ls\t%ls [JD: %lld]", local.ToString ().c_str (), utc.ToString ().c_str (), localRoundtripped.ToString ().c_str (), 
                                                       utcCrt.ToString ().c_str (), localCrtRoundtripped.ToString ().c_str (),
                                                       jdInHns);
        }
    else
        {
        LOG.tracev (L"%ls\t%ls\t%ls\t[JD: %lld]", local.ToString ().c_str (), utc.ToString ().c_str (), localRoundtripped.ToString ().c_str (), jdInHns);
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
void AnalyzeLocalTimeRoundtrip
(
DateTimeList& testSeries
)
    {
    LOG.trace (L"Local\t->\tUTC\t->\tLocal\t\tCRT:\t->\tUTC\t->\tLocal\t\tJD");

    DateTimeList::iterator it;
    for (it = testSeries.begin (); it != testSeries.end (); it++)
        {
        DateTime local = *it;
        AnalyzeLocalTimeRoundtrip (local);
        }
    }


//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    10/12
//---------------------------------------------------------------------------------------
TEST (DateTimeDstAnalysis, PrintDstDifferences)
    {
    //make sure that TZ env var is deleted
        {
        TzSetter tz;
        }
    LOG.trace (L"TZ unset. Machine's current time zone: CET (UTC+01)");
    bvector<DateTime> cetDstSeries;
    CreateDstTestSeries (cetDstSeries, 2012, 3, 25, 10, 28);
    AnalyzeLocalTimeRoundtrip (cetDstSeries);

    bvector<DateTime> tzDstSeries;
    CreateDstTestSeries (tzDstSeries, 2012, 3, 11, 11, 4);

        {
        LOG.trace (L"TZ: MEZ-1MESZ (CET UTC+01)");
        TzSetter tz;
        tz.Set ("MEZ-1MESZ");

        AnalyzeLocalTimeRoundtrip (tzDstSeries);
        }

        {
        LOG.trace (L"TZ: GMT0BST (GMT UTC00)");
        TzSetter tz;
        tz.Set ("GMT0BST");

        AnalyzeLocalTimeRoundtrip (tzDstSeries);
        }

        {
        LOG.trace (L"TZ: EST5EDT (EST UTC-05)");
        TzSetter tz;
        tz.Set ("EST5EDT");

        AnalyzeLocalTimeRoundtrip (tzDstSeries);
        }
    }
#endif
