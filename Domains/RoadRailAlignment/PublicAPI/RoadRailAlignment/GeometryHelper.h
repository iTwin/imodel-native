/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/GeometryHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignment.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! Helper class for horizontal and vertical geometry.
//=======================================================================================
struct GeometryHelper
{
public:

    ROADRAILALIGNMENT_EXPORT static bool ClosestPoint(CurveVectorPtr const& hz, DPoint3dCR pickPt, DPoint3dR retPt, DMatrix4dCR localToView = DMatrix4d());
    ROADRAILALIGNMENT_EXPORT static CurveVectorPtr GetPartialAlignment(CurveVectorPtr const& hz, DPoint3dCR fromPt, DPoint3dCR toPt, DMatrix4dCR localToView = DMatrix4d());
    ROADRAILALIGNMENT_EXPORT static CurveVectorPtr GetPartialAlignment(CurveVectorWithDistanceIndex const& hzIdx, double startStation, double endStation);
    ROADRAILALIGNMENT_EXPORT static CurveVectorPtr GetPartialVerticalAlignment(CurveVectorWithDistanceIndex const& vtIdx, double startStation, double endStation, double startStationOnResult = 0.0);
    ROADRAILALIGNMENT_EXPORT static double DistanceFromStart(CurveVectorCR hz, DPoint3dCR atPt, DMatrix4dCP localToView = nullptr, double * pOffset = nullptr);
    ROADRAILALIGNMENT_EXPORT static double DistanceFromStart (CurveVectorWithDistanceIndexPtr const& hzIdx, DPoint3dCR atPt);
    ROADRAILALIGNMENT_EXPORT static double GetVerticalElevationAtStation (CurveVectorCR vtAlign, double sta);
    ROADRAILALIGNMENT_EXPORT static double GetVerticalElevationAtStation(CurveVectorWithDistanceIndexPtr const& vtIdx, double sta);
    ROADRAILALIGNMENT_EXPORT static double GetVerticalAlignementLength (CurveVectorCR vtAlign);
    ROADRAILALIGNMENT_EXPORT static DPoint3d GetPointFromStation(CurveVectorWithDistanceIndexPtr const& hzIdx, double sta);
    ROADRAILALIGNMENT_EXPORT static DPoint3d GetPointFromStation (CurveVectorCR hzAlign, double sta);
    ROADRAILALIGNMENT_EXPORT static BentleyStatus GetPointAndTangentFromStation(DPoint3dR point, DVec3dR tangent, CurveVectorWithDistanceIndexPtr const& hzIdx, double sta);
    ROADRAILALIGNMENT_EXPORT static BentleyStatus GetPointAndTangentFromStationWithZ (DPoint3dR point, DVec3dR tangent, CurveVectorWithDistanceIndexPtr const& hzIdx,
                                                                                                                        CurveVectorWithDistanceIndexPtr const& vtIdx, double sta);
    
    ROADRAILALIGNMENT_EXPORT static IFacetOptionsPtr CreateDefaultFacetOptionsForHzAlign(CurveVectorCR hzAlign);
    ROADRAILALIGNMENT_EXPORT static bvector<DPoint3d> GetStrokedAlignment(CurveVectorPtr hzAlign, CurveVectorPtr vtAlign);
    ROADRAILALIGNMENT_EXPORT static bvector<PathLocationDetailPair> GetStrokedAlignmentLocationPairs(CurveVectorR hzAlign, CurveVectorR vtAlign, IFacetOptionsCR options);
    ROADRAILALIGNMENT_EXPORT static bool IntersectCurvesXY(bvector<DPoint3d>& intersections, CurveVectorR curve1, CurveVectorR curve2);

    //! Returns a linestring
    ROADRAILALIGNMENT_EXPORT static CurveVectorPtr GetInterpolateOffsetCurve(CurveVectorPtr curve, double startOffset, double endOffset, double interpolateStepDistance = 2.0);
    //ROADRAILALIGNMENT_EXPORT static CurveVectorPtr CreateVerticalAlignmentFromHorizontalAndDrape(CurveVectorCR hzAlign, ConceptualDrapeCR draper);

    ROADRAILALIGNMENT_EXPORT static bool IsOdd (size_t n);

    // Debug method, just useful to capture curvevectors in a form where we can send them to Earlin
    ROADRAILALIGNMENT_EXPORT static Utf8String SerializeCurveVector(CurveVectorPtr curveVector);
    ROADRAILALIGNMENT_EXPORT static CurveVectorPtr DeserializeCurveVector(Utf8CP serializedJson);

    ROADRAILALIGNMENT_EXPORT static CurveVectorPtr CloneOffsetCurvesXYNoBSpline (CurveVectorPtr originarlCurve, CurveOffsetOptions options);
 };

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE