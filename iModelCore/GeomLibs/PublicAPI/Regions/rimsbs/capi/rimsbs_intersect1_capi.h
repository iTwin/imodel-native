/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_cb_accumulatePointAndLocalFraction              |
|                                                                       |
|                                                                       |
| Accumlate a parameter+point pair to arrays.  Accumulated parameter    |
| is normalized as a fraction of specified start-end pair.              |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void        bspcurv_cb_accumulatePointAndLocalFraction
(
MSBsplineCurve      *pCurve,
double              startFraction,
double              endFraction,
double              rootFraction,
void                *pUserData1,
void                *pUserData2
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_intersectXYCircle                               |
|                                                                       |
|                                                                       |
| Intersect with a circle in xy plane.                                  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void                bspcurv_intersectXYCircle
(
MSBsplineCurve  *curveP,            /* IN      input curve */
double          startFraction,      /* IN      starting fractional parameter */
double          endFraction,        /* IN      ending fractional parameter */
double          x0,                 /* IN      circle center x */
double          y0,                 /* IN      circle center y */
double          r0,                 /* IN      circle radius */
double          absTol,             /* IN      absolute tolerance for double roots. */
MSBsplineCurve_AnnounceParameter F,
void            *pUserData0,
void            *pUserData1
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_curveCircleIntersectionXY                        |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_curveCircleIntersectionXY
(
RIMSBS_Context          *pContext,          /* IN      general context */
bvector<double>     *pParameterArray,   /* OUT     curve parameters of intersections. */
EmbeddedDPoint3dArray   *pPointArray,       /* OUT     curve points of intersections */
RG_EdgeData             *pEdgeData,         /* IN      segment edge data */
const DPoint3d          *pCenter,           /* IN      circle center */
double                  radius
);

END_BENTLEY_GEOMETRY_NAMESPACE

