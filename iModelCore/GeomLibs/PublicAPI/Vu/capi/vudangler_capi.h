/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------*//**
* @description Set a mask on edges that dangle into faces.
* @param pGraph         IN OUT  vu graph
* @param nullFaceMask   IN      optional mask preset on faces that are to be ignored in determining if an edge dangles
*                               (e.g., as set by ~mvu_markSmallFaces with numEdge = 2)
* @param danglerMask    IN      mask to set on both sides of dangling edges
* @group "VU Edges"
* @see vu_deleteDanglingEdges, vu_markSmallFaces
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markDanglingEdges
(
VuSetP  pGraph,
VuMask  nullFaceMask,
VuMask  danglerMask
);

/*---------------------------------------------------------------------*//**
* @description Set a mask on faces with numEdge or fewer edges.
* @param pGraph     IN OUT  vu graph
* @param numEdge    IN      maximum number of edges for a marked face
* @param mask       IN      mask to set
* @group "VU Edges"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markSmallFaces
(
VuSetP  pGraph,
int     numEdge,
VuMask  mask
);

/*---------------------------------------------------------------------*//**
* @description Delete all edges that dangle into faces of the graph.
* @param pGraph     IN OUT  vu graph
* @group "VU Edges"
* @see vu_markDanglingEdges
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    vu_deleteDanglingEdges
(
VuSetP  pGraph
);

END_BENTLEY_GEOMETRY_NAMESPACE

