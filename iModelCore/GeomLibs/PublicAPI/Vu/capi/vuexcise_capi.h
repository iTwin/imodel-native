/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description Disconnect (small) edges from the graph and mark them.
* @remarks The callback determines which edges to disconnect, performs the incident vertex re-assignment, and signals whether or not
*       resulting degenerate faces should be disconnected as well.
* @remarks On return, the marked edges remain in the graph, but are disconnected from the graph.  This allows
*       the caller to mark more edges with this mask before deleting all masked edges at once.  Thus it is
*       assumed that on input, discardMask, if not cleared in the graph, has only been applied to both or neither
*       sides of each edge of the graph, and that any edges with this mask are disconnected from the rest of the graph.
* @param graphP         IN OUT  vu graph
* @param discardMask    IN      mask to apply to discarded edges
* @param testFuncP      IN      callback for various operations
* @param userDataP      IN      callback arg
* @return number of edges marked and disconnected from the graph
* @group "VU Edges"
* @see vu_markAndExciseSmallEdges, vu_markAndExciseSliverEdges
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_testAndExciseSmallEdges
(
VuSetP              graphP,
VuMask              discardMask,
VuEdgeTestFunction  testFuncP,
void*               userDataP
);

END_BENTLEY_GEOMETRY_NAMESPACE

