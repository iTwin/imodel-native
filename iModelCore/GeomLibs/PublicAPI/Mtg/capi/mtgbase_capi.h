/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* Query the number of nodes in a graph.
* @param pGraph    IN      Graph whose nodes are to be counted.
* @see
* @return the number of nodes in the graph.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_getNodeCount
(
MTGGraphCP        pGraph
);

/**
* Query the maximum node id that occurs in a graph.  This is at least
* as large as the node count (see jmdlMTGGraph_getNodeCount), but may be
* larger if the graph has deleted nodes.
* @param pGraph    IN      Containing graph
* @see
* @return the node id count.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_getNodeIdCount
(
MTGGraphCP        pGraph
);

/**
* Test if nodeId is a valid node id in the graph.
* @param pGraph    IN      Containing graph
* @param nodeId IN      id to test
* @see
* @return true if nodeId is valid.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_isValidNodeId
(
MTGGraphCP  pGraph,
MTGNodeId   nodeId
);

/**
* @param pGraph    IN      Containing graph
* @param candidateNodeid IN      suggested nodeId.
* @return any valid node id, or null node id if there are no nodes.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getAnyValidNodeId
(
MTGGraphCP  pGraph,
MTGNodeId   candidateNodeId
);

/**
* Create a new graph.  The return value from this function is the
* to be used as the pGraph argument on all subsequent operations on the
* graph.
* @see
* @return pointer to newly allocated and initialized graph.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGGraphP jmdlMTGGraph_newGraph
(
void
);

/**
* Initialize a graph header.
* @param pGraph    IN      Containing graph
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_initGraph
(
MTGGraphP pGraph
);

/**
* Eliminate all nodes from the graph, but preserve allocated memory
* to allow quick rebuild.   Label data is destroyed!!!
* @param pGraph                  IN OUT  graph to be emptied.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_empty
(
MTGGraphP pGraph
);

/**
*
* Eliminate all nodes from the graph, but preserve allocated memory
* to allow quick rebuild.  Optionally preserves label definitions.
*
* @param  pGraph   IN OUT  graph to be emptied.
* @param  preserveLabelDefinitions IN      true if the graph is going to be reused with
*               the same label structure.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_emptyNodes
(
MTGGraphP pGraph,
bool        preserveLabelDefinitions
);

/**
* Eliminate all nodes from the graph, and free memory that was used
* for them.
* @param pGraph  IN OUT  graph to be emptied.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_releaseMem
(
MTGGraphP pGraph
);

/**
* @param pDestGraph    OUT     destination graph
* @param pSourceGraph IN      source graph
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_copy
(
MTGGraphP   pDestGraph,
MTGGraphCP      pSourceGraph
);

/**
* Free graph header and all associated memory.
* @param pGraph  IN OUT  graph to be freed.
* @see
* @return MTGGraphCP
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGGraphP jmdlMTGGraph_freeGraph
(
MTGGraphP pGraph
);

/*---------------------------------------------------------------------------------**//**
* @param pGraph    IN      Containing graph
* @param nodeId     IN      requested node
* @return a pointer to the requested node's structure.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTG_Node     *jmdlMTGGraph_getNodePtr
(
MTGGraph     *pGraph,
MTGNodeId    nodeId
);

/*---------------------------------------------------------------------------------**//**
* @param pGraph    IN      Containing graph
* @param nodeId     IN      requested node
* @return a pointer to the requested node's structure.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP const MTG_Node   *jmdlMTGGraph_getConstNodePtr
(
const   MTGGraph   *pGraph,
const   MTGNodeId  nodeId
);

/**
* Free the edge (2 nodes) originating at a given node.
* If nodeId is a deleted node (e.g. the edge was already freed from
* the other end, nothing is done.
* @param pGraph    IN OUT  containing graph
* @param nodeId IN      one of the two node ids on the edge.
* @return true if the node id was valid.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlMTGGraph_dropEdge
(
MTGGraphP pGraph,
MTGNodeId  nodeId
);

/**
* Inverse of split.   Input nodeId = one of the two nodes added by split.
* Delete both nodes and reconnect the edge.
* @param pGraph    IN OUT  containing graph
* @param node0Id IN      one of the two node ids on the edge.
* @return true if the input node id was one of (exactly) two nodes at its vertex.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_healEdge
(
MTGGraphP       pGraph,
MTGNodeId  node0Id
);

/**
* Add to the number of labels on each node of the graph.
* Call this immediately after the graph is created.
* @param pGraph   IN OUT  graph in which label count is set.
* @param userTag IN      tag for later queries
* @param labelType IN      special properties to use for this label.
* @param defaultValue IN      value to assign to new nodes.
* @see
* @return offset that may be used to set and get the label values from nodes.
*                       -1 if the graph is null or cannot store more labels.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_defineLabel
(
MTGGraphP               pGraph,
int             userTag,
MTGLabelMask   labelType,
int             defaultValue
);

/**
* @param pGraph    IN      graph being queried.
* @param userTag    IN      tag
* @see
* @return label offset for the indicated tag.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_getLabelOffset
(
MTGGraphCP pGraph,
int                 userTag
);

/**
* @param pGraph    IN      graph whose label count is being queried.
* @see
* @return number of labels per node.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_getLabelCount
(
MTGGraphCP pGraph
);

/**
*
* Set the value of one label of one node of the graph.
*
* @param pGraph    IN OUT  graph in which label is applied
* @param nodeId IN      node whose label is set.
* @param offset IN      offset within labels of node
* @param value IN      new label value.
* @see
* @return true unless graph pointer, node id, or offset is invalid.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_setLabel
(
MTGGraphP               pGraph,
MTGNodeId      nodeId,
int             offset,
int             value
);

/**

* @param pGraph    IN OUT  graph in which data is copied
* @param MTGNodeId      sourceNodeId IN OUT  destination node
* @param destNodeId IN      source node
* @see
* @return SUCCESS unless graph pointer or either node id is invalid.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_copyMasksAndLabels
(
MTGGraphP               pGraph,
MTGNodeId      sourceNodeId,
MTGNodeId      destNodeId
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGGraph_copyPartialLabels                          |
|                                                                       |
|                                                                       |
| Copy masks and labels indicated by parameters.  Others are left       |
| unchanged.                                                            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_copyPartialLabels
(
MTGGraphP      pGraph,
MTGNodeId      destNodeId,
MTGNodeId      sourceNodeId,
MTGMask maskSelect,
unsigned long   labelSelect
);

/**
*
* @param pGraph    IN      containing graph.
* @param pLabel OUT     label value.
* @param nodeId IN      node id whose label is queried.
* @param offset IN      offset of queried label
* @see
* @return true unless graph pointer, node id, or offset is invalid.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_getLabel
(
MTGGraphCP pGraph,
int             *pLabel,
MTGNodeId      nodeId,
int             offset
);

/**
* Create an edge with two VU nodes in two distinct vertex loops.
* Return ids of both new nodes.
* @param pGraph    IN OUT  Containing graph
* @param pId0 OUT     start id of new edge
* @param pId1 OUT     end id of new edge
* @return true unless nodes could not be created.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_createEdge
(
MTGGraphP               pGraph,
MTGNodeId       *pId0,
MTGNodeId       *pId1
);

/**
* Create an edge with two VU nodes at a single node. Return both new
* node id's.
* @param pGraph    IN OUT  Containing graph
* @param pId0 OUT     'inside' id
* @param pId1 OUT     'outside' id
* @see
* @return SUCCESS unless nodes could not be created
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_createSling
(
MTGGraphP               pGraph,
MTGNodeId       *pId0,
MTGNodeId       *pId1
);

/**
* Create a new vertex that splits an existing edge.  Return node ids
* for the two VU nodes at the vertex.
* If idBase is MTG_NULL_NODEID, create a new sling and return
* pointers to its inside and outside.   This is useful when isolated
* faces  intialize a base pointer to MTG_NULL_NODEID, then apply
* splitEdge, each time advancing the base pointer to one of the
* new nodes.
* @param pGraph    IN OUT  Containing graph
* @param pId0 OUT     New node on same side as idBase
* @param pId1 OUT     New node on opposite side
* @param idBase IN      base of edge being split
* @return true unless nodes could not be created.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_splitEdge
(
MTGGraphP               pGraph,
MTGNodeId       *pId0,
MTGNodeId       *pId1,
MTGNodeId      idBase
);

/**
* Create a new edge that joins node A to node B.
* @param pGraph    IN OUT  Containing graph
* @param pIdNewA OUT     New node at vertex of node A
* @param pIdNewB OUT     New node at vertex of node B
* @param idA IN      node A
* @param idB IN      node B
* @param maskA IN      mask to apply to NewA
* @param maskB IN      mask to apply to newB
* @return true if nodes were created.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_join
(
MTGGraphP               pGraph,
MTGNodeId       *pIdNewA,
MTGNodeId       *pIdNewB,
MTGNodeId      idA,
MTGNodeId      idB,
MTGMask maskA,
MTGMask maskB
);

/**
* Create multiple edges along a pair of paths.
* @param pGraph             IN OUT  Containing graph
* @param idA        IN      start of first path.  Successive nodes to join are
*                       reached by FACE SUCCESSOR from this point.
* @param idB        IN      start of second path.  Successive nodes to join are
*                       reached by FACE SUCCESSOR from this point.
* @param numJoin    IN      number of edges to add
* @param maskA      IN      mask to apply to new nodes on the A path
* @param maskB      IN      mask to apply to new nodes on the B path
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_multipleJoin
(
MTGGraphP               pGraph,
MTGNodeId      idA,
MTGNodeId      idB,
int             numJoin,
MTGMask maskA,
MTGMask maskB
);

/**
* Add multiple nodes within a given edge.
* @param pGraph    IN OUT  Containing graph
* @param idA IN      base node of existing edge
* @param numVert IN      number of internal vertices to create
* @param maskLeft IN      mask to apply on the idA side.
* @param maskRight IN      mask to apply on the opposite side.
* @return true unless nodes could not be created.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_multipleSplit
(
MTGGraphP               pGraph,
MTGNodeId      idA,
int             numVert,
MTGMask maskLeft,
MTGMask maskRight
);

/**
* @param pGraph    IN      containing graph.
* @param nodeId IN      starting node id.
* @see
* @return node id of this node's vertex successor
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getVSucc
(
MTGGraphCP pGraph,
        MTGNodeId       nodeId
);

/**

* @param pGraph             IN      containing graph.
* @param nodeId     IN      starting node id.
* @see
* @return node id of this node's vertex predecessor
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getVPred
(
MTGGraphCP pGraph,
        MTGNodeId       nodeId
);

/**
* @param pGraph       IN      containing graph.
* @param nodeId IN      starting node id.
* @see
* @return node id of this node's face predecessor
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getFPred
(
MTGGraphCP pGraph,
        MTGNodeId       nodeId
);

/**
* @param pGraph    IN      containing graph.
* @param pFPred OUT     face predecessor
* @param pFSucc OUT     face successor
* @param pVPred OUT     vertex predecessor
* @param pVSucc OUT     vertex successor
* @param pEdgeMate OUT     edge mate on face successor edge
* @param pPredEdgeMate OUT     edge mate on face predecessor edge
* @param nodeId IN      starting node id.
* @see
* @return true if node id's are valid
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_getNeighbors
(
   MTGGraphCP pGraph,
        MTGNodeId      *pFPred,
        MTGNodeId      *pFSucc,
        MTGNodeId      *pVPred,
        MTGNodeId      *pVSucc,
        MTGNodeId      *pEdgeMate,
        MTGNodeId      *pPredEdgeMate,
        MTGNodeId       nodeId
);

/**
* @param pGraph    IN      containing graph.
* @param nodeId IN      starting node id.
* @see
* @return node id of this node's edge mate
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getEdgeMate
(
MTGGraphCP pGraph,
        MTGNodeId       nodeId
);

/**

* @param pGraph    IN      containing graph.
* @param nodeId IN      starting node id.
* @see
* @return node id of this node's face successor
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getFSucc
(
MTGGraphCP pGraph,
        MTGNodeId       nodeId
);

/**
* Temporarily borrow a mask for use in the graph.  The mask should be
* returned with jmdlMTGGraph_dropMask.
* @param  pGraph         IN OUT  graph from which a mask is borrowed.
* @see
* @return the loaned mask.  0 if no free masks available.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGMask jmdlMTGGraph_grabMask
(
MTGGraphP       pGraph
);

/**
* @param pGraph    IN      graph that is queried
* @param mask IN      mask to change
* @param MTGLabelMask       selector = indicates what property is defined.
* @param value IN      true to set, false to clear
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setMaskProperty
(
MTGGraphP       pGraph,
MTGMask     mask,
MTGLabelMask        selector,
bool                value
);

/**
* Query the current mask property settings.
*
* This query only responds to single masks in the selector!!!!
*
* @param pGraph IN      graph that is queried
* @param queryMask      IN      masks to query.
* @param selector       IN      indicates what property is defined.
* @see
* @return queried mask.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGMask jmdlMTGGraph_getMaskProperty
(
MTGGraphCP pGraph,
MTGMask     queryMask,
MTGLabelMask        selector
);

/**
*
* Return a mask previously borrowed from the graph.
*
* (Use jmdlMTGGraph_grab mask to borrow the mask.)
* @param pGraph    IN OUT  graph to which the mask is returned.
* @param mask IN      mask to return.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_dropMask
(
MTGGraphP       pGraph,
MTGMask mask
);

/**
* Yank the edge that ends at nodeId out of its vertex loop.
* Returns the id of the node that takes its place in the vertex
* sector, or MTG_NULL_ID if the edge is dangling already.
* @param pGraph    IN OUT  containing graph.
* @param nodeId IN      end node of edge to yank
* @see
* @return the id of the exposed node in the sector where the edge was removed.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_yankEdgeFromVertex
(
MTGGraphP               pGraph,
MTGNodeId       nodeId
);

/**
* Apply the twist operation between a pair of nodes.  That is, exchange
* SUCCESSORS in their vertex loops, predecessors in face loops.
* @param pGraph    IN OUT  containing graph.
* @param node0 IN      first node of twist operation
* @param node1 IN      second node of twist operation
* @see
* @return true if both nodes are valid.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_vertexTwist
(
MTGGraphP               pGraph,
MTGNodeId       node0,
MTGNodeId       node1
);

/**

* @param pGraph    IN OUT  containing graph.
* @param nodeX0 IN      first node of t
* @param nodeY0 IN      second node of twist operation
* @return true if nodes are valid
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_edgeTwist
(
MTGGraphP               pGraph,
MTGNodeId       nodeX0,
MTGNodeId       nodeY0
);

/**
* Reverse the sense of both the vertex and face successors of each node.
* @param pGraph  IN OUT  containing graph.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_reverse
(
MTGGraphP               pGraph
);

/**
* Get a (potentially large) set of new (uninitialized) nodes.
* Use jmdlMTGGraph_dropNodeArrays to return the array memory.
* @param pGraph    IN OUT  containing graph.
* @param MTG_Node *     **pppNode OUT     array of pointers to new nodes.
* @param MTGNodeId      **ppNodeId OUT     array of new node ids.
* @param numNode IN      number of nodes to create.
* @see
* @return true unless unable to get nodes.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_grabNodeArrays
(
MTGGraph       *pGraph,
MTG_Node *      **pppNode,
MTGNodeId       **ppNodeId,
int             numNode
);

/**
* Return the arrays created by a prior call to jmdlMTGGraph_grabNodeArrays.
* @param pGraph    IN OUT  containing graph.
* @param MTG_Node *     **pppNode IN OUT  array of node pointers.  Handle is nulled out.
* @param MTGNodeId      **ppNodeId       IN OUT  array of node ids.  Handle is nulled out.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_dropNodeArrays
(
MTGGraph       *pGraph,
MTG_Node        ***pppNode,
MTGNodeId       **ppNodeId
);

/**
* Build mtg nodes from explicit permutations.
* pFSucc[i] = face successor of node i
* pEdgeMate[i] = edge mate of node i
* Both arrays must be permutations, i.e.  for each i there is only one
*   j such that pFSucc[j]==i and one k such that pEdgeMate[k]==i.
* The edge mate relation must be a "nonsingular involution",
*   i.e. pEdgeMate[i] != i and pEdgeMate[pEdgeMate[i]]==i
*
* @param pGraph    IN OUT  containing graph.
* @param pFSuccArray IN      array giving the face successor of each node.
* @param pEdgeMateArray     IN      array giving the edge mate of each node
* @param numNode            IN      number of nodes.
* @see
* @return SUCCESS unless nodes could not be allocated or permutations are invalid.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_buildFromFSuccAndEdgeMatePermutations
(
MTGGraph       *pGraph,
const int       *pFSuccArray,
const int       *pEdgeMateArray,
int             numNode
);

END_BENTLEY_GEOMETRY_NAMESPACE

