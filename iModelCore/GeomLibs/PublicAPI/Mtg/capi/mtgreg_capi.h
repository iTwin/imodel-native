/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_halfRegularizeGraph                          |
|                                                                       |
|                                                                       |
| Sweep from bottom to top to connect each downward min to a node below |
| (without crossing any lines of the polygon)                           |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int jmdlMTGReg_halfRegularizeGraph
(
MTGGraph            *pGraph,
int                 vertexLabelOffset,
EmbeddedDPoint3dArray     *pCoordinateArray,
bool                reverseCoordinates
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_regularize                                   |
|                                                                       |
|                                                                       |
| Bi-directional regularization sweeps                                  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool         jmdlMTGReg_regularize
(
MTGGraph            *pGraph,
int                 vertexLabelOffset,
EmbeddedDPoint3dArray     *pCoordinateArray
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGTri_triangulateAllCCWRegularFacesByVerticalSweep |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlMTGTri_triangulateAllCCWRegularFacesByVerticalSweep
(
MTGGraph            *pGraph,
int                 vertexLabelOffset,
EmbeddedDPoint3dArray     *pCoordinateArray
);

END_BENTLEY_GEOMETRY_NAMESPACE

