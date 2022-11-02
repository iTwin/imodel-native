/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Use UV area to identify true exterior faces; start recursive traversal
* from each negative area face to set exterior masks according to parity
* rules as boundary edges are crossed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGParity_markFaceParityFromNegativeAreaFaces
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         parityChangeMask,
MTGMask         exteriorMask
);

END_BENTLEY_GEOMETRY_NAMESPACE

