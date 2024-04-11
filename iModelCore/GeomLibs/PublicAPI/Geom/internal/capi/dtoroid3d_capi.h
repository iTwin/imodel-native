/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Initialize an toroid from center, major and minor radii and optional parameter range.
//! Orientation is set parallel to global axes.
//! @param pCenter IN      toroid center.
//! @param radius  IN      radius of toroid.
//! @param pParameterRange IN      parameter range. If NULL, default is applied.
//!
//!
Public GEOMDLLIMPEXP void               bsiDToroid3d_setCenterRadii
(
DToroid3dP pInstance,
DPoint3dCP pCenter,
double          majorRadius,
double          minorRadius,
DRange2dCP pParameterRange
);

//!
//! Initialize an toroid from full frame and range
//! @param pTransform         IN      coordinate frame.  If NULL, default is applied.
//! @param minorRadiusRatio   IN      radius of minor circles in the local coordinate system
//!                               where major radius is 1.
//! @param pRange             IN      parameter range.  If NULL, default is applied.
//!
//!
Public GEOMDLLIMPEXP void               bsiDToroid3d_set
(
DToroid3dP pInstance,
TransformCP pFrame,
double          minorRadiusRatio,
DRange2dCP pParameterRange
);

//!
//! Set the reference frame of the toroid.
//! @param pFrame IN      coordinate frame.  null indicates an identity transformation.
//!
//!
Public GEOMDLLIMPEXP void               bsiDToroid3d_setFrame
(
DToroid3dP pInstance,
TransformCP pFrame
);

//!
//! Set the parameter range of the toroid.
//! @param pParameterRange IN      limits of longitude and latitude.
//!
//!
Public GEOMDLLIMPEXP void               bsiDToroid3d_setNaturalParameterRange
(
DToroid3dP pInstance,
DRange2dCP pParameterRange
);

//!
//! Convert a local cartesian point into the local toroidal coordinates.
//! @param pTheta     OUT     angle in unit spherical coordiantes
//! @param pPhi       OUT     z coordinate in spherical coordinates
//! @param pR         OUT     radius from major circle, as a multiple of the minor circle radius.
//! @return false if the local point is on the z axis or the major circle.
//!
//!
Public GEOMDLLIMPEXP bool       bsiDToroid3d_localToToroidal
(
DToroid3dCP pToroid,
double    *pTheta,
double    *pPhi,
double    *pR,
DPoint3dCP pPoint
);

//!
//! test if a longitude angle is in the ellipsoid's parameter range.
//!
//!
Public GEOMDLLIMPEXP bool        bsiDToroid3d_longitudeInRange
(
DToroid3dCP pInstance,
double      longitude
);

//!
//! test if a latitude is in the ellipsoid's parameter range.
//!
//!
Public GEOMDLLIMPEXP bool        bsiDToroid3d_latitudeInRange
(
DToroid3dCP pInstance,
double          latitude
);

//!
//! Intersect the sphere with a line segment.
//! @param pXYZ   OUT     array of 0, 1, or 2 coordinates of cartesian intersection points.
//!                       May be NULL.
//! @param pUVW   OUT     array of 0, 1, or 2 coordinates of intersection points in the
//!                       local coordinates of the toroid frame.   (Points on the
//!                       unit sphere).  May be NULL.
//! @param pLineParameter OUT     array of 0, 1, or 2 parameters with respect to the line.
//! @param DRay3d         IN      ray to intersect.
//! @return number of intersections.
//!
//!
Public GEOMDLLIMPEXP int                bsiDToroid3d_intersectDSegment3d
(
DToroid3dCP pInstance,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double          *pLineParameter,
DSegment3dCP pSegment
);

//!
//! Intersect the toroid with a ray.   Beware that up to 4 intersection points may
//! be returned.
//! @param pXYZ   OUT     array of 0 to 4 (!!!) coordinates of cartesian intersection points.
//!                       May be NULL.
//! @param pUVW   OUT     array of 0 to 4 (!!!) coordinates of intersection points in the
//!                       local coordinates of the toroid frame.   (Points on the
//!                       unit sphere).  May be NULL.
//! @param pLineParameter OUT     array of 0 to 4 (!!!) parameters with respect to the line.
//! @return number of intersections.
//!
//!
Public GEOMDLLIMPEXP int                bsiDToroid3d_intersectDRay3d
(
DToroid3dCP pInstance,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double          *pLineParameter,
DRay3dCP pRay
);

//!
//!
//! Apply a transformation to the source ellipseoid.  Only affine parts of the homogeneous
//! matrix are applied.
//! @param pTransform IN      transformation to apply.
//! @param pSource IN      source ellipse.
//!
//!
Public GEOMDLLIMPEXP void     bsiDToroid_applyAffineDMatrix4d
(
DToroid3dP pDest,
DMatrix4dCP pMatrix,
DToroid3dCP pSource
);

//!
//! Return a (full) meridian at specified longitude.
//! @param pEllipse OUT     full ellipse at specified longitude.   0-degree vector is on
//!           is on the equator.  90 degree vector is to the north pole.
//! @param longitude IN      longitude angle (radians)
//!
//!
Public GEOMDLLIMPEXP void     bsiDToroid3d_getMeridian
(
DToroid3dCP pToroid,
DEllipse3dP pEllipse,
double      longitude
);

//!
//! Return a (range restricted) meridian at specified longitude.
//! @param pEllipse <= full ellipse at specified longitude.   0-degree vector is on
//!           is on the equator.  90 degree vector is to the north pole.
//! @param longitude => longitude angle (radians)
//!
//!
Public GEOMDLLIMPEXP void     bsiDToroid3d_getMeridianLocal
(
DToroid3dCP pToroid,
DEllipse3dP pEllipse,
double      longitude
);

//!
//! Return a (range restricted) meridian at specified cos and sine coordinates in the minor circle space.
//!  (usual usage A: x=+-1, y=+-1 to get "square duct" edges for loose range)
//!  (usual usage B: x=cos(phi), y=sin(phi) to get true parallel on torus
//! @param pEllipse <= full ellipse at specified longitude.   0-degree vector is on
//!           is on the equator.  90 degree vector is to the north pole.
//! @param x "cosine like" coordinate in small circle plane.
//! @param y "sine like" coordinate in small circle plane
//!
//!
Public GEOMDLLIMPEXP void     bsiDToroid3d_getParallelFromSmallCircleCoordinates
(
DToroid3dCP pToroid,
DEllipse3dP pEllipse,
double      x,
double      y
);

//!
//! Return a (full) parallel of latitude.
//! @param pEllipse OUT     full ellipse at specified latitude.  0 and 90 degree vectors
//!           are 0 and 90 degrees latitude.
//! @param latitude IN      latitude angle (radians)
//!
//!
Public GEOMDLLIMPEXP void     bsiDToroid3d_getParallel
(
DToroid3dCP pToroid,
DEllipse3dP pEllipse,
double      latitude
);

//!
//! Convert from parametric to cartesian, with the parametric coordinate given as trig values.
//!
//!
Public GEOMDLLIMPEXP void    bsiDToroid3d_trigParameterToPoint
(
DToroid3dCP pToroid,
DPoint3dP pPoint,
double      cosTheta,
double      sinTheta,
double      cosPhi,
double      sinPhi
);

//!
//! @param pPoint OUT     evaluated point
//! @param thetaFraction IN      angular position, as a fraction of the patch longitude range.
//! @param phiFraction IN      axial position, as a fraction of the patch latitude range.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDToroid3d_fractionParameterToDPoint3d
(
DToroid3dCP pToroid,
DPoint3dP pPoint,
double    thetaFraction,
double    phiFraction
);

//!
//! @param thetaFraction  OUT     longitude, as a fraction of the patch longitude range
//! @param phiFraction    OUT     latitude, as a fraction of the latitude range
//! @param pPoint         IN      evaluated point
//!
//!
Public GEOMDLLIMPEXP bool    bsiDToroid3d_dPoint3dToFractionParameter
(
DToroid3dCP pToroid,
double    *pThetaFraction,
double  *pPhiFraction,
DPoint3dCP pPoint
);

//!
//! @param pTheta IN      longitude
//! @param pPhi   IN      latitude
//! @param thetaFraction IN      longitude, as a fraction of the patch longitude range.
//! @param phiFraction IN      latitude, as a fraction of the patch latitude range.
//!
//!
Public GEOMDLLIMPEXP void     bsiDToroid3d_fractionParameterToNaturalParameter
(
DToroid3dCP pToroid,
double    *pTheta,
double    *pPhi,
double    thetaFraction,
double    phiFraction
);

//!
//! @param pTheta IN      longitude
//! @param pPhi IN      latitude
//! @param theta  IN      longitude
//! @param phi    IN      latitude
//!
//!
Public GEOMDLLIMPEXP void     bsiDToroid3d_naturalParameterToFractionParameter
(
DToroid3dCP pToroid,
double    *pThetaFraction,
double    *pPhiFraction,
double    theta,
double    phi
);

//!
//! Test if the ellipsoid range is the full parameter space.
//!
//!
//!
Public GEOMDLLIMPEXP bool               bsiDToroid3d_isComplete (DToroid3dCP pInstance);

//!
//! @return true if the 1st natural parameter covers the complete range of the underlying
//!           analytic surface.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDToroid3d_isParam1Complete (DToroid3dCP pToroid);

//!
//! @return true if the 2nd natural parameter covers the complete range of the underlying
//!           analytic surface.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDToroid3d_isParam2Complete (DToroid3dCP pToroid);

//!
//! Get the reference frame of the ellipsoid.
//! @param pFrame     OUT     coordinate frame with (0,0,0) at ellispoid center, (0,0,1) at
//!                       is the out-of-plane vector, (1,0,0) and (0,1,0) at 0 and 90 degrees longitude
//!                       on the major circle.
//!
//!
//!
Public GEOMDLLIMPEXP void               bsiDToroid3d_getFrame
(
DToroid3dCP pInstance,
TransformP pFrame
);

//!
//! Get the ellise's inverse coordinate, i.e. transform from the global space to
//! a the system where the base is a unit xy circle.
//! @param pInverseFrame OUT     inverse frame.
//! @return true if the inverse is was computed.
//!
//!
Public GEOMDLLIMPEXP bool               bsiDToroid3d_getInverseFrame
(
DToroid3dCP pInstance,
TransformP pInverseFrame
);

//!
//! Convert a world cartesian point to the local cartesian system.
//! @param pWorld OUT     world coordinates
//! @param pLocal IN      coordinates in local frame of toroid.
//! @return true if the local to world transformation was invertible.
//!
//!
Public GEOMDLLIMPEXP void    bsiDToroid3d_localToWorld
(
DToroid3dCP pInstance,
DPoint3dP pWorld,
DPoint3dCP pLocal
);

//!
//! Convert a local cartesian point to the world coordinate system.
//! @param pLocal OUT     coordinates in local frame
//! @param pWorld IN      world coordinates
//!
//!
Public GEOMDLLIMPEXP bool        bsiDToroid3d_worldToLocal
(
DToroid3dCP pInstance,
DPoint3dP pLocal,
DPoint3dCP pWorld
);

//!
//! Evaluate the implicit function for the ellipsoid.
//! @param pPoint IN      point where the implicit function is evaluated.
//!
//!
Public GEOMDLLIMPEXP double bsiDToroid3d_implicitFunctionValue
(
DToroid3dCP pToroid,
DPoint3dCP pPoint
);

/* METHOD(DToroid3d,none,implicitFunctionValueLocal) */
//!
//! Evaluate the implicit function for the ellipsoid.
//! @param pPoint => point where the implicit function is evaluated, already in local coordinates
//!
//!
Public GEOMDLLIMPEXP double bsiDToroid3d_implicitFunctionValueLocal
(
DToroid3dCP pToroid,
DPoint3dCP pLocalPoint
);

//!
//! @param pPoint OUT     evaluated point
//! @param theta  IN      longitude
//! @param phi    IN      latitude
//!
//!
Public GEOMDLLIMPEXP bool        bsiDToroid3d_naturalParameterToDPoint3d
(
DToroid3dCP pToroid,
DPoint3dP pPoint,
double  theta,
double  phi
);

//!
//! Compute the spherical coordinates of a given xyz point.  When the point is exactly on the
//! surface, this function is the inverse of naturalParameterToPoint.   For a point off of the
//! surface, this inversion returns the parameter where the ellipsoid intersects a line
//! to the center.  This is a true projection if the surface is a perfect sphere.
//! @param pParam1 OUT     natural parameter
//! @param pParam2 OUT     natural parameter
//! @param pPoint  IN      xyz coordinates
//!
//!
Public GEOMDLLIMPEXP bool         bsiDToroid3d_dPoint3dToNaturalParameter
(
DToroid3dCP pToroid,
double  *pParam1,
double  *pParam2,
DPoint3dCP pPoint
);

//!
//! Return the range of the natural parameter for the active surface patch.
//! @param pParam1Start IN      start value of natural parameter.
//! @param pParam1End   IN      end value of natural parameter.
//! @param pParam2Start IN      start value of natural parameter.
//! @param pParam2End   IN      end value of natural parameter.
//!
//!
Public GEOMDLLIMPEXP void     bsiDToroid3d_getScalarNaturalParameterRange
(
DToroid3dCP pToroid,
double    *pParam1Start,
double    *pParam1End,
double    *pParam2Start,
double    *pParam2End
);

//!
//! Get the parameter range of the ellipsoid.
//!
//!
//!
Public GEOMDLLIMPEXP void               bsiDToroid3d_getNaturalParameterRange
(
DToroid3dCP pInstance,
DRange2dP pParameterRange
);

//!
//! Get the parameter range as start/sweep pairs.
//!
//!
Public GEOMDLLIMPEXP void    bsiDToroid3d_getScalarNaturalParameterSweep
(
DToroid3dCP pInstance,
double          *pTheta0,
double          *pThetaSweep,
double          *pPhi0,
double          *pPhiSweep
);

//!
//! Return the range of the natural parameters for a complete surface.
//! @param pParam1Start IN      start value of natural parameter.
//! @param pParam1End   IN      end value of natural parameter.
//! @param pParam2Start IN      start value of natural parameter.
//! @param pParam2End   IN      end value of natural parameter.
//!
//!
Public GEOMDLLIMPEXP void     bsiDToroid3d_getCompleteNaturalParameterRange
(
DToroid3dCP pToroid,
double  *pParam1Start,
double  *pParam1End,
double  *pParam2Start,
double  *pParam2End
);

//!
//! @description Test whether the given local point lies within selective parametric extents of the cone.
//! @return true if the local point is "in" the cone.
//! @param pCone => torus to evaluate
//! @param pUVW => local coordinates to test.
//! @param applyLongitudeLimits => true to test longitude
//! @param applyLatitudeLimits => true to test latitude
//! @param applyRadiusLimits => true to test for radius
//! @group "DToroid3d Local Coordinates"
//!
Public GEOMDLLIMPEXP bool        bsiDToroid3d_isLocalDPoint3dInSelectiveBounds
(
DToroid3dCP pTorus,
DPoint3dCP pUVW,
bool            applyLongitudeLimits,
bool            applyLatitudeLimits,
bool            applyRadiusLimits
);


//!
//! @description Intersect an (unbounded) ray with the (bounded) torus, considering the minor circles as
//!   complete but considering angle range on the major circle.
//! The result may include 4 points from the torus and 2 from caps.
//! In perfect math, there can really only be 4 points.  However, if the line passes through
//! a circle point on the end cap, there are coincident points from the end cap and torus intersections.
//! This function does not try to detect this -- it just loads up the output arrays with up to 6 points.
//! @param pCone  => torus to evaluate
//! @param pXYZ   <= array of 0 to 6 (SIX) coordinates of cartesian intersection points.
//!                       May be NULL.
//! @param pUVW   <= array of 0 to 6 (SIX) coordinates of intersection points in the
//!                       local coordinates of the cone frame.   May be NULL.
//! @param pLineParameter <= array of 0 to 4 (SIX) parameters with respect to the line.
//! @param pRay => (unbounded) ray to intersect with torus and caps
//! @param includeEndCaps => Whether to include the elliptical caps
//! @return number of intersections.
//! @group "DTorus3d Intersection"
//!
Public GEOMDLLIMPEXP int             bsiDTorus3d_intersectDRay3dCaps
(
DToroid3dCP pTorus,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double    *pLineParameter,
DRay3dCP pRay,
bool              includeEndCaps
);

//!
//! @description Return intersections of a bezier curve with the toroid.
//! @param pToroid        => toroid to evaluate
//! @param pIntersectionParam OUT intersection parameters on the bezier
//! @param pIntersectionXYZ OUT world coordinate intersection points
//! @param pIntersectionUVW OUT local frame intersection points
//! @param pNumToroidIntersection OUT number of intersections on toroid
//! @param pNumCapIntersection OUT number of intersections on cap.
//! @param maxIntersection IN size limit for all output arrays.
//! @param pBezierPoles IN bezier curve
//! @param order IN bezier order
//! @param toroidSelect IN
//!    <list>
//!    <item>0 for no toroid intersections.</item>
//!    <item>1 for toroid intersections with unbounded toroid.</item>
//!    <item>2 for toroid intersections with bounded patch</item>
//!    </list>
//! @param capSelect IN
//!    <list>
//!    <item>0 for no cap intersections.</item>
//!    <item>1 for intersections with cap disks at bounded patch limits.</item>
//!    </list>
//! @param extendCurve IN true to extend beyond curve ends
//! @group "DToroid3d Parameter Range"
//!
Public GEOMDLLIMPEXP bool    bsiDToroid3d_intersectBezierCurve
(
DToroid3dCP pToroid,
double *pIntersectionParam,
DPoint3d *pIntersectionXYZ,
DPoint3d *pIntersectionUVW,
int *pNumToroidIntersection,
int *pNumCapIntersection,
int maxIntersection,
DPoint4dCP pWorldPoles,
int        order,
int        toroidSelect,
int        capSelect,
bool       extendCurve
);

END_BENTLEY_GEOMETRY_NAMESPACE

