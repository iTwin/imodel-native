/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentPair.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignmentApi.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

struct AlignmentPair : NonCopyableClass, RefCountedBase
{
ROADRAILALIGNMENT_EXPORT static const double MetersToEnglishFeet;
ROADRAILALIGNMENT_EXPORT static const double MetersToEnglishSurveyFeet;

enum CurveVectorType
    {
    CURVE_VECTOR_Horizontal = 0,
    CURVE_VECTOR_Vertical = 1
    };

private:
    CurveVectorPtr m_horizontalCurveVector;
    CurveVectorPtr m_verticalCurveVector;

    mutable CurveVectorWithDistanceIndexPtr m_hzIndex;
    mutable CurveVectorWithDistanceIndexPtr m_vtIndex;
    mutable CurveVectorWithXIndexPtr m_vtXIndex;

protected:
    //! Create and cache the index vector on demand
    CurveVectorWithDistanceIndexPtr _HorizontalIndexVector() const;
    CurveVectorWithDistanceIndexPtr _VerticalIndexVector() const;
    CurveVectorWithXIndexPtr _VerticalXIndexVector() const;

    CurveVectorPtr _ConvertedHorizontalVector(double convFactor) const;
    CurveVectorPtr _ConvertedVerticalVector(double convFactor) const;

    //! Return the distance along the alignment to the referencePoint
    double _DistanceFromStart(CurveLocationDetailCR location) const;
    CurveVectorPtr _GetPartialAlignment(CurveVectorType cvType, double startStation, double endStation) const;

protected:
    ROADRAILALIGNMENT_EXPORT AlignmentPair() { }
    ROADRAILALIGNMENT_EXPORT AlignmentPair(CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment);

public:
    //! Return the 3D start and end points of this alignment
    ROADRAILALIGNMENT_EXPORT void GetStartEnd(DPoint3dR startPt, DPoint3dR endPt) const;
    ROADRAILALIGNMENT_EXPORT void GetStartAndEndStations(double& startStation, double& endStation) const;

    ROADRAILALIGNMENT_EXPORT double LengthXY() const;
    ROADRAILALIGNMENT_EXPORT DPoint3d GetPointFromStation(double sta) const;
    ROADRAILALIGNMENT_EXPORT DPoint3d GetPointFromStationWithZ(double sta) const;
    ROADRAILALIGNMENT_EXPORT BentleyStatus GetPointAndTangentFromStation(DPoint3dR point, DVec3dR tangent, double sta) const;
    ROADRAILALIGNMENT_EXPORT BentleyStatus GetPointAndTangentFromStationWithZ(DPoint3dR point, DVec3dR tangent, double sta) const;
    ROADRAILALIGNMENT_EXPORT DPoint3d GetPointFromStationAndOffset(double sta, double offset) const;
    ROADRAILALIGNMENT_EXPORT double GetVerticalElevationAtStation(double sta) const;

    //! Return a point on the alignment relative to the reference point if there is one
    ROADRAILALIGNMENT_EXPORT bool ClosestPoint(DPoint3dR locationPoint, DPoint3dCR referencePoint) const;
    ROADRAILALIGNMENT_EXPORT bool ClosestPoint(DPoint3dR locationPoint, DPoint3dCR referencePoint, ICurvePrimitive::CurvePrimitiveType& curveType) const;

    //! Return a point on the alignment relative to the reference point if there is one
    ROADRAILALIGNMENT_EXPORT bool ClosestPointXY(DPoint3dR locationPoint, DPoint3dCR referencePoint) const;
    ROADRAILALIGNMENT_EXPORT bool ClosestPointAndTangentXY(DPoint3dR locationPoint, DPoint3dCR referencePoint, DVec3dR tangent);
    //! Return a point on the alignment relative to the reference point if there is one
    ROADRAILALIGNMENT_EXPORT bool ClosestPointXY(DPoint3dR locationPoint, DPoint3dCR referencePoint, ICurvePrimitive::CurvePrimitiveType& curveType) const;

    //! Get the horizontal distance from the start of the alignment, the reference point does not need to be on the alignment,
    //! optionally get the offet to the reference point
    //! @return -1 if failure
    ROADRAILALIGNMENT_EXPORT double HorizontalDistanceFromStart(DPoint3dCR referencePoint, double * pOffset = nullptr) const;
    ROADRAILALIGNMENT_EXPORT double HorizontalDistanceFromEnd(DPoint3dCR referencePoint, double * pOffset = nullptr) const;
    // return the primitive that is found at this point
    ROADRAILALIGNMENT_EXPORT const ICurvePrimitivePtr GetPrimitiveAtPoint(DPoint3dCR referencePoint) const;

    ROADRAILALIGNMENT_EXPORT AlignmentPairPtr Clone() const;

    // partial alignments
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr GetPartialHorizontalAlignment(DPoint3dCR fromPt, DPoint3dCR toPt) const;
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr GetPartialHorizontalAlignment(double startStation, double endStation) const;
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr GetPartialVerticalAlignment(double startStation, double endStation) const;

    ROADRAILALIGNMENT_EXPORT AlignmentPairPtr GetPartialAlignment(DPoint3dCR fromPt, DPoint3dCR toPt) const;
    ROADRAILALIGNMENT_EXPORT virtual AlignmentPairPtr GetPartialAlignment(double startStation, double endStation) const;

    ROADRAILALIGNMENT_EXPORT bvector<DPoint3d> GetStrokedAlignment() const;

    //! Should be used only when you need a curve vector for display or persistenc
    ROADRAILALIGNMENT_EXPORT const CurveVectorPtr HorizontalCurveVector(Dgn::StandardUnit unit = Dgn::StandardUnit::MetricMeters) const;
    ROADRAILALIGNMENT_EXPORT const CurveVectorPtr VerticalCurveVector(Dgn::StandardUnit unit = Dgn::StandardUnit::MetricMeters) const;

    //! Update internal geometry and clear cached objects
    ROADRAILALIGNMENT_EXPORT void UpdateCurveVectors(CurveVectorCP horizontalAlignment, CurveVectorCP verticalAlignment);
    ROADRAILALIGNMENT_EXPORT void UpdateHorizontalCurveVector(CurveVectorCP horizontalAlignment);
    ROADRAILALIGNMENT_EXPORT void UpdateVerticalCurveVector(CurveVectorCP verticalAlignment);

    // Find intersection of two alignments in XY. Optionally returns station of the intersect. Also optionally locates the intersection
    // closest to the given point if there are multiple intersects (otherwise it will return the first intersect it finds)
    ROADRAILALIGNMENT_EXPORT bool ComputeIntersectionWith(DPoint3dR result, AlignmentPairCP second, DPoint3dCP nearestToReference = nullptr,
        double * primaryStation = nullptr, double * secondaryStation = nullptr);

public:
    ROADRAILALIGNMENT_EXPORT static AlignmentPairPtr Create(CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment);
}; // AlignmentPair

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE