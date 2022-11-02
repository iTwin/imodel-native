/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Returns a lines segment defined by its start and end coordinates.
//! @param pSegment OUT initialized segment
//! @param x0 IN x coordinate of point 0.
//! @param y0 IN y coordinate of point 0.
//! @param z0 IN z coordinate of point 0.
//! @param x1 IN x coordinate of point 1.
//! @param y1 IN y coordinate of point 1.
//! @param z1 IN z coordinate of point 1.
//!  @group "DSegment3d Initialization"
//!
Public GEOMDLLIMPEXP void            bsiDSegment3d_initFromStartEndXYZXYZ
(
DSegment3dP pSegment,
double            x0,
double            y0,
double            z0,
double            x1,
double            y1,
double            z1
);

//!
//! @description Down-weight a DSegment4d back to DSegment3d.  Beware that this can fail
//! (if weights are zero), and changes parameterization when weights of the two
//! points are different.
//! @param pSegment OUT initialized segment
//! @param pSource IN source segment.
//! @return false if either point of the source segment has zero weights.
//! @group "DSegment3d Initialization"
//!
Public GEOMDLLIMPEXP bool            bsiDSegment3d_initFromDSegment4d
(
DSegment3dP pSegment,
DSegment4dCP pSource
);

//!
//! @description Initialize a segment from endpoints.
//! @param pSegment OUT initialized segment.
//! @param pPoint0 IN start point.
//! @param pPoint1 IN end point.
//! @group "DSegment3d Initialization"
//!
Public GEOMDLLIMPEXP void            bsiDSegment3d_initFromDPoint3d
(
DSegment3dP pSegment,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
);

//!
//! @description Return a segment defined by its 2d endpoints with z=0.
//! @param pSegment OUT initialized segment.
//! @param pPoint0 IN start point.
//! @param pPoint1 IN end point.
//! @group "DSegment3d Initialization"
//!
Public GEOMDLLIMPEXP void            bsiDSegment3d_initFromDPoint2d
(
DSegment3dP pSegment,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1
);

//!
//! @description Return a segment defined by start point and extent vector.
//! @param pSegment OUT initialized segment.
//! @param pPoint0 IN start point.
//! @param pTangent IN extent vector.
//! @group "DSegment3d Initialization"
//!
Public GEOMDLLIMPEXP void            bsiDSegment3d_initFromDPoint3dTangent
(
DSegment3dP pSegment,
DPoint3dCP pPoint0,
DPoint3dCP pTangent
);

//!
//! @description Initialize a segment from a ray.  Ray origin becomes start point. Ray point at parameter 1 becomes end point.
//! @param pSegment OUT initialized segment.
//! @param pRay IN source segment.
//! @group "DSegment3d Initialization"
//!
Public GEOMDLLIMPEXP void            bsiDSegment3d_initFromDRay3d
(
DSegment3dP pSegment,
DRay3dCP pRay
);

//!
//! @description Return a point at a fractional position along a segment.
//! @param pSegment OUT initialized segment.
//! @param pPoint OUT computed point
//! @param param IN fractional parameter
//! @group "DSegment3d Evaluate"
//!
Public GEOMDLLIMPEXP void            bsiDSegment3d_evaluateDPoint3d
(
DSegment3dCP pSegment,
DPoint3dP pPoint,
double            param
);

//!
//! @description Return the endpoints of the segment.
//! @param pSegment IN segment
//! @param pPoint0 OUT start point
//! @param pPoint1 OUT end point
//! @group "DSegment3d Evaluate"
//!
Public GEOMDLLIMPEXP void            bsiDSegment3d_evaluateEndPoints
(
DSegment3dCP pSegment,
DPoint3dP pPoint0,
DPoint3dP pPoint1
);

//!
//! @description Return the (unnormalized) tangent vector along the segment.
//! @param pSegment IN segment
//! @param pTangent OUT tangent vector
//! @group "DSegment3d Evaluate"
//!
Public GEOMDLLIMPEXP void            bsiDSegment3d_evaluateTangent
(
DSegment3dCP pSegment,
DPoint3dP pTangent
);

//!
//! @description Compute the squared length of a segment.
//! @param pSegment IN segment
//! @return squared length of the segment.
//! @group "DSegment3d Queries"
//!
Public GEOMDLLIMPEXP double          bsiDSegment3d_lengthSquared (DSegment3dCP pSegment);

//!
//! @description Compute the range box around a segment.
//! @param pSegment IN segment
//! @param pRange OUT     range of segment.
//! @return always true
//! @group "DSegment3d Queries"
//!
Public GEOMDLLIMPEXP bool            bsiDSegment3d_getRange
(
DSegment3dCP pSegment,
DRange3dP pRange
);

//!
//! @description Project a point onto the extended line in 3D.
//! @param pSegment IN segment
//! @param pClosestPoint OUT computed point
//! @param pClosestParam OUT parameter of closest point
//! @param pPoint IN space point.
//! @return false if the segment degenerates to a point.
//! @group "DSegment3d Closest Point"
//!
Public GEOMDLLIMPEXP bool            bsiDSegment3d_projectPoint
(
DSegment3dCP pSegment,
DPoint3dP pClosestPoint,
double          *pClosestParam,
DPoint3dCP pPoint
);

//!
//! @description Project a point onto the extended line, using only xy parts for distance calculation.
//! @param pSegment IN segment
//! @param pClosestPoint OUT computed point
//! @param pClosestParam OUT parameter of closest point
//! @param pPoint IN space point.
//! @return false if the segment degenerates to a point.
//! @group "DSegment3d Closest Point"
//!
Public GEOMDLLIMPEXP bool            bsiDSegment3d_projectPointXY
(
DSegment3dCP pSegment,
DPoint3dP pClosestPoint,
double          *pClosestParam,
DPoint3dCP pPoint
);

//!
//! @description Project a point onto the bounded line in 3D.  If nearest point of extended line
//! is outside the 0..1 parameter range, returned values are for nearest endpoint.
//! @param pSegment IN segment
//! @param pClosestPoint OUT computed point
//! @param pClosestParam OUT parameter of closest point
//! @param pPoint IN space point.
//! @return false if the segment degenerates to a point.
//! @group "DSegment3d Closest Point"
//!
Public GEOMDLLIMPEXP bool            bsiDSegment3d_projectPointBounded
(
DSegment3dCP pSegment,
DPoint3dP pClosestPoint,
double          *pClosestParam,
DPoint3dCP pPoint
);

//!
//! @description Return the intersection of the (unbounded) segment with a plane.
//! @param pSegment IN segment
//! @param pIntPoint IN      intersection point
//! @param pIntParam IN      parameter along the line
//! @param pPlane IN      plane (origin and normal)
//! @return false if line, plane are parallel.
//! @group "DSegment3d Intersection"
//!
Public GEOMDLLIMPEXP bool            bsiDSegment3d_intersectDPlane3d
(
DSegment3dCP pSegment,
DPoint3dP pIntPoint,
double          *pIntParam,
DPlane3dCP pPlane
);

//!
//! @description Return the intersection of the (unbounded) segment with a circle, using only
//! xy coordinates.
//! @param pSegment IN segment
//! @param pIntPoint OUT     0, 1, or 2 intersection points.
//! @param pInParam  OUT     0, 1, or 2 intersection parameters.
//! @param pIntParam IN      parameter along the line
//! @param pCenter IN      circle center.
//! @param radius  IN      circle radius.
//! @return   number of intersections.
//! @group "DSegment3d Intersection"
//!
Public GEOMDLLIMPEXP int             bsiDSegment3d_intersectCircleXY
(
DSegment3dCP pSegment,
DPoint3dP pIntPoint,
double          *pIntParam,
DPoint3dCP pCenter,
double          radius
);

//!
//! @description Apply a transformation to the source segment.
//! @param pDest OUT transformed segment
//! @param pTransform IN      transformation to apply.
//! @param pSource IN      source segment
//! @return always true
//! @group "DSegment3d Initialization"
//!
Public GEOMDLLIMPEXP bool    bsiDSegment3d_transform
(
DSegment3dP pDest,
DTransform3dCP pTransform,
DSegment3dCP pSource
);

//!
//! @description Apply a transformation to the source segment.
//! @param pDest OUT transformed segment
//! @param pTransform IN      transformation to apply.
//! @param pSource IN      source segment
//! @return always true
//! @group "DSegment3d Initialization"
//!
Public GEOMDLLIMPEXP bool    bsiDSegment3d_multiplyTransformDSegment3d
(
DSegment3dP pDest,
TransformCP pTransform,
DSegment3dCP pSource
);

//!
//! @description Find the closest approach of two (unbounded) lines.
//! @param pSegment IN segment
//! @param pParam0 OUT     parameter on first ray.
//! @param pParam1 OUT     parameter on second ray.
//! @param pPoint0 OUT     point on first ray.
//! @param pPoint1 OUT     point on second ray.
//! @param pSegment0   OUT     first segment
//! @param pSegment1   OUT     second segment
//! @return false if the segments are parallel.
//! @group "DSegment3d Closest Point"
//!
Public GEOMDLLIMPEXP bool            bsiDSegment3d_closestApproach
(
double          *pParam0,
double          *pParam1,
DPoint3dP pPoint0,
DPoint3dP pPoint1,
DSegment3dCP pSegment0,
DSegment3dCP pSegment1
);

//!
//! @description Compute the parameters and points where the xy projections of two rays intersect.
//! @param pSegment IN segment
//! @param pPoint0 OUT     intersection point on segment 0.
//! @param pParam0 OUT     parametric coordinate on segment 0
//! @param pPoint1 OUT     intesection point on segment 1.
//! @param pParam1 OUT     parametric coordinate on segment 1
//! @param pSegment0 IN      segment 0
//! @param pSegment1 IN      segment 1
//! @return true unless lines are parallel
//! @group "DSegment3d Intersection"
//!
Public GEOMDLLIMPEXP bool    bsiGeom_intersectXYDSegment3dDSegment3d
(
DPoint3dP pPoint0,
double              *pParam0,
DPoint3dP pPoint1,
double              *pParam1,
DSegment3dCP pSegment0,
DSegment3dCP pSegment1
);

//!
//! @description get the start point of the segment.
//! @param pSegment IN segment
//! @param pPoint OUT start point
//! @group "DSegment3d Evaluate"
//!
Public GEOMDLLIMPEXP void     bsiDSegment3d_getStartPoint
(
DSegment3dCP pSegment,
DPoint3dP pPoint
);

//!
//! @description return the end point of the segment.
//! @param pSegment IN segment
//! @param pPoint OUT end point
//! @group "DSegment3d Evaluate"
//!
Public GEOMDLLIMPEXP void     bsiDSegment3d_getEndPoint
(
DSegment3dCP pSegment,
DPoint3dP pPoint
);

//!
//! Set the "start" point for the segment.
//! @param pSegment IN segment
//! @param pPoint IN new start point.
//! @group "DSegment3d Queries"
//!
Public GEOMDLLIMPEXP void     bsiDSegment3d_setStartPoint
(
DSegment3dP pSegment,
DPoint3dCP pPoint
);

//!
//! Set the "end" point for the segment.
//! @param pSegment IN segment
//! @param pPoint  IN new end point.
//! @group "DSegment3d Queries"
//!
Public GEOMDLLIMPEXP void     bsiDSegment3d_setEndPoint
(
DSegment3dP pSegment,
DPoint3dCP pPoint
);

//!
//! @description Evaluate the point at a fractional position along a segment.
//! @param pSegment IN segment
//! @param pPoint  OUT     coordinates at fractional parameter.
//! @param param      IN      fractional parameter
//! @return always true
//! @group "DSegment3d Evaluate"
//!
Public GEOMDLLIMPEXP bool     bsiDSegment3d_fractionParameterToDPoint3d
(
DSegment3dCP pSegment,
DPoint3dP pPoint,
double  param
);

//!
//! @description Returns the parameter at which a point projects to the *unbounded)
//!   line containing the segment.  Parameters less than zero and greater than one
//!   mean the projection is outside the bounds of the line segment.
//! @param pSegment IN segment
//! @param pParam  OUT     where pPoint projects to the line.
//! @param pPoint     IN      point to project to the line.
//! @return false if segment degenerates to a point.
//! @group "DSegment3d Evaluate"
//!
Public GEOMDLLIMPEXP bool        bsiDSegment3d_dPoint3dToFractionParameter
(
DSegment3dCP pSegment,
double          *pParam,
DPoint3dCP pPoint
);

//!
//! @description Evaluate the point and tangent vector at a fractional parameter.
//! @param pSegment IN segment
//! @param pPoint      OUT     point on line at fractional parameter.
//! @param pTangent    OUT     tangent vector at fractional parameter.
//! @param param           IN      fractional parameter.
//! @return always true
//! @group "DSegment3d Evaluate"
//!
Public GEOMDLLIMPEXP bool     bsiDSegment3d_fractionParameterToDPoint3dTangent
(
DSegment3dCP pSegment,
DPoint3dP pPoint,
DPoint3dP pTangent,
double      param
);

//!
//! @description return one of the endpoints of a segment, selected by integer index 0 or 1.
//! @param pSegment IN segment
//! @param pPt OUT     returned point.
//! @param index IN      index of point to return.
//! @return false if index is other than 0 or 1.  In this case the point is left unassigned.
//! @group "DSegment3d Evaluate"
//!
Public GEOMDLLIMPEXP bool             bsiDSegment3d_getIndexedDPoint3d
(
DSegment3dCP pSegment,
DPoint3dP pPt,
int             index
);

//!
//! @description Set one of the endpoints of a segment, selected by integer 0 or 1.
//! @param pSegment IN segment
//! @param pPt IN point to copy
//! @param index IN      index of point to update.
//! @return false if index is other than 0 or 1.  In this case the point is left unassigned.
//! @group "DSegment3d Queries"
//!
Public GEOMDLLIMPEXP bool             bsiDSegment3d_setIndexedDPoint3d
(
DSegment3dP pSegment,
DPoint3dCP pPt,
int             index
);

//!
//! @description Computes equidistant points along this instance.
//! @param pSegment IN segment
//! @param    pPoints     OUT     array of computed points
//! @param    numPoints   IN      # points to compute
//! @param    bInclusive   IN      true to include endpoints in numPoints count
//! @group "DSegment3d Evaluate"
//!
Public GEOMDLLIMPEXP void             bsiDSegment3d_interpolateUniformDPoint3dArray
(
DSegment3dCP pSegment,
DPoint3dP pPoints,
int             numPoints,
bool            bInclusive
);

//!
//! @description compute the length of a line segment.
//! @param pSegment IN segment
//! @return line segment length.
//! @group "DSegment3d Queries"
//!
Public GEOMDLLIMPEXP double bsiDSegment3d_length (DSegment3dCP pSegment);

//!
//! Compute the (signed) arc length between specified fractional parameters.
//! @param pSegment IN segment to query
//! @param pArcLength OUT     computed arc length.  Negative if fraction1 < fraction0.
//! @param fraction0 IN      start fraction for interval to measure.
//! @param fraction1 IN      end fraction for interval to measure.
//! @return true if the arc length was computed.
//! @group "DSegment3d Queries"
//!
Public GEOMDLLIMPEXP bool     bsiDSegment3d_fractionToLength
(
DSegment3dCP pSegment,
double      *pArcLength,
double      fraction0,
double      fraction1
);

//!
//! Compute the fraction parameter corresponding to a specified arc length away from
//!   a specified start fraction. (inverse of fractions to arcStep)
//! @param pSegment IN segment to query
//! @param pFraction1 OUT     fraction at end of interval.
//! @param fraction0 IN      start fraction for interval to measure.
//! @param arcStep IN      arc length to move.  Negative arc length moves backwards.
//! @return true if the fractional step was computed.
//! @group "DSegment3d Queries"
//!
Public GEOMDLLIMPEXP bool     bsiDSegment3d_lengthToFraction
(
DSegment3dCP pSegment,
double      *pFraction1,
double      fraction0,
double      arcStep
);

END_BENTLEY_GEOMETRY_NAMESPACE

