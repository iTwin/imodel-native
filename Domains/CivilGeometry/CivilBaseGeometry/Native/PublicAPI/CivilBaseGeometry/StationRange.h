/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CivilBaseGeometry.h"

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_CIVILGEOMETRY_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(StationRange)
DEFINE_POINTER_SUFFIX_TYPEDEFS(StationRangeEdit)

//=======================================================================================
// Possible overlap types: (refer to comment in StationRange.h)
//
// IsLeftOf
// |---|
//       |---|
//
// IsLeftAdjacentOf
// |---|
//     |---|
//
// IsRightOf
//       |---|
// |---|
//
// IsRightAdjacentOf
//     |---|
// |---|
//
// IsLeftOverlapOf
// |---|
//    |---|
//
// IsRightOverlapOf
//    |---|
// |---|
//
// IsLeftEncapsulatedIn
// |---|
// |-------|
//
// IsRightEncapsulatedIn
//     |---|
// |-------|
//
// IsFullyEncapsulatedIn
//   |---|
// |-------|
//
// ContainsExclusive
// |-------|
//   |---|
//
// ContainsInclusive
// |-------| or |-------| or |-------| but not  |---|
//   |---|      |---|            |---|          |---|
//=======================================================================================
enum class StationRangeOverlap
    {
    InvalidRange,           //< there is at least 1 invalid range for computation
    IsLeftOf,               //< rhs is after
    IsLeftAdjacentOf,       //< rhs starts where current ends and ends after
    IsRightOf,              //< rhs is before
    IsRightAdjacentOf,      //< rhs starts before and ends where current starts
    IsLeftOverlapOf,        //< rhs starts inside and ends after
    IsRightOverlapOf,       //< rhs starts before and ends inside
    IsLeftEncapsulatedIn,   //< rhs starts where current starts and ends after
    IsRightEncapsulatedIn,  //< rhs starts before current and ends where current ends
    IsFullyEncapsulatedIn,  //< rhs starts before and ends after current,
    ContainsExclusive,      //< rhs strictly starts and ends inside current (no edge overlap)
    ContainsInclusive,      //< rhs starts and ends inside current
    EqualRange              //< self-explanatory
    };
//=======================================================================================
// @bsiclass
// Wrapper that helps interpreting Overlap results
//=======================================================================================
struct StationRangeOverlapDetail
{
friend struct StationRange;

protected:
    StationRangeOverlap m_overlap;

public:
    explicit StationRangeOverlapDetail(StationRangeOverlap overlap) : m_overlap(overlap) {}

    StationRangeOverlap GetOverlap() const { return m_overlap; }

    // Ranges are equal
    bool IsEqualRange() const { return StationRangeOverlap::EqualRange == m_overlap; }

    // Ranges do not meet
    bool IsNoOverlap() const { return StationRangeOverlap::IsLeftOf == m_overlap || StationRangeOverlap::IsRightOf == m_overlap; }

    // Ranges meet at a single location (start or end)
    bool IsSinglePointOverlap() const { return StationRangeOverlap::IsLeftAdjacentOf == m_overlap || StationRangeOverlap::IsRightAdjacentOf == m_overlap; }

    // Ranges do not meet, or meet at a single location (start or end)
    bool IsSinglePointOrNoOverlap() const { return IsNoOverlap() || IsSinglePointOverlap(); }

    // Current range is strictly contained inside other range
    // rhs.startStation < this < rhs.endStation
    bool IsEncapsulatedExclusive() const { return StationRangeOverlap::IsFullyEncapsulatedIn == m_overlap; }

    // Current range is contained inside other range including cases where ranges start/end at the same location
    // rhs.startStation <= this <= rhs.endStation
    bool IsEncapsulatedInclusive() const { return StationRangeOverlap::IsLeftEncapsulatedIn == m_overlap || StationRangeOverlap::IsRightEncapsulatedIn == m_overlap || StationRangeOverlap::IsFullyEncapsulatedIn == m_overlap; }

    // Ranges are equal, or current range is contained inside other range including cases where ranges start/end at the same location
    bool IsEqualOrEncapsulatedInclusive() const { return IsEqualRange() || IsEncapsulatedInclusive(); }

    // Current range strictly contains other range
    // this.startStation < rhs < this.endStation
    bool ContainsExclusive() const { return StationRangeOverlap::ContainsExclusive == m_overlap; }
    // Current range contains other range
    // this.startStation <= rhs <= this.endStation
    bool ContainsInclusive() const { return StationRangeOverlap::ContainsInclusive == m_overlap || StationRangeOverlap::ContainsExclusive == m_overlap; }

    // Ranges are equal, or current range fully contains other range including cases where ranges start/end at the same location
    bool IsEqualOrContainsInclusive() const { return IsEqualRange() || ContainsInclusive(); }
};

//=======================================================================================
// @bsiclass
// StationRange is an object carrying start and end stations
//=======================================================================================
struct StationRange
{
public:
    double startStation;
    double endStation;

public:
    // Creates the object with (reversed) infinite range
    StationRange() : startStation(DBL_MAX), endStation(-DBL_MAX) {}
    StationRange(double station) : startStation(station), endStation(station) {}
    StationRange(double start, double end) : startStation(start), endStation(end) {}

    // Should always do comparisons using the appropriate method
    bool operator==(StationRangeCR rhs) const = delete;

    bool IsValid() const { return DBL_MAX != startStation && -DBL_MAX != endStation && startStation <= endStation; }

    // Returns the Length of the range
    double Length() const { return fabs(endStation - startStation); }
    // Returns the Center of the range
    double Center() const { return 0.5 * (startStation + endStation); }

    bool ContainsInclusive(double val) const { return (val >= startStation && val <= endStation); }
    bool ContainsExclusive(double val) const { return (val > startStation && val < endStation); }

    // Extends the range
    CIVILBASEGEOMETRY_EXPORT void Extend(double value);
    // Extends the range
    CIVILBASEGEOMETRY_EXPORT void Extend(StationRangeCR rhs);
    // Returns the (signed) distance to the range
    //! @remarks returns 0.0 if the station is inside the range
    CIVILBASEGEOMETRY_EXPORT double SignedDistance(double station) const;

    // Returns information about the overlap of the two ranges
    // @remarks this method assumes ascending and non-null ranges
    CIVILBASEGEOMETRY_EXPORT StationRangeOverlapDetail Overlaps(StationRangeCR rhs) const;

}; // StationRange

//__PUBLISH_SECTION_END__
//=======================================================================================
//! A StationRangeEdit is a context object carrying information about a 
//! station change in a section of a RoadRange alignment
//=======================================================================================
struct StationRangeEdit
{
public:
    StationRange preEditRange;
    StationRange postEditRange;

public:
    StationRangeEdit() {}
    StationRangeEdit(StationRangeCR stationRange) : preEditRange(stationRange), postEditRange(stationRange) {}
    StationRangeEdit(StationRangeCR preEdit, StationRangeCR postEdit) : preEditRange(preEdit), postEditRange(postEdit) {}

    double Delta() const { return postEditRange.Length() - preEditRange.Length(); }
    double Ratio() const { return (0.0 == preEditRange.Length()) ? 1.0 : postEditRange.Length() / preEditRange.Length(); }

}; // StationRangeEdit
//__PUBLISH_SECTION_START__
END_BENTLEY_CIVILGEOMETRY_NAMESPACE
