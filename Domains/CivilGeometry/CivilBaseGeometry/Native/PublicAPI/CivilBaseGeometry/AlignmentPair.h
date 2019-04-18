/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "CivilBaseGeometry.h"

BEGIN_BENTLEY_CIVILGEOMETRY_NAMESPACE

#define CS_SPI_INFINITY ((double)1.0E+38)
#define CS_PVI_INFINITY ((double)1.0E+38)

//=======================================================================================
// @bsiclass
// Access queries for alignment geometry.
// An AlignmentPair combines the horizontal and vertical geometry of an alignment
//=======================================================================================
struct AlignmentPair : NonCopyableClass, RefCountedBase
{
public:
    static constexpr const double MetersToEnglishFeet = 3.28083989501;
    static constexpr const double MetersToEnglishSurveyFeet = 3.2808333333465;
    static constexpr const double DefaultMaxStrokeLength = 0.03;

private:
    CurveVectorPtr m_horizontalCurveVector;
    CurveVectorPtr m_verticalCurveVector;

    mutable CurveVectorWithDistanceIndexPtr m_hzIndex;
    mutable CurveVectorWithXIndexPtr m_vtXIndex;

protected:
    //! Create and cache the index vector on demand
    CurveVectorWithDistanceIndexPtr HorizontalIndexVector() const;
    CurveVectorWithXIndexPtr VerticalXIndexVector() const;

    //! Returns a scaled deep copy of a curve vector
    CurveVectorPtr GetScaledCurveVector(CurveVectorCR cv, double scale) const;

    //! Returns the distance along the alignment to the referencePoint
    double DistanceAlongFromStart(CurveLocationDetailCR location) const;

    enum class CurveVectorType { Horizontal, Vertical };
    CurveVectorPtr GetPartialAlignment(CurveVectorType type, double startDistanceAlongFromStart, double endDistanceAlongFromStart) const;

protected:
    virtual ~AlignmentPair() {}
    AlignmentPair() {}
    CIVILBASEGEOMETRY_EXPORT AlignmentPair(CurveVectorCP pHorizontalAlignment, CurveVectorCP pVerticalAlignment);
    CIVILBASEGEOMETRY_EXPORT virtual AlignmentPairPtr _Clone() const;
    CIVILBASEGEOMETRY_EXPORT virtual void _UpdateHorizontalCurveVector(CurveVectorCP pHorizontalAlignment);
    CIVILBASEGEOMETRY_EXPORT virtual void _UpdateVerticalCurveVector(CurveVectorCP pVerticalAlignment);

public:
    // Multi-Thread support. Initialize the lazily created member variables
    // Do this before we run any multithreaded calls on this class
    CIVILBASEGEOMETRY_EXPORT void InitLazyCaches() const;

    //! Allocate an AlignmentPair object.
    //! @remarks The constructor will deep copy both horizontal and vertical alignments in parameter
    CIVILBASEGEOMETRY_EXPORT static AlignmentPairPtr Create(CurveVectorCP pHorizontalAlignment, CurveVectorCP pVerticalAlignment);

    //! Returns a const-pointer to the Horizontal Alignment
    CIVILBASEGEOMETRY_EXPORT CurveVectorCP GetHorizontalCurveVector() const;
    //! Returns a const-pointer to the Vertical Alignment or nullptr
    CIVILBASEGEOMETRY_EXPORT CurveVectorCP GetVerticalCurveVector() const;
    //! Returns a deep-copy of the structure
    CIVILBASEGEOMETRY_EXPORT AlignmentPairPtr Clone() const;

    //! Update internal geometry and clear cached objects
    CIVILBASEGEOMETRY_EXPORT void UpdateCurveVectors(CurveVectorCP pHorizontalAlignment, CurveVectorCP pVerticalAlignment);
    CIVILBASEGEOMETRY_EXPORT void UpdateHorizontalCurveVector(CurveVectorCP pHorizontalAlignment);
    CIVILBASEGEOMETRY_EXPORT void UpdateVerticalCurveVector(CurveVectorCP pVerticalAlignment);

    CIVILBASEGEOMETRY_EXPORT bool IsValidHorizontal() const { return m_horizontalCurveVector.IsValid(); }
    CIVILBASEGEOMETRY_EXPORT bool IsValidVertical() const { return m_verticalCurveVector.IsValid(); }
    //! Returns true if the AlignmentPair has spirals
    CIVILBASEGEOMETRY_EXPORT bool HasSpirals() const;
    //! Returns the length of the hz alignment
    CIVILBASEGEOMETRY_EXPORT double LengthXY() const;

    //! Returns the elevation at the given station
    //! @param[in] extendVertical - If true and outside the vertical alignment, returns the closest valid elevation
    CIVILBASEGEOMETRY_EXPORT double GetVerticalElevationAt(double distanceAlongFromStart, bool extendVertical = true) const;

    //! Return the 3D start and end points of this alignment
    //! @param[in] extendVertical - If true and outside the vertical alignment, returns the closest valid elevation
    CIVILBASEGEOMETRY_EXPORT bool GetStartEnd(DPoint3dR startPt, DPoint3dR endPt, bool extendVertical = true) const;
    //! Returns the start and end station of the hz alignment
    CIVILBASEGEOMETRY_EXPORT bool GetStartAndEndDistancesAlong(double& startDistanceAlong, double& endDistanceAlong) const;
    //! Get the point at a given distance along an alignment
    //! @param[in] Distance along the AlignmentPair, in meters
    CIVILBASEGEOMETRY_EXPORT ValidatedDPoint3d GetPointAt(double distanceAlongFromStart) const;

    CIVILBASEGEOMETRY_EXPORT ValidatedDPoint3d GetPointAtAndOffset(double distanceAlongFromStart, double offset) const;
    //! Get the point and tangent at the given distance along the AligmentPair
    //! @param[out] hzPoint The point at a given distance along the AlignmentPair
    //! @param[out] hzTangent The horizontal tangent at the point a given distance along the AlignmentPair
    //! @param[in] distanceAlongFromStart The distance along the alignment, in meters
    CIVILBASEGEOMETRY_EXPORT bool GetPointAndTangentAt(DPoint3dR hzPoint, DVec3dR hzTangent, double distanceAlongFromStart) const;

    //! Returns the 3D point at the given distance from start.
    //! @param[in] extendVertical - If true and outside the vertical alignment, returns the closest valid elevation
    CIVILBASEGEOMETRY_EXPORT ValidatedDPoint3d GetPointAtWithZ(double distanceAlongFromStart, bool extendVertical = true) const;
    //! Returns the 3D point and XY tangent at the given distance from start.
    //! @param[in] extendVertical - If true and outside the vertical alignment, returns the closest valid elevation
    CIVILBASEGEOMETRY_EXPORT bool GetPointAndTangentAtWithZ(DPoint3dR locationPoint, DVec3dR hzTangent, double distanceAlongFromStart, bool extendVertical = true) const;

    //! Creates a stroked 3d alignment using the provided max stroke length.
    //! @remarks For alignment without vertical, the horizontal alignment should be used instead
    CIVILBASEGEOMETRY_EXPORT bvector<DPoint3d> GetStrokedAlignment(double maxStrokeLength = DefaultMaxStrokeLength) const;

    //! Return a point on the alignment relative to the reference point if there is one
    //! @param[in] extendVertical - If true and outside the vertical alignment, returns the closest valid elevation
    //! @remarks UNBOUNDED. If referencePoint is before or after alignment, returns start or end point
    //! @remarks optionally returns the CurvePrimitiveType
    CIVILBASEGEOMETRY_EXPORT bool ClosestPoint(DPoint3dR locationPoint, DPoint3dCR referencePoint, bool extendVertical = true, ICurvePrimitive::CurvePrimitiveType* pType = nullptr) const;

    //! Return a point on the alignment relative to the reference point if there is one
    //! @remarks UNBOUNDED. If referencePoint is before or after alignment, returns start or end point
    CIVILBASEGEOMETRY_EXPORT bool ClosestPointXY(DPoint3dR locationPoint, DPoint3dCR referencePoint) const;

    //! Returns the point and tangent on the alignment relative to the reference point if there is one
    //! @remarks UNBOUNDED. If referencePoint is before or after alignment, returns start or end point
    CIVILBASEGEOMETRY_EXPORT bool ClosestPointAndTangentXY(DPoint3dR locationPoint, DVec3dR tangent, DPoint3dCR referencePoint);

    //! Get the horizontal distance from the start of the alignment, the reference point does not need to be on the alignment
    //! @remarks UNBOUNDED. If referencePoint is before or after alignment, returns start or end station
    //! optionally get the offset to the reference point
    //! @return -1 if failure
    CIVILBASEGEOMETRY_EXPORT double HorizontalDistanceAlongFromStart(DPoint3dCR referencePoint, double* pOffset = nullptr) const;
    CIVILBASEGEOMETRY_EXPORT double HorizontalDistanceAlongFromEnd(DPoint3dCR referencePoint, double* pOffset = nullptr) const;
    //! Returns the horizontal primitive that is found at this point, the reference point does not need to be on the alignment
    CIVILBASEGEOMETRY_EXPORT ICurvePrimitiveCPtr GetPrimitiveAtPoint(DPoint3dCR referencePoint) const;
    //! Returns the horizontal primitive that is found at this station.
    CIVILBASEGEOMETRY_EXPORT ICurvePrimitiveCPtr GetPrimitiveAtStation(double distanceAlongFromStart) const;
    //! Returns the horizontal PathLocationDetail at this station
    CIVILBASEGEOMETRY_EXPORT ValidatedPathLocationDetail GetPathLocationDetailAtStation(double distanceAlongFromStart) const;

    // partial alignments
    //! Get a curve representing a partial horizontal alignment between the \p fromPt and \p toPt
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr GetPartialHorizontalAlignment(DPoint3dCR fromPt, DPoint3dCR toPt) const;
    //! Get a curve representing a partial horizontal alignment between the \p startDistanceAlongFromStart and \p endDistanceAlongFromStart
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr GetPartialHorizontalAlignment(double startDistanceAlongFromStart, double endDistanceAlongFromStart) const;
    //! Get a curve representing a partial vertical alignment between \p startDistanceAlongFromStart and \p endDistanceAlongFromStart
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr GetPartialVerticalAlignment(double startDistanceAlongFromStart, double endDistanceAlongFromStart) const;
    //! Get a partial AlignmentPairPtr created between \p fromPt and \p toPt
    CIVILBASEGEOMETRY_EXPORT AlignmentPairPtr GetPartialAlignment(DPoint3dCR fromPt, DPoint3dCR toPt) const;
    //! Get a partial AlignmentPairPtr created between \p startDistanceAlongFromStart and \p endDistanceAlongFromStart
    CIVILBASEGEOMETRY_EXPORT AlignmentPairPtr GetPartialAlignment(double startDistanceAlongFromStart, double endDistanceAlongFromStart) const;

    //! Returns a deep copy of the Horizontal, optionally converted in a different unit
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr CloneHorizontalCurveVector(double scaleFactor = 1.0) const;
    //! Returns a deep copy of the Vertical, optionally converted in a different unit
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr CloneVerticalCurveVector(double scaleFactor = 1.0) const;

    //! Returns the radius of the smallest arc primitive that turns left on the horizontal alignment
    //! @remarks Only circular arcs are taken into account
    //! @remarks offsetting the alignment to the left (-) with a value greater than this radius will result in invalid geometry
    CIVILBASEGEOMETRY_EXPORT double ComputeLeftMinimumRadius() const;
    //! Returns the radius of the smallest arc primitive that turns right on the horizontal alignment
    //! @remarks Only circular arcs are taken into account
    //! @remarks offsetting the alignment to the right (+) with a value greater than this radius will result in invalid geometry
    CIVILBASEGEOMETRY_EXPORT double ComputeRightMinimumRadius() const;

    // Find intersection of two alignments in XY. Optionally returns distanceAlong of the intersect. Also optionally locates the intersection
    // closest to the given point if there are multiple intersects (otherwise it will return the first intersect it finds)
    CIVILBASEGEOMETRY_EXPORT bool ComputeIntersectionWith(DPoint3dR result, AlignmentPairCP second, DPoint3dCP nearestToReference = nullptr,
        double * primaryDistanceAlongFromStart = nullptr, double * secondaryDistanceAlongFromStart = nullptr);

    //! Safely transform curves that have partial alignments.
    //! @remarks also works with regular curves
    CIVILBASEGEOMETRY_EXPORT static void TransformCurveWithPartialCurves(CurveVectorR curve, TransformCR transform);

    CIVILBASEGEOMETRY_EXPORT double SlopeAtDistanceAlong(const double& station) const;

}; // AlignmentPair

END_BENTLEY_CIVILGEOMETRY_NAMESPACE
