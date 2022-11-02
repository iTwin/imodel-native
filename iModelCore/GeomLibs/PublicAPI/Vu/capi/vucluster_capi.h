/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* @description Identify clusters of nearly-identical nodes.
* @remarks Only xy-coordinates are compared.
* @param pGraph                     IN OUT  graph to examine
* @param pClusterArray              OUT     array of NULL-deliminted blocks of nodes whose xy-coordinates lie within tolerance
* @param absTol                     IN      absolute tolerance for xy-coordinate comparison
* @param bSaveClusterIndexInNode    IN      whether to fill nodes' userId field with cluster index
* @param bReassignXYZ               IN      whether to assign common coordinates to the nodes within each cluster
* @group "VU Coordinates"
* @see vu_consolidateClusterCoordinates, vu_consolidateClusterCoordinatesAndSortByAngle
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_collectClusters
(
VuSetP          pGraph,
VuArrayP        pClusterArray,
double          absTol,
bool            bSaveClusterIndexInNode,
bool            bReassignXYZ
);

/**
* @description Identify clusters of nearly-identical nodes and assign common coordinates to the nodes within each cluster.
* @remarks Only xy-coordinates are compared.
* @remarks This function just calls ~mvu_collectClusters (pGraph, NULL, abstol, false, true).
* @param pGraph     IN OUT  graph to examine
* @param abstol     IN      absolute tolerance for xy-coordinate comparison
* @group "VU Coordinates"
* @see vu_collectClusters, vu_consolidateClusterCoordinatesAndSortByAngle
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_consolidateClusterCoordinates
(
VuSetP  pGraph,
double abstol
);

/**
* @description Identify clusters of nearly-identical nodes, assign common coordinates to the nodes within each cluster, and sort duplicate
*       edges by angle and mergeType.
* @remarks Only xy-coordinates are compared.
* @param pGraph     IN OUT  graph to examine
* @param mergeType  IN      rule to apply to cull duplicate edges, e.g. VUUNION_PARITY, VUUNION_KEEP_ONE_AMONG_DUPLICATES or otherwise for no culling
* @param abstol     IN      absolute tolerance for xy-coordinate comparison
* @group "VU Coordinates"
* @see vu_collectClusters, vu_consolidateClusterCoordinates
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_consolidateClusterCoordinatesAndSortByAngle
(
VuSetP      pGraph,
VuMergeType mergeType,
double      abstol
);

END_BENTLEY_GEOMETRY_NAMESPACE

