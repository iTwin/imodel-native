/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-------------------------------------------------------------------*//**
@nodoc "VU Internals"
@description Swap (twist) the "next" pointers of two vu nodes.
@remarks This is extremely dangerous and is to be done only by vu internal code.
@param P IN first node for swap.
@param Q IN second node for swap.
@bsimethod
+----------------------------------------------------------------------*/
Public void     vu_ntwist
(
VuP     P,
VuP     Q
);

/*---------------------------------------------------------------------------------**//**
@description A useful place to set a breakpoint.  Does nothing.
@param node0P IN first node at point of error detection.
@param node1P IN first node at point of error detection.
@group "VU Debugging"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_panicPair
(
VuP             node0P,
VuP             node1P
);

/*-------------------------------------------------------------------*//**
@description Check the fundamental invariant of a single cylic loop:
   each vu has exactly one predecessor in the V and F loops.
@remarks Call ~mvu_panicPair for each mismatch.
@param graphP IN graph header
@group "VU Debugging"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void            vu_checkPredecessors
(
VuSetP graphP
);

/*---------------------------------------------------------------------------------**//**
@description Check that the VUs in a set are coupled properly via their 'next' pointers.
@remarks Call ~mvu_panicPair for each mismatch.
@param graphP IN graph header.
@group "VU Debugging"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_checkNextInSet
(
VuSetP graphP
);

/*---------------------------------------------------------------------------------**//**
@description Function called by various vu internals when severe error conditions are detected.
@param graphP IN graph header.
@group "VU Debugging"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_panic
(
VuSetP graphP
);

/*---------------------------------------------------------------------------------**//**
@nodoc "VU Internals"
@description Create a new vertex use in a vu graph.
@remarks This is an ISOLATED, i.e. INVALID node for manifold representations;
    this function is only to be called by privileged functions that promise
    to make TWO VUs and join them properly.
@param graphP IN OUT graph header.
@return pointer to a node with both vertex and face pointers leading back to itself.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP             vu_newVuP
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Set the application function callback to be called for important transitions in a VU graph.
@param graphP IN OUT graph header
@param functionP IN message function
@param userDataP IN pointer to pass back on function calls
@group "VU Debugging"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setCMessageFunction
(
VuSetP          graphP,
VuMessageFunction functionP,
void            *userDataP
);

/*---------------------------------------------------------------------------------**//**
@description Set (store in graph) descriptions of periodic properties of x,y,z.
@remarks Period of 0 indicates conventional data in respective direction.
@remarks This data is available for query; this data is a late (2002) addition to VU, and is not widely used in calculations.
@param graphP IN OUT graph header
@param periodsP IN point whose x,y,z components are x,y,z periods.
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setPeriods
(
VuSetP          graphP,
const DPoint3d  *periodsP
);

/*---------------------------------------------------------------------------------**//**
@description Return descriptions of periodic properties of x,y,z.
@remarks Period of 0 indicates conventional data in respective direction.
@remarks This data is available for query; this data is a late (2002) addition to VU, and is not widely used in calculations.
@param graphP IN graph header
@param periodsP OUT point whose x,y,z components are x,y,z periods.
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_getPeriods
(
VuSetP      graphP,
DPoint3d    *periodsP
);

/*---------------------------------------------------------------------------------**//**
@description Set absolute and relative tolerances for the graph.
@remarks This data is available for query; this data is a late (2002) addition to VU, and is not widely used in calculations.
@param graphP IN OUT graph header
@param abstol IN absolute tolerance
@param reltol IN relative tolerance
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setTol
(
VuSetP          graphP,
double          abstol,
double          reltol
);

/*---------------------------------------------------------------------------------**//**
@description Returns absolute and relative tolerances for the graph.
@remarks This data is available for query; this data is a late (2002) addition to VU, and is not widely used in calculations.
@param graphP IN graph header
@param pAbstol OUT absolute tolerance setting
@param pReltol OUT relative tolerance setting
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_getTol
(
VuSetP      graphP,
double      *pAbstol,
double      *pReltol
);

/*---------------------------------------------------------------------------------**//**
@description Returns tolerance used in most recent merge.
   This tolerance was computed from the graph abstol, reltol, and coordinate range.
   (It is NOT recomputed within vu_getMergeTol)
@param graphP IN graph header
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double vu_getMergeTol (VuSetP graphP);

/*---------------------------------------------------------------------------------**//**
@description Sets the mege tolerance in the graph header.
@param graphP IN graph header
@param tol IN tolerance to save.
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setMergeTol (VuSetP graphP, double tol);
/*---------------------------------------------------------------------------------*//**
@description Swap vertex loop successors and face loop predecessors of two nodes.
@param graphP IN OUT graph header
@param node0P IN first node of swap
@param node1P IN second node of swap
@group "VU Nodes"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_vertexTwist
(
VuSetP          graphP,
VuP             node0P,
VuP             node1P
);

/*---------------------------------------------------------------------------------**//**
@description On first call, enable save mode in which the graph most recently
    passed to ~mvu_freeVuSet is retained for post-mortem inspection.  On subsequent calls,
    the most recently freed graph is turned over to the caller.
@remarks Caller becomes responsible for freeing the returned graph (with ~mvu_freeVuSet).
@return the most recently freed graph.
@group "VU Debugging"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuSetP   vu_getDebugGraph
(
);

/*---------------------------------------------------------------------------------**//**
@description Free a graph header and all associated memory.
@param graphP IN OUT header for graph to be freed.
@return Always returns NULL.
@group "VU Graph Header"
@see vu_newVuSet
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuSetP            vu_freeVuSet
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Make a "deep" copy of a graph.
@remarks If destination graph is not provided, a new one is created via ~mvu_newVuSet.
    If destination graph is provided, its extra data size must match the source exactly.
@param destGraphP IN OUT preallocated header which receives the graph data, or NULL.
@param sourceGraphP IN source graph.
@group "VU Graph Header"
@return pointer to the copied graph's header.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuSetP vu_copyVuSet
(
VuSetP          destGraphP,
VuSetP          sourceGraphP
);

/*---------------------------------------------------------------------------------**//**
@description Test if a graph is empty, e.g., if it has no nodes.
@param graphP IN graph to test.
@group "VU Graph Header"
@return true if graph is empty.
@see vu_reinitializeVuSet
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool           vu_emptyGraph
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Allocate (from heap) a new VU graph with no nodes.
@param extraBytesPerVuNode IN unused
@return pointer to newly allocated and initialized graph
@group "VU Graph Header"
@see vu_freeVuSet
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuSetP          vu_newVuSet
(
int extraBytesPerVuNode
);

/*---------------------------------------------------------------------------------**//**
@description Store a default value for the user data field of each node subsequently created.
@param graphP IN OUT graph header
@param value IN integer value to store.
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setDefaultUserDataPAsInt
(
VuSetP  graphP,
int value
);

/*---------------------------------------------------------------------------------**//**
@description Store a default value for the userData1 field of each node subsequently created.
@param [inout] graphP graph header
@param [in] value integer value to store.
@parm [in] isVertexProperty true to have the value copied "around vertex" during modifications
@parm [in] isEdgeProperty true to have the value copied "along the edge" during split
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setDefaultUserData1
(
VuSetP  graphP,
ptrdiff_t value,
bool  isVertexProperty,
bool  isEdgeProperty
);

/*---------------------------------------------------------------------------------**//**
@description In the graph header, store a flag indicating whether or not the user data pointer
   field of each node is a vertex property and is to be maintained during flip and
   join operations.
@param graphP IN OUT graph header
@param bIsVertexProperty IN true if the user data pointer is to be managed as a vertex
    property during flip and join operations.
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setUserDataPIsVertexProperty
(
VuSetP  graphP,
bool    bIsVertexProperty
);

/*---------------------------------------------------------------------------------**//**
@description In the graph header, store a flag indicating whether or not the user data pointer
   field of each node is an edge property and is to be maintained during edge split.
@param graphP IN OUT graph header
@param bIsEdgeProperty IN true if the user data pointer is to be managed as an edge property
        during edge split operation.
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setUserDataPIsEdgeProperty
(
VuSetP  graphP,
bool    bIsEdgeProperty
);

/*---------------------------------------------------------------------------------**//**
@description Set the function to be called to announce intermediate graph states to debuggers.
@param func IN callback announcement function
@group "VU Debugging"
@see vu_postGraphToTrapFunc
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setGraphTrapFunc
(
VuGraphTrapFunc func
);

/*---------------------------------------------------------------------------------**//**
@description Trigger an announcement of the graph state to a debugger.
@remarks It is expected that a well-behaved trap function will inspect or copy
   the graph but never modify.
@remarks The multiple identifier arguments allow applications to post many graphs at little cost, while the debugger
   ignores all but a limited number of the posts.
@param pGraph IN graph to post
@param pAppName IN string description of caller
@param id0 IN integer state data
@param id1 IN integer state data
@group "VU Debugging"
@see vu_setGraphTrapFunc
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_postGraphToTrapFunc
(
VuSetP  pGraph,
char const*pAppName,
int     id0,
int     id1
);

/*---------------------------------------------------------------------------------**//**
@description Reset a graph to empty state.
@remarks Memory allocated to the graph is kept so that subsequent graph construction can be fast.
@remarks Fixed masks are left unchanged.
@remarks The free mask pool is left unchanged --- the caller is expected to have returned them.
@remarks All nodes are shifted to the free pool.
@param graphP IN OUT graph header
@group "VU Graph Header"
@see vu_emptyGraph
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void          vu_reinitializeVuSet
(
VuSetP graphP
);

/*---------------------------------------------------------------------------------**//**
@description Return the number of nodes that have been allocated.
@param graphP IN graph header
@return number of nodes allocated.
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_countNodeAllocations
(
VuSetP  graphP
);

/*---------------------------------------------------------------------------------**//**
@description Return pointer to first node of a graph traversal.
@remarks The actual order of nodes is arbitrary: there is no inherent relationship to the order of creation.
@param graphP IN graph header
@return pointer to first node of a graph traversal.
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_firstNodeInGraph      /* OUT     first node in entire graph */
(
VuSetP  graphP          /* IN      graph to select node from */
);

/*---------------------------------------------------------------------------------**//**
@description Step to the next node in a graph traversal.
@param graphP IN Graph header
@param nodeP IN prior node of traversal
@return successor of nodeP in the global ordering of nodes in the graph.
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_nextNodeInGraph
(
VuSetP  graphP,
VuP     nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Free edges for which either node (start, end) is marked by a given mask.
@remarks The deletion process has two parts:
<ul>
<li>Find each node that has a specified seed mask; mark the node's edge mate and excise both from their vertex loops.</li>
<li>Move all such ndoes to the free list.</li>
</ul>
@remarks Be aware that this function invalidates pointers to the deleted nodes.  This function should not be called from within an algorithm
    that has saved pointers.
@remarks There are two concerns in deletion: integrity and efficiency.  Deleting both VUs on an edge guarantees integrity of the graph
    Deleting many marked edges in one call is more efficient than many calls, each with only a single edge marked, because all edges are
    removed from the (singly linked) node list in a single pass.
@param graphP IN OUT graph to modify
@param seedMask IN mask to identify edges to delete
@group "VU Edges"
@see vu_freeNonMarkedEdges
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_freeMarkedEdges
(
VuSetP graphP,
VuMask seedMask
);

/*---------------------------------------------------------------------------------**//**
@description Free edges for which either node (start, end) is <EM>not</EM> marked by a given mask.
@remarks The deletion process has two parts:
<ul>
<li>Find each node that lacks a specified seed mask; mark the node's edge mate and excise both from their vertex loops.</li>
<li>Move all such ndoes to the free list.</li>
</ul>
@remarks Be aware that this function invalidates pointers to the deleted nodes.  This function should not be called from within an algorithm
    that has saved pointers.
@remarks There are two concerns in deletion: integrity and efficiency.  Deleting both VUs on an edge guarantees integrity of the graph
    Deleting many marked edges in one call is more efficient than many calls, each with only a single edge marked, because all edges are
    removed from the (singly linked) node list in a single pass.
@param graphP IN OUT graph to modify
@param seedMask IN mask whose absence identifies edges to delete
@group "VU Edges"
@see vu_freeMarkedEdges
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_freeNonMarkedEdges
(
VuSetP          graphP,
VuMask          seedMask
);

/*---------------------------------------------------------------------------------**//**
@description Count the number of times (0, 1 or 2) a mask appears on the nodes of the edge, and conditionally delete the edge by count test.
@param graphP IN OUT graph to modify
@param mask IN mask to count
@param free0 IN true to free edges with mask not appearing at all.
@param free1 IN true to free edges with mask appearing once
@param free2 IN true to free edges with mask appearing twice
@group "VU Edges"
@see vu_freeMarkedEdges
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_freeEdgesByMaskCount
(
VuSetP          graphP,
VuMask          mask,
bool            free0,
bool            free1,
bool            free2
);

/*---------------------------------------------------------------------------------**//**
@description Heal, or merge, two edges into one.
@param graphP IN OUT graph to modify.
@param nodeP IN one of the nodes at the degree-2 vertex being excised between the two edges to be healed.
*       On return, this node and its vertex successor are a sling.
@return false if vertex condition is not satisfied.
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    vu_healEdge
(
VuSetP graphP,
VuP    nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Heal, or merge, two side-by-side edges into one.
@param graphP IN OUT graph to modify.
@param nodeP IN one of the nodes at the degree-2 face being excised.
*       On return, this node and its vertex successor are a simple pair that has been pulled out.
*       The two "outside" vu edges are now mates !!!
*       The caller is responsible for deleting the floating edge.
@return false if the face has other than 2 edges.
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    vu_healNullFace
(
VuSetP graphP,
VuP    nodeP
);

/*---------------------------------------------------------------------------------**//**
@description remove all null faces (optionally limit to those with mask)
@param graphP IN OUT graph to modify.
@param mask IN optional mask.  If nonzero, only remove null faces with this mask on both edges.
@return number of faces deleted
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int    vu_exciseNullFaces
(
VuSetP graphP,
VuMask mask
);

/*---------------------------------------------------------------------------------**//**
@description Allocate two vu nodes that define an edge with two distinct vertices.
@param graphP IN OUT graph header
@param outnode1PP OUT newly created node
@param outnode2PP OUT newly created node
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_makePair
(
VuSetP          graphP,
VuP           *outnode1PP,
VuP           *outnode2PP
);

/*---------------------------------------------------------------------------------**//**
@description Make two vu nodes that define a sling edge with one vertex.
@param graphP IN OUT graph header
@param outnode1PP OUT newly created node
@param outnode2PP OUT newly created node
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_makeSling
(
VuSetP          graphP,
 VuP           *outnode1PP,
 VuP           *outnode2PP
);

/*---------------------------------------------------------------------------------**//**
@description Create a new vertex, typically within an existing edge but possibly as an isolated sling if no edges is given.
@remarks If P is not NULL, insert a new vertex within its existing edge.   The insertion is done as a primitive operation that preserves
   the vertex and face incidences of all preexisting vertex uses.
  If P is NULL, a new loop is created with VU_BOUNDARY_EDGE mask applied to both sides.   Subsequent split operations will
   propagate this mask to split edges.
@remarks This is useful in building up loops of many nodes when given coordinates in an array.  Initialize P=0, and then repeatedly
   add points to the loop, with no need to add the first point any differently from any other.
@param graphP       IN OUT  graph header
@param P            IN OUT  base node of edge to split
@param outnode1PP   IN      new node on same side as P
@param outnode2PP   IN      new node on opposite side
@group "VU Edges"
@see vu_splitEdgeAtPoint, vu_splitEdgeAtDPoint3d
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_splitEdge
(
VuSetP          graphP,
VuP             P,
VuP            *outnode1PP,
VuP            *outnode2PP
);

/*---------------------------------------------------------------------------------**//**
@description Create a new vertex, typically within an existing edge but possibly as an isolated sling if no edges is given.
@remarks If P is not NULL, insert a new vertex within its existing edge.   The insertion is done as a primitive operation that preserves
   the vertex and face incidences of all preexisting vertex uses.
@remarks This is useful in building up loops of many nodes when given coordinates in an array.  Initialize P=0, and then repeatedly
   add points to the loop, with no need to add the first point any differently from any other.
@param graphP       IN OUT  graph header
@param P            IN OUT  base node of edge to split
@param outnode1PP   IN      new node on same side as P
@param outnode2PP   IN      new node on opposite side
@param mask1        IN      mask to be applied to outNode1 when P is NULL.
@param mask2        IN      mask to be applied to outNode2 when P is NULL.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_splitEdge
(
VuSetP          graphP,
VuP             P,
VuP            *outnode1PP,
VuP            *outnode2PP,
VuMask          mask1,
VuMask          mask2
);



/*---------------------------------------------------------------------------------**//**
@description Copy from P to Q those mask bits that are designated in the graph for copying around vertex loops.
 @param graphP IN OUT graph header
 @param P IN source node
 @param Q IN OUT destination node
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_copyMaskAroundVertex
(
VuSetP graphP,
VuP P,
VuP Q
);

/*---------------------------------------------------------------------------------**//**
@description If graph says the "user data" field of nodes is vertex data, copy the user data from P to Q.   Otherwise do nothing.
@param graphP IN OUT graph header
@param P IN source node
@param Q IN OUT destination node
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_copyConditionalVertexData
(
VuSetP graphP,
VuP P,
VuP Q
);

/*---------------------------------------------------------------------------------**//**
@description Create a new edge and insert it from vertex P to vertex Q.
@remarks The nodes of the new edge receive mask bits according to the parent vu graph.
@param graphP IN OUT graph header
@param P IN OUT preexisting node.  If NULL, new edge dangles in space at this end.
@param Q IN OUT preexisting node.  If NULL, new edge dangles in space at this end.
@param outnode1PP OUT node of new edge, at P end.
@param outnode2PP OUT node of new edge, at Q end.
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_join
(
VuSetP graphP,
VuP P,
VuP Q,
VuP *outnode1PP,
VuP *outnode2PP
);

/*---------------------------------------------------------------------------------**//**
@description Excise both ends of the edge starting at nodeP from their vertices.
@param graphP IN OUT graph header
@param nodeP IN OUT node to detach from its vertex loops.
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_detachEdge
(
VuSetP          graphP,
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Get any available mask bit from the graph header.
@remarks The application must return the mask via ~mvu_returnMask
@param graphP IN OUT graph header
@return the borrowed mask
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMask vu_grabMask
(
VuSetP graphP
);

/*---------------------------------------------------------------------------------**//**
@description Return (drop) a mask bit previously grabbed (borrowed) via ~mvu_grabMask.
@param graphP IN OUT graph header
@param m IN the mask to drop
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_returnMask
(
VuSetP graphP,
VuMask m
);

/*---------------------------------------------------------------------------------**//**
@description Get any available array from the graph header.
@remarks The application must return the array via ~mvu_returnArray
@param graphP IN OUT graph header
@return a pointer to the borrowed array header.
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuArrayP vu_grabArray
(
VuSetP graphP
);

/*---------------------------------------------------------------------------------**//**
@description Return an array previously grabbed via ~mvu_grabArray.
@param graphP IN OUT graph header
@param arrayP IN pointer to array to drop.
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_returnArray
(
VuSetP graphP,
VuArrayP arrayP
);

/*---------------------------------------------------------------------------------**//**
@description Clear a mask bit in all nodes in a graph.
@param graphP IN OUT graph header
@param m IN mask to clear
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_clearMaskInSet
(
VuSetP          graphP,         /* Set in which the mask is to be cleared */
VuMask          m               /* Mask to be cleared */
);

/*---------------------------------------------------------------------------------**//**
@description Set a mask bit in all nodes in a graph.
@param graphP IN OUT graph header
@param m IN mask to set
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_setMaskInSet
(
VuSetP          graphP,         /* Set in which the mask is to be set */
VuMask          m               /* Mask to be cleared */
);

/*---------------------------------------------------------------------------------**//**
@description toggle a mask bit in all nodes in a graph.
@param graphP IN OUT graph header
@param m IN mask to change
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_toggleMaskInSet
(
VuSetP          graphP,         /* Set in which the mask is to be set */
VuMask          m               /* Mask to be toggled */
);


/*---------------------------------------------------------------------------------**//**
@description Copy value of one mask bit to another throughout the graph.
@param graphP IN OUT graph header
@param oldMask IN mask to read
@param newMask IN mask to set or clear
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_copyMaskInSet
(
VuSetP          graphP,         /* Set in which the mask is to be set */
VuMask          oldMask,        /* Mask to read */
VuMask          newMask         /* Mask to write */
);

/*---------------------------------------------------------------------------------**//**
@description Set a given mask bit in all nodes around the vertex loop starting at vertexP.
@param vertexP IN any node in the vertex loop
@param m IN the mask bits
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_setMaskAroundVertex
(
VuP             vertexP,
VuMask          m
);

/*---------------------------------------------------------------------------------**//**
@description Set a given mask bit in nodes around the face loop starting at startP.
@param startP IN any node at the face loop
@param m IN the mask bits
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_setMaskAroundFace
(
VuP             startP,
VuMask          m
);

/*---------------------------------------------------------------------------------**//**
@description Assign a unique index to each VU in the current graph.
@remarks These ids are considered volatile and may change if nodes are added to the graph.
@remarks In canonical indices, each edge pair (nodeP and vu_fsucc(nodeP)) is an even-odd sequential pair, i.e. (0,1), (2,3), etc.
@param graphP IN graph header
@return The number of nodes in the graph, or -1 if numbers could not be assigned.
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_assignCanonicalEdgeIndices
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Get the unique index assigned to a node by ~mvu_assignCanonicalEdgeIndices.
@param nodeP IN node to inspect
@return node index
@group "VU Nodes"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_getIndex     /* OUT     index of the node. */
(
VuP             nodeP                   /* IN      node to read */
);

END_BENTLEY_GEOMETRY_NAMESPACE

