/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! sets pIntersection to the point of intersection of two lines defined
//! by respective start points and rays, and sets s0P and s1P to
//! the respective parametric coordiantes.
//!
//! @param pIntersection OUT     intersection point
//! @param s0P OUT     parametric coordinate on ray 0
//! @param s1P OUT     parametric coordinate on ray 1
//! @param pStart0 IN      start of first ray
//! @param pVec0 IN      first vector
//! @param pStart1 IN      start of second ray
//! @param pVec1 IN      second vector
//! @return true unless rays are parallel
//!
Public GEOMDLLIMPEXP bool    bsiVector2d_intersectRays
(
DPoint2dP pIntersection,
double          *s0P,
double          *s1P,
DPoint2dCP pStart0,
DPoint2dCP pVec0,
DPoint2dCP pStart1,
DPoint2dCP pVec1
);

//!
//!
//! Compute the parameters and points where the xy projections of two rays intersect.
//!
//! @param pPoint0 OUT     intersection point on line 0.
//! @param s0P OUT     parametric coordinate on segment 0
//! @param pPoint1 OUT     intesection point on line 1.
//! @param s1P OUT     parametric coordinate on segment 1
//! @param pStart0 IN      start of first ray
//! @param pVec0 IN      first vector
//! @param pStart1 IN      start of second ray
//! @param pVec1 IN      second vector
//! @return true unless rays are parallel
//!
Public GEOMDLLIMPEXP bool    bsiGeom_intersectXYRays
(
DPoint3dP pPoint0,
double          *pParam0,
DPoint3dP pPoint1,
double          *pParam1,
DPoint3dCP pStart0,
DPoint3dCP pVec0,
DPoint3dCP pStart1,
DPoint3dCP pVec1
);

//!
//! Compute the parameters and points where the xy projections of two rays intersect.
//!
//! @param pPoint0 OUT     intersection point on line 0.
//! @param s0P OUT     parametric coordinate on segment 0
//! @param pPoint1 OUT     intesection point on line 1.
//! @param s1P OUT     parametric coordinate on segment 1
//! @param pStart0 IN      start of first line segment
//! @param pEnd0 IN      end of first line
//! @param pStart1 IN      start of second segment
//! @param pEnd1 IN      end of second segment
//! @return true unless lines are parallel
//!
Public GEOMDLLIMPEXP bool    bsiGeom_intersectXYLines
(
DPoint3dP pPoint0,
double          *pParam0,
DPoint3dP pPoint1,
double          *pParam1,
DPoint3dCP pStart0,
DPoint3dCP pEnd0,
DPoint3dCP pStart1,
DPoint3dCP pEnd1
);

//!
//! sets pIntersection to the point of intersection of two lines defined
//! by respective start and end pointss, and sets s0P and s1P to
//! the respective parametric coordiantes.
//!
//! @param pIntersection OUT     intersection point
//! @param s0P OUT     parametric coordinate on segment 0
//! @param s1P OUT     parametric coordinate on segment 1
//! @param pStart0 IN      start of first line segment
//! @param pEnd0 IN      end of first line
//! @param pStart1 IN      start of second segment
//! @param pEnd1 IN      end of second segment
//! @return true unless lines are parallel
//!
Public GEOMDLLIMPEXP bool    bsiVector2d_intersectLines
(
DPoint2dP pIntersection,
double          *s0P,
double          *s1P,
DPoint2dCP pStart0,
DPoint2dCP pEnd0,
DPoint2dCP pStart1,
DPoint2dCP pEnd1
);

//!
//! sets pProjection to the projection of pPoint on the line from pStart
//! to pEnd, and sP to the parametric coordinate of pProjection.
//!
//! @param pProjection OUT     projection of point on line
//! @param sP OUT     parametric coordinate along line
//! @param pPoint IN      point to project
//! @param pStart IN      line start
//! @param pEnd IN      line end
//! @return true unless line is degenerate.
//!
Public GEOMDLLIMPEXP bool    bsiVector2d_projectPointToLine
(
DPoint2dP pProjection,
double          *sP,
DPoint2dCP pPoint,
DPoint2dCP pStart,
DPoint2dCP pEnd
);

//!
//! @param pProjection OUT     Projection of pPoint on line segment
//! @param pLambda OUT     Parametric coordinates of projection
//! @param pVector IN      Vector to be projected
//! @param pTarget IN      Target vector
//! @return true unless target vector has zero length.
//!
Public GEOMDLLIMPEXP bool    bsiVector2d_projectVectorToVector
(
DPoint2dP pProjection,
double      *pLambda,
DPoint2dCP pVector,
DPoint2dCP pTarget
);

//!
//! Given an array of three points that are origin, s=1, and t=1 points
//! on a skewed plane, evaluate the 3d point at s,t, and the derivatives
//! with respect to s and t.  The point is defined as
//! pPoint = pPlanePoint[0] + s *(pPlanePoint[1]pPlanePoint[0])
//! + t*(pPlanePoint[2]pPlanePoint[0]).
//!
//! @param pPoint OUT     space point
//! @param deriv1P OUT     derivative vectors wrt u, v
//! @param deriv2P OUT     2nd derivative vectors wrt uu, vv, uv
//! @param pPlanePoint IN      origin, s=1, and t=1 points
//! @param s IN      s coordinate of point
//! @param t IN      t coordinate of point
//!
Public GEOMDLLIMPEXP void bsiGeom_evaluateSkewedPlane
(
DPoint3dP pPoint,
DPoint3dP deriv1P,
DPoint3dP deriv2P,
DPoint3dCP pPlanePoint,
double           s,
double           t
);

//!
//! Given a space point spacePontP, finds the closest point on the plane
//! containing the 3 points in pPlanePoint.  Stores the closest point
//! coordinates in pClosePoint, and the s and t coordinates (as defined
//! in bsiGeom_evaluateSkewedPlane) in sP and tP.
//!
//! @param pClosePoint OUT     point on plane.  May be null pointer
//! @param sP OUT     parametric coordinate on s axis
//! @param tP OUT     parametric coordinate on t axis
//! @param pPlanePoint IN      origin, s=1, and t=1 points
//! @param pSpacePoint IN      point to be projected
//! @return true unless the plane points are collinear
//!
Public GEOMDLLIMPEXP bool     bsiGeom_closestPointOnSkewedPlane
(
DPoint3dP pClosePoint,
double          *sP,
double          *tP,
DPoint3dCP pPlanePoint,
DPoint3dCP pSpacePoint
);

//!
//! Given a space point spacePontP, finds the closest point on the plane
//! with given origin and 2 vectors.    Stores the closest point
//! coordinates in pClosePoint, and the s and t coordinates (as defined
//! in bsiGeom_evaluateSkewedPlane) in sP and tP.
//!
//! @param pClosePoint OUT     point on plane.  May be null pointer
//! @param sP OUT     parametric coordinate on s axis
//! @param tP OUT     parametric coordinate on t axis
//! @param pPlanePoint IN      origin, s=1, and t=1 points
//! @param pSpacePoint IN      point to be projected
//! @return true unless vectors are parallel
//!
Public GEOMDLLIMPEXP bool    bsiGeom_closestPointOnSkewedVectors
(
DPoint3dP pClosePoint,
double          *sP,
double          *tP,
DPoint3dCP pOrigin,
DPoint3dCP pVectorU,
DPoint3dCP pVectorV,
DPoint3dCP pSpacePoint
);

//!
//! finds the point of closest approach between two (possibly skewed,
//! nonintersecting) straight lines in 3 space.  The lines are describe
//! by two endpoints.  The return points are described by their parametric
//! and cartesian coordinates on the respective lines.
//! Input maxParam is the largest parameter value permitted in the output
//! points.    If only points within the lines are ever of interest, send
//! a smallish maxParam, e.g. 10 or 100.  A zero parameter value indicates
//! large parameters are acceptable.
//! indicated no point (which might be due to parallel lines or to
//! the intersection point being far from the points.)
//!
//! @param pParamA OUT     parametric coordinate on line A
//! @param pParamB OUT     parametric coordinate on line B
//! @param pPointAP OUT     closest point on A. May be NULL
//! @param pPointBP OUT     closest point on B. May be NULL
//! @param pStartA IN      start point of line A
//! @param pTangentA IN      direction vector of line A
//! @param pStartB IN      start point of line B
//! @param pTangentB IN      direction vector of line B
//! @return true unless rays are parallel
//!
Public GEOMDLLIMPEXP bool    bsiGeom_closestApproachOfRays
(
double      *pParamA,
double      *pParamB,
DPoint3dP pPointAP,
DPoint3dP pPointBP,
DPoint3dCP pStartA,
DPoint3dCP pTangentA,
DPoint3dCP pStartB,
DPoint3dCP pTangentB
);

//!
//! finds the point of closest approach between two (possibly skewed,
//! nonintersecting) straight lines in 3 space.  The lines are describe
//! by two endpoints.  The return points are described by their parametric
//! and cartesian coordinates on the respective lines.
//! Input maxParam is the largest parameter value permitted in the output
//! points.    If only points within the lines are ever of interest, send
//! a smallish maxParam, e.g. 10 or 100.  A zero parameter value indicates
//! large parameters are acceptable.
//!
//! @param pParamA OUT     parametric coordinate on line A
//! @param pParamB OUT     parametric coordinate on line B
//! @param pPointAP OUT     closest point on A. May be NULL
//! @param pPointBP OUT     closest point on B. May be NULL
//! @param pStartA IN      start point of line A
//! @param pEndA IN      end point of line A
//! @param pStartB IN      start point of line B
//! @param pEndB IN      end point of line B
//! @return true if lines are non-parallel
//!
Public GEOMDLLIMPEXP bool    bsiGeom_closestApproachOfLines
(
double      *pParamA,
double      *pParamB,
DPoint3dP pPointAP,
DPoint3dP pPointBP,
DPoint3dCP pStartA,
DPoint3dCP pEndA,
DPoint3dCP pStartB,
DPoint3dCP pEndB
);

//!
//! @description Compute the intersection point of a ray and a plane.
//!
//! @param pParam OUT     intersection parameter within line
//! @param pPoint OUT     intersection point
//! @param pLineStart IN      origin of ray
//! @param pLineTangent IN      direction vector of ray
//! @param pOrigin IN      any point on plane
//! @param pNormal IN      normal vector for plane
//! @return true unless the ray is parallel to the plane.
//!
Public GEOMDLLIMPEXP bool    bsiGeom_rayPlaneIntersection
(
double      *pParam,
DPoint3dP pPoint,
DPoint3dCP pLineStart,
DPoint3dCP pLineTangent,
DPoint3dCP pOrigin,
DPoint3dCP pNormal
);

//!
//! Compute the line of intersection between two planes.
//!
//! @param pLineStart OUT     start point of line
//! @param pLineDirection OUT     direction vector
//! @param pOriginA IN      any point on  plane A
//! @param pNormalA IN      normal vector for plane A
//! @param pOriginB IN      any point on plane B
//! @param pNormalB IN      normal vector for plane B
//! @return true if planes have simple intersection
//!
Public GEOMDLLIMPEXP bool    bsiGeom_planePlaneIntersection
(
DPoint3dP pLineStart,
DPoint3dP pLineDirection,
DPoint3dCP pOriginA,
DPoint3dCP pNormalA,
DPoint3dCP pOriginB,
DPoint3dCP pNormalB
);

//!
//! @description Compute the intersection point of a line and a plane.
//!
//! @param pParam OUT     intersection parameter within line
//! @param pPoint OUT     intersection point
//! @param pLineStart IN      point on line at parameter 0.0
//! @param pLineEnd IN      point on line at parameter 1.0
//! @param pOrigin IN      any point on plane
//! @param pNormal IN      normal vector for plane
//! @return true unless the line is parallel to the plane.
//!
Public GEOMDLLIMPEXP bool    bsiGeom_linePlaneIntersection
(
double      *pParam,
DPoint3dP pPoint,
DPoint3dCP pLineStart,
DPoint3dCP pLineEnd,
DPoint3dCP pOrigin,
DPoint3dCP pNormal
);

//!
//! Compute the parts of pVector1 tangent and perpendicular to
//! pVector0 and in the plane containing both.
//!
//! @param pTangent       OUT     projection of pVector1 on pVector0
//! @param pParam         OUT     parameter
//! @param pPerpendicular OUT     pVector1 - pPerpendicular
//! @param pVector0       IN      reference vector
//! @param pVector1       IN      vector being split
//! @return true if vector0 has nonzero length.
//!
Public GEOMDLLIMPEXP bool    bsiGeom_tangentAndPerpendicularVectorParts
(
DPoint3dP pTangent,
double          *pParam,
DPoint3dP pPerpendicular,
DPoint3dCP pVector0,
DPoint3dCP pVector1
);

//!
//! Project a point to a plane defined by origin and (not necessarily unit)
//! normal vector.
//!
//! @param pOutPoint OUT     projected point (or NULL)
//! @param pInPoint  IN      point to project to plane
//! @param pNormal   IN      plane normal
//! @param pOrigin   IN      plane origin
//! @return signed distance from point to plane.  If the plane normal has zero length,
//!           distance to plane origin.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_distancePointToPlane
(
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
DPoint3dCP pNormal,
DPoint3dCP pOrigin
);

//!
//! Sets pProjection to the projection of pPoint onto the infinite
//! ray through pStart in the direction of pDir, and sets lambdaP to the parametric
//! coordinate of the projection point.
//!
//! @param pProjection OUT     Projection of pPoint on ray
//! @param lambdaP OUT     Parametric coordinates of projection
//! @param pPoint IN      Point to be projected
//! @param pStart IN      Ray start
//! @param pDir IN      Ray direction
//! @return true unless direction vector has zero length.
//!
Public GEOMDLLIMPEXP bool    bsiGeom_projectPointToRay
(
DPoint3dP pProjection,
double      *lambdaP,
DPoint3dCP pPoint,
DPoint3dCP pStart,
DPoint3dCP pDir
);

//!
//! Sets pProjection to the projection of pPoint onto the infinite
//! line through pStart and pEnd, and sets lambdaP to the parametric
//! coordinate of the projection point.
//!
//! @param pProjection OUT     Projection of pPoint on line
//! @param pParameter OUT     Parametric coordinates of projection
//! @param pPoint IN      Point to be projected
//! @param pStart IN      Line start
//! @param pEnd IN      Line end
//! @return true unless the line start and end are coincident
//!
Public GEOMDLLIMPEXP bool    bsiGeom_projectPointToLine
(
DPoint3dP pProjection,
double      *pParameter,
DPoint3dCP pPoint,
DPoint3dCP pStart,
DPoint3dCP pEnd
);

//!
//! sets prjectionP to the projecion of pVector onto vector pTarget, and
//! sets lambdaP to the parametric coordinate of the projection along
//! pTarget.
//!
//! @param pProjection OUT     Projection of pVector on pTarget
//! @param pPerpendicular OUT     Projection of pVector on plane perpendicular to pTarget
//! @param pLambda OUT     Parametric coordinates of projection
//! @param pVector IN      Vector to be projected
//! @param pTarget IN      plane normal
//! @return true unless the target vector has zero length
//!
Public GEOMDLLIMPEXP bool     bsiGeom_projectVectorToVector
(
DPoint3dP pProjection,
DPoint3dP pPerpendicular,
double      *pLambda,
DPoint3dCP pVector,
DPoint3dCP pTarget
);

//!
//! @param pKeyPoint OUT     closest grid point
//! @param pKeyParam OUT     closest grid point param
//! @param pOrg IN      line points
//! @param pEnd IN      line end
//! @param pNear IN      nearest point
//! @param param0 IN      parameter to be considered 0 for grid purposes
//! @param paramDelta IN      grid step
//! @param minParam IN      smallest allowed parameter
//! @param maxParam IN      largets allowed parameter
//!
Public GEOMDLLIMPEXP void bsiGeom_closestGridPoint
(
DPoint3dP pKeyPoint,
double          *pKeyParam,
DPoint3dCP pOrg,
DPoint3dCP pEnd,
DPoint3dCP pNear,
double          param0,
double          paramDelta,
double          minParam,
double          maxParam
);

//!
//! Project a point to the xy image of a homogeneous line.  Use only (normalized) x and y
//! in distance calculations.
//!
//! @param    pProjection     OUT     projection.  Start point if line degenerated to point.
//! @param    pLambda         OUT     parametric coordinate of projection.
//! @param    pPoint          IN      point being projected
//! @param    pStart          IN      line start
//! @param    pEnd            IN      line end.
//! @return   true unless line degenerated to a point.
//!
Public GEOMDLLIMPEXP bool    bsiGeom_projectDPoint3dToDPoint4dLineXY
(
DPoint4dP pProjection,
double      *pParam,
DPoint3dCP pPoint,
DPoint4dCP pStart,
DPoint4dCP pEnd
);

END_BENTLEY_GEOMETRY_NAMESPACE

