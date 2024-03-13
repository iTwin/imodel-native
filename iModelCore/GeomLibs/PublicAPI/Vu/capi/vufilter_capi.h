/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description Disconnect small edge <em>features</em> from the graph and mark them.
* @remarks On return, the marked edges remain in the graph, but are disconnected from the graph.  This allows
*       the caller to mark more edges with this mask before deleting all masked edges at once.  Thus it is
*       assumed that on input, discardMask, if not cleared in the graph, has only been applied to both or neither
*       sides of each edge of the graph, and that any edges with this mask are disconnected from the rest of the graph.
* @remarks This function searches for successive nodes A, B, C and D in a face such that:
* <ul>
* <li>AB and CD have a well-defined intersection E.</li>
* <li>The triangle BEC is small.</li>
* <li>Cutting out BC and leaving edges AE and ED only changes the area by a small amount.</li>
* </ul>
* @param graphP         IN OUT  graph to examine
* @param tol            IN      absolute xy-distance tolerance for the perturbation of nodes B and C to E
* @param discardMask    IN      mask to apply to discarded edges
* @return number of edges marked and disconnected from the graph
* @group "VU Edges"
* @see vu_markAndExciseSmallEdges, vu_testAndExciseSmallEdges
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_markAndExciseSliverEdges
(
VuSetP          graphP,
double          tol,
VuMask          discardMask
);

/*---------------------------------------------------------------------------------**//**
* @description Disconnect small edges from the graph and mark them.
* @remarks On return, the marked edges remain in the graph, but are disconnected from the graph.  This allows
*       the caller to mark more edges with this mask before deleting all masked edges at once.  Thus it is
*       assumed that on input, discardMask, if not cleared in the graph, has only been applied to both or neither
*       sides of each edge of the graph, and that any edges with this mask are disconnected from the rest of the graph.
* @remarks The incident edges to an excised edge have their incident vertices moved to the midpoint of the excised edge.
* @param graphP             IN OUT  graph to examine
* @param tol                IN      absolute xy-distance tolerance for edge size
* @param protectEdgeMask    IN      mask for edges that may not be disconnected
* @param fixedVertexMask    IN      mask for vertices that may not be moved
* @param discardMask        IN      mask to apply to discarded edges
* @return number of edges marked and disconnected from the graph
* @group "VU Edges"
* @see vu_markAndExciseSliverEdges, vu_testAndExciseSmallEdges
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_markAndExciseSmallEdges
(
VuSetP          graphP,
double          tol,
VuMask          protectEdgeMask,
VuMask          fixedVertexMask,
VuMask          discardMask
);

/*---------------------------------------------------------------------------------**//**
* @description Split all sliver triangles in the graph.
* @remarks For the purposes of this function, a "sliver triangle" is defined to be a triangle ABC where vertex C is "near" edge AB,
*       but all three edges are "long".  Such a triangle is split along a new edge from C to its projection onto AB.
* @param graphP         IN OUT  graph to examine
* @param minHeight      IN      absolute minimum xy-altitude of a non-sliver triangle
* @param legFactor      IN      absolute minimum xy-length of sliver triangle legs, as fraction of xy-altitude
* @return number of sliver triangles split
* @group "VU Edges"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_splitSliverTriangles
(
VuSetP          graphP,
double          minHeight,
double          legFactor               /* IN      require all legs to be larger than this multiple
                                        of the height */
);

END_BENTLEY_GEOMETRY_NAMESPACE

