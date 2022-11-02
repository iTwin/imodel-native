/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* Construct parallel arrays of node and vertex ids for matching vertices.
* @param    pFacetHeader IN      facets to examine.
* @param pNodeIdArray OUT     array of nodeId's grouped by vertex cluster.
* @param pVertexIdArray OUT     parallel array of vertex ids.
* @param absTol IN      absolute tolerance for coordinate comparison.
* @param relTol IN      relative tolerance for coordinate comparison.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_createVertexClusterArrays
(
MTGFacets           *pFacetHeader,
EmbeddedIntArray    *pNodeIdArray,
EmbeddedIntArray    *pVertexIdArray,
double              absTol,
double              relTol
);

//--------------------------------------------------------------------------------------
// @bsiclass
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void jmdlMTGFacets_relableVertexClustersFromArrays
(
MTGFacets *     pFacetHeader,
EmbeddedIntArray      *pNodeIdArray,
EmbeddedIntArray      *pVertexIdArray
);

/**
* @param    pFacetHeader OUT     facet set to be stithed.
* @param absTol IN      absolute tolerance for vertex consolidation.
* @param relTol IN      relative tolerance for vertex consolidation.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_stitchFacets
(
MTGFacets * pFacetHeader,
double      absTol,
double      relTol
);


/**
* 2015 update of jmdlMTGFacets_stitchFacets
* @param    pFacetHeader <= facet set to be stithed.
* @param absTol => absolute tolerance for vertex consolidation.
* @param relTol => relative tolerance for vertex consolidation.
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_stitchFacetsForMultiVolume
(
MTGFacets * pFacetHeader,
double      absTol,
double      relTol
);

END_BENTLEY_GEOMETRY_NAMESPACE

