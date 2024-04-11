/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description Perform a merge operation on (disjoint) face loops in the graph, with specified duplicate edge handling.
* @remarks The graph may have intersections and containments other than those indicated by explicit graph topology.
*       This function finds the intersections/containments using the given rule and augments the graph structure accordingly.
* @param pGraph     IN OUT  graph header
* @param mergeType  IN      rule to apply to graph for handling duplicate edges: VUUNION_PARITY or VUUNION_KEEP_ONE_AMONG_DUPLICATES, or
*                           any other value to keep all duplicates
* @param absTol     IN      absolute tolerance, passed into ~mvu_toleranceFromGraphXY to compute merge tolerance
* @param relTol     IN      relative tolerance, passed into ~mvu_toleranceFromGraphXY to compute merge tolerance
* @group "VU Booleans"
* @see vu_unionLoops, vu_mergeLoops
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_merge2002
(
VuSetP pGraph,
VuMergeType mergeType,
double absTol,
double relTol
);

END_BENTLEY_GEOMETRY_NAMESPACE

