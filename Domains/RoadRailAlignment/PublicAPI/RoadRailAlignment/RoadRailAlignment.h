/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/RoadRailAlignment.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignmentApi.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

enum class StationRangeOverlap
    {
    NoOverlap = 0,    //! self-explanatory
    LeftOverlap = 1,    //! rhs starts before and ends inside
    RightOverlap = 2,    //! rhs starts inside and ends after
    FullyEncapsulated = 3,    //! rhs starts before and ends after
    FullyEnvelops = 4     //! rhs starts and ends inside
    };

//=======================================================================================
//! A StationRange is a context object carrying the start and end stations
//! of a section of a RoadRange alignment
//=======================================================================================
struct StationRange
{
    double startStation;
    double endStation;

    StationRange() :startStation(NAN), endStation(NAN) {}
    StationRange(double start, double end) :startStation(start), endStation(end) {}

    bool operator==(StationRangeCR rhs) const { return (startStation == rhs.startStation) && (endStation == rhs.endStation); }

    double Distance() const { return fabs(endStation - startStation); }
    bool IsValid() const { return (!isnan(startStation) && !isnan(endStation) && startStation <= endStation); }

    bool ContainsInclusive(double val) const { return (val >= startStation && val <= endStation); }
    bool ContainsExclusive(double val) const { return (val > startStation && val < endStation); }

    void Extend(StationRangeCR rhs)
        {
        if (!rhs.IsValid())
            return;

        startStation = MIN(startStation, rhs.startStation);
        endStation = MAX(endStation, rhs.endStation);
        }

    //! Calculates the distance to the closest station of the range
    //! @remarks returns 0.0 if the value is inside the range
    double DistanceFromRange(double val) const
        {
        if (!IsValid())
            return -1.0;

        if (ContainsInclusive(val))
            return 0.0;

        if (startStation > val)
            return fabs(startStation - val);

        if (endStation < val)
            return fabs(endStation - val);

        BeAssert(0);
        return 0.0;
        }


    // Return the enumeration as this compares to the stationRange argument 'rhs'
    StationRangeOverlap Overlaps(StationRangeCR stationRange) const { return Overlaps(*this, stationRange); }
    static StationRangeOverlap Overlaps(StationRangeCR stationRange1, StationRangeCR stationRange2)
        {
        // start is in
        bool startin = false; bool endin = false;
        if (stationRange2.startStation >= stationRange1.startStation && stationRange2.startStation <= stationRange1.endStation)
            startin = true;
        if (stationRange2.endStation >= stationRange1.startStation && stationRange2.endStation <= stationRange1.endStation)
            endin = true;
        if (startin && endin)
            return StationRangeOverlap::FullyEnvelops;
        if (startin)
            return StationRangeOverlap::RightOverlap;
        if (endin)
            return StationRangeOverlap::LeftOverlap;

        startin = false; endin = false;
        if (stationRange1.startStation >= stationRange2.startStation && stationRange1.startStation <= stationRange2.endStation)
            startin = true;
        if (stationRange1.endStation >= stationRange2.startStation && stationRange1.endStation <= stationRange2.endStation)
            endin = true;
        if (startin && endin)
            return StationRangeOverlap::FullyEncapsulated;
        if (startin)
            return StationRangeOverlap::LeftOverlap;
        if (endin)
            return StationRangeOverlap::RightOverlap;

        return StationRangeOverlap::NoOverlap;
        }
}; // StationRange

//=======================================================================================
//! A StationRangeEdit is a context object carrying information about a 
//! station change in a section of a RoadRange alignment
//=======================================================================================
struct StationRangeEdit
{
    StationRange preEditRange;
    StationRange postEditRange;

    StationRangeEdit() {}
    StationRangeEdit(StationRangeCR stationRange) : preEditRange(stationRange), postEditRange(stationRange) {}
    StationRangeEdit(StationRangeCR preEdit, StationRangeCR postEdit) : preEditRange(preEdit), postEditRange(postEdit) {}

    double Delta() const { return postEditRange.Distance() - preEditRange.Distance(); }
    double Ratio() const
        {
        if (preEditRange.Distance() == 0.0) return 1.0;
        return postEditRange.Distance() / preEditRange.Distance();
        }
}; // StationRangeEdit

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE