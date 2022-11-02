/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* @param pFacetHeader
* @param pNodeIdArray
* @param pVertexIdArray
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGChain_mergeNodes
(
MTGFacets *     pFacetHeader,
EmbeddedIntArray      *pNodeIdArray,
EmbeddedIntArray      *pVertexIdArray
);

/**
* @param pFacetHeader
* @param pNodeIdArray
* @param pVertexIdArray
* @param applyMask  IN      true to apply the mask
* @param maskToApply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGChain_identifyMatchedVerticesExt
(
MTGFacets * pFacetHeader,
EmbeddedIntArray  *pNodeIdArray,
EmbeddedIntArray  *pVertexIdArray,
bool        applyMask,
int         maskToApply,
double      absTol,
double      relTol
);

/**
* @param pFacetHeader   IN      facet header to examine.
* @param pNodeIdArray
* @param pVertexIdArray
* @param bool        applyMask
* @param int        maskToApply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGChain_identifyMatchedVertices
(
MTGFacets * pFacetHeader,
EmbeddedIntArray  *pNodeIdArray,
EmbeddedIntArray  *pVertexIdArray,
bool        applyMask,
int         maskToApply
);

/**
* @param pFacetHeader    OUT     MTG Graph resulting from compression
*                                           due to coincinding points
* @param pSourcePoints array of alternating start and end points
* @param int        sourcePointCount         the number of points multiple of 2
* @param abstol
* @param relTol,
* @return SUCCESS if
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_formChainGraphExt
(
MTGFacets * pFacetHeader,
DPoint3d    *pSourcePoints,
int         sourcePointCount,
double      abstol,
double      reltol
);

/**
* @param pFacetHeader    OUT     MTG Graph resulting from compression
*                                           due to coincinding points
* @param pSourcePoints array of alternating start and end points
* @param int        sourcePointCount         the number of points multiple of 2
* @return SUCCESS if
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_formChainGraph
(
MTGFacets * pFacetHeader,
DPoint3d    *pSourcePoints,
int         sourcePointCount
);

/**
* Extracts multiple chains from an input MTG_graph
* @param pFacetHeader   IN      facet header to examine.
* @param pChainPoints OUT     polyline formed by chaining segments in PsourcePoints together
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGChain_extractAsGPA
(
GraphicsPointArray  *  pChainPoints,
MTGFacets *  pFacetHeader
);

/**
* Extracts multiple chains from an input array storing line
* segments in a sequential manner.
*NOTE:  The elements in matchPoints and scratchArea are utilized for
*storing and manipulating intermediate information during extraction
*process.
* @param pChainPoints OUT     polyline formed by chaining segments
                                            in PsourcePoints together
* @param pSourcePoints array of alternating start and end points
* @param int        sourcePointCount         the number of points multiple of 2
* @see
* @return SUCCESS if
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGChain_segmentArrayToGPA
(
GraphicsPointArray  * pChainPoints,
DPoint3d    *pSourcePoints,
int         sourcePointCount
);

/**
* @param pGraph
* @param mask
* @param startNodeId
* @see
* @return int
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGChain_flipComponentOrientation
(
MTGGraph *      pGraph,
MTGMask mask,
MTGNodeId      startNodeId
);

/**
* @param pFacetHeader
* @see
* @return SUCCESS if
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGChain_twistEdgeStars
(
MTGFacets * pFacetHeader
);

/**
* This function is deprecated. Use jmdlMTGFacets_stitchFacets.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGChain_stitchMaskedEdgesExt
(
MTGFacets * pFacetHeader,
MTGMask    mask,
double      absTol,
double      relTol
);

/**
* @param pFacetHeader
* @param mask
* @see
* @return SUCCESS if
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGChain_stitchMaskedEdges
(
MTGFacets * pFacetHeader,
MTGMask    mask
);

/**
* This function is deprecated. Use jmdlMTGFacets_stitchFacets.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGChain_stitchMaskedEdgesWithOrientationFlips
(
MTGFacets * pFacetHeader,
MTGMask    mask
);

/**
* @param pGraph
* @param pMinLabel
* @param pMaxLabel
* @param offset
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGChain_findLabelRange
(
MTGGraph *  pGraph,
int         *pMinLabel,
int         *pMaxLabel,
int         offset
);

/**
* @param  pFacetHeader
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGChain_renumberVertices
(
MTGFacets * pFacetHeader
);

/**
* @param pFacetHeader
* @param pComponentFacet
* @param pStartArray
* @param pComponentArray
* @param componentNumber
* @see
* @return SUCCESS if
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGChain_getConnectedSubgraph
(
const   MTGFacets           *pFacetHeader,
        MTGFacets           *pComponentFacet,
        EmbeddedIntArray            *pStartArray,
        EmbeddedIntArray            *pComponentArray,
        int                 componentNumber
);

/*---------------------------------------------------------------------------------**//**
Allocate a sorter for chained fragments.
@return pointer to the sorter.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGFragmentSorterP bsiMTGFragmentSorter_new
(
);

/*---------------------------------------------------------------------------------**//**
Free a sorter for chained fragments.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiMTGFragmentSorter_free
(
MTGFragmentSorterP pSorter
);

/*---------------------------------------------------------------------------------**//**
@description Add a fragment for later end-to-end sorting.
@param pSorter IN OUT evolving sort structure.
@param parentId IN caller's non-negative id of the curve between the endpoints.
@param xyz0 IN start coordinate.
@param xyz1 IN   end coordinate.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiMTGFragmentSorter_addFragment
(
MTGFragmentSorterP pSorter,
int parentId,
DPoint3dCP pXYZ0,
DPoint3dCP pXYZ1
);

/*---------------------------------------------------------------------------------**//**
@description Add a fragment for later end-to-end sorting.
@param pSorter IN OUT evolving sort structure.
@param pLoopArray OUT filled with "signed one based" indices of fragments traversed by completely isolated loops.
@param pChainArray OUT filled with "signed one based" indices of fragments traversed by chains that are not part of loops.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiMTGFragmentSorter_sortAndExtractSignedLoopsAndChains
(
MTGFragmentSorterP pSorter,
EmbeddedIntArray *pLoopArray,
EmbeddedIntArray *pChainArray
);

/*---------------------------------------------------------------------------------**//**
@description Extract chain and loop data.
@param pSorter IN evolving sort structure.
@param pLoopArray OUT filled with "signed one based" indices of fragments traversed by completely isolated loops.
@param pChainArray OUT filled with "signed one based" indices of fragments traversed by chains that are not part of loops.
@param pLoopArray OUT filled with coordinates of completely isolated loops.
@param pChainArray OUT filled with coordinates of chains not parts of loops.
@remark The loop and chain coordinate arrays may be the same pointer.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiMTGFragmentSorter_sortAndExtractSignedLoopsAndChains
(
MTGFragmentSorterP pSorter,
bvector<int> *pLoopArray,
bvector<int> *pChainArray,
bvector<bvector<DPoint3d>> *pLoopCoordinates,
bvector<bvector<DPoint3d>> *pChainCoordinates
);
END_BENTLEY_GEOMETRY_NAMESPACE

