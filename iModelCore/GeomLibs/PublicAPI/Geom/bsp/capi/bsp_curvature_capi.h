/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Compute poles of 0 to 4 cubic bezier curves which interpolate point, tangent direction,
 and curvature at start and end.  The tangent mangitude is adjusted to obtain the
 desired curvature.  Among the multiple curves, the one usually desired is the one
 with two positive tangent magnitudes.
@param pPoles OUT poles of 0 to 4 curves.  (i.e. allocate at least 16 doubles)
@param pDist0 OUT signed start tangent magnitude.
@param pDist1 OUT signed end tangent magnitude.
@param pNumCurve OUT number of curves
@param pXYZ0 IN start point.
@param pDir0 IN start vector in desired direction of curve.
@param r0 IN initial radius of curvature.  Use zero to blend to straight line (infinite radius).
@param pXYZ1 IN end point
@param pDir1 final vector, pointing "back" towards arriving curve.
@param r1 IN final radius of curvature.  Use zero to blend to straight line (infinite radius).
@returns ERROR if conditions cannot be met.  This is not uncommon because the cubic
   has limited flexibility.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_bezierCubicTangentCurvature
(
DPoint3d *pPoles16,
double *pDist0,
double *pDist1,
int *pNumCurve,
DPoint3d const *pXYZ0,
DVec3d   const *pDir0,
double   r0,
DPoint3d const *pXYZ1,
DVec3d const *pDir1,
double r1
);

/*---------------------------------------------------------------------------------**//**
Construct a quartic bezier which interpolates point, tangent direction, tangent magnitude,
   and curvature at both start and end.  These conditions fully specify the middle control
   control point of the bezier.
@param pBezierXYZ OUT 5 poles.
@param pXYZ0 IN start point.
@param pDir0 IN start vector in desired direction of curve.
@param r0 IN initial radius of curvature.  Use zero to blend to straight line (infinite radius).
@param pXYZ1 IN end point
@param pDir1 final vector, pointing "back" towards arriving curve.
@param r1 IN final radius of curvature.  Use zero to blend to straight line (infinite radius).
@returns ERROR if dirctions are parallel.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_bezierQuarticWithTangentAndCurvature
(
DPoint3d *pBezierXYZ,
DPoint3d const *pXYZ0,
DVec3d   const *pTangent0,
double r0,
DPoint3d const *pXYZ1,
DVec3d   const *pTangent1,
double   r1
);

/*---------------------------------------------------------------------------------**//**
Construct a quartic bezier which has given tangent and curvature at endpoints.  The quartic
uses the explictly given middle control point.  The MAGNTIUDE of the tangents is varied to achieve
curvature control.
@param pBezierXYZ OUT 5 poles.
@param pXYZ0 IN start point.
@param pDir0 IN start vector in desired direction of curve.
@param r0 IN initial radius of curvature.  Use zero to blend to straight line (infinite radius).
@param pXYZ1 IN end point
@param pDir1 final vector, pointing "back" towards arriving curve.
@param r1 IN final radius of curvature.  Use zero to blend to straight line (infinite radius).
@param pControlPoint Explicit middle control point.
@returns ERROR if either radius is zero.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_bezierQuarticWithTangentCurvatureControlPoint
(
DPoint3d *pBezierXYZ,
DPoint3d const *pXYZ0,
DVec3d   const *pTangent0,  /* Start direction, magnitude to be changed to achieve curvature. */
double r0,
DPoint3d const *pXYZ1,
DVec3d   const *pTangent1,  /* End direction, magnitude to be changed to achieve curvature. */
double   r1,
DPoint3d const *pControlPoint   /* middle control point. */
);

/*---------------------------------------------------------------------------------**//**
@description Construct a cubic bspline which interpolates given points with end conditions
@param pCurve OUT computed curve.
@param pXYZ IN points to interpolate
@param numXYZ IN number of interpolation points.
@param pTangent0 IN tangent at start.
@param r0 IN radius of curvature at start
@param pTangent1 IN tangent (towards curve) at end
@param r1 IN radius of curvature at end.
   that have both direction and curvature.  It is not always possible to meet these conditions!!!
@remark Radius values of zero indicate no curvature straight line condition, i.e. infinite radius of curvature.
@returns ERROR if unable to compute curve.   Curvature end conditions are very
   demanding -- be prepared to notice failures!!!
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_interpolateCubicWithTangentAndCurvature
(
MSBsplineCurve  *pCurve,
DPoint3d const  *pXYZ,
int             numXYZ,
DVec3d   const  *pTangent0,
double          r0,
DVec3d   const *pTangent1,
double          r1
);

END_BENTLEY_GEOMETRY_NAMESPACE

