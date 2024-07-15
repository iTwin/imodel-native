/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* @param pGraph    IN      containing graph
* @param nodeId IN      node id
* @see
* @return int
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_countNodesAroundFace
(
const MTGGraph *      pGraph,
MTGNodeId           nodeId
);

/**
* @param pGraph   IN      graph containing vertex
* @param nodeId IN      node id
* @see
* @return int
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_countNodesAroundVertex
(
const MTGGraph *    pGraph,
      MTGNodeId    nodeId
);

/**
* Search the graph for FACE loops.   The i'th face is reported in two ways:
* 1) pStartArray[[i]] is a node on the face i.
* 2) For each node k on the face i, pFaceId[[k]] == i.
*
* @param    pGraph      IN      graph to search
* @param pStartArray    IN OUT  For each face loop, this array tells one start mtg node.
*                               May be NULL
* @param pFaceId        IN OUT  For each mtg node, this array tells the face loop it is in.
*                               -1 if node id not in use  May be NULL
* @see
* @return int
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectAndNumberFaceLoops
(
const MTGGraph     *pGraph,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pFaceId
);

/**
*
* Search the graph for EDGEs.   The i'th edge is reported in
* two ways:
* 1) pStartArray[[i]] is a node on the edge.
* 2) For each node k on the edge i, pEdgeId[[k]] == i.
*
* @param pGraph               IN      graph to search.
* @param pStartArray    OUT     For each edge, this array tells one start mtg node. May be NULL
* @param pEdgeId        OUT     For each mtg node, this array tells the edge on.
*                           -1 if node id not in use May be NULL
* @see
* @return int
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectAndNumberEdges
(
const MTGGraph     *pGraph,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pEdgeId
);

/**
*
* Search the graph for VERTEX loops.   The i'th vertex is reported in
* two ways
* 1) pStartArray[[i]] is some node at vertex i.
* 2) For each node k on the vertex i, pVertexId[[k]] == i.
*
* @param pGraph               IN      graph to search.
* @param pStartArray    OUT     For each vertex loop, this array tells one start node. May be NULL
* @param pVertexId      OUT     For each mtg array node in the graph, this array tells
*                           the vertex id.          May be NULL
*                           -1 if node id not in use
* @see
* @return number of vertex loops
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectAndNumberVertexLoops
(
const MTGGraph     *pGraph,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pVertexId
);

/**
*
* Construct a spanning tree on the MTG graph, with callback used to
* assign candidate edges among several priorities.
*
* @param pGraph               IN      graph to search
* @param pTreeEdge      OUT     Edges in order used for tree
* @param pStartArray    OUT     For each vertex loop, this array tells one start node.
* @param pVertexId      OUT     For each mtg array node in the graph, this array tells
*                               the vertex id.
*                               -1 if node id not in use
* @param isPreferred    IN      test function
* @param mask           IN      mask to pass to test function
* @param pUserData      IN      user data for callback
* @see
* @return number of vertex loops
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_prioritizedSpanningTree
(
const MTGGraph *      pGraph,
EmbeddedIntArray            *pTreeEdge,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pVertexId,
MTG_NodeBoolFunc    isPreferred,
MTGMask     mask,
void                *pUserData
);

/**
* Initialize for a search conducted cooperatively by the caller and
* these functions.
* The search is guided by two node arrays.
* pStack is a stack of seeds for future exploration.
* pLoopId is an index from each node id A to the node id B
*   which is in the same vertex loop as A and was the entry point to
*   that vertex loop during the DFS.
* jmdlMTGGraph_seedSearch (... seedNodeId..) sets seedNodeId as the seed
* of its vertex loop, and adds all others around the loop to the search
* stack.
* nodeId = jmdlMTGGraph_harvest (...) extracts one node in an unvisited
*   vertex loop.   The newlyvisited node is planted as a seed for
*   further search.   Face successors are only followed if a mask is
*   set.
* @param pGraph    IN      graph being searched.
* @param pStack IN OUT  array for seeds of unexplored paths
* @param pLoopId IN OUT  array for loop id's
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_initSearch
(
const MTGGraph *     pGraph,
EmbeddedIntArray            *pStack,
EmbeddedIntArray            *pLoopId
);

/**
* (see jmdlMTGGraph_initMaskedDFS)
* @param pGraph               IN      graph to search
* @param pStack         IN OUT  array for seeds of unexplored paths
* @param pLoopId        IN OUT  array for loop id's
* @param seedNodeId     IN      seed node id
* @param preTest        IN      if true, test if the node is already
*                           labeled in the LoopId array.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_seedSearch
(
const MTGGraph *     pGraph,
EmbeddedIntArray            *pStack,
EmbeddedIntArray            *pLoopId,
MTGNodeId           seedNodeId,
bool                preTest
);

/**
* (see jmdlMTGGraph_initMaskedDFS)
* @param pGraph    IN      graph to search
* @param pStack IN OUT  array for seeds of unexplored paths
* @param pLoopId IN OUT  array for loop id's
* @param mask IN      mask to restrict search
* @see
* @return MTGNodeId
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_harvestMaskedDFS
(
const MTGGraph     *pGraph,
EmbeddedIntArray            *pStack,
EmbeddedIntArray            *pLoopId,
MTGMask     mask
);

/**
* Search (recursively) outward from nodes in pStartStack, applying
* a stopMask maks to each vertex loop.
* @param pGraph    IN OUT  graph to search and mark
* @param pStartStack IN OUT  On input, array of start nodes.
* @param                                         Used internally as a holding stack
*                                   On output, empty array.
* @param outMask IN      mask to marking 'out' edges from vertex loops
* @param stopMask IN      mask marking vertices that are already visited
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_spreadVertexMarker
(
MTGGraph *          pGraph,
EmbeddedIntArray            *pStartStack,
MTGMask     outMask,
MTGMask     stopMask
);

/**
* Search the graph for connected components.   The i'th component is
* reported in two ways:
* 1) pStartArray[[i]] is a seed node on component i.
* 2) For each node k on component i, pComponentId[[k]] == i.
* @param pStartArray OUT     For each component, this array tells one start mtg node. May be NULL
* @param pComponentId OUT     For each mtg node, this array tells the component loop it is in.
*                               -1 if node id not in use  May be NULL
* @param pGraph   IN      graph to search
* @see
* @return int
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectAndNumberConnectedComponents
(
const MTGGraph     *pGraph,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pComponentId
);

/**
* Search the graph for connected components, crossing only edges
*   not marked by a barrier mask.
* Collect node id's of each component in contiguous arrays, separated by null node ids.
* Barrier mask is assumed to be set completely on each face.
* @param pGraph IN      graph to search.
* @param pBlockedNodeIdArray IN      array of blocked node id's, separated by null node ids.
*           May be null pointer.
* @param barrierMask IN      mask for edges to be excluded.   May be MTG_NULL_MASK
* @return number of components.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectFaceLoopsInEdgeConnectedComponents
(
const MTGGraph     *pGraph,
EmbeddedIntArray            *pBlockedNodeIdArray,
MTGMask             barrierMask
);

/**
* Search the graph for connected components in a subgraph reachable by seed node
* and a mask.
* @param pAllNodeId OUT     Array of all node ids visited.  May be null.
* @param pOneNodeIdPerVertex OUT     Array with only one node id from each vertex loop.  May be null.
* @param mask IN      mask for edges included in the subgraph.
* @param directed IN      if true, only traverse edges with the mask set in the direction of
*               traversal.  If false, edges can be traversed if either side is masked.
* @return number of components.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectMaskedComponent
(
const MTGGraph  *pGraph,
EmbeddedIntArray      *pAllNodeId,
EmbeddedIntArray      *pOneNodeIdPerVertex,
MTGNodeId       seedNodeId,
int             mask,
bool            directed
);

/**
* Search the graph for connected components in a subgraph identified by a mask.
*   The i'th component is
* reported in three ways:
* 1) pStartArray[[i]] is a seed node on component i.
* 2) For each node k on component i, pComponentId[[k]] == i.
* @param pStartArray OUT     For each component, this array tells one start mtg node. May be NULL
* @param pComponentId OUT     For each mtg node, this array tells the component loop it is in.
*                               -1 if node id not in use  May be NULL
* @param pNodeIdGroupedByComponent OUT     Array of node ids grouped by component, separated by
*                   null node id.  May be NULL.
* @param pOneNodePerVertex OUT     array containing one node id per visited vertex.  May be NULL.
* @param mask IN      mask for edges included in the subgraph.
* @param directed IN      if true, only traverse edges with the mask set in the direction of
*               traversal.  If false, edges can be traversed if either side is masked.
* @return number of components.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectAndNumberMaskedConnectedComponents
(
const MTGGraph  *pGraph,
EmbeddedIntArray      *pStartArray,
EmbeddedIntArray        *pComponentId,
EmbeddedIntArray      *pNodeIdGroupedByComponent,
EmbeddedIntArray      *pOneNodeIdPerVertex,
int             mask,
bool            directed
);

/**
* Assign a partial order to nodes in the graph.
* Assume directionMask is ON at the tail of edges that must be observed
* by the order.   Edges with mask not set at all are ok.
* @param pGraph    IN      graph to search
* @param pOrder OUT     Nodes in search order
* @param directionMask IN      mask which is set on forward edges
* @see
* @return true if the order was computed.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_partialOrder
(
const MTGGraph *      pGraph,
EmbeddedIntArray            *pOrder,
MTGMask      directionMask
);

/*----------------------------------------------------------------------*//**
* Search a (directed) graph for strong components, applying masks to identify
* edges that join components from edges within components.
*
*<ul>
*<li>Clear bridge and component masks everywhere.</li>
*<li></li>
*</ul>
* @param directionMask IN      mask which identifies the forward direction
*           of the directed edges.
* @param componentMask IN      mask to be applied to all nodes which have direction
*               mask set and whose face successors are in the same strong component.
*               May be zero.
* @param bridgeMask IN      mask to be applied to all nodes which have the direction
*               mask set and whose successors are in a different connected
*               component.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_markEdgesWithinStrongComponents
(
MTGGraph        *pGraph,
MTGMask         directionMask,
MTGMask         componentMask,
MTGMask         bridgeMask
);

/*----------------------------------------------------------------------*//**
* Search and mark an undirected graph for vertices and edges that are
* on any path from nodeIdA to nodeIdB.  This does not enumerate the paths;
* it marks which edges and vertices are part of the paths.
* @param pathEdgeMask IN      mask to place on (both sides of) all edges crossed on any path.
* @param pathVertexMask IN      mask to place around every vertex along any path.
* @param activeEdgeMask IN      mask identifying face successor pointers that are to be crossed.
*                   If this is a null mask, all edges are crossed, i.e. the entire graph
*                           is undirected.
* @return true if any paths were found.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGMask    jmdlMTGGraph_markPathsBetweenNodes
(
MTGGraph        *pGraph,
MTGNodeId   nodeIdA,
MTGNodeId   nodeIdB,
MTGMask     pathEdgeMask,
MTGMask     pathVertexMask,
MTGMask     activeEdgeMask
);

/*-----------------------------------------------------------------*//**
* Search for the highest degree (number of vertices) of a face
* with given masking conditions.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlMTGGraph_maxFaceDegreeInSet
(
MTGGraph     *pGraph,
MTGMask             includeMask,
MTGMask             skipMask
);

/*---------------------------------------------------------------------------------**//**
* @description Query (a subset of) the faces of the given graph to determine the type of
*       the given label.  The label type as stored in the MTG header is ignored.
* @remarks If label values are constant around each face, then return MTG_LabelMask_FaceProperty;
*       if constant around each vertex, MTG_LabelMask_VertexProperty; if constant around each
*       edge, MTG_LabelMask_EdgeProperty; otherwise, return MTG_LabelMask_SectorProperty.
* <P>
* The optional boundaryMask restricts node search around each vertex to one side of the manifold.
*       A boundary mask may be set throughout the MTG by jmdlMTGGraph_setEdgeStarClassificationMasks.
*
* @param    pGraph              IN OUT  graph to search
* @param    labelOffset         IN      offset of label whose mask (e.g., MTG_LabelMask_SectorProperty) to compute
* @param    boundaryMask        IN      edge mask preset along MTG boundary (or zero to treat MTG as 2-manifold)
* @param    pFaceLoopSeedArray  IN      optimization; array as returned by jmdlMTGGraph_collectAndNumberFaceLoops,
*                                       jmdlMTGGraph_collectFaceLoopsInEdgeConnectedComponents, etc. (or NULL)
* @param    startIndex          IN      (if pFaceLoopSeedArray) first face loop seed to consider
* @param    endIndex            IN      (if pFaceLoopSeedArray) last face loop seed to consider
* @return label mask as computed from the labels, or MTG_LabelMask_None if error
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGLabelMask    jmdlMTGGraph_computeLabelMask
(
const MTGGraph*         pGraph,
int                     labelOffset,
MTGMask                 boundaryMask,
const EmbeddedIntArray* pFaceLoopSeedArray,
int                     startIndex,
int                     endIndex
);

/*-----------------------------------------------------------------*//**
Set a label at each node around a vertex.
@param pGraph IN containing graph
@param vertexNodeId IN any node at the vertex
@param labelOffset IN offset to address the label
@param labelValue IN value to store.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setLabelAroundVertex
(
MTGGraph     *pGraph,
MTGNodeId    vertexNodeId,
int          labelOffset,
int          labelValue
);

/*-----------------------------------------------------------------*//**
Set a label at each node around a face
@param pGraph IN containing graph
@param vertexNodeId IN any node at the face
@param labelOffset IN offset to address the label
@param labelValue IN value to store.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setLabelAroundFace
(
MTGGraph     *pGraph,
MTGNodeId    vertexNodeId,
int          labelOffset,
int          labelValue
);

END_BENTLEY_GEOMETRY_NAMESPACE

