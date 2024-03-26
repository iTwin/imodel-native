/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Set all fields of the cone from arguments.
//! @param pCone OUT     cone to initialize
//! @param pFrame IN      Transformation whose translation, x, y, and z columns are the
//!                   center, 0-degree vector, 90-degree vector, and axis direction.
//!                   if <code>null</code>, an identity is used.
//! @param radiusFraction IN      top circle radius divided by bottom circle radius.
//! @param pRange     IN      parameter range in <code>(theta, phi)</code> coordinates.
//! @group "DCone3d Initialization"
//!
Public GEOMDLLIMPEXP void    bsiDCone3d_set
(
DCone3dP pCone,
TransformCP pFrame,
double       radiusFraction,
DRange2dCP pRange
);

//!
//! @description Set all fields of the cone from arguments.
//! @param pCone OUT     cone to initialize
//! @param pFrame IN      Transformation whose translation, x, y, and z columns are the
//!                   center, 0-degree vector, 90-degree vector, and axis direction.
//!                   if <code>null</code>, an identity is used.
//! @param radiusFraction IN      top circle radius divided by bottom circle radius.
//! @param pRange     IN      parameter range in <code>(theta, z)</code> coordinates.
//! @group "DCone3d Initialization"
//!
Public GEOMDLLIMPEXP void    bsiDCone3d_setFrameAndFraction
(
DCone3dP pCone,
TransformCP pFrame,
double       radiusFraction,
DRange2dCP pRange
);

//!
//! @description Set the cone with given center, axis in z direction, and given top and bottom radii.
//! The cone origin may be placed at either end of the cone, i.e. might not be
//! at the given center.  This will be done to get a nonzero radius at the center.
//! @param pCone OUT     cone to initialize
//! @param pCenter IN      nominal origin of cone.
//! @param r0     IN      radius at z=0
//! @param r1     IN      radius at z=1
//! @param h      IN      cone height.
//! @param pRange IN      parameter range.
//! @group "DCone3d Initialization"
//!
Public GEOMDLLIMPEXP void    bsiDCone3d_setCenterRadiiHeight
(
DCone3dP pCone,
DPoint3dP pCenter,
double    r0,
double    r1,
double    h,
DRange2dCP pRange
);

//!
//! @description Set the reference frame of the cone.
//! @param pCone OUT     cone to modify
//! @param pFrame IN      coordinate frame.  If NULL, default is applied.
//! @group "DCone3d Local Coordinates"
//!
Public GEOMDLLIMPEXP void            bsiDCone3d_setFrame
(
DCone3dP pCone,
TransformCP pFrame
);

//!
//! @description Set the parameter range of the cone.
//! @param pCone OUT     cone to modify
//! @param pParameterRange IN      parameter range to set
//! @group "DCone3d Parameter Range"
//!
Public GEOMDLLIMPEXP void            bsiDCone3d_setParameterRange
(
DCone3dP pCone,
DRange2dCP pParameterRange
);

//!
//! @description Extract the cone definition.
//! @param pCone IN      cone to evaluate
//! @param pFrame     OUT     coordinate frame.   The z axis is the cone axis.  The intersection
//!                           with the xy plane is a unit circle.
//! @param pTaper     OUT     in local coordinates, the radius of the cone cross section (circle)
//!                           at z = 1.
//! @param pRange     OUT     the conic parameter range.
//! @group "DCone3d Queries"
//!
Public GEOMDLLIMPEXP void    bsiDCone3d_get
(
DCone3dCP pCone,
TransformP pFrame,
double          *pTaper,
DRange2dP pRange
);

//!
//! @description Test whether the longitude angle is in the cone's parameter range.
//! @return true if the longitude is in range.
//! @param pCone IN      cone to evaluate
//! @param longitude IN      longitude angle to test
//! @group "DCone3d Parameter Range"
//!
Public GEOMDLLIMPEXP bool        bsiDCone3d_longitudeInRange
(
DCone3dCP pCone,
double          longitude
);

//!
//! @description Test whether the altitude is in the cone's parameter range.
//! @return true if the altitude is in range.
//! @param pCone IN      cone to evaluate
//! @param altitude IN      altitude to test
//! @group "DCone3d Parameter Range"
//!
Public GEOMDLLIMPEXP bool        bsiDCone3d_altitudeInRange
(
DCone3dCP pCone,
double          altitude
);

//!
//! @description Test whether the given local point lies within selective parametric extents of the cone.
//! @return true if the local point is "in" the cone.
//! @param pCone IN      cone to evaluate
//! @param pUVW IN      local coordinates to test.
//! @param applyAngleLimits IN      true to test angle of (x,y) part against angular bounds
//!               (x part of parameter space bounds)
//! @param applyAltitudeLimits IN      true to test local z coordinate against the altitude
//!               bounds (y part of parameter space bounds)
//! @param applyRadiusLimits IN      true to test (x,y) part against the radius
//!               at the local point's z coordinate.
//! @group "DCone3d Local Coordinates"
//!
Public GEOMDLLIMPEXP bool        bsiDCone3d_isLocalDPoint3dInSelectiveBounds
(
DCone3dCP pCone,
DPoint3dCP pUVW,
bool            applyAngleLimits,
bool            applyAltitudeLimits,
bool            applyRadiusLimits
);

//!
//! @description Convert a world cartesian point to the local cartesian system.
//! @param pCone IN      cone to evaluate
//! @param pWorld OUT     world coordinates
//! @param pLocal IN      coordinates in local frame of cone.
//! @group "DCone3d Local Coordinates"
//!
Public GEOMDLLIMPEXP void    bsiDCone3d_localToWorld
(
DCone3dCP pCone,
DPoint3dP pWorld,
DPoint3dCP pLocal
);

//!
//! @description Convert a local cartesian point to the world coordinate system.
//! @param pCone IN      cone to evaluate
//! @param pLocal OUT     coordinates in local frame
//! @param pWorld IN      world coordinates
//! @group "DCone3d Local Coordinates"
//! @return true if the local to world transformation was invertible.
//!
Public GEOMDLLIMPEXP bool        bsiDCone3d_worldToLocal
(
DCone3dCP pCone,
DPoint3dP pLocal,
DPoint3dCP pWorld
);

//!
//! @description Convert a local cartesian point into the local reference cylindrical coordinates.
//!       The theta and z parts of this coordinate system are identical to those
//!       of the conical coordinates.   This is faster than the full conical coordinates
//!       calculation and has no potential divide by zero.
//!       is safe from divide-by-zero.
//! @param pCone IN      cone to evaluate
//! @param pTheta     OUT     angle in unit cylindrical coordiantes
//! @param pZ         OUT     z coordinate in cylindrical coordinates
//! @param pR         OUT     radius in cylindrical coordinates
//! @param pPoint     IN      local Cartesian point
//! @return false if the local point is on the cone/cylinder axis.
//! @group "DCone3d Local Coordinates"
//!
Public GEOMDLLIMPEXP bool       bsiDCone3d_localToCylindrical
(
DCone3dCP pCone,
double    *pTheta,
double    *pZ,
double    *pR,
DPoint3dCP pPoint
);

//!
//! @description Convert a local cartesian point into the local conical coordinates.   This is
//! more expensive than ~mbsiDCone3d_localToCylindrical; the additional expense produces
//! a radius that is normalized to the cone radius, but does not affect the
//! angle and altitude.
//! @param pCone IN      cone to evaluate
//! @param pTheta     OUT     angle in unit conical coordiantes
//! @param pZ         OUT     z coordinate in conical coordinates
//! @param pR         OUT     radius, as a multiple of the cone radius at this z.
//!                       The radius is defined as zero everywhere on the constant-z
//!                       plane through the apex.
//! @param pPoint     IN      local Cartesian point
//! @return true if the inversion is invertible (i.e. point not on axis or apex plane).
//! @group "DCone3d Local Coordinates"
//!
Public GEOMDLLIMPEXP bool       bsiDCone3d_localToConical
(
DCone3dCP pCone,
double    *pTheta,
double    *pZ,
double    *pR,
DPoint3dCP pPoint
);

//!
//! @description Convert from parametric to cartesian, with the parametric coordinate given as trig values
//! and height.
//! @param pCone  IN      cone to evaluate
//! @param pPoint OUT     Cartesian point
//! @param cosTheta IN      cosine of cone angle parameter
//! @param sinTheta IN      sine of cone angle parameter
//! @param z      IN      cone altitude parameter
//! @group "DCone3d Parameterization"
//!
Public GEOMDLLIMPEXP void    bsiDCone3d_trigParameterToPoint
(
DCone3dCP pCone,
DPoint3dP pPoint,
double    cosTheta,
double    sinTheta,
double    z
);

//!
//! @description Intersect the cone with a line segment.
//! @param pCone  IN      cone to evaluate
//! @param pXYZ   OUT     array of 0, 1, or 2 coordinates of cartesian intersection points.
//!                       May be NULL.
//! @param pUVW   OUT     array of 0, 1, or 2 coordinates of intersection points in the
//!                       local coordinates of the cone frame.   May be NULL.
//! @param pLineParameter OUT     array of 0, 1, or 2 parameters with respect to the line.
//! @param pSegment       IN      segment to intersect.
//! @return number of intersections.
//! @group "DCone3d Intersection"
//!
Public GEOMDLLIMPEXP int             bsiDCone3d_intersectDSegment3d
(
DCone3dCP pCone,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double            *pLineParameter,
DSegment3dCP pSegment
);

//!
//! @description Intersect the cone with a line.
//! @param pCone  IN      cone to evaluate
//! @param pXYZ   OUT     array of 0, 1, or 2 coordinates of cartesian intersection points.
//!                       May be NULL.
//! @param pUVW   OUT     array of 0, 1, or 2 coordinates of intersection points in the
//!                       local coordinates of the cone frame.   (Points on the
//!                       unit sphere).  May be NULL.
//! @param pLineParameter OUT     array of 0, 1, or 2 parameters with respect to the line.
//! @param pRay IN      (unbounded) ray to intersect with cone.
//! @return number of intersections.
//! @group "DCone3d Intersection"
//!
Public GEOMDLLIMPEXP int             bsiDCone3d_intersectDRay3d
(
DCone3dCP pCone,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double            *pLineParameter,
DRay3dCP pRay
);

//!
//! @description Intersect an (unbounded) ray with the angularly complete cone/cylinder surface patch
//!   and (optionally) the cap ellipse surface patches.  ANGULAR LIMITS ARE NOT CONSIDERED.
//! @param pCone  IN      cone to evaluate
//! @param pXYZ   OUT     array of 0, 1, or 2 coordinates of cartesian intersection points.
//!                       May be NULL.
//! @param pUVW   OUT     array of 0, 1, or 2 coordinates of intersection points in the
//!                       local coordinates of the cone frame.   May be NULL.
//! @param pLineParameter OUT     array of 0, 1, or 2 parameters with respect to the line.
//! @param pRay IN      (unbounded) ray to intersect with (capped) cone/cylinder.
//! @param includeEndCaps IN      Whether to include the elliptical caps of the cone
//! @return number of intersections.
//! @group "DCone3d Intersection"
//!
Public GEOMDLLIMPEXP int             bsiDCone3d_intersectDRay3dBoundedComplete
(
DCone3dCP pCone,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double            *pLineParameter,
DRay3dCP pRay,
bool              includeEndCaps
);

//!
//! @description Intersect an (unbounded) line with the angularly complete cone/cylinder patch and the (optional) planar caps.
//! @param pCone  IN      cone to evaluate
//! @param pXYZ   OUT     array of 0, 1, or 2 coordinates of cartesian intersection points.
//!                       May be NULL.
//! @param pUVW   OUT     array of 0, 1, or 2 coordinates of intersection points in the
//!                       local coordinates of the cone frame.   May be NULL.
//! @param pLineParameter OUT     array of 0,1, or 2 parameters with respect to the line.
//! @param pSegment IN      (unbounded) line to intersect with (capped) cone/cylinder.
//! @param includeEndCaps IN      Whether to include the elliptical caps of the cone
//! @return number of intersections.
//! @group "DCone3d Intersection"
//!
Public GEOMDLLIMPEXP int             bsiDCone3d_intersectDSegment3dBoundedComplete
(
DCone3dCP pCone,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double            *pLineParameter,
DSegment3dCP pSegment,
bool              includeEndCaps
);

//!
//! @description Return the cone's local radius at the specified local height.
//! @param pCone  IN      cone to evaluate
//! @param z IN      height in local coordinates
//! @return the cone's radius in local coordinates
//! @group "DCone3d Local Coordinates"
//!
Public GEOMDLLIMPEXP double   bsiDCone3d_heightToRadius
(
DCone3dCP pCone,
double          z
);

//!
//! @description Compute angles for silhouettes of the cone with respect to a (possibly perspective) view transformation.
//! @param pCone  IN      cone to evaluate
//! @param pTrigPoint OUT     array where x,y are cosine, sine of
//!                      silhouette angles. z is zero -- maybe a convenient
//!                      place for the angles if you need to stash them
//! @param pMap       IN      view transformation
//! @param pEyePoint IN      eyepoint, in same coordinates.
//!                     For perspective, from xyz, set w=1
//!                     For flat view in direction xyz, set w=0
//! @return number of silhouette angles.
//! @group "DCone3d Silhouette"
//!
Public GEOMDLLIMPEXP int      bsiDCone3d_silhouetteAngles
(
DCone3dCP pCone,
DPoint3dP pTrigPoint,
DMap4dCP pMap,
DPoint4dCP pEyePoint
);

//!
//! @description Return a rule line at specified longitude (angle around cone).
//! to the z range
//! @param pCone  IN      cone to evaluate
//! @param pSegment  OUT     ruling segment.
//! @param theta IN      longitude angle (radians)
//! @return true if theta is within the parameter range for the cone.
//! @group "DCone3d Rule Lines"
//!
Public GEOMDLLIMPEXP bool     bsiDCone3d_getRuleLine
(
DCone3dCP pCone,
DSegment3dP pSegment,
double          theta
);

//!
//! @description Return a cross section at given height along the cone axis.
//! @param pCone  IN      cone to evaluate
//! @param pEllipse OUT     full ellipse at specified latitude.  0 and 90 degree vectors
//!           are 0 and 90 degrees latitude.
//! @param z      IN      altitude parameter
//! @return true if the altitude value is within the cone parameter range.
//! @group "DCone3d Rule Lines"
//!
Public GEOMDLLIMPEXP bool     bsiDCone3d_getCrossSection
(
DCone3dCP pCone,
DEllipse3dP pEllipse,
double          z
);

//!
//! @description Compute a point on the cone given fractional parameters.
//! @param pCone  IN      cone to evaluate
//! @param pPoint OUT     evaluated point
//! @param angleFraction IN      angular position, as a fraction of the patch angle range.
//! @param zFraction IN      axial position, as a fraction of the patch z range.
//! @group "DCone3d Parameterization"
//! @return true
//!
Public GEOMDLLIMPEXP bool     bsiDCone3d_fractionParameterToDPoint3d
(
DCone3dCP pCone,
DPoint3dP pPoint,
double    angleFraction,
double    zFraction
);

//!
//! @description Invert the point onto the cone and return fractional parameters.
//! @param pCone  IN      cone to evaluate
//! @param pThetaFraction  OUT     angular position, as a fraction of the patch angle range.
//! @param pZFraction      OUT     axial position, as a fraction of the patch z range.
//! @param pPoint         IN      evaluated point
//! @group "DCone3d Parameterization"
//! @return false if the local point is on the cone/cylinder axis or the local to world transformation is singular.
//!
Public GEOMDLLIMPEXP bool    bsiDCone3d_dPoint3dToFractionParameter
(
DCone3dCP pCone,
double    *pThetaFraction,
double    *pZFraction,
DPoint3dCP pPoint
);

//!
//! @description Convert fractional parameters to natural parameters.
//! @param pCone  IN      cone to evaluate
//! @param pTheta IN      angle around cross sectional circle
//! @param pZ     IN      height along (local) z axis
//! @param thetaFraction IN      angular position, as a fraction of the patch angle range.
//! @param zFraction IN      axial position, as a fraction of the patch z range.
//! @group "DCone3d Parameterization"
//!
Public GEOMDLLIMPEXP void     bsiDCone3d_fractionParameterToNaturalParameter
(
DCone3dCP pCone,
double    *pTheta,
double    *pZ,
double    thetaFraction,
double    zFraction
);

//!
//! @description Convert natural parameters to fractional parameters.
//! @param pCone  IN      cone to evaluate
//! @param pThetaFraction IN      angle parameter as a fraction of the patch.
//! @param pZFraction IN      axis parameter as fraction of the patch.
//! @param theta IN      natural angular position
//! @param z IN      natural axial position
//! @group "DCone3d Parameterization"
//!
Public GEOMDLLIMPEXP void     bsiDCone3d_naturalParameterToFractionParameter
(
DCone3dCP pCone,
double    *pThetaFraction,
double    *pZFraction,
double    theta,
double    z
);

//!
//! @description Test whether the cone range is the full parameter space.
//! @return true if the cone range is complete.
//! @param pCone  IN      cone to evaluate
//! @group "DCone3d Parameterization"
//!
Public GEOMDLLIMPEXP bool            bsiDCone3d_isComplete (DCone3dCP pCone);

//!
//! @description Test whether the cone angle parameter range covers the complete circular range of the cross sections.
//! @return true if the cone angle parameter range is complete.
//! @param pCone  IN      cone to evaluate
//! @group "DCone3d Parameterization"
//!
Public GEOMDLLIMPEXP bool     bsiDCone3d_isParam1Complete (DCone3dCP pCone);

//!
//! @description Test whether the cone z range covers the complete axis of the local coordinate system.
//! @return true if the z range is complete.
//! @param pCone  IN      cone to evaluate
//! @group "DCone3d Parameterization"
//!
Public GEOMDLLIMPEXP bool     bsiDCone3d_isParam2Complete (DCone3dCP pCone);


//!
//! @description Get the reference frame of the cone.
//! @param pCone      IN      cone to evaluate
//! @param pFrame         OUT     reference frame
//! @group "DCone3d Local Coordinates"
//!
Public GEOMDLLIMPEXP void            bsiDCone3d_getTransform
(
DCone3dCP pCone,
TransformP pFrame
);

//!
//! @description Get the reference frame of the cone.
//! @param pCone      IN      cone to evaluate
//! @param pFrame         OUT     reference frame
//! @group "DCone3d Local Coordinates"
//!
Public GEOMDLLIMPEXP void            bsiDCone3d_getFrame
(
DCone3dCP pCone,
TransformP pFrame
);

//!
//! @description Get the cone's inverse coordinate, i.e. transform from the global space to
//! a the system where the base is a unit xy circle.
//! @param pCone      IN      cone to evaluate
//! @param pInverseFrame  OUT     inverse frame.
//! @return true if the inverse is was computed.
//! @group "DCone3d Local Coordinates"
//!
Public GEOMDLLIMPEXP bool            bsiDCone3d_getInverseFrame
(
DCone3dCP pCone,
TransformP pInverseFrame
);

/* METHOD(DCone3d,none,getInverseFrame) */
//!
//! @description Get the cone's inverse coordinate, i.e. transform from the global space to
//! a the system where the base is a unit xy circle.
//! @param pCone      => cone to evaluate
//! @param pInverseFrame  <= inverse frame.
//! @return true if the inverse is was computed.
//! @group "DCone3d Local Coordinates"
//!
Public GEOMDLLIMPEXP bool            bsiDCone3d_getInverseTransform
(
DCone3dCP pCone,
TransformP pInverseFrame
);

//!
//! @description Evaluate the implicit function for the cone.
//! @param pCone  IN      cone to evaluate
//! @param pPoint IN      point where the implicit function is evaluated.
//! @return Value of implicit function (equals zero if point is on the cone)
//! @group "DCone3d Implicit"
//!
Public GEOMDLLIMPEXP double bsiDCone3d_implicitFunctionValue
(
DCone3dCP pCone,
DPoint3dCP pPoint
);

//!
//! @description Compute a point on the cone given its parameters.
//! @param pCone  IN      cone to evaluate
//! @param pPoint OUT     evaluated point
//! @param theta IN      angle around cone
//! @param z     IN      fraction of z axis.
//! @return true
//! @group "DCone3d Parameterization"
//!
Public GEOMDLLIMPEXP bool     bsiDCone3d_naturalParameterToDPoint3d
(
DCone3dCP pCone,
DPoint3dP pPoint,
double    theta,
double    z
);

//!
//! @description Compute a natural parameter of a given xyz point.  When the point is exactly on the
//! surface, this function is the inverse of naturalParameterToPoint.   For a point off the
//! surface, this inversion should be predictable but need not be a true projection.
//! @param pCone  IN      cone to evaluate
//! @param pParam1 OUT     natural parameter
//! @param pParam2 OUT     natural parameter
//! @param pPoint  IN      xyz coordinates
//! @return false if the local point is on the cone/cylinder axis or the local to world transformation is singular.
//! @group "DCone3d Projection"
//!
Public GEOMDLLIMPEXP bool         bsiDCone3d_dPoint3dToNaturalParameter
(
DCone3dCP pCone,
double    *pParam1,
double    *pParam2,
DPoint3dCP pPoint
);

//!
//! @description Return the range of the natural parameter for the active surface patch.
//! @param pCone  IN      cone to evaluate
//! @param pParam1Start IN      start value of natural parameter.
//! @param pParam1End   IN      end value of natural parameter.
//! @param pParam2Start IN      start value of natural parameter.
//! @param pParam2End   IN      end value of natural parameter.
//! @group "DCone3d Parameter Range"
//!
Public GEOMDLLIMPEXP void     bsiDCone3d_getScalarNaturalParameterRange
(
DCone3dCP pCone,
double    *pParam1Start,
double    *pParam1End,
double    *pParam2Start,
double    *pParam2End
);

//!
//! @description Get the parameter range of the cone.
//! @param pCone          IN      cone to evaluate
//! @param pParameterRange    OUT     natural parameter range
//! @group "DCone3d Parameter Range"
//!
Public GEOMDLLIMPEXP void            bsiDCone3d_getNaturalParameterRange
(
DCone3dCP pCone,
DRange2dP pParameterRange
);

//!
//! @description Get the parameter range as start/sweep pairs.
//! @param pCone      IN      cone to evaluate
//! @param pTheta0        OUT     start angle
//! @param pThetaSweep    OUT     angle sweep
//! @param pZ0            OUT     start altitude
//! @param pZSweep        OUT     altitude sweep
//! @group "DCone3d Parameter Range"
//!
Public GEOMDLLIMPEXP void    bsiDCone3d_getScalarNaturalParameterSweep
(
DCone3dCP pCone,
double          *pTheta0,
double          *pThetaSweep,
double          *pZ0,
double          *pZSweep
);

//!
//! @description Return the range of the natural parameters for a complete surface.
//! @param pCone        IN      cone to evaluate
//! @param pParam1Start OUT     start value of natural parameter.
//! @param pParam1End   OUT     end value of natural parameter.
//! @param pParam2Start OUT     start value of natural parameter.
//! @param pParam2End   OUT     end value of natural parameter.
//! @group "DCone3d Parameter Range"
//!
Public GEOMDLLIMPEXP void     bsiDCone3d_getCompleteNaturalParameterRange
(
DCone3dCP pCone,
double    *pParam1Start,
double    *pParam1End,
double    *pParam2Start,
double    *pParam2End
);

//!
//! @description Return intersections of a bezier curve with the cone.
//! @param pCone        => cone to evaluate
//! @param pIntersectionParam OUT intersection parameters on the bezier
//! @param pIntersectionXYZ OUT world coordinate intersection points
//! @param pIntersectionUVW OUT local frame intersection points
//! @param pNumConeIntersection OUT number of intersections on cone
//! @param pNumCapIntersection OUT number of intersections on cap.
//! @param maxIntersection IN size limit for all output arrays.
//! @param pBezierPoles IN bezier curve
//! @param order IN bezier order
//! @param coneSelect IN
//!    <list>
//!    <item>0 for no cone intersections.</item>
//!    <item>1 for cone intersections with unbounded cone.</item>
//!    <item>2 for cone intersections within 01 w limits.</item>
//!    <item>3 for cone intersections with bounded patch</item>
//!    </list>
//! @param capSelect IN
//!    <list>
//!    <item>0 for no cap intersections.</item>
//!    <item>1 for intersections with unbounded cap planes at 01 w.</item>
//!    <item>2 for intersections within bounded cap disks at 01 w.</item>
//!    <item>3 for intersections within bounded cap disks at bounded patch w limits.</item>
//!    </list>
//! @group "DCone3d Parameter Range"
//!
Public GEOMDLLIMPEXP bool    bsiDCone3d_intersectBezierCurve
(
DCone3dCP pCone,
double *pIntersectionParam,
DPoint3d *pIntersectionXYZ,
DPoint3d *pIntersectionUVW,
int *pNumConeIntersection,
int *pNumCapIntersection,
int maxIntersection,
DPoint4dCP pWorldPoles,
int        order,
int        coneSelect,
int        capSelect
);
END_BENTLEY_GEOMETRY_NAMESPACE

