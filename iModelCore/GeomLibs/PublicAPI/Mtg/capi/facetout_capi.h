/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

#ifdef __cplusplus
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* Collect specified edges in GraphicsPointArray form.  Directly sequenced edges
* are chained (using vertex id to identify chaining opportunities)
* but no other searches are performed to enhance chaining.
* @param pFacetHeader   IN      source mesh
* @param pGraphicsPointHeader       OUT     collected edges
* @param pEdgeArray     IN      edges to collect
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_collectGraphicsPointEdges
(
    const MTGFacets    *pFacetHeader,
    GraphicsPointArray              *pGraphicsPointHeader,
const EmbeddedIntArray  *pEdgeArray
);

/**
* Collect faces as major-break-delimited loops.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_collectInteriorFacesToGPA
(
    const MTGFacets    *pFacets,
    GraphicsPointArray  *pGPA
);

/**
*
* Extract masked edges from facets to GraphicsPointArray.   Attempts to keep
* edges within a face in the same polyline.
*
* @param pFacetHeader    IN      source mesh
* @param pGraphicsPointHeader OUT     collected edges
* @param searchMask IN      search mask
* @param maskOn IN      target value of mask
*                    true collects masked edges.
*                    false collects unmasked edges.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_collectGraphicsPointMaskedEdges
(
const MTGFacets    *pFacetHeader,
      GraphicsPointArray        *pGraphicsPointHeader,
      MTGMask       searchMask,
      bool          maskOn
);

/**
* Output selected edges from a mesh.  Each edge is identified by
* its start node.   Edges marked exterior are skipped.
* @param pFacetHeader    IN      mesh to emit
* @param pEdgeStartArray IN      start nodes of selected edges.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_collectGraphicsPointEdgeStrokes
(
const MTGFacets    *pFacetHeader,
      GraphicsPointArray        *pGraphicsPointHeader,
      EmbeddedIntArray    *pEdgeStartArray
);

END_BENTLEY_GEOMETRY_NAMESPACE

#endif