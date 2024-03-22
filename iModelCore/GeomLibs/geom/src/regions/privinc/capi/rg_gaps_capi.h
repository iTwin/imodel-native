/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

/* DO NOT EDIT!  THIS FILE IS GENERATED. */

#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlRGGapList_init
(
RG_GapList  *pGapList,
RG_Header   *pRG,
MTGMask     mask,
double      vertexSize,
double      minVertexVertexGap,
double      maxVertexVertexGap,
double      minVertexEdgeGap,
double      maxVertexEdgeGap
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jdmlRGGapList_releaseMemory
(
RG_GapList  *pGapList
);

/*---------------------------------------------------------------------------------**//**
* Search for and record "gap" vertex pairs.
*
* @param vertexSize IN      tolerance for direct vertex clustering (without creating
*       gap edges)
* @param vertexVertexMin IN      min vertex-vertex gap length
* @param vertexVertexMax IN      max vertex-vertex gap length
* @param vertexVertexMin IN      min vertex-edge gap length
* @param vertexEdgeMax IN      max vertex-edge gap length
* @param vertexSize IN      size for initial vertex clustering
* @param boxFactor IN      when building a box (rather than closing gap)
*                   make the box this factor of the gap size.
* @param maxDiagFraction IN      but limit box to this fraction of range diagonal.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRGGapList_addGapEdgesToGraph
(
RG_Header       *pRG,
MTGMask         mask,
double          vertexSize,
double          vertexVertexMin,
double          vertexVertexMax,
double          vertexEdgeMin,
double          vertexEdgeMax,
double          boxFactor,
double          maxDiagBoxFraction
);

/*---------------------------------------------------------------------------------**//**
* Search for and close small gaps by moving vertex coordinates.
*
* @param vertexSize IN      tolerance for direct vertex clustering (without creating
*       gap edges)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_closeSimpleGaps
(
RG_Header       *pRG,
double          vertexSize
);

END_BENTLEY_GEOMETRY_NAMESPACE

