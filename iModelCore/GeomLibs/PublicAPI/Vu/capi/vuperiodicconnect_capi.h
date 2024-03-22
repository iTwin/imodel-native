/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
@description From each node with the seed mask, mark all nodes in the face loop and their edge mates if the edge
    isn't marked (on either side) by the barrier mask.
@remarks The flood mask must be distinct from the seed and barrier masks.  Seed and barrier masks may be duplicate.
    Any duplication of masks or violation of null conditions triggers return with no action.
@param pGraph       IN OUT  graph to examine
@param seedMask     IN      identifies nodes that are to serve as seeds for the search.  Must be non-null.
@param floodMask    IN      the mask to apply to accessible nodes.  This mask is cleared in the set at start.  Must be non-null.
@param barrierMask  IN      mask identifying edges not to be crossed.  May be null (This means that the
                            flood mask is set throughout any component containing a seed).
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_spreadMask
(
VuSetP pGraph,
VuMask seedMask,
VuMask floodMask,
VuMask barrierMask
);

/*-----------------------------------------------------------------*//**
@nodoc
@description Add edges to oriented loops using period data from the graph header.
@remarks This function is useful for generating connections to holes in tricky toroidal parameter spaces.
@remarks Multiple sweeps are performed, one from each cardinal direction.
@param pGraph IN OUT graph to examine
@param minGridXCell IN suggested grid count in x direction
@param minGridYCell IN suggested grid count in y direction
@group "VU Meshing"
@see vu_buildNonInterferingGrid
@bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_periodicRotateAndConnect
(
VuSetP pGraph,
int minGridXCell,
int minGridYCell
);

END_BENTLEY_GEOMETRY_NAMESPACE

