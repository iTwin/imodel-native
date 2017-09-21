/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentPair.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignment.h"
#include "GeometryDebug.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AlignmentPair : NonCopyableClass, RefCountedBase
{
    //! Unit conversion factors
    ROADRAILALIGNMENT_EXPORT static constexpr const double MetersToEnglishFeet = 3.28083989501;
    ROADRAILALIGNMENT_EXPORT static constexpr const double MetersToEnglishSurveyFeet = 3.2808333333465;

    enum class CurveVectorType
        {
        CURVE_VECTOR_Horizontal = 0,
        CURVE_VECTOR_Vertical = 1
        };

private:
    static constexpr const double DefaultMaxStrokeLength = 0.03;

protected:
    CurveVectorPtr m_horizontalCurveVector;
    CurveVectorPtr m_verticalCurveVector;

    mutable CurveVectorWithDistanceIndexPtr m_hzIndex;
    mutable CurveVectorWithDistanceIndexPtr m_vtIndex;
    mutable CurveVectorWithXIndexPtr m_vtXIndex;

protected:
    //! Create and cache the index vector on demand
    CurveVectorWithDistanceIndexPtr HorizontalIndexVector() const;
    CurveVectorWithDistanceIndexPtr VerticalIndexVector() const;
    CurveVectorWithXIndexPtr VerticalXIndexVector() const;

    //! Returns the converted vector
    CurveVectorPtr GetConvertedCurveVector(CurveVectorCR cv, double convFactor) const;

    //! Returns the distance along the alignment to the referencePoint
    double DistanceAlongFromStart(CurveLocationDetailCR location) const;
    CurveVectorPtr GetPartialAlignment(CurveVectorType cvType, double startDistanceAlongFromStart, double endDistanceAlongFromStart) const;

protected:
    ROADRAILALIGNMENT_EXPORT AlignmentPair() { }
    ROADRAILALIGNMENT_EXPORT AlignmentPair(CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment);

public:
    //! Return the 3D start and end points of this alignment
    //! @remarks If the start or end doesn't hit vt alignment, its z-value is set to 0.0
    ROADRAILALIGNMENT_EXPORT void GetStartEnd(DPoint3dR startPt, DPoint3dR endPt) const;
    //! Returns the start and end station of the hz alignment
    ROADRAILALIGNMENT_EXPORT void GetStartAndEndDistancesAlong(double& startDistanceAlong, double& endDistanceAlong) const;

    //! Returns the length of the hz alignment
    ROADRAILALIGNMENT_EXPORT double LengthXY() const;
    ROADRAILALIGNMENT_EXPORT ValidatedDPoint3d GetPointAt(double distanceAlongFromStart) const;
    ROADRAILALIGNMENT_EXPORT ValidatedDPoint3d GetPointAtAndOffset(double distanceAlongFromStart, double offset) const;
    ROADRAILALIGNMENT_EXPORT ValidatedDPoint3d GetPointAtWithZ(double distanceAlongFromStart) const;
    ROADRAILALIGNMENT_EXPORT bool GetPointAndTangentAt(DPoint3dR locationPoint, DVec3dR hzTangent, double distanceAlongFromStart) const;
    ROADRAILALIGNMENT_EXPORT bool GetPointAndTangentAtWithZ(DPoint3dR locationPoint, DVec3dR hzTangent, double distanceAlongFromStart) const;

    //! Returns the elevation at the given station
    //! @remarks if distanceAlongFromStart is outside the vertical alignment, returns 0.0
    ROADRAILALIGNMENT_EXPORT double GetVerticalElevationAt(double distanceAlongFromStart) const;

    //! Creates a stroked 3d alignment using the provided max stroke length.
    //! @remarks For alignment without vertical, the horizontal alignment should be used instead
    ROADRAILALIGNMENT_EXPORT bvector<DPoint3d> GetStrokedAlignment(double maxStrokeLength = DefaultMaxStrokeLength) const;

    //! Return a point on the alignment relative to the reference point if there is one
    //! @remarks UNBOUNDED. If referencePoint is before or after alignment, returns start or end point
    //! @remarks optionally returns the CurvePrimitiveType
    ROADRAILALIGNMENT_EXPORT bool ClosestPoint(DPoint3dR locationPoint, DPoint3dCR referencePoint, ICurvePrimitive::CurvePrimitiveType* pType = nullptr) const;

    //! Return a point on the alignment relative to the reference point if there is one
    //! @remarks UNBOUNDED. If referencePoint is before or after alignment, returns start or end point
    ROADRAILALIGNMENT_EXPORT bool ClosestPointXY(DPoint3dR locationPoint, DPoint3dCR referencePoint) const;

    //! Returns the point and tangent on the alignment relative to the reference point if there is one
    //! @remarks UNBOUNDED. If referencePoint is before or after alignment, returns start or end point
    ROADRAILALIGNMENT_EXPORT bool ClosestPointAndTangentXY(DPoint3dR locationPoint, DVec3dR tangent, DPoint3dCR referencePoint);

    //! Get the horizontal distance from the start of the alignment, the reference point does not need to be on the alignment
    //! @remarks UNBOUNDED. If referencePoint is before or after alignment, returns start or end station
    //! optionally get the offset to the reference point
    //! @return -1 if failure
    ROADRAILALIGNMENT_EXPORT double HorizontalDistanceAlongFromStart(DPoint3dCR referencePoint, double* pOffset = nullptr) const;
    ROADRAILALIGNMENT_EXPORT double HorizontalDistanceAlongFromEnd(DPoint3dCR referencePoint, double* pOffset = nullptr) const;
    //! Returns the primitive that is found at this point, the reference point does not need to be on the alignment
    ROADRAILALIGNMENT_EXPORT ICurvePrimitiveCPtr GetPrimitiveAtPoint(DPoint3dCR referencePoint) const;

    // partial alignments
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr GetPartialHorizontalAlignment(DPoint3dCR fromPt, DPoint3dCR toPt) const;
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr GetPartialHorizontalAlignment(double startDistanceAlongFromStart, double endDistanceAlongFromStart) const;
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr GetPartialVerticalAlignment(double startDistanceAlongFromStart, double endDistanceAlongFromStart) const;

    ROADRAILALIGNMENT_EXPORT AlignmentPairPtr GetPartialAlignment(DPoint3dCR fromPt, DPoint3dCR toPt) const;
    ROADRAILALIGNMENT_EXPORT virtual AlignmentPairPtr GetPartialAlignment(double startDistanceAlongFromStart, double endDistanceAlongFromStart) const;

#if 1 //&&AG NEEDSWORK TODO this method is fairly inconsistent
    //! Should be used only when you need a curve vector for display or persistence
    //! @remarks when called with MetricMeters, returns a reference to the curves used by the RoadAlignment
    //! @remarks when called with a different unit, returns a deep copy of the curve properly scaled
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr HorizontalCurveVector(Dgn::StandardUnit unit = Dgn::StandardUnit::MetricMeters) const;
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr VerticalCurveVector(Dgn::StandardUnit unit = Dgn::StandardUnit::MetricMeters) const;
#endif

    //! Returns a deep-copy of the structure
    ROADRAILALIGNMENT_EXPORT AlignmentPairPtr Clone() const;

    //! Update internal geometry and clear cached objects
    ROADRAILALIGNMENT_EXPORT void UpdateCurveVectors(CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment);
    ROADRAILALIGNMENT_EXPORT void UpdateHorizontalCurveVector(CurveVectorCR horizontalAlignment);
    ROADRAILALIGNMENT_EXPORT void UpdateVerticalCurveVector(CurveVectorCP pVerticalAlignment);

    // Find intersection of two alignments in XY. Optionally returns distanceAlong of the intersect. Also optionally locates the intersection
    // closest to the given point if there are multiple intersects (otherwise it will return the first intersect it finds)
    ROADRAILALIGNMENT_EXPORT bool ComputeIntersectionWith(DPoint3dR result, AlignmentPairCP second, DPoint3dCP nearestToReference = nullptr,
        double * primaryDistanceAlongFromStart = nullptr, double * secondaryDistanceAlongFromStart = nullptr);

    //! Safely transform curves that have partial alignments.
    //! @remarks also works with regular curves
    ROADRAILALIGNMENT_EXPORT static void TransformCurveWithPartialAlignments(CurveVectorR curve, TransformCR transform);

public:
    ROADRAILALIGNMENT_EXPORT static AlignmentPairPtr Create(CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment);
}; // AlignmentPair

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE