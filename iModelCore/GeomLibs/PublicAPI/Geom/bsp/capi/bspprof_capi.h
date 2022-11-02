/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspprof_evaluateBezier
(
DPoint3d            *pointP,
DPoint3d            *dPdU,
DPoint3d            *dPdV,
DPoint3d            *dPdUU,
DPoint3d            *dPdVV,
DPoint3d            *dPdUV,
double              u,
double              v,
MSBsplineSurface    *patchBezP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspprof_fitPoints
(
MSBsplineCurve      *curve,
DPoint3d            *points,
int                 numPoints
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int     bspprof_extractSilhouette
(
bvector<MSBsplineCurvePtr>  &curves,            /* <= silhoutte curves */
MSBsplineSurfaceCP surfaceP,          /* => surface */
double                  tolerance,          /* => tolerance */
bool                    cubicFit,           /* => T: cubic, F: linear. */
DPoint3d                *cameraPos          /* => camera position or NULL */
);
END_BENTLEY_GEOMETRY_NAMESPACE

