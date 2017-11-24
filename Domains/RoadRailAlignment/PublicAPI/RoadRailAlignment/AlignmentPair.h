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
// Access queries for alignment geometry.
// An AlignmentPair combines the horizontal and vertical of an alignment
//=======================================================================================
struct AlignmentPair : NonCopyableClass, RefCountedBase
{
private:
    static constexpr const double MetersToEnglishFeet = 3.28083989501;
    static constexpr const double MetersToEnglishSurveyFeet = 3.2808333333465;
    static constexpr const double DefaultMaxStrokeLength = 0.03;
    enum class CurveVectorType { Horizontal, Vertical };

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

    //! Returns a transformed CurveVector
    CurveVectorPtr GetConvertedCurveVector(CurveVectorCR cv, double convFactor) const;

    //! Returns the distance along the alignment to the referencePoint
    double DistanceAlongFromStart(CurveLocationDetailCR location) const;
    CurveVectorPtr GetPartialAlignment(CurveVectorType type, double startDistanceAlongFromStart, double endDistanceAlongFromStart) const;

protected:
    virtual ~AlignmentPair() {}
    AlignmentPair() {}
    AlignmentPair(CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment);
    virtual AlignmentPairPtr _Clone() const;

public:
    //! Allocate an AlignmentPair object.
    //! @remarks The constructor will deep copy both horizontal and vertical alignments in parameter
    ROADRAILALIGNMENT_EXPORT static AlignmentPairPtr Create(CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment);

    //! Returns a const-ref to the Horizontal Alignment
    ROADRAILALIGNMENT_EXPORT CurveVectorCR GetHorizontalCurveVector() const;
    //! Returns a const-pointer to the Vertical Alignment or nullptr
    ROADRAILALIGNMENT_EXPORT CurveVectorCP GetVerticalCurveVector() const;
    //! Returns a deep-copy of the structure
    ROADRAILALIGNMENT_EXPORT AlignmentPairPtr Clone() const;

    //! Update internal geometry and clear cached objects
    ROADRAILALIGNMENT_EXPORT void UpdateCurveVectors(CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment);
    ROADRAILALIGNMENT_EXPORT void UpdateHorizontalCurveVector(CurveVectorCR horizontalAlignment);
    ROADRAILALIGNMENT_EXPORT void UpdateVerticalCurveVector(CurveVectorCP pVerticalAlignment);

    bool IsValidVertical() const { return m_verticalCurveVector.IsValid(); }

    //! Returns the length of the hz alignment
    ROADRAILALIGNMENT_EXPORT double LengthXY() const;

    //! Returns the elevation at the given station
    //! @param[in] extendVertical - If true and outside the vertical alignment, returns the closest valid elevation
    ROADRAILALIGNMENT_EXPORT double GetVerticalElevationAt(double distanceAlongFromStart, bool extendVertical = true) const;

    //! Return the 3D start and end points of this alignment
    //! @param[in] extendVertical - If true and outside the vertical alignment, returns the closest valid elevation
    ROADRAILALIGNMENT_EXPORT bool GetStartEnd(DPoint3dR startPt, DPoint3dR endPt, bool extendVertical = true) const;
    //! Returns the start and end station of the hz alignment
    ROADRAILALIGNMENT_EXPORT bool GetStartAndEndDistancesAlong(double& startDistanceAlong, double& endDistanceAlong) const;

    ROADRAILALIGNMENT_EXPORT ValidatedDPoint3d GetPointAt(double distanceAlongFromStart) const;
    ROADRAILALIGNMENT_EXPORT ValidatedDPoint3d GetPointAtAndOffset(double distanceAlongFromStart, double offset) const;
    ROADRAILALIGNMENT_EXPORT bool GetPointAndTangentAt(DPoint3dR hzPoint, DVec3dR hzTangent, double distanceAlongFromStart) const;

    //! Returns the 3D point at the given distance from start.
    //! @param[in] extendVertical - If true and outside the vertical alignment, returns the closest valid elevation
    ROADRAILALIGNMENT_EXPORT ValidatedDPoint3d GetPointAtWithZ(double distanceAlongFromStart, bool extendVertical = true) const;
    //! Returns the 3D point and XY tangent at the given distance from start.
    //! @param[in] extendVertical - If true and outside the vertical alignment, returns the closest valid elevation
    ROADRAILALIGNMENT_EXPORT bool GetPointAndTangentAtWithZ(DPoint3dR locationPoint, DVec3dR hzTangent, double distanceAlongFromStart, bool extendVertical = true) const;

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

    //! Returns a deep copy of the Horizontal, optionally converted in a different unit
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr CloneHorizontalCurveVector(Dgn::StandardUnit unit = Dgn::StandardUnit::MetricMeters) const;
    //! Returns a deep copy of the Vertical, optionally converted in a different unit
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr CloneVerticalCurveVector(Dgn::StandardUnit unit = Dgn::StandardUnit::MetricMeters) const;

    // Find intersection of two alignments in XY. Optionally returns distanceAlong of the intersect. Also optionally locates the intersection
    // closest to the given point if there are multiple intersects (otherwise it will return the first intersect it finds)
    ROADRAILALIGNMENT_EXPORT bool ComputeIntersectionWith(DPoint3dR result, AlignmentPairCP second, DPoint3dCP nearestToReference = nullptr,
        double * primaryDistanceAlongFromStart = nullptr, double * secondaryDistanceAlongFromStart = nullptr);

    //! Safely transform curves that have partial alignments.
    //! @remarks also works with regular curves
    ROADRAILALIGNMENT_EXPORT static void TransformCurveWithPartialCurves(CurveVectorR curve, TransformCR transform);

}; // AlignmentPair

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE