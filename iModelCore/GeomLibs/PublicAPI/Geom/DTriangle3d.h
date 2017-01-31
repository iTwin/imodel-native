/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/DTriangle3d.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
END_BENTLEY_GEOMETRY_NAMESPACE
