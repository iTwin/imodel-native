/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Interpolate within a polyline.  First and last points of polyline
//! are at parameters 0 and 1.  Intermediate points have parameter
//! spacing 1 / (n2), regardless of physical distance.
//! (Note: This corresponds to a uniform knot order 2 bspline.
//!
//! @param pPoint OUT     interpolated point
//! @param pTangent OUT     forward tangent vector
//! @param pPointArray IN      points in polyline
//! @param n IN      number of points in polyline
//! @param param IN      parametric coordinate
//! @return true if polyline has 1 or more points.
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3d_interpolatePolyline
(
DPoint3dP pPoint,
DPoint3dP pTangent,
DPoint3dCP pPointArray,
int              n,
double           param
);

Public GEOMDLLIMPEXP bool    bsiDPoint3d_interpolatePolyline

(
DPoint3dR point,
DVec3dR tangentA,
DVec3dR tangentB,
DPoint3dCP pPointArray,
int              n,
double           param
);

//!
//! Return the length of the polyline.
//!
//! @param pPointArray IN      points in polyline
//! @param n IN      number of points in polyline
//! @return double
//!
Public GEOMDLLIMPEXP double bsiGeom_polylineLength
(
DPoint3dCP pPointArray,
int              n
);

//!
//! Return the length of the polyline.
//! @parma pLength OUT     computed length.
//! @param pPointArray IN      points in polyline
//! @param pWeights IN      optional weights
//! @param n IN      number of points in polyline
//! @return true if all weights are positive.  If false, returned length is zero.
//!
Public GEOMDLLIMPEXP bool    bsiGeom_positiveWeightedPolylineLength
(
double      *pLength,
DPoint3dCP pPointArray,
double     *pWeights,
int              n
);

//!
//! Interpolate within a polyline.  First point is at arc length 0.
//! Last point is at arc length as returned by bsiGeom_polylineLength.
//! Out of bounds cases are extrapolated.
//!
//! @param pPoint OUT     interpolated point
//! @param pTangent OUT     forward tangent vector unit
//! @param pPointArray IN      points in polyline
//! @param n IN      number of points in polyline
//! @param dist IN      distance from start
//! @return true if polyline has 1 or more points.
//!
Public GEOMDLLIMPEXP bool    bsiGeom_pointAtDistanceOnPolyline
(
DPoint3dP pPoint,
DPoint3dP pTangent,
DPoint3dCP pPointArray,
int              n,
double           dist
);

//!
//! @description Construct points at equal arc distances along a prior (possibly
//! unevenly spaced) polyline.
//! Note that distance between the new points is measured along the old
//! polyline, not directly between new points.
//!
//! @param pNewPoint OUT     interpolated point
//! @param maxCount IN      max point to be returned
//! @param pPointArray IN      points in polyline
//! @param n IN      number of points in polyline
//! @param dist IN      distance between points
//! @return number of points actually placed
//!
Public GEOMDLLIMPEXP int bsiGeom_pointsAlongPolyline
(
DPoint3dP pNewPoint,
int             maxCount,
DPoint3dCP pPointArray,
int              n,
double           dist
);

//!
//!
//! @param pPoint OUT     interpolated point
//! @param pTangent OUT     forward tangent at interpolated point.
//! @param pParam OUT     parameter at nearest point
//! @param pPointArray IN      points in polyline
//! @param n IN      number of points in polyline
//! @param pTestPoint IN      space point being projected
//! @return true if polyline has 1 or more points.
//!
Public GEOMDLLIMPEXP bool    bsiGeom_nearestPointOnPolylineExt
(
DPoint3dP pPoint,
DPoint3dP pTangent,
double          *pParam,
DPoint3dCP pPointArray,
int              n,
DPoint3dCP pTestPoint
);

//!
//!
//! @param pPoint OUT     interpolated point
//! @param pParam OUT     parameter at nearest point
//! @param pPointArray IN      points in polyline
//! @param n IN      number of points in polyline
//! @param pTestPoint IN      space point being projected
//! @return true if polyline has 1 or more points.
//!
Public GEOMDLLIMPEXP bool    bsiGeom_nearestPointOnPolyline
(
DPoint3dP pPoint,
double          *pParam,
DPoint3dCP pPointArray,
int              n,
DPoint3dCP pTestPoint
);

//!
//! @param pPointArray IN OUT array to sort
//! @param numPoint IN number of points
//!
Public GEOMDLLIMPEXP void bsiDPoint3dArray_sortToChainFromSeed
(
DPoint3dP pPointArray,
int         numPoint,
DPoint3dCP pSeedPoint
);

//!
//! Sort points in place to a heuristically-driven short chain.
//! The algorithm is to choose a random point (actually, the first) and
//! walk to closest neighbors as first pass.   In the first pass sort order,
//! find the longest edge and repeat the closest neighbor logic using one end of
//! that long edge as seed.
//! @param pPointArray IN OUT array to sort
//! @param numPoint IN number of points
//!
Public GEOMDLLIMPEXP void bsiDPoint3dArray_sortToChain
(
DPoint3dP pPointArray,
int         numPoint
);

//!
//! @description Search for any intersection among line segments in two polylines.
//! @param pPointArrayA IN points on polyline A
//! @param numA IN number of points in polyline A
//! @param bCloseA IN true to force additional closure segment on polyline A
//! @param pPointArrayB IN points on polyline B
//! @param numB IN number of points in polyline B
//! @param bCloseB IN true to force additional closure segment on polyline B
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_polylineClashXY
(
DPoint3dCP pPointArrayA,
int         numA,
bool        bCloseA,
DPoint3dCP pPointArrayB,
int         numB,
bool        bCloseB
);

//!
//! @description Search for all xy intersection among line segments in two polylines.
//!
//! @param pIntersectionA OUT intersection points on A.
//! @param pIndexA OUT segment indices on A.
//! @param pFractionA OUT segment fractions on A.
//!
//! @param pIntersectionB OUT intersection points on B.
//! @param pIndexB OUT segment indices on B.
//! @param pFractionB OUT segment fractions on B.
//!
//! @param pNumOut OUT number of intersections reported.
//! @param pNumExtraIntersection OUT number of intersections not reported.
//! @param maxIntersection IN max allowed in arrays.
//! @param pPointArrayA IN points on polyline A
//! @param numA IN number of points in polyline A
//! @param bCloseA IN true to force additional closure segment on polyline A
//! @param pPointArrayB IN points on polyline B
//! @param numB IN number of points in polyline B
//! @param bCloseB IN true to force additional closure segment on polyline B
//!
Public GEOMDLLIMPEXP void bsiDPoint3dArray_polylineIntersectXY
(
DPoint3dP pIntersectionA,
int       *pIndexA,
double    *pFractionA,
DPoint3dP pIntersectionB,
int       *pIndexB,
double    *pFractionB,
int       *pNumOut,
int         *pNumExtraIntersection,
int         maxIntersection,
DPoint3dCP pPointArrayA,
int         numA,
bool        bCloseA,
DPoint3dCP pPointArrayB,
int         numB,
bool        bCloseB
);

//!
//! @description Test if a linestring or polygon is self intersecting.
//! @param pXYZArray IN points on polyline
//! @param numPoints IN number of points in polyline or polygon
//! @param bClosed IN true to treat as polygon.
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_isSelfIntersectingXY
(
DPoint3dCP pXYZArray,
int         numXYZ,
bool        bClosed
);

END_BENTLEY_GEOMETRY_NAMESPACE

