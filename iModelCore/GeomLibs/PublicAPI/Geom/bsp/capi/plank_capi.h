/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*
@description Compute a planking path by iteratively improving an initial path.
@param pUV IN OUT parametric coordinates of points on curve.
@param pXYZ OUT xyz coordinates.
@param numPoint IN number of points
@param surfaceFunc IN function to evaluate surface.
@param pSurfaceData IN surface context, contents known to surfaceFunc.
@param uvTol IN parameter-space tolerance.
*/
Public GEOMDLLIMPEXP bool    planking_computePathExt
(
DPoint2d *pUV,
DPoint3d *pXYZ,
int      numPoint,
PlankingSurfaceEvaluator surfaceFunc,
void *pSurfaceData,
double uvTol,
double xPeriod,
double yPeriod
);

/*
@description Compute a planking path by iteratively improving an initial path.
@param pUV IN OUT parametric coordinates of points on curve.
@param pXYZ OUT xyz coordinates.
@param numPoint IN number of points
@param surfaceFunc IN function to evaluate surface.
@param pSurfaceData IN surface context, contents known to surfaceFunc.
@param uvTol IN parameter-space tolerance.
*/
Public GEOMDLLIMPEXP bool    planking_computePath
(
DPoint2d *pUV,
DPoint3d *pXYZ,
int      numPoint,
PlankingSurfaceEvaluator surfaceFunc,
void *pSurfaceData,
double uvTol
);

END_BENTLEY_GEOMETRY_NAMESPACE

