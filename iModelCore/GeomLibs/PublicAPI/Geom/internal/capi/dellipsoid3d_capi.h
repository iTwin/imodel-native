/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Initialize an ellipsoid from center and radius and optional parameter range.
//! Orientation is set parallel to global axes.
//! @param pCenter IN      ellipsoid center.
//! @param radius  IN      radius of ellipsoid.
//! @param pParameterRange IN      parameter range. If NULL, default is applied.
//!
//!
Public GEOMDLLIMPEXP void               bsiDEllipsoid3d_setCenterRadius
(
DEllipsoid3dP pInstance,
DPoint3dCP pCenter,
double          radius,
DRange2dCP pParameterRange
);

//!
//! Initialize an ellipsoid from full frame and range
//! @param pTransform IN      coordinate frame.  If NULL, default is applied.
//! @param pRange IN      parameter range.  If NULL, default is applied.
//!
//!
Public GEOMDLLIMPEXP void               bsiDEllipsoid3d_set
(
DEllipsoid3dP pInstance,
TransformCP pFrame,
DRange2dCP pParameterRange
);

//!
//! Set the reference frame of the ellipsoid.
//! @param pTransform IN      coordinate frame.  If NULL, default is applied.
//!
//!
Public GEOMDLLIMPEXP void               bsiDEllipsoid3d_setFrame
(
DEllipsoid3dP pInstance,
TransformCP pFrame
);

//!
//! Set the parameter range of the ellipsoid.
//!
//!
//!
Public GEOMDLLIMPEXP void               bsiDEllipsoid3d_setNaturalParameterRange
(
DEllipsoid3dP pInstance,
DRange2dCP pParameterRange
);

//!
//! Test if the ellipsoid range is the full parameter space.
//!
//!
//!
Public GEOMDLLIMPEXP bool               bsiDEllipsoid3d_isFull (DEllipsoid3dCP pInstance);

//!
//! Get the silhouette of the ellipse from a given homogeneous eyepoint.  This is a single,
//! complete conic.  To get the (up to 4) fragments of the silhouette within the parameter range,
//! use GraphicsPointArray method addSilhouette().
//!
//! @param pEllipse   OUT     silhouette ellipse.
//! @param pMap       IN      additional transform to apply to the ellipsoid.
//! @param pEyePoint  IN      eye point.
//!
//!
Public GEOMDLLIMPEXP bool               bsiDEllipsoid3d_getSilhouette
(
DEllipsoid3dCP pInstance,
DConic4dP pEllipse,
DMap4dCP pMap,
DPoint4dCP pEyePoint
);

//!
//! Intersect the sphere with a line segment.
//! @param pXYZ   OUT     array of 0, 1, or 2 coordinates of cartesian intersection points.
//!                       May be NULL.
//! @param pUVW   OUT     array of 0, 1, or 2 coordinates of intersection points in the
//!                       local coordinates of the ellipsoid frame.   (Points on the
//!                       unit sphere).  May be NULL.
//! @param pLineParameter OUT     array of 0, 1, or 2 parameters with respect to the line.
//! @param DRay3d         IN      ray to intersect.
//! @return number of intersections.
//!
//!
Public GEOMDLLIMPEXP int                bsiDEllipsoid3d_intersectDSegment3d
(
DEllipsoid3dCP pInstance,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double          *pLineParameter,
DSegment3dCP pSegment
);

//!
//! Intersect the sphere with a line segment.
//! @param pXYZ   OUT     array of 0, 1, or 2 coordinates of cartesian intersection points.
//!                       May be NULL.
//! @param pUVW   OUT     array of 0, 1, or 2 coordinates of intersection points in the
//!                       local coordinates of the ellipsoid frame.   (Points on the
//!                       unit sphere).  May be NULL.
//! @param pLineParameter OUT     array of 0, 1, or 2 parameters with respect to the line.
//! @return number of intersections.
//!
//!
Public GEOMDLLIMPEXP int                bsiDEllipsoid3d_intersectDRay3d
(
DEllipsoid3dCP pInstance,
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
Public GEOMDLLIMPEXP void     bsiDEllipsoid_applyAffineDMatrix4d
(
DEllipsoid3dP pDest,
DMatrix4dCP pMatrix,
DEllipsoid3dCP pSource
);

//!
//! Convert a local cartesian point into the local spherical coordinates.
//! @param pTheta     OUT     angle in unit spherical coordiantes
//! @param pPhi       OUT     z coordinate in spherical coordinates
//! @param pR         OUT     radius in spherical coordinates
//! @return false if the local point is at the origin.
//!
//!
Public GEOMDLLIMPEXP bool       bsiDEllipsoid3d_localToSpherical
(
DEllipsoid3dCP pEllipsoid,
double    *pTheta,
double    *pPhi,
double    *pR,
DPoint3dCP pPoint
);

//!
//! Convert a world cartesian point to the local cartesian system.
//! @param pWorld OUT     world coordinates
//! @param pLocal IN      coordinates in local frame of ellipsoid.
//! @return true if the local to world transformation was invertible.
//!
//!
Public GEOMDLLIMPEXP void    bsiDEllipsoid3d_localToWorld
(
DEllipsoid3dCP pEllipsoid,
DPoint3dP pWorld,
DPoint3dCP pLocal
);

//!
//! Convert a local cartesian point to the world coordinate system.
//! @param pLocal OUT     coordinates in local frame
//! @param pWorld IN      world coordinates
//!
//!
Public GEOMDLLIMPEXP bool        bsiDEllipsoid3d_worldToLocal
(
DEllipsoid3dCP pEllipsoid,
DPoint3dP pLocal,
DPoint3dCP pWorld
);

//!
//! Convert from parametric to cartesian, with the parametric coordinate given as trig values.
//!
//!
Public GEOMDLLIMPEXP void    bsiDEllipsoid3d_trigParameterToPoint
(
DEllipsoid3dCP pEllipsoid,
DPoint3dP pPoint,
double      cosTheta,
double      sinTheta,
double      cosPhi,
double      sinPhi
);

//!
//! test if a longitude angle is in the ellipsoid's parameter range.
//!
//!
Public GEOMDLLIMPEXP bool        bsiDEllipsoid3d_longitudeInRange
(
DEllipsoid3dCP pInstance,
double          longitude
);

//!
//! test if a latitude is in the ellipsoid's parameter range.
//!
//!
Public GEOMDLLIMPEXP bool        bsiDEllipsoid3d_latitudeInRange
(
DEllipsoid3dCP pInstance,
double          latitude
);

//!
//! Return a meridian at specified longitude.  The angular sweep of the meridian is restricted
//!   to the parameter range of the surface.
//! @param pEllipse OUT     full ellipse at specified longitude.   0-degree vector is on
//!           is on the equator.  90 degree vector is to the north pole.
//! @param longitude IN      longitude angle (radians)
//! @return true if the longitude is within the parameter range of the surface patch.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDEllipsoid3d_getMeridian
(
DEllipsoid3dCP pEllipsoid,
DEllipse3dP pEllipse,
double      longitude
);

//!
//! Return a (full) parallel of latitude.
//! @param pEllipse OUT     full ellipse at specified latitude.  0 and 90 degree vectors
//!           are 0 and 90 degrees latitude.
//! @param latitude IN      latitude angle (radians)
//! @return true if the latitude is within the parameter range of the surface.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDEllipsoid3d_getParallel
(
DEllipsoid3dCP pEllipsoid,
DEllipse3dP pEllipse,
double      latitude
);

//!
//! @param pPoint OUT     evaluated point
//! @param thetaFraction IN      angular position, as a fraction of the patch longitude range.
//! @param phiFraction IN      axial position, as a fraction of the patch latitude range.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDEllipsoid3d_fractionParameterToDPoint3d
(
DEllipsoid3dCP pEllipsoid,
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
Public GEOMDLLIMPEXP bool    bsiDEllipsoid3d_dPoint3dToFractionParameter
(
DEllipsoid3dCP pEllipsoid,
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
Public GEOMDLLIMPEXP void     bsiDEllipsoid3d_fractionParameterToNaturalParameter
(
DEllipsoid3dCP pEllipsoid,
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
Public GEOMDLLIMPEXP void     bsiDEllipsoid3d_naturalParameterToFractionParameter
(
DEllipsoid3dCP pEllipsoid,
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
Public GEOMDLLIMPEXP bool               bsiDEllipsoid3d_isComplete (DEllipsoid3dCP pInstance);

//!
//! @return true if the 1st natural parameter covers the complete range of the underlying
//!           analytic surface.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDEllipsoid3d_isParam1Complete (DEllipsoid3dCP pEllipsoid);

//!
//! @return true if the 2nd natural parameter covers the complete range of the underlying
//!           analytic surface.
//!
//!
Public GEOMDLLIMPEXP bool     bsiDEllipsoid3d_isParam2Complete (DEllipsoid3dCP pEllipsoid);

//!
//! Get the reference frame of the ellipsoid.
//!
//!
//!
Public GEOMDLLIMPEXP void               bsiDEllipsoid3d_getFrame
(
DEllipsoid3dCP pInstance,
TransformP pFrame
);

//!
//! Get the ellise's inverse coordinate, i.e. transform from the global space to
//! a the system where the base is a unit xy circle.
//! @param pInverseFrame OUT     inverse frame.
//! @return true if the inverse is was computed.
//!
//!
Public GEOMDLLIMPEXP bool               bsiDEllipsoid3d_getInverseFrame
(
DEllipsoid3dCP pInstance,
TransformP pInverseFrame
);

//!
//! Evaluate the implicit function for the ellipsoid.
//! @param pPoint IN      point where the implicit function is evaluated.
//!
//!
Public GEOMDLLIMPEXP double bsiDEllipsoid3d_implicitFunctionValue
(
DEllipsoid3dCP pEllipsoid,
DPoint3dCP pPoint
);

//!
//! @param pPoint OUT     evaluated point
//! @param theta  IN      longitude
//! @param phi    IN      latitude
//!
//!
Public GEOMDLLIMPEXP bool     bsiDEllipsoid3d_naturalParameterToDPoint3d
(
DEllipsoid3dCP pEllipsoid,
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
Public GEOMDLLIMPEXP bool         bsiDEllipsoid3d_dPoint3dToNaturalParameter
(
DEllipsoid3dCP pEllipsoid,
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
Public GEOMDLLIMPEXP void     bsiDEllipsoid3d_getScalarNaturalParameterRange
(
DEllipsoid3dCP pEllipsoid,
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
Public GEOMDLLIMPEXP void               bsiDEllipsoid3d_getNaturalParameterRange
(
DEllipsoid3dCP pInstance,
DRange2dP pParameterRange
);

//!
//! Get the parameter range as start/sweep pairs.
//!
//!
Public GEOMDLLIMPEXP void    bsiDEllipsoid3d_getScalarNaturalParameterSweep
(
DEllipsoid3dCP pInstance,
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
Public GEOMDLLIMPEXP void     bsiDEllipsoid3d_getCompleteNaturalParameterRange
(
DEllipsoid3dCP pEllipsoid,
double  *pParam1Start,
double  *pParam1End,
double  *pParam2Start,
double  *pParam2End
);

END_BENTLEY_GEOMETRY_NAMESPACE

