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
Public GEOMDLLIMPEXP bool    bspcurv_areCurvesCoincident
(
MSBsplineCurve      *segment,
MSBsplineCurve      *testCurve,
double              tolerance,
RotMatrix           *rotMatrixP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_makeCurvesCompatible
(
MSBsplineCurve  **outputCurves,
MSBsplineCurve  **inputCurves,
int             numCurves,
int             enableReverse,         /* IN      allows reversing output */
int             openAll                /* IN      forces opening */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_make2CurvesCompatible
(
MSBsplineCurve  *curve1,
MSBsplineCurve  *curve2
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_makeCurvesCompatibleWithConsByApprox
(
MSBsplineCurve  *pOut,          /* <= array of input curves, should be allocated before calling this */
MSBsplineCurve  *pIn,           /* => array of input curves */
int             numCurves,      /* => number of curves */
double          tolerance,      /* => approximation tolerance, set to zero for precise compatibility */
int             tangentControl, /* => 0: no tangent control, 1: start point, 2: end point, 3: Both ends */
bool            keepMagnitude,  /* => true: derivatives maintained at specified ends, false: only directions */
int             derivative,     /* => Highest derivatives maintained. Ignored when keepMagnitude is false */
bool            fastMode        /* => true: to remove less data but faster */
);

END_BENTLEY_GEOMETRY_NAMESPACE

