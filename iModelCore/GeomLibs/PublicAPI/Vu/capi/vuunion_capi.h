/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @nodoc
* @description Perform a merge/boolean operation on (disjoint) face loops in the graph.
* @remarks The graph may have intersections and containments other than those indicated by explicit graph topology.
*       This function finds the intersections/containments using the given rule and augments the graph structure accordingly.
* @remarks This function uses the graph tolerances set by ~mvu_setTol.
* @param graphP     IN OUT  graph header
* @param mergeType  IN      rule to apply to graph
* @group "VU Booleans"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_mergeOrUnionLoops
(
VuSetP graphP,
VuMergeType mergeType
);

/*---------------------------------------------------------------------------------**//**
* @description Perform a merge operation on (disjoint) face loops in the graph, with all duplicate edges preserved.
* @remarks The graph may have intersections and containments other than those indicated by explicit graph topology.
*       This function finds the intersections/containments using the given rule and augments the graph structure accordingly.
* @remarks This function uses the graph tolerances set by ~mvu_setTol.
* @remarks This is a shortcut for calling ~mvu_merge2002 with ~tVuMergeType VUUNION_UNION.
* @param graphP     IN OUT  graph header
* @group "VU Booleans"
* @see vu_orLoops
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_unionLoops
(
VuSetP graphP
);

/*---------------------------------------------------------------------------------**//**
* @description Find the largest face of negative area in the graph.
* @param arrayP IN OUT  array to collect faces with negative area
* @param graphP IN      graph header
* @return node in the maximally negative area face
* @group "VU Coordinates"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP      vu_getNegativeAreaFace
(
VuArrayP        arrayP,
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
* @nodoc
* @remarks Assumption: the graph is merged and connected.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markWindingNumber
(
VuSetP          graphP,
VuMask          maskToCheck
);

/*---------------------------------------------------------------------------------**//**
* @nodoc
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markExteriorBoundariesFromWindingNumberAndRule
(
VuSetP          graphP,
VuMergeType     mergeType
);

/*---------------------------------------------------------------------------------**//**
* @description Apply a mask bit to every node reachable from a seed, i.e. mark the
     connected component.
* @param graphP IN OUT graph to mark
* @param seedP  IN start node for search.
* @param mVisited   IN mask to apply.
* @group "VU Booleans"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    vu_markComponent
(
VuSetP          graphP,
VuP             seedP,
VuMask          mVisited
);

/*---------------------------------------------------------------------------------**//**
* @description If the graph is already a single connected component, leave it alone.
    If not, call vu_regularize to add edges that connect the various parts.
* @param graphP IN OUT graph to change.
* @group "VU Booleans"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    vu_connectGraph
(
VuSetP          graphP
);





/*---------------------------------------------------------------------------------**//**
* @description Perform a boolean union on the face loops of the graph.
* @remarks Before applying the boolean operation, this function first merges the graph (~mvu_unionLoops),
*   then regularizes it (~mvureg_regularizeGraph) if it is disconnected.
* @param graphP IN OUT graph header
* @group "VU Booleans"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_orLoops
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
* @description Perform a boolean symmetric difference (logical XOR) on the face loops of the graph.
* @remarks Before applying the boolean operation, this function first merges the graph (~mvu_unionLoops),
*   then regularizes it (~mvureg_regularizeGraph) if it is disconnected.
* @param graphP IN OUT graph header
* @group "VU Booleans"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_xorLoops
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
* @description Perform a boolean intersection on the face loops of the graph.
* @remarks Before applying the boolean operation, this function first merges the graph (~mvu_unionLoops),
*   then regularizes it (~mvureg_regularizeGraph) if it is disconnected.
* @param graphP IN OUT graph header
* @group "VU Booleans"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_andLoops
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
* @description Perform a boolean subtraction (A intersect the complement of B) on the face loops of the graph.
* @remarks Before applying the boolean operation, this function first merges the graph (~mvu_unionLoops),
*   then regularizes it (~mvureg_regularizeGraph) if it is disconnected.
* @param graphP IN OUT graph header
* @group "VU Booleans"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_andComplementLoops
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
* @description Invert the face loops of the graph.
* @remarks This routine moves the VU_EXTERIOR_EDGE mask to the edge mate, essentially turning the faces inside out.
* @param graphP IN OUT graph header
* @group "VU Booleans"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_notLoops
(
VuSetP          graphP
);

END_BENTLEY_GEOMETRY_NAMESPACE

