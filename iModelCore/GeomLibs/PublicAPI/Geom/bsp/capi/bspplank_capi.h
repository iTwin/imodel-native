/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description A plank curve is (a) is contained within a surface and (b) has no bending around
    the axis perpendicular to the larger plank surface.   This function
    refines the positions of a sequence of points on a surface so that they approximate these
    conditions.
* @param pSurface IN Surface to contain plank path.
* @param pUVOut OUT improved path parameters.  (May be NULL if not needed.)  Caller must
    allocate for numXYZ points.
* @param pXYZOut OUT improved path coordinates. (May be NULL if not neeeded.)  Caller must
    allocate for numXYZ points.
* @param pUVIn IN original path parameters.  If NULL, parameters of pXYZIn will be obtained via
    minimum distance to surface calculation (expensive) from pXYZIn.
* @param pXYZIn IN original path coordinates.  If NULL, will be evaluated (cheap) from pUVIn.
        If both pUVIn and pXYZIn are given, pUVIn is used.
* @param numXYZ IN number of points on paths
* @return SUCCESS if and only if the plank path has been improved
* @group        "B-spline Query"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_improvePlankPath
(
MSBsplineSurface *pSurface,
DPoint2d *pUVOut,
DPoint3d *pXYZOut,
const DPoint2d *pUVIn,
const DPoint3d *pXYZIn,
int      numXYZ
);

END_BENTLEY_GEOMETRY_NAMESPACE

