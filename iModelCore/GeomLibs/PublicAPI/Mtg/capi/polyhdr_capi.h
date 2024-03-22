/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|FUNC           mdl_polyhedronBoundingBox                               |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlPolyhedron_getBoundingBox
(
DRange3d    *pRangeBox,
DPoint3d    *pPointArray
);

/*----------------------------------------------------------------------+
|FUNC           jmdlPolyhedron_frustumRangeBoxEnclosed                  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlPolyhedron_frustumRangeBoxEnclosed
(
DRange3d            *pFrustBoundBox,
DPlane3d_SmallSet   *pPlaneSetP,
const DRange3d      *pRangeBox
);

/*----------------------------------------------------------------------+
|FUNC           jmdlPolyhedron_getSilhouette                            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPolyhedron_getSilhouette
(
EmbeddedDPoint3dArray  *pPointArray,    /* z-coord will be disregarded */
int             projType
);

/*----------------------------------------------------------------------+
| name          mdlPolyhdron_getRangeBox                                |
|DESC: will return corner points of a range box in a specific order     |
|the returned points 0,..., 3 will correspond to the plane that         |
|contains the lower range point and points 4,...7 will correspond to    |
|the plane that contains higher range point                             |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlPolyhedron_getRangeCorners
(
DPoint3d        *pOutPoints, /* an array of just eight points */
                             /* returned points are ordered */
const DRange3d  *pRangeBox    /* in z-up right handed coord. system */
);

/*----------------------------------------------------------------------+
| name          mdlPolyhdronHPlane_getExtremePoints                     |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlPolyhedronHPlane_getExtremePoints
(
double          *pMinNeg,
double          *pMaxPos,
DPoint4d        *pHPlane,
DPoint3d        *pRangePoints
);

/*----------------------------------------------------------------------+
|FUNC           jmdl_planeSetFromFrustumCorners                         |
|DESC loads the plane sets of a view frustum from a given point array   |
|ordered according to the standard corner point indexing                |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPolyhedron_planeSetFromFrustumCorners
(
DPlane3d_SmallSet   *planeSetP,
DPoint3d            *pointArray
);

/*----------------------------------------------------------------------+
|FUNC           jmdlPolyhedron_frustumRangeBoxOverlap                   |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP RangeTestResult  jmdlPolyhedron_frustumRangeBoxOverlap
(
RangeTest           *pRangeTest,
const DRange3d      *pRangeBox
);

END_BENTLEY_GEOMETRY_NAMESPACE

