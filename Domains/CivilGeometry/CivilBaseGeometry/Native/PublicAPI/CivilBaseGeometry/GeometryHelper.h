/*--------------------------------------------------------------------------------------+
|
|     $Source: CivilBaseGeometry/Native/PublicAPI/CivilBaseGeometry/GeometryHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "CivilBaseGeometry.h"

BEGIN_BENTLEY_CIVILGEOMETRY_NAMESPACE

//=======================================================================================
//! Helper class for horizontal and vertical geometry.
//=======================================================================================
struct GeometryHelper
{
public:

    CIVILBASEGEOMETRY_EXPORT static bool ClosestPoint(CurveVectorPtr const& hz, DPoint3dCR pickPt, DPoint3dR retPt, DMatrix4dCR localToView = DMatrix4d());
    CIVILBASEGEOMETRY_EXPORT static CurveVectorPtr GetPartialAlignment(CurveVectorPtr const& hz, DPoint3dCR fromPt, DPoint3dCR toPt, DMatrix4dCR localToView = DMatrix4d());
    CIVILBASEGEOMETRY_EXPORT static CurveVectorPtr GetPartialAlignment(CurveVectorWithDistanceIndex const& hzIdx, double startStation, double endStation);
    CIVILBASEGEOMETRY_EXPORT static CurveVectorPtr GetPartialVerticalAlignment(CurveVectorWithDistanceIndex const& vtIdx, double startStation, double endStation, double startStationOnResult = 0.0);
    CIVILBASEGEOMETRY_EXPORT static double DistanceFromStart(CurveVectorCR hz, DPoint3dCR atPt, DMatrix4dCP localToView = nullptr, double * pOffset = nullptr);
    CIVILBASEGEOMETRY_EXPORT static double DistanceFromStart (CurveVectorWithDistanceIndexPtr const& hzIdx, DPoint3dCR atPt);
    CIVILBASEGEOMETRY_EXPORT static double GetVerticalElevationAtStation (CurveVectorCR vtAlign, double sta);
    CIVILBASEGEOMETRY_EXPORT static double GetVerticalElevationAtStation(CurveVectorWithDistanceIndexPtr const& vtIdx, double sta);
    CIVILBASEGEOMETRY_EXPORT static double GetVerticalAlignementLength (CurveVectorCR vtAlign);
    CIVILBASEGEOMETRY_EXPORT static DPoint3d GetPointFromStation(CurveVectorWithDistanceIndexPtr const& hzIdx, double sta);
    CIVILBASEGEOMETRY_EXPORT static DPoint3d GetPointFromStation (CurveVectorCR hzAlign, double sta);
    CIVILBASEGEOMETRY_EXPORT static BentleyStatus GetPointAndTangentFromStation(DPoint3dR point, DVec3dR tangent, CurveVectorWithDistanceIndexPtr const& hzIdx, double sta);
    CIVILBASEGEOMETRY_EXPORT static BentleyStatus GetPointAndTangentFromStationWithZ (DPoint3dR point, DVec3dR tangent, CurveVectorWithDistanceIndexPtr const& hzIdx,
                                                                                                                        CurveVectorWithDistanceIndexPtr const& vtIdx, double sta);
    
    CIVILBASEGEOMETRY_EXPORT static IFacetOptionsPtr CreateDefaultFacetOptionsForHzAlign(CurveVectorCR hzAlign);
    CIVILBASEGEOMETRY_EXPORT static bvector<DPoint3d> GetStrokedAlignment(CurveVectorPtr hzAlign, CurveVectorPtr vtAlign);
    CIVILBASEGEOMETRY_EXPORT static bvector<PathLocationDetailPair> GetStrokedAlignmentLocationPairs(CurveVectorR hzAlign, CurveVectorR vtAlign, IFacetOptionsCR options);
    CIVILBASEGEOMETRY_EXPORT static bool IntersectCurvesXY(bvector<DPoint3d>& intersections, CurveVectorR curve1, CurveVectorR curve2);

    //! Returns a linestring
    CIVILBASEGEOMETRY_EXPORT static CurveVectorPtr GetInterpolateOffsetCurve(CurveVectorPtr curve, double startOffset, double endOffset, double interpolateStepDistance = 2.0);
    //CIVILBASEGEOMETRY_EXPORT static CurveVectorPtr CreateVerticalAlignmentFromHorizontalAndDrape(CurveVectorCR hzAlign, ConceptualDrapeCR draper);

    CIVILBASEGEOMETRY_EXPORT static bool IsOdd (size_t n);

    //__PUBLISH_SECTION_END__
    // Debug method, just useful to capture curvevectors in a form where we can send them to Earlin
    CIVILBASEGEOMETRY_EXPORT static Utf8String SerializeCurveVector(CurveVectorPtr curveVector);
    CIVILBASEGEOMETRY_EXPORT static CurveVectorPtr DeserializeCurveVector(Utf8CP serializedJson);
    //__PUBLISH_SECTION_START__

    CIVILBASEGEOMETRY_EXPORT static CurveVectorPtr CloneOffsetCurvesXYNoBSpline (CurveVectorPtr originarlCurve, CurveOffsetOptions options);

    //! Tests which side the point is relative to the plane (normal of the plane represents the "front" of it, or "inside", where as the "back" is "outside".
    //! @param[in] plane - Plane to test against.
    //! @param[in] pt - Point to test with.
    //! @return Containment of whether the point is "inside" (in front of the plane) or "outside" (behind the plane). If on the plane, it is considered "inside".
    CIVILBASEGEOMETRY_EXPORT static ClipPlaneContainment WhichSide(DPlane3dCR plane, DPoint3dCR pt);

    //! Tests if the plane and the AABB range intersect each other.
    //! @param[in] range - AABB to test against.
    //! @param[in] plane - Plane to test with.
    //! @return True if the plane intersects the AABB, false if otherwise.
    CIVILBASEGEOMETRY_EXPORT static bool IntersectRangePlane(DRange3dCR range, DPlane3dCR plane);

    //! Initializes the range to encompass the input points, but sets the Z values of the range to zero.
    //! @param[out] range - XY range result.
    //! @param[in] points - Points to compute range for.
    CIVILBASEGEOMETRY_EXPORT static void InitXYRange(DRange3dR range, bvector<DPoint3d> const& points);
 };

END_BENTLEY_CIVILGEOMETRY_NAMESPACE