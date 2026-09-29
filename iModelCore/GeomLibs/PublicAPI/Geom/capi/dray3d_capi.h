/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Initialize a ray from start and target points
//!
//!
//!
Public GEOMDLLIMPEXP void            bsiDRay3d_initFromDPoint3dStartEnd
(
DRay3dP pInstance,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
);

//!
//! Initialize a ray from a segment.
//!
//!
//!
Public GEOMDLLIMPEXP void            bsiDRay3d_initFromDSegment3d
(
DRay3dP pInstance,
DSegment3dCP pSegment
);

//!
//! Initialize a ray from 2d endpoints
//!
//!
//!
Public GEOMDLLIMPEXP void            bsiDRay3d_initFromDPoint2dStartEnd
(
DRay3dP pInstance,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1
);

//!
//! Initialize a ray from origin and direction.
//!
//!
//!
Public GEOMDLLIMPEXP void            bsiDRay3d_initFromDPoint3dTangent
(
DRay3dP pInstance,
DPoint3dCP pPoint0,
DVec3dCP pTangent
);

//!
//! Evaluate the segment at a parametric coordinate.
//!
//!
//!
Public GEOMDLLIMPEXP void            bsiDRay3d_evaluateDPoint3d
(
DRay3dCP pInstance,
DPoint3dP pPoint,
double            param
);

//!
//! Return the origin and target of the ray.
//!
//!
//!
Public GEOMDLLIMPEXP void            bsiDRay3d_evaluateEndPoints
(
DRay3dCP pInstance,
DPoint3dP pPoint0,
DPoint3dP pPoint1
);

//!
//! Return the (unnormalized) tangent vector along the segment.
//!
//!
//!
Public GEOMDLLIMPEXP void            bsiDRay3d_evaluateTangent
(
DRay3dCP pInstance,
DVec3dP pTangent
);

//!
//! @return squared length of the ray's direction vector.
//!
//!
//!
Public GEOMDLLIMPEXP double          bsiDRay3d_lengthSquared (DRay3dCP pInstance);

//!
//! Project a point onto the extended ray in 3D.
//!
//!
//!
Public GEOMDLLIMPEXP bool            bsiDRay3d_projectPoint
(
DRay3dCP pInstance,
DPoint3dP pClosestPoint,
double          *pClosestParam,
DPoint3dCP pPoint
);

//!
//! Project a point onto the bounded line in 3D.  If nearest point of extended line
//! is outside the 0..1 parameter range, returned values are for nearest endpoint.
//!
//!
//!
Public GEOMDLLIMPEXP bool            bsiDRay3d_projectPointBounded
(
DRay3dCP pInstance,
DPoint3dP pClosestPoint,
double          *pClosestParam,
DPoint3dCP pPoint
);

//!
//!
//! Apply a transformation to the source ray.
//! @param pTransform IN      transformation to apply.
//! @param pSource IN      source ray
//!
//!
Public GEOMDLLIMPEXP bool    bsiDRay3d_transform
(
DRay3dP pDest,
DTransform3dCP pTransform,
DRay3dCP pSource
);

Public GEOMDLLIMPEXP bool    bsiDRay3d_multiplyTransformDRay3d
(
DRay3dP pDest,
TransformCP pTransform,
DRay3dCP pSource
);


//!
//! Find the closest approach of two (unbounded) rays.
//!
//! @param pParam0 OUT     parameter on first ray.
//! @param pParam1 OUT     parameter on second ray.
//! @param pPoint0 OUT     point on first ray.
//! @param pPoint1 OUT     point on second ray.
//! @param pRay0   IN      first ray.
//! @param pRay1   IN      second ray.
//!
Public GEOMDLLIMPEXP bool            bsiDRay3d_closestApproach
(
double          *pParam0,
double          *pParam1,
DPoint3dP pPoint0,
DPoint3dP pPoint1,
DRay3dCP pRay0,
DRay3dCP pRay1
);

//!
//! Return the intersection of the (unbounded) ray with a plane.
//! @param pIntPoint OUT     intersection point
//! @param pIntParam OUT     parameter along the ray
//! @param pPlane IN      plane (origin and normal)
//! @return false if ray, plane are parallel.
//!
Public GEOMDLLIMPEXP bool            bsiDRay3d_intersectDPlane3d
(
DRay3dCP pInstance,
DPoint3dP pIntPoint,
double          *pIntParam,
DPlane3dCP pPlane
);

//!
//! Return the intersection of the (unbounded) ray with a circle, using only
//! xy coordinates.
//! @param pIntPoint OUT     0, 1, or 2 intersection points.
//! @param pInParam  OUT     0, 1, or 2 intersection parameters.
//! @param pIntParam IN      parameter along the line
//! @param pCenter IN      circle center.
//! @param radius  IN      circle radius.
//! @return   number of intersections.
//!
Public GEOMDLLIMPEXP int             bsiDRay3d_intersectCircleXY
(
DRay3dCP pRay,
DPoint3dP pIntPoint,
double          *pIntParam,
DPoint3dCP pCenter,
double          radius
);

//!
//! Return the intersection of the (unbounded) ray with a circle, using only
//! xy coordinates.
//! @param pIntPoint OUT     0, 1, or 2 intersection points.
//! @param pRayParam  OUT     0, 1, or 2 intersection parameters on line
//! @param pPatchParam OUT     0, 1, or 2 s,t patch parameter pairs.
//! @return   number of intersections.
//!
Public GEOMDLLIMPEXP int             bsiDRay3d_intersectHyperbolicParaboloid
(
DRay3dCP pRay,
DPoint3dP pIntPoint,
double          *pRayParameter,
DPoint2dP pPatchParameter,
DPoint3dCP pPoint00,
DPoint3dCP pPoint10,
DPoint3dCP pPoint01,
DPoint3dCP pPoint11
);

//!
//! Return the intersection of the (unbounded) ray with a circle, using only
//! xy coordinates.
//! @param pIntPoint OUT     0, 1, or 2 intersection points.
//! @param pRayParam  OUT     0, 1, or 2 intersection parameters on line
//! @param pPatchParam OUT     0, 1, or 2 s,t patch parameter pairs.
//! @param pPoint00   IN      00 corner of patch.
//! @param w00        IN      00 weight
//! @param pPoint10   IN      10 corner of patch.
//! @param w10        IN      10 weight
//! @param pPoint01   IN      01 corner of patch.
//! @param w01        IN      01 weight
//! @param pPoint11   IN      11 corner of patch.
//! @param w11        IN      11 weight
//! @return   number of intersections.
//!
Public GEOMDLLIMPEXP int             bsiDRay3d_intersectWeightedHyperbolicParaboloid
(
DRay3dCP pRay,
DPoint3dP pIntPoint,
double          *pRayParameter,
DPoint2dP pPatchParameter,
DPoint3dCP pPoint00,
double          w00,
DPoint3dCP pPoint10,
double          w10,
DPoint3dCP pPoint01,
double          w01,
DPoint3dCP pPoint11,
double          w11
);

//!
//! @description Compute (simple) intersection of a line and a triangle.
//!
//! @remarks If the returned barycentric coordinates all lie within [0,1], then the intersection lies on or inside the triangle.
//! @remarks If the returned ray parameter is nonnegative, then the intersection lies on the ray.
//!
//! @param pRay           IN      ray with which to intersect triangle
//! @param pXYZ           OUT     point of intersection
//! @param pBarycentric   OUT     intersection's barycentric coords relative to triangle
//! @param pRayParameter  OUT     intersection's parameter along the ray (intersection = rayOrigin + parameter * rayDirection)
//! @param pTriangleXYZ   IN      array of 3 triangle vertex coords
//! @return true if intersection computed
//!
Public GEOMDLLIMPEXP bool     bsiDRay3d_intersectTriangle
(
DRay3dCP      pRay,
DPoint3dP    pXYZ,
DPoint3dP    pBarycentric,
double*     pRayParameter,
DPoint3dCP    pTriangleXYZ
);

//!
//! @description Compute (simple) intersection of a ray and a triangle.
//!
//! @remarks Tolerances work best when scaled by the size of the input coordinates. Best practice is for callers to
//! precompute the distance tolerance once, and then pass this distance tol into all subsequent invocations of this
//! method. Callers do not have to do this for the parameter tolerance, as this is unitless.
//!
//! @param pRay           IN      ray with which to intersect triangle
//! @param pXYZ           OUT     point of intersection
//! @param pTriangleXYZ   IN      array of 3 triangle vertex coords
//! @param distanceTol    IN      distance tolerance used to check if ray is parallel to the triangle or if we have
//! line intersection but not ray intersection. If negative or not given, defaults to 1.0e-6.
//! @param parameterTol   IN      parameter tolerance used to allow intersections just beyond an edge/vertex in
//! barycentric coordinate space. If negative or not given, defaults to 1.0e-15.
//! @return true if intersection computed. false if ray does not intersect the triangle or intersects behind ray origin.
//!
Public GEOMDLLIMPEXP bool bsiDRay3d_intersectTriangleFast
(
    DRay3dCP pRay,
    DPoint3dP pXYZ,
    DPoint3dCP pTriangleXYZ,
    double distanceTol = 1.0e-6,
    double parameterTol = 1.0e-15
);

END_BENTLEY_GEOMETRY_NAMESPACE
