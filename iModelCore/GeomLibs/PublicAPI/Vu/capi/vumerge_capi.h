/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description Perform a merge operation on (disjoint) face loops in the graph, with duplicate edges culled using the parity rule.
* @remarks The graph may have intersections and containments other than those indicated by explicit graph topology.
*       This function finds the intersections/containments using the given rule and augments the graph structure accordingly.
* @remarks This function uses the graph tolerances set by ~mvu_setTol.
* @remarks This is a shortcut for calling ~mvu_merge2002 with ~tVuMergeType VUUNION_PARITY and with tolerances extracted by ~mvu_getTol.
* @param graphP     IN OUT  graph header
* @group "VU Booleans"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_mergeLoops
(
VuSetP graphP
);

/*---------------------------------------------------------------------------------**//**
* @description Set the VU_EXTERIOR_EDGE mask on the edges of each face of the graph that has negative area.
* @remarks First the VU_EXTERIOR_EDGE mask is cleared throughout the graph (~mvu_clearMaskInSet).
* @param graphP                 IN OUT  graph header
* @param suppressTrivialFaces   IN      whether to ignore faces with less than three edges
* @group "VU Node Masks"
* @see vu_setMaskByArea, vu_markFaceAndComputeArea
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_markExteriorFacesByArea
(
VuSetP  graphP,
bool    suppressTrivialFaces
);

/*---------------------------------------------------------------------------------**//**
* @description Set VU_EXTERIOR_EDGE masks throughout the graph using boundary-crossing parity.
* @remarks Assumed initial condition: some subset of edges has VU_EXTERIOR_EDGE set, while various other
*       edges have VU_BOUNDARY_EDGE mask set.
* @remarks The algorithm proceeds by flood search:
*       <ul>
*       <li>set the exterior mask on edges reachable from an exterior edge without crossing any boundary edge</li>
*       <li>clear the exterior mask on the other side of those boundary edges</li>
*       </ul>
* @param graphP                 IN OUT  graph header
* @param checkExteriorMarks     IN      whether to reset exterior masks (~mvu_markExteriorFacesByArea) beforehand
* @group "VU Node Masks"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_markAlternatingExteriorBoundaries
(
VuSetP graphP,
int checkExteriorMarks
);

END_BENTLEY_GEOMETRY_NAMESPACE

