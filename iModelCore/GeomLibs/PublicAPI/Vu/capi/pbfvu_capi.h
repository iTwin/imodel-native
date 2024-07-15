/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
* @description Refine a triangulation by splitting edges and joining them with new edges to form new triangles.
* @param pGraph     IN OUT  vu graph (triangulated) to refine
* @param testFunc   IN      function to test and prioritize edge split candidate
* @param splitFunc  IN      function to split edge
* @param joinFunc   IN      function to post-process a newly added edge
* @param pFuncData  IN      first arg for test and split funcs
* @param flipFunc   IN      unused
* @return number of edges split
* @group "VU Meshing"
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int  vu_refine
(
VuSetP  pGraph,
VuRefinementPriorityFunc testFunc,
VuRefinementEdgeSplitFunc splitFunc,
VuRefinementJoinFunc joinFunc,
void *pFuncData,
VuRefinementFlipTestFunc  flipFunc
);

/*-----------------------------------------------------------------*//**
* @nodoc
* @deprecated vu_triangulateBetweenCyclesByZ
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    pbfvu_triangulateyBetweenCyclesByZ
(
VuSetP  pGraph,
VuP     pLoopA,
VuP     pLoopB,
double  zPeriod,
VuRefinementJoinFunc joinFunc,
void *pFuncData,
VuMask newEdgeMask
);

/*-----------------------------------------------------------------*//**
* @description Triangulate between two face loops.
* @remarks This function is appropriate for triangulating the annulus on the end face of a pipe, or between top and bottom rings of a cylinder.
*       The triangulation direction is based on periodically adjusted values of the z-coordinate in each node.
* @param pGraph         IN OUT  vu graph
* @param pLoopA         IN      node in first face loop
* @param pLoopB         IN      node in second face loop
* @param zPeriod        IN      length of period of z-coordinate
* @param joinFunc       IN      function to post-process a newly added edge
* @param pFuncData      IN      pointer passed into callback
* @param newEdgeMask    IN      node mask to apply to new edges
* @return true if triangulation is successful
* @group "VU Meshing"
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    vu_triangulateBetweenCyclesByZ
(
VuSetP  pGraph,
VuP     pLoopA,
VuP     pLoopB,
double  zPeriod,
VuRefinementJoinFunc joinFunc,
void *pFuncData,
VuMask newEdgeMask
);

END_BENTLEY_GEOMETRY_NAMESPACE

