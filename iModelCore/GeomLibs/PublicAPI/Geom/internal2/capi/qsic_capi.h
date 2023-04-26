/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiRotatedConic_intersectPlanarRotatedConic             |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_intersectPlanarRotatedConic

(
const RotatedConic  *pSurface0,     /* IN      first rotated conic */
const RotatedConic  *pSurface1,     /* IN      second rotated conic. Must be planar */
DPoint4dCP pEyePoint,       /* IN      optional eyepoint.  Intersection curve
has breaks at silhouette  points */
double      tolerance,      /* IN      chord tolerance */
SilhouetteArrayHandler handlerFunc, /* IN      callback for points */
void                *pUserData,      /* IN      arbitrary pointer */
bool                showDebug       /* IN      true to otutput debug geometry
(e.g. critical angle traces) */
);

/*------------------------------------------------------------------*//**
* Classify a qudric surface by its diagonal characteristic.
* @param pIndex OUT     returned. For each standard form index i, pIndex[i]
*   is the corresponding column of the original data.
* @param pSigma OUT     values from pSigma0, sorted into standard form.
* @param pNegated OUT     true if a negative factor was applied to reach
*           standard form. (I.e. input had more negative than positive
*           entries.)
* @param pSigma0 IN      original eigenvalues of quadric form.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_sortCharacteristic

(
int       *pIndex,
DPoint4dP pSigma,
QuadricSurfaceClass   *pClass,
bool      *pNegated,
DPoint4dCP pSigma0
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiRotatedConic_intersectEllipsoids                     |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_intersectEllipsoids

(
const RotatedConic  *pSurface0,     /* IN      first rotated conic */
const RotatedConic  *pSurface1,     /* IN      second rotated conic */
DPoint4dCP pEyePoint,       /* IN      optional eyepoint.  Intersection curve
has breaks at silhouette  points */
double      tolerance,      /* IN      chord tolerance */
SilhouetteArrayHandler handlerFunc, /* IN      callback for points */
void                *pUserData,      /* IN      arbitrary pointer */
bool                showDebug       /* IN      true to otutput debug geometry
(e.g. critical angle traces) */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiRotatedConic_intersectRotatedConic                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_intersectRotatedConic

(
const RotatedConic  *pSurface0,         /* IN      first rotated conic  */
const RotatedConic  *pSurface1,         /* IN      second rotated conic */
DPoint4dCP pEyePoint,           /* IN      optional eyepoint.  Intersection curve
has breaks at silhouette  points */
double      tolerance,          /* IN      chord tolerance */
SilhouetteArrayHandler handlerFunc,     /* IN      callback for points */
void                *pUserData,         /* IN      arbitrary pointer */
bool                showDebug           /* IN      true to otutput debug geometry (e.g. critical angle traces) */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiRotatedConic_intersectRotatedConicExt                |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_intersectRotatedConicExt

(
const RotatedConic  *pSurface0,     /* IN      first rotated conic  */
const RotatedConic  *pSurface1,     /* IN      second rotated conic */
const RotatedConic  *pClipSurface0, /* IN      first clip surface (commonly the same as pSurface0)  */
const RotatedConic  *pClipSurface1, /* IN      second clip surface (commonly the same as pSurface1) */
const DPoint4d      *pEyePoint,     /* IN      optional eyepoint.  Intersection curve
                                            has breaks at silhouette  points */
      double        tolerance,      /* IN      chord tolerance */
SilhouetteArrayHandler handlerFunc, /* IN      callback for points */
void                *pUserData,      /* IN      arbitrary pointer */
bool                showDebug       /* IN      true to otutput debug geometry
                                            (e.g. critical angle traces) */
);

