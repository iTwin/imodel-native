/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "CivilBaseGeometryInternal.h"
#include <CivilBaseGeometry/StationRange.h>


//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void StationRange::Extend(double value)
    {
    if (startStation > value)
        startStation = value;

    if (endStation < value)
        endStation = value;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void StationRange::Extend(StationRangeCR rhs)
    {
    Extend(rhs.startStation);
    Extend(rhs.endStation);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
double StationRange::SignedDistance(double station) const
    {
    if (ContainsInclusive(station))
        return 0.0;

    // Negative
    if (station < startStation)
        return station - startStation;

    // Positive
    return station - endStation;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
StationRangeOverlapDetail StationRange::Overlaps(StationRangeCR rhs) const
    {
    if (!IsValid() || !rhs.IsValid())
        return StationRangeOverlapDetail(StationRangeOverlap::InvalidRange);

    if (endStation < rhs.startStation)
        return StationRangeOverlapDetail(StationRangeOverlap::IsLeftOf);
    if (endStation == rhs.startStation)
        return StationRangeOverlapDetail(StationRangeOverlap::IsLeftAdjacentOf);

    if (startStation > rhs.endStation)
        return StationRangeOverlapDetail(StationRangeOverlap::IsRightOf);
    if (startStation == rhs.endStation)
        return StationRangeOverlapDetail(StationRangeOverlap::IsRightAdjacentOf);

    if (startStation < rhs.startStation)
        {
        // RHS starts inside
        if (endStation < rhs.endStation)
            return StationRangeOverlapDetail(StationRangeOverlap::IsLeftOverlapOf);
        if (endStation > rhs.endStation)
            return StationRangeOverlapDetail(StationRangeOverlap::ContainsExclusive);
        
        return StationRangeOverlapDetail(StationRangeOverlap::ContainsInclusive);
        }
    if (startStation == rhs.startStation)
        {
        // RHS starts at the same place
        if (endStation < rhs.endStation)
            return StationRangeOverlapDetail(StationRangeOverlap::IsLeftEncapsulatedIn);
        if (endStation == rhs.endStation)
            return StationRangeOverlapDetail(StationRangeOverlap::EqualRange);

        return StationRangeOverlapDetail(StationRangeOverlap::ContainsInclusive);
        }

    // startStation > rhs.startStation
    // RHS starts before
    if (endStation < rhs.endStation)
        return StationRangeOverlapDetail(StationRangeOverlap::IsFullyEncapsulatedIn);
    if (endStation == rhs.endStation)
        return StationRangeOverlapDetail(StationRangeOverlap::IsRightEncapsulatedIn);

    return StationRangeOverlapDetail(StationRangeOverlap::IsRightOverlapOf);
    }
