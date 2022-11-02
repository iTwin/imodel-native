/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*------------------------------------------------------------------*//**
* Search a graph in which a mark set identifies distinguished faces.
* Mark geometrically exterior faces as depth 0.  For all other faces,
* the depth is the smallest number of transition edges which must be crossed to
* reach that face.
*
* @param pNodeIdDepthArray IN OUT  array of node depths.
* @param pMarkSet IN      Set membership defines depth transitions.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRG_setMarksetDepthByInwardSearch
(
RG_Header           *pRG,
EmbeddedIntArray    *pNodeIdToDepthArray,
MTG_MarkSet         *pMarkSet
);

/*------------------------------------------------------------------*//**
* Given a depth as determined by inward parity search, return the
* depth by the compressed depth definition that odd depth greater than 1
* is an island and any even depth greater than 1 is a canal.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     jmdlRG_depthToCompressedDepth
(
int depth
);

/*------------------------------------------------------------------*//**
* Compute classifications (unused, primary, canal, island) on both sides
* of an edge.
* @return true if classification completed.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool         jmdlRG_getCompressedDepthsOnEdge
(
RG_Header           *pRG,
int                 *pNodeCompressedDepth,
int                 *pMateCompressedDepth,
EmbeddedIntArray    *pNodeIdToDepthArray,
MTGNodeId           nodeId
);

END_BENTLEY_GEOMETRY_NAMESPACE

