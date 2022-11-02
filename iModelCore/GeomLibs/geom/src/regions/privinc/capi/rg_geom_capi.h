/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

/* DO NOT EDIT!  THIS FILE IS GENERATED. */

#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
*
* @param pLineParam OUT     0, 1, or 2 intersection parameters on the line.
* @param pIntersectionPoint OUT     0, 1, or 2 intersection points, computed from the
*               line start, end, and parameter.
* @param bounded IN      if true, only intersections in the parameter interval [0,1]
*                   are returned.  Note that the parameter interval is closed but
*                   not toleranced.
* @return number of intersections
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      jmdlRG_intersectCircleSegmentXY
(
bvector<double>     *pParamArray,
EmbeddedDPoint3dArray   *pPointArray,
DPoint3d    *pStart,
DPoint3d    *pEnd,
DPoint3d    *pCenter,
double      r,
bool        bounded
);

END_BENTLEY_GEOMETRY_NAMESPACE

