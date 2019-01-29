/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/dray3d.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define MS_DRAY3D_DEFINED


/**
Start point and direction (NOT necessarily unit) for a 3D ray.
@ingroup GROUP_Geometry
*/
struct GEOMDLLIMPEXP DRay3d
{
//! Start point of ray.
DPoint3d origin;
//! Direction vector.  This is NOT necessarily a unit vector.
DVec3d   direction;
#ifdef __cplusplus

//! Initialize a ray from a segment.
static DRay3d From (DSegment3dCR segment);

//! Initialize a ray from origin and vector.
static DRay3d FromOriginAndVector (DPoint3dCR origin, DVec3dCR vector);

//! Initialize a ray from 2d endpoints
static DRay3d FromOriginAndTarget (DPoint2dCR point0, DPoint2dCR point1);

//! Initialize a ray from 3d endpoints
static DRay3d FromOriginAndTarget (DPoint3dCR point0, DPoint3dCR point1);

//! Return a ray with origin interpolated between points.   The ray direction is the point0 to point1 vector scaled by vectorScale
static DRay3d FromIinterpolateWithScaledDifference (DPoint3dCR point0, double fraction, DPoint3dCR point1, double vectorScale);

//! Return a (validated) copy with normalized direction.
ValidatedDRay3d ValidatedNormalize () const;

//! Initialize a ray from a segment.
void InitFrom (DSegment3dCR segment);

//! Initialize a ray from origina and vector
void InitFromOriginAndVector (DPoint3dCR origin, DVec3dCR vector);

//! Initialize a ray from 2d endpoints
void InitFromOriginAndTarget (DPoint2dCR point0, DPoint2dCR point1);

//! Initialize a ray from 2d endpoints
void InitFromOriginAndTarget (DPoint3dCR point0, DPoint3dCR point1);

//! Initialize as intersection of two planes.
//! @return false if planes are parallel.
bool InitFromPlanePlaneIntersection (DPlane3dCR planeA, DPlane3dCR planeB);

//! Return the origin and target of the ray.
void EvaluateEndPoints (DPoint3dR point0, DPoint3dR point1) const;

//! @param [in] param fractional parameter
//! @return evaluated point.
DPoint3d FractionParameterToPoint
(
double  param
) const;


//! @param [in] target
//! @return dot product of (unnormalized) ray vector with vector from ray origin to given point.
double DirectionDotVectorToTarget (DPoint3dCR target) const;

//! @param [in] vector
//! @return dot product of (unnormalized) ray vector with input vector. 
double DirectionDotVector (DVec3d vector) const;

//! Project a point onto the bounded line in 3D.  If nearest point of extended line
//! is outside the 0..1 parameter range, returned values are for nearest endpoint.
//! @param [out] closestPoint computed point
//! @param [out] closestParam fraction parameter at closest point.
//! @param [in] point space point
bool ProjectPointBounded
(
DPoint3dR       closestPoint,
double          &closestParam,
DPoint3dCR      point
) const;

//! Project a point onto the unbounded.
//! @param [out] closestPoint computed point
//! @param [out] closestParam fraction parameter at closest point.
//! @param [in] point space point
bool ProjectPointUnbounded
(
DPoint3dR       closestPoint,
double          &closestParam,
DPoint3dCR      point
) const;

//! @param [out] fractionRay computed fraction on (unbounded) ray
//! @param [out] fractionSegment computed fraction on (bounded) segment
//! @param [out] pointRay computed point on (unbounded) ray
//! @param [out] pointSegment computed point on (bounded) segment
//! @param [in] ray unbounded ray
//! @param [in] segment bounded segment
static void ClosestApproachUnboundedRayBoundedSegment
(
double &fractionRay,
double &fractionSegment,
DPoint3dR pointRay,
DPoint3dR pointSegment,
DRay3dCR ray,
DSegment3dCR segment
);

//! @param [out] fractionA computed fraction on (unbounded) ray
//! @param [out] fractionB computed fraction on (bounded) segment
//! @param [out] pointA computed point on (unbounded) ray
//! @param [out] pointB computed point on (bounded) segment
//! @param [in] rayA unbounded ray
//! @param [in] rayB bounded segment
//! @return false (with rayA start projected to rayB) if parallel rays.
static bool ClosestApproachUnboundedRayUnboundedRay
(
double &fractionA,
double &fractionB,
DPoint3dR pointA,
DPoint3dR pointB,
DRay3dCR rayA,
DRay3dCR rayB
);

//!
//! Return the intersection of the (unbounded) ray with a plane.
//! @param [out] intPoint intersection point
//! @param [out] intParam parameter along the ray
//! @param [in] plane plane (origin and normal)
//! @return false if ray, plane are parallel.
//!
bool Intersect
(
DPoint3dR       intPoint,
double          &intParam,
DPlane3dCR      plane
) const;

//! Return the intersection of this ray with a specified z plane of a coordinate frame.
//! @param [in] frame coordinate frame, e.g. often called localWorldFrame for an object
//! @param [in] localZ z coordinate in local frame (e.g. 0 for local xy plane pierce point.)
//! @param [out] uvw local uvw coordinates.  (w matches input z)
//! @param [out] t parameter along ray.
//! @return true if the ray is not parallel to the plane
bool IntersectZPlane
(
TransformR frame,
double     localZ,
DPoint3dR  uvw,
double     &t
) const;

//!
//! Return the intersection of the (unbounded) ray with a circle, using only
//! xy coordinates.
//! @param [out] intPoint 0, 1, or 2 intersection points.
//! @param [in] pIntParam parameter along the line
//! @param [in] center circle center.
//! @param [in] radius circle radius.
//! @return   number of intersections.
//!
int IntersectCircleXY
(
DPoint3dP       intPoint,
double          *pIntParam,
DPoint3dCR      center,
double          radius
) const;

/*__PUBLISH_SECTION_END__*/

//!
//! Return the intersection of the (unbounded) ray with a bilinear surface
//! @param [out] intPoint 0, 1, or 2 intersection points.
//! @param [out] pRayParameter 0, 1, or 2 intersection parameters on line
//! @param [out] patchParameter 0, 1, or 2 s,t patch parameter pairs.
//! @param [out] point00 00 (lower left) point of hyperboloid
//! @param [out] point10 10 (lower right) point of hyperboloid
//! @param [out] point01 01 (upper left) point of hyperboloid
//! @param [out] point11 11 (upper right) poitn of hyperboloid
//! @return   number of intersections.
//!
int IntersectHyperbolicParaboloid
(
DPoint3dP       intPoint,
double          *pRayParameter,
DPoint2dP       pPatchParameter,
DPoint3dCR      point00,
DPoint3dCR      point10,
DPoint3dCR      point01,
DPoint3dCR      point11
) const;

//!
//! Return the intersection of the (unbounded) ray with a weighted bilinear patch.
//! @param [out] intPoint 0, 1, or 2 intersection points.
//! @param [out] pRayParameter 0, 1, or 2 intersection parameters on line
//! @param [out] patchParameter 0, 1, or 2 s,t patch parameter pairs.
//! @param [in] point00 00 corner of patch.
//! @param [in] w00 00 weight
//! @param [in] point10 10 corner of patch.
//! @param [in] w10 10 weight
//! @param [in] point01 01 corner of patch.
//! @param [in] w01 01 weight
//! @param [in] point11 11 corner of patch.
//! @param [in] w11 11 weight
//! @return   number of intersections.
//!
int IntersectHyperbolicParaboloid
(
DPoint3dP       intPoint,
double          *pRayParameter,
DPoint2dP       patchParameter,
DPoint3dCR      point00,
double          w00,
DPoint3dCR      point10,
double          w10,
DPoint3dCR      point01,
double          w01,
DPoint3dCR      point11,
double          w11
) const;
/*__PUBLISH_SECTION_START__*/

//! Return the transverse intersection of the (unbounded) ray with the plane of an ellipse.
//! @param [in] ellipse center, vector0, vector90 define the plane.
//! @param [out] intersectionPoint intersection in world coordinates. 
//! @param [out] rayFraction fraction along ray
//! @param [out] ellipsePlaneCoordinates intersection coordinates as multiples of vector0 and vector90.
//! @return false if the ray and ellipse vectors are not independent.
//! @remark On a false return, the rayParameter is set to 0, the intersection point and plane coordinates
//!    are the projection of the ray origin on the ellipse plane. (See DEllipse3d::PointToXYLocal)
bool IntersectPlane
(
DEllipse3d      ellipse,
DPoint3dR       intersectionPoint,
double &        rayFraction,
DPoint2dR        ellipsePlaneCoordinates
) const;


// @return true if the ray intersects the range.  false if no intersection.
// @param [in] range clip range
// @param [out] segment clipped segment.
// @param [out] rayFractionRange start and end fractions for portion of ray "in" the range.
bool ClipToRange
(
DRange3dCR range,
DSegment3dR segment, // clipped segment.
DRange1dR  rayFractionRange
) const;

//! Construct transforms for viewing along the ray.
//! @param [out] localToWorld Coordinate frame with origin from ray, z axis in ray direction.
//! @param [out] worldToLocal inverse transformation
bool TryGetViewingTransforms
(
TransformR localToWorld,
TransformR worldToLocal
) const;


#endif
};
END_BENTLEY_GEOMETRY_NAMESPACE
