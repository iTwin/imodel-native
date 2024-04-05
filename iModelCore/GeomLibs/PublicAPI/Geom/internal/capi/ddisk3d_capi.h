/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Initialize all fields of the disk.
//! @param pFrame IN      Transformation whose translation, x, y, and z columns are the
//!                   center, 0-degree vector, 90-degree vector, and off-plane direction.
//!                   if <code>null</code>, an identity is used.
//! @param pRange     IN      parameter range in <code>(r, theta)</code> coordinates.
//!
//!
Public GEOMDLLIMPEXP void    bsiDDisk3d_set
(
DDisk3dP pDisk,
TransformCP pFrame,
DRange2dCP pRange
);

//!
//! Initialize the disk with given center, axis in z direction, reference radius, and parameter
//! range.
//! @param r      IN      reference radius.  A the unit circle of the local system maps
//!                   to a circle of this radius in the xy plane.  The out-of-plane
//!                   direction is construted with the same z axis length.
//! @param pRange IN      parameter range.
//!
//!
Public GEOMDLLIMPEXP void    bsiDDisk3d_setCenterRadii
(
DDisk3dP pDisk,
DPoint3dP pCenter,
double    r,
DRange2dCP pRange
);

//!
//! Set the reference frame of the disk.
//! @param pTransform IN      coordinate frame.  If NULL, default is applied.
//!
//!
Public GEOMDLLIMPEXP void               bsiDDisk3d_setFrame
(
DDisk3dP pInstance,
TransformCP pFrame
);

//!
//! Set the parameter range of the disk.
//!
//!
//!
Public GEOMDLLIMPEXP void               bsiDDisk3d_setParameterRange
(
DDisk3dP pInstance,
DRange2dCP pParameterRange
);

//!
//! @param pFrame     OUT     coordinate frame.   The z axis is the disk axis.  The disk
//!                       parameterization (polar coordinates) is in the xy plane.
//! @param pRange     OUT     the conic parameter range.
//!
//!
Public GEOMDLLIMPEXP void    bsiDDisk3d_get
(
DDisk3dCP pDisk,
TransformP pFrame,
DRange2dP pRange
);

//!
//! test if an angle is in the disk's parameter range.
//!
//!
Public GEOMDLLIMPEXP bool        bsiDDisk3d_angleInRange
(
DDisk3dCP pInstance,
double          longitude
);

//!
//! test if a radius in the disk's parameter range.
//!
//!
Public GEOMDLLIMPEXP bool        bsiDDisk3d_radiusInRange
(
DDisk3dCP pInstance,
double          altitude
);

//!
//! Convert a world cartesian point to the local cartesian system.
//! @param pWorld OUT     world coordinates
//! @param pLocal IN      coordinates in local frame of disk.
//!
//!
Public GEOMDLLIMPEXP void    bsiDDisk3d_localToWorld
(
DDisk3dCP pDisk,
DPoint3dP pWorld,
DPoint3dCP pLocal
);

//!
//! Convert a local cartesian point to the world coordinate system.
//! @param pLocal OUT     coordinates in local frame
//! @param pWorld IN      world coordinates
//! @return true if the coordinate frame is invertible.
//!
//!
Public GEOMDLLIMPEXP bool        bsiDDisk3d_worldToLocal
(
DDisk3dCP pDisk,
DPoint3dP pLocal,
DPoint3dCP pWorld
);

//!
//! Convert a local cartesian point into the local polar coordinates.
//!       The z part of this coordinate system is identical to the cartesian z.
//!       of the conical coordinates.   This is faster than the full conical coordinates
//!       calculation and has no potential divide by zero.
//!       is safe from divide-by-zero.
//! @param pTheta     OUT     angle in unit cylindrical coordiantes
//! @param pZ         OUT     z coordinate in cylindrical coordinates
//! @param pR         OUT     radius in cylindrical coordinates
//! @return false if the local point is on the disk/cylinder axis.
//!
//!
Public GEOMDLLIMPEXP bool       bsiDDisk3d_localToPolar
(
DDisk3dCP pDisk,
double    *pR,
double    *pTheta,
double    *pZ,
DPoint3dCP pPoint
);

//!
//! Convert from parametric to cartesian, with the parametric coordinate given as
//! radius and pre-evaluated sine/cosine values.
//! @param pPoint OUT     point on the plane of the disk.
//! @param radius IN      nominal radius in local polar coordinates.  (Actual radius may
//!                   differ if cosine and sine values are scaled rather than true trig values.)
//! @param cosTheta IN      cosine of angle.
//! @param sinTheta IN      sine of angle.
//!
//!
Public GEOMDLLIMPEXP void    bsiDDisk3d_trigParameterToPoint
(
DDisk3dCP pDisk,
DPoint3dP pPoint,
double  radius,
double  cosTheta,
double  sinTheta
);

//!
//! Intersect the disk with a line segment.
//! @param pXYZ   OUT     array of 0, 1, or 2 coordinates of cartesian intersection points.
//!                       May be NULL.
//! @param pUVW   OUT     array of 0, 1, or 2 coordinates of intersection points in the
//!                       local coordinates of the (r,theta,z) frame.   (Points on the
//!                       xy plane.)
//! @param pLineParameter OUT     array of 0, 1, or 2 parameters with respect to the line.
//! @param DRay3d         IN      ray to intersect.
//! @return number of intersections.
//!
//!
Public GEOMDLLIMPEXP int                bsiDDisk3d_intersectDSegment3d
(
DDisk3dCP pInstance,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double          *pLineParameter,
DSegment3dCP pSegment
);

//!
//! Intersect the disk with a line segment.
//! @param pXYZ   OUT     coordinates of cartesian intersection point. May be NULL.
//! @param pUVW   OUT     intersection point in the local coordinate frame. (Use
//!                       localToPolar to convert to (r,theta, z)).
//! @param pLineParameter OUT     parameter on line.
//! @return number of intersections.
//!
//!
Public GEOMDLLIMPEXP bool       bsiDDisk3d_intersectDRay3d
(
DDisk3dCP pDisk,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double      *pLineParameter,
DRay3dCP pRay
);

//!
//! Return a rule line at specified longitude (angle around disk).
//! to the z range
//! @param pSegment  OUT     ruling segment.
//! @param theta IN      longitude angle (radians)
//! @return true if theta is within the parameter range for the disk.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDDisk3d_getRuleLine
(
DDisk3dCP pDisk,
DSegment3dP pSegment,
double          theta
);

//!
//! Return a cross section at given radius in the polar coordinate system.
//! @param pEllipse OUT     ellipse at specified latitude.  0 and 90 degree vectors
//!           are 0 and 90 degrees latitude.
//! @param z      IN      altitude parameter
//! @return true if the radius value is within the disk parameter range.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDDisk3d_getCrossSection
(
DDisk3dCP pDisk,
DEllipse3dP pEllipse,
double          r
);

//!
//! @param pPoint OUT     evaluated point
//! @param radiusFraction IN      radial position, as a fraction of the patch radial range.
//! @param angleFraction IN      angular position, as a fraction of the patch angle range.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDDisk3d_fractionParameterToDPoint3d
(
DDisk3dCP pDisk,
DPoint3dP pPoint,
double    radiusFraction,
double    angleFraction
);

//!
//! @param pRadiusFraction OUT     radial position, as a fraction of the patch radial range.
//! @param pAngleFraction  OUT     angular position, as a fraction of the patch angle range.
//! @param pPoint         IN      evaluated point
//!
//!
Public GEOMDLLIMPEXP bool    bsiDDisk3d_dPoint3dToFractionParameter
(
DDisk3dCP pDisk,
double    *pThetaFraction,
double  *pZFraction,
DPoint3dCP pPoint
);

//!
//! @param pRadius IN      radius in xy plane.
//! @param pAngle IN      angle around cross sectional circle
//! @param radiusFraction IN      radial position, as a fraction of patch radius range.
//! @param angleFraction IN      angular position, as a fraction of the patch angle range.
//!
//!
Public GEOMDLLIMPEXP void     bsiDDisk3d_fractionParameterToNaturalParameter
(
DDisk3dCP pDisk,
double    *pRadius,
double    *pAngle,
double    radiusFraction,
double    angleFraction
);

//!
//! @param pRadiusFraction IN      radius parameter as a fraction of the pach.
//! @param pAngleFraction IN      natural angle parameter as fraction of the patch.
//! @param radius IN      radius, in local coordinate frame.
//! @param angle  IN      angular position
//!
//!
Public GEOMDLLIMPEXP void     bsiDDisk3d_naturalParameterToFractionParameter
(
DDisk3dCP pDisk,
double    *pRadiusFraction,
double    *pAngleFraction,
double    radius,
double    angle
);

//!
//! Test if the disk range is the full parameter space.
//!
//!
//!
Public GEOMDLLIMPEXP bool               bsiDDisk3d_isComplete (DDisk3dCP pInstance);

//!
//! @return true if the disk angle parameter covers the complete circular range of the cross sections.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDDisk3d_isParam2Complete (DDisk3dCP pDisk);

//!
//! @return true if the radius range covers the nominal 0..1 complete range.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDDisk3d_isParam1Complete (DDisk3dCP pDisk);

//!
//! Get the reference frame of the disk.
//!
//!
//!
Public GEOMDLLIMPEXP void               bsiDDisk3d_getFrame
(
DDisk3dCP pInstance,
TransformP pFrame
);

//!
//! Get the disk's inverse coordinate, i.e. transform from the global space to
//! a the system where the disk's circular rulings are circles around the origin.
//! @param pInverseFrame OUT     inverse frame.
//! @return true if the inverse is was computed.
//!
//!
Public GEOMDLLIMPEXP bool               bsiDDisk3d_getInverseFrame
(
DDisk3dCP pInstance,
TransformP pInverseFrame
);

//!
//! Evaluate the implicit function for the disk.
//! @param pPoint IN      point where the implicit function is evaluated.
//!
//!
Public GEOMDLLIMPEXP double bsiDDisk3d_implicitFunctionValue
(
DDisk3dCP pDisk,
DPoint3dCP pPoint
);

//!
//! @param pPoint OUT     evaluated point
//! @param theta IN      angle around disk
//! @param z     IN      fraction of z axis.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDDisk3d_naturalParameterToDPoint3d
(
DDisk3dCP pDisk,
DPoint3dP pPoint,
double  radius,
double  angle
);

//!
//! Compute a natural parameter of a given xyz point.  When the point is exactly on the
//! surface, this function is the inverse of naturalParameterToPoint.   For a point off of the
//! surface, this inversion should be predictable but may be other than a true projection.
//! @param pRadius OUT     radial parameter
//! @param pAngle OUT     angular parameter
//! @param pPoint  IN      xyz coordinates
//!
//!
Public GEOMDLLIMPEXP bool         bsiDDisk3d_dPoint3dToNaturalParameter
(
DDisk3dCP pDisk,
double  *pRadius,
double  *pAngle,
DPoint3dCP pPoint
);

//!
//! Return the range of the natural parameter for the active surface patch.
//! @param pRadius0 IN      start value of natural parameter.
//! @param pRadius1 IN      end value of natural parameter.
//! @param pAngle0  IN      start value of natural parameter.
//! @param pAngle1  IN      end value of natural parameter.
//!
//!
Public GEOMDLLIMPEXP void     bsiDDisk3d_getScalarNaturalParameterRange
(
DDisk3dCP pDisk,
double    *pRadius0,
double    *pRadius1,
double    *pAngle0,
double    *pAngle1
);

//!
//! Get the parameter range of the disk.
//!
//!
//!
Public GEOMDLLIMPEXP void               bsiDDisk3d_getNaturalParameterRange
(
DDisk3dCP pInstance,
DRange2dP pParameterRange
);

//!
//! Get the parameter range as start/sweep pairs.
//!
//!
Public GEOMDLLIMPEXP void    bsiDDisk3d_getScalarNaturalParameterSweep
(
DDisk3dCP pInstance,
double          *pRadius0,
double          *pRadiusDelta,
double          *pAngle0,
double          *pAngleSweep
);

//!
//! Return the range of the natural parameters for a complete surface.
//! @param pRadius0 IN      start value of natural parameter.
//! @param pRadius1 IN      end value of natural parameter.
//! @param pAngle0  IN      start value of natural parameter.
//! @param pAngle1  IN      end value of natural parameter.
//!
//!
Public GEOMDLLIMPEXP void     bsiDDisk3d_getCompleteNaturalParameterRange
(
DDisk3dCP pDisk,
double    *pRadius0,
double    *pRadius1,
double    *pAngle0,
double    *pAngle1
);

END_BENTLEY_GEOMETRY_NAMESPACE

