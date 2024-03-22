/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* Drop all edges that match a given mask condition.
* @param pGraph    IN OUT  containing graph
* @param mask IN      identifying mask
* @param value IN      0 to drop unmasked, 1 to drop masked
* @see
* @return number of dropped edges.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_dropByMask
(
MTGGraphP   pGraph,
MTGMask    mask,
int         value
);

/**
* Drop all edges that match a given mask condition on both sides of the edge
* @param pGraph    IN OUT  containing graph
* @param mask IN      identifying mask
* @param value IN      0 to drop unmasked, 1 to drop masked
* @see
* @return number of dropped edges.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_dropByDoubleMask
(
MTGGraphP   pGraph,
MTGMask    mask,
int         value
);

/**
* @param pGraph    IN      containing graph
* @param nodeId IN      node Id whose mask bits are queried.
* @param mask IN      selector for mask bits desired.
* @see
* @return MTGMask
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGMask jmdlMTGGraph_getMask
(
MTGGraphCP         pGraph,
        MTGNodeId       nodeId,
        MTGMask mask

);

/**
* Set selected mask bits on a node in a graph.
* @param pGraph    OUT     graph in which mask is to be set.
* @param nodeId IN      node id in which mask is to be set.
* @param mask IN      selector for mask bits to be set.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setMask
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask mask
);

/**
* Clear mask bits on all nodes in the graph.
* @param pGraph    OUT     graph in which mask is to be cleared.
* @param mask IN      selector for mask bits to be cleared.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_clearMaskInSet
(
MTGGraphP       pGraph,
MTGMask mask
);

/**
* Set a mask bit on all nodes in the graph.
* @param pGraph    OUT     graph in which mask is to be set.
* @param mask IN      selector for mask bits to be set.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setMaskInSet
(
MTGGraphP       pGraph,
MTGMask mask
);

/**
* Toggle the value of a given mask throughout a graph.
* @param pGraph    OUT     graph in which mask is to be set.
* @param mask IN      selector for mask bits to be set.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_toggleMaskInSet
(
MTGGraphP       pGraph,
MTGMask mask
);

/**
* Perform an AND, OR, XOR, or NAND operation between two specified masks
* on all nodes in the graph.
* @param pGraph    IN OUT  containing graph.
* @param mask0 IN      first mask to read
* @param mask1 IN      second mask to read
* @param result00 IN      result for !mask0, !mask1
* @param result01 IN      result for !mask0,  mask1
* @param result10 IN      result for  mask0, !mask1
* @param result11 IN      result for  mask0,  mask1
* @param mask2 IN      mask to set when boolOp is satisfied.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_maskedBooleanInSet
(
MTGGraphP   pGraph,
MTGMask    mask0,
MTGMask    mask1,
bool        result00,
bool        result01,
bool        result10,
bool        result11,
MTGMask    mask2
);

/**
* @param pGraph    OUT     graph in which mask is to be cleared.
* @param nodeId IN      node id in which mask is to be cleared.
* @param mask IN      selector for mask bits to be cleared.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_clearMask
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask mask
);

/**
* @param pGraph    IN OUT  containing graph.
* @param nodeId IN      specific node whose mask is to be changed.
* @param mask IN      mask selector
* @param offOn IN      0 to turn mask off, nonzero to turn on.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_writeMask
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask mask,
int             offOn
);

/**
* @param pGraph    IN OUT  containing graph.
* @param nodeId IN      specific node whose mask is to be changed.
* @param mask IN      mask selector
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_toggleMask
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask mask
);

/**
* @param pGraph    IN OUT  graph to be changed
* @param nodeId IN      node id
* @param mask IN      mask to apply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setMaskAroundFace
(
MTGGraphP       pGraph,
MTGNodeId      nodeId,
MTGMask mask
);

/**
* @param pGraph    IN OUT  graph to be changed
* @param nodeId IN      node id
* @param mask IN      mask to apply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setMaskAroundVertex
(
MTGGraphP       pGraph,
MTGNodeId      nodeId,
MTGMask mask
);

/*------------------------------------------------------------------*//**
* Walk around a face.  Set a mask on the edge mate of each node reached.
* @param pGraph    IN OUT  graph to be changed
* @param nodeId IN      node id
* @param mask IN      mask to apply
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setEdgeMateMaskAroundFace
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask         mask
);

/*------------------------------------------------------------------*//**
* Walk around a vertex.  Set a mask on the edge mate of each node reached.
* @param pGraph    IN OUT  graph to be changed
* @param nodeId IN      node id
* @param mask IN      mask to apply
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setEdgeMateMaskAroundVertex
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask         mask
);

/*------------------------------------------------------------------*//**
* Walk around a face.  Search for a mask on the edge mate of each node reached.
* Return the (edge mate) node id or null node id.
* @param pGraph    IN OUT  graph to be changed
* @param nodeId IN      node id
* @param mask IN      mask to search for
* @return node id where mask is found, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findEdgeMateMaskAroundFace
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask         mask
);

/*------------------------------------------------------------------*//**
* Walk around a vertex.  Search for a mask on the edge mate of each node reached.
* Return the (edge mate) node id or null node id.
* @param pGraph    IN OUT  graph to be changed
* @param nodeId IN      node id
* @param mask IN      mask to search for
* @return node id where mask is found, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findMaskOnEdgeMateAroundVertex
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask         mask
);

/**
* Advance one face loop step and 0 or more vertex loop predecessor
* steps to find a node with a given mask.
* @param pGraph    IN      graph to be changed
* @param nodeId IN      node id
* @param mask IN      mask to search for
* @see
* @return MTGNodeId
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getMaskedFVSucc
(
MTGGraphCP  pGraph,
MTGNodeId      nodeId,
MTGMask mask
);

/**
* Search the given node and its vertex loop for a masked node.
* @param pGraph    IN      graph
* @param nodeId IN      first node to examine.
* @param mask IN      mask to search for
* @return first masked node reached, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findMaskAroundVertex
(
MTGGraphCP  pGraph,
MTGNodeId      nodeId,
MTGMask mask
);

/**
* Search the given node and its face loop for a masked node.
* @param pGraph    IN      graph
* @param nodeId IN      first node to examine.
* @param mask IN      mask to search for
* @return first masked node reached, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findMaskAroundFace
(
MTGGraphCP  pGraph,
MTGNodeId      nodeId,
MTGMask mask
);

/**
* Search the given node and its vertex loop for a non masked node.
* @param pGraph    IN      graph
* @param nodeId IN      first node to examine.
* @param mask IN      mask to search for
* @return first masked node reached, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findNonMaskAroundVertex
(
MTGGraphCP  pGraph,
MTGNodeId      nodeId,
MTGMask mask
);

/**
* Search the given node and its face loop for a non masked node.
* @param pGraph    IN      graph
* @param nodeId IN      first node to examine.
* @param mask IN      mask to search for
* @return first masked node reached, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findNonMaskAroundFace
(
MTGGraphCP  pGraph,
MTGNodeId      nodeId,
MTGMask mask
);

/**
* @param  pGraph   IN OUT  graph to be changed
* @param nodeId IN      node id
* @param mask IN      mask to apply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_clearMaskAroundFace
(
MTGGraphP       pGraph,
MTGNodeId      nodeId,
MTGMask mask
);

/**
* @param pGraph    IN OUT  graph to be changed
* @param nodeId IN      node id
* @param mask IN      mask to apply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_clearMaskAroundVertex
(
MTGGraphP       pGraph,
MTGNodeId      nodeId,
MTGMask mask
);

/**
* @param pGraph    IN OUT  graph to be changed
* @param nodeId IN      node id
* @param mask IN      mask to apply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setMaskAroundEdge
(
MTGGraphP       pGraph,
MTGNodeId      nodeId,
MTGMask mask
);

/**
* @param pGraph    IN OUT  graph to be changed
* @param nodeId IN      node id
* @param mask IN      mask to apply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_clearMaskAroundEdge
(
MTGGraphP       pGraph,
MTGNodeId      nodeId,
MTGMask mask
);

/**
* @param pGraph    IN      graph
* @param pNumOn IN      number of masked nodes
* @param pNum0ff IN      number of unmasked nodes
* @param nodeId IN      node id
* @param mask IN      mask to check
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_countMasksAroundVertex
(
MTGGraphCP pGraph,
int                 *pNumOn,
int                 *pNum0ff,
MTGNodeId           nodeId,
MTGMask     mask
);

/**
* @param pNumOn IN      number of masked edge mate nodes
* @param pNum0ff IN      number of unmasked edge matenodes
* @param pGraph    IN      containing graph
* @param nodeId IN      node id
* @param mask IN      mask to check
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_countMateMasksAroundVertex
(
int                 *pNumOn,
int                 *pNum0ff,
MTGGraphCP pGraph,
MTGNodeId           nodeId,
MTGMask     mask
);

/**
* @param pGraph           IN      containing graph
* @param pNumOn     OUT     number of masked nodes (or null)
* @param pNum0ff    OUT     number of unmasked nodes (or null)
* @param nodeId     IN      start node
* @param mask       IN      mask to check
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_countMasksAroundFace
(
MTGGraphCP pGraph,
int             *pNumOn,
int             *pNum0ff,
MTGNodeId       nodeId,
MTGMask         mask
);

/**
* @param pNumOn IN      number of masked edge mate nodes
* @param pNum0ff IN      number of unmasked edge matenodes
* @param pGraph    IN      containing graph
* @param nodeId IN      node id
* @param mask IN      mask to check
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_countMateMasksAroundFace
(
int                 *pNumOn,
int                 *pNum0ff,
MTGGraphCP pGraph,
MTGNodeId           nodeId,
MTGMask     mask
);

/**
* @param pNumOn IN      number of masked edge mate nodes
* @param pNum0ff IN      number of unmasked edge matenodes
* @param pGraph    IN      containing graph
* @param mask IN      mask to check
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_countMasksInSet
(
int                 *pNumOn,
int                 *pNum0ff,
MTGGraphCP pGraph,
MTGMask     mask
);

/*---------------------------------------------------------------------------------**//**
At each node initially marked with an given mask, set the specified mask around each face
incident to the original vertex.
The motivating use of this is that in an xy triangulation with its range expanded by adding
points "outside" the primary points, this step pushes the exterior masks "into" the triangulation
so the artificial exterior points can be removed.
@param pGraph IN OUT graph to mark
@param seedMask IN mask being spread.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void        jmdlMTGGraph_spreadMaskToAdjacentFaces
(
MTGGraph *pGraph,
MTGMask  seedMask
);

END_BENTLEY_GEOMETRY_NAMESPACE

