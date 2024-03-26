/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Build a grid structure that does not interfere with the edges of the graph.
@remarks The graph is assumed to have correct boundary and exterior masks.
@param pGraph   IN OUT graph header
@param numXCell IN number of cells in x direction
@param numYCell IN number of cells in y direction
@param interiorMask IN mask to apply to the 'inside' of completely enclosed
        cells of the grid, i.e. to edge sides not reachable from other parts of the
        graph without crossing grid edges.
@param exteriorMask IN mask to apply to 'outside'
@group "VU Meshing"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_buildNonInterferingGrid
(
VuSetP pGraph,
int numXCell,
int numYCell,
VuMask interiorMask,
VuMask exteriorMask
);

END_BENTLEY_GEOMETRY_NAMESPACE

