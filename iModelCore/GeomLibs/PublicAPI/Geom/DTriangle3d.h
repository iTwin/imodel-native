/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/DTriangle3d.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/**
4-sided patch defined by its 4 vertices.
The patch is parameterized by u,v as
   {xyz = (1-u)*(1-v)*xyz00 + u*(1-v)*xyz10 + (1-u)*v*xyz10 + u*v*xyz11}
*/
struct GEOMDLLIMPEXP DTriangle3d
{
//! corner points addressable as point[i][j]
DPoint3d point[3];
//! Initialize as zeros.
DTriangle3d ();

//! Constructor from points in 00,10,01 index order.
DTriangle3d (DPoint3dCR xyz0, DPoint3dCR xyz10, DPoint3dCR xyz01);
//! Constructor from points in 00,10,01index order.
DTriangle3d (DPoint2dCR xyz0, DPoint2dCR xyz10, DPoint2dCR xyz01);

//! Return surface coordinates at u,v parameters
DPoint3d Evaluate (double u, double v) const;

//! Return surface coordinates at u,v parameters
DPoint3d EvaluateBarycentric (double u, double v, double w) const;
//! Return surface coordinates at u,v parameters
DPoint3d EvaluateBarycentric (DPoint3dCR uvw) const;

//! Return surface coordinates and first derivatives at u,v parameters
void Evaluate (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const;
//! Return surface coordinates and unit normal at u,v parameters
void EvaluateNormal (double u, double v, DPoint3dR xyz, DVec3dR unitNormal) const;

//! Return (as a validated ray) the surface coordinates and unit normal at u,v parameters
ValidatedDRay3d EvaluateUnitNormalRay (double u, double v) const;

//! Return the two edge vectors outward from the origin.
void GetVectorsFromOrigin (DVec3dR vectorU, DVec3dR vectorV) const;
//! return uv and xyz coordinates of the projection to the unbounded plane of the triangle.
//! @return false if axes are not independent.
bool ClosestPointUnbounded (DPoint3dCR spacePoint, DPoint2d &uv, DPoint3d &xyz) const;

//! return uv and xyz coordinates of the projection to the unbounded plane of the triangle.
//! @return false if axes are not independent.
bool ClosestPointUnbounded (DPoint3dCR spacePoint, DPoint2d &uv) const;

//! Return specified edge as a line segment.
//! Edge order is: bottom, diagonal, left with CCW loop direction.
DSegment3d GetCCWEdgeDSegment3d (int i) const;

//! Return specified edge as a ray.
//! Edge order is: bottom, diagonal, left with CCW loop direction.
DRay3d GetCCWEdgeDRay3d (int i) const;


//! Return specified edge as a vector
//! Edge order is: bottom, diagonal, left with CCW loop direction.
DVec3d GetCCWEdgeDVec3d (int i) const;

//! Return a measure of triangle quality.
//! The value is 0 for a 0 area triangle, increases as aspect angles become more equal.
double AspectRatio () const;

//! Return the cross product of VectorsFromOrigin
DVec3d CrossVectorsFromOrigin () const;

//! @description Compute (simple) intersection of a line and a triangle.
//!
//! @remarks If the returned barycentric coordinates all lie within [0,1], then the intersection lies on or inside the triangle.
//! @remarks If the returned ray parameter is nonnegative, then the intersection lies on the (forward) ray.
//!
//! @param ray           IN      ray with which to intersect triangle
//! @param xyzIntersection           OUT     point of intersection
//! @param uvwIntersection   OUT     intersection's barycentric coords relative to triangle
//! @param rayParameter  OUT     intersection's parameter along the ray (intersection = rayOrigin + parameter * rayDirection)
//! @return true if intersection computed
//!
bool TransverseIntersection
(
DRay3dCR     ray,
DPoint3dR    xyzIntersection,
DPoint3dR    uvwIntersection,
double       &rayParameter
) const;
//! Return true if the point is within the triangle's xy.
//! This is valid regardless of whether the triangle points are CW or CCW.
bool IsPointInOrOnXY (DPoint3dCR xyz) const;

//! Test if (u,v) is within the first quadrant unit triangle. (u >= 0, v >= 0, u + v <= 1.0)
static bool IsBarycentricInteriorUV (double u, double v);
};


struct GEOMDLLIMPEXP DPoint3dDVec3dDVec3d
{
DPoint3d origin;
DVec3d vectorU;
DVec3d vectorV;
DPoint3dDVec3dDVec3d (DPoint3dCR _origin, DVec3dCR _vectorU, DVec3d _vectorV);
//! Construct with all explicit xyz coordinates
DPoint3dDVec3dDVec3d (
double ax, double ay, double az,
double ux, double uy, double uz,
double vx, double vy, double vz
);

//! Construct basis for XY plane.
static DPoint3dDVec3dDVec3d FromXYPlane ();
//! Construct basis for XZ plane.
static DPoint3dDVec3dDVec3d FromXZPlane ();
//! Construct basis for YZ plane.
static DPoint3dDVec3dDVec3d FromYZPlane ();
//! Convert triangle edges to vectors.
DPoint3dDVec3dDVec3d (DTriangle3dCR triangle);

//! origin and X Y axes.
DPoint3dDVec3dDVec3d ();
//! origin and axes from ellipse.
DPoint3dDVec3dDVec3d (DEllipse3dCR ellipse);
//! Return a plane parallel to XY from given origin.
static DPoint3dDVec3dDVec3d FromOriginAndParallelToXY (DPoint3d origin, double sizeU, double sizeV);
//! Return a plane parallel to YZ from given origin.
static DPoint3dDVec3dDVec3d FromOriginAndParallelToYZ (DPoint3d origin, double sizeU, double sizeV);
//! Return a plane parallel to XZ from given origin.
static DPoint3dDVec3dDVec3d FromOriginAndParallelToXZ (DPoint3d origin, double sizeU, double sizeV);

//! Return a plane from orgin and u,v target points
static DPoint3dDVec3dDVec3d FromOriginAndTargets (DPoint3dCR origin, DPoint3dCR UTarget, DPoint3dCR VTarget);

//! Return DPlane3d form with unit normal.
ValidatedDPlane3d GetDPlane3dWithUnitNormal () const;
//! return {origin + u * vectorU + v * vectorV}
DPoint3d Evaluate (double u, double v) const;
//! return {u*vectorU + v*vectorV}
DVec3d EvaluateVectorOnly (double u, double v) const;

//! return {origin + u * vectorU + uv.x * vectorV * uv.y}
DPoint3d Evaluate (DPoint2dCR uv) const;
//! return {uv.x * vectorU + uv.y * vectorV}
DVec3d EvaluateVectorOnly (DPoint2dCR uv) const;


//! Evaluate point and tangents at u,v.  (Of course, the tangents are just the originals)
DPoint3dDVec3dDVec3d EvaluateTangents (double u, double v) const;
//! Return coordinate transforms, with unit perpendicular as Z vector in local to world.
bool GetTransformsUnitZ (TransformR localToWorld, TransformR worldToLocal) const;
//! Compute the simple intersection of a segment with the plane.
//! Returns false in any parallel case (both in-plane and off-plane)
bool TransverseIntersection (DSegment3dCR segment, DPoint2dR uv, double &segmentFraction) const;
//! return max difference between corresponding components
double MaxDiff (DPoint3dDVec3dDVec3dCR other) const;

ValidatedDPoint2d ProjectPointToUV (DPoint3dCR xyz) const;

//! return a coordinate frame with normalized X axis along vectorU, normalized Y axis in plane of vectorV and vectorV, unit vector perpendicular.
ValidatedTransform NormalizedLocalToWorldTransform () const;
//! return a transform into the NormalizedLocal frame.
ValidatedTransform WorldToNormalizedLocalTransform () const;

//! Return a transform with the full length VectorU and VectorV axes as X and Y axes, unit vector perpendicular.
ValidatedTransform LocalToWorldTransform () const;
//! Return a the inverse of the LocalToWorldTransform.
ValidatedTransform WorldToLocalTransform () const;
};

/*----------------------------------------------------------------------------+
| BARYCENTRIC COORDINATE FUNCTIONS:
|
| For a given triangle T with vertices v0, v1, v2, every point q in the plane
| of T is uniquely represented by its barycentric coordinates (b0, b1, b2)
| relative to T:
|
| q = b0 * v0 + b1 * v1 + b2 * v2,
| 1 = b0 + b1 + b2.
|
+----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in the xy-plane.
*
* @param pInstance   [out] barycentric coordinates of pPoint relative to T
* @param pPoint         [in] point in plane
* @param pVertex0       [in] vertex 0 of triangle T
* @param pVertex1       [in] vertex 1 of triangle T
* @param pVertex2       [in] vertex 2 of triangle T
* @return true if and only if the area of T is sufficiently large.
+---------------+---------------+---------------+---------------+------*/
Public  GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint2dTriangle

(
DPoint3dP pInstance,
DPoint2dCP pPoint,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2
);

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in the xy-plane.
*
* @param uvw   [out] barycentric coordinates of pPoint relative to T
* @param dUVWdX   [out] derivatives wrt point.x
* @param dUVWdY   [out] derivatives wrt point.y
* @param area  [out] triangle area
* @param point         [in] point in plane
* @param vertex0       [in] vertex 0 of triangle T
* @param vertex1       [in] vertex 1 of triangle T
* @param vertex2       [in] vertex 2 of triangle T
* @return true if and only if the area of T is sufficiently large.
+---------------+---------------+---------------+---------------+------*/
Public  GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint2dTriangle
(
DPoint3dR uvw,
DPoint3dR dUVWdX,
DPoint3dR dUVWdY,
double   &area,
DPoint2dCR point,
DPoint2dCR vertex0,
DPoint2dCR vertex1,
DPoint2dCR vertex2
);

/*-----------------------------------------------------------------*//**
* Given a space point spacePontP, finds the closest point on the plane
* containing the 3 points in pPlanePoint.  Stores the closest point
* coordinates in pClosePoint, and the s and t coordinates (as defined
* in bsiGeom_evaluateSkewedPlane) in sP and tP.
*
* @param pClosePoint [out] point on plane.  May be null pointer
* @param sP [out] parametric coordinate on s axis
* @param tP [out] parametric coordinate on t axis
* @param pPlanePoint [in] origin, s=1, and t=1 points
* @param pSpacePoint [in] point to be projected
* @return true unless the plane points are collinear
+---------------+---------------+---------------+---------------+------*/
Public  GEOMDLLIMPEXP bool     bsiGeom_closestPointOnSkewedPlane

(
DPoint3dP pClosePoint,
double          *sP,
double          *tP,
DPoint3dCP pPlanePoint,
DPoint3dCP pSpacePoint
);

/*-----------------------------------------------------------------*//**
* @description Compute the minimum distance from a point to a triangle.
* @param pSpacePoint   [out] point in space
* @param pVertex0       [in] vertex of T
* @param pVertex1       [in] vertex of T
* @param pVertex2       [in] vertex of T
* @param pClosePoint    [out] projection of space point onto plane of triangle
* @param pBoundedUVW       [out] barycentric coordinates of closest point restricted to triangle
* @param pUnboundedUVW       [out] barycentric coordinates of closest point on plane
* @return minimum distance
+---------------+---------------+---------------+---------------+------*/
Public  GEOMDLLIMPEXP double bsiDPoint3d_minDistToTriangle

(
DPoint3dCP pSpacePoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2,
DPoint3dP pClosePoint,
DPoint3dP pBoundedUVW,
DPoint3dP pUnboundedUVW
);


/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pOrigin, pVector1-pOrigin, pVector2-pOrigin)
* in the xy-plane.
*
* @param pInstance   [out] barycentric coordinates of pPoint relative to T
* @param pPoint         [in] point in plane
* @param pOrigin        [in] vertex of triangle T (may be null for origin)
* @param pVector1       [in] side vector of T (emanating from pOrigin)
* @param pVector2       [in] side vector of T (emanating from pOrigin)
* @return true if and only if the area of T is sufficiently large.
+---------------+---------------+---------------+---------------+------*/
Public  GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint2dTriangleVectors

(
DPoint3dP pInstance,
DPoint2dCP pPoint,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2
);

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in space.
* Points p and r in space have the same barycentric coordinates relative to
* T if and only if they project to the same point q in the plane of T;
* then their barycentric coordinates relative to T are those of q.
*
* @param pInstance   [out] barycentric coordinates of pPoint relative to T
* @param pPoint         [in] point in space
* @param pVertex0       [in] vertex of triangle T
* @param pVertex1       [in] vertex of triangle T
* @param pVertex2       [in] vertex of triangle T
* @return true if and only if the area of T is sufficiently large.
+---------------+---------------+---------------+---------------+------*/
Public  GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint3dTriangle

(
DPoint3dP pInstance,
DPoint3dCP pPoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2
);
/*-----------------------------------------------------------------*//**
* @description Sets this instance to the point in the plane with the given barycentric
* coordinates relative to triangle T (pVertex0, pVertex1, pVertex2).
*
* @param pInstance   [out] point with given barycoords relative to T
* @param pBaryCoords    [in] barycentric coordinates relative to T
* @param pVertex0       [in] vertex 0 of triangle T
* @param pVertex1       [in] vertex 1 of triangle T
* @param pVertex2       [in] vertex 2 of triangle T
+---------------+---------------+---------------+---------------+------*/
Public  GEOMDLLIMPEXP void bsiDPoint2d_fromBarycentricAndDPoint2dTriangle

(
DPoint2dP pInstance,
DPoint3dCP pBaryCoords,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2
);

END_BENTLEY_GEOMETRY_NAMESPACE
