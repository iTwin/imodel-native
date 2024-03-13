/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description Make multiple attempts to regularize the graph by sweeping up.
* @remarks Regularization of (the y-coordinates of) a graph entails inserting edges so that each
*       face has a single local minimum and a single local maximum, and so that holes have a bridge to their surrounding face.
* @param graphP     IN OUT  graph header
* @param attempts   IN      maximum number of attempts
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vureg_halfRegularizeGraphMultipleAttempts
(
VuSetP          graphP,
int             attempts
);

/*---------------------------------------------------------------------------------**//**
* @description Regularize the graph by sweeping up and down.
* @remarks Regularization of (the y-coordinates of) a graph entails inserting edges so that each
*       face has a single local minimum and a single local maximum, and so that holes have a bridge to their surrounding face.
* @remarks This is a critical step in triangulation.   Once the faces are regularized, triangulation
*       edges are easy to add in a single bottom-to-top sweep.
* @remarks This routine calls ~mvureg_halfRegularizeGraphMultipleAttempts twice---the second time
*       on a locally 180-degree rotated graph.
* @param graphP     IN OUT  graph header
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vureg_regularizeGraph
(
VuSetP graphP
);

/*---------------------------------------------------------------------------------**//**
* @description Regularize each individual face of the graph by sweeping up and down.
* @remarks Regularization of (the y-coordinates of) a graph entails inserting edges so that each
*       face has a single local minimum and a single local maximum, and so that holes have a bridge to their surrounding face.
* @remarks This routine calls ~mvureg_halfRegularizeGraphMultipleAttempts twice---the second time
*       on a locally 180-degree rotated graph.
* @remarks This routine assumes some prior process has connected holes to parents.
        Per-face logic respects periodics: each face is assumed consistent, but cross-edge seam is preserved.
* @param graphP     IN OUT  graph header
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vureg_regularizeConnectedInteriorFaces
(
VuSetP graphP
);

END_BENTLEY_GEOMETRY_NAMESPACE

