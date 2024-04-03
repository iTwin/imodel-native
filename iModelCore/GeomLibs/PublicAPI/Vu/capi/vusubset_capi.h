/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
@description Step forward around a face loop.
@param nodeP IN node to query
@returns next node around face
@group "VU Nodes"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_fsucc
(
VuP             nodeP
);


/*---------------------------------------------------------------------------------**//**
@description Step around a face loop a variable number of times.
@param nodeP IN node to query
@param numStep IN signed number of steps -- negative means step backwards.
@returns final node after steps
@group "VU Nodes"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP vu_fsucc (VuP nodeP, int numStep);

/*---------------------------------------------------------------------------------**//**
@description Step forward around a vertex loop.
@param nodeP IN node to query
@returns next node around vertex
@group "VU Nodes"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_vsucc
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Step to a node's predecessor in its face loop.
@param nodeP IN node to query
@returns previous node around face
@group "VU Nodes"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_fpred
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Step to a node's predecessor in its vertex loop.
@param nodeP IN node to query
@returns previous node around vertex
@group "VU Nodes"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_vpred
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Step to the node on the opposite side and at the opposite end of an edge.
@param nodeP IN node to query
@returns the edge mate of the node
@group "VU Nodes"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_edgeMate

(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Step to the node on the opposite side and at the opposite end of an edge, in the reverse direction of the face.
@param nodeP IN node to query
@returns the reverse edge mate of the node
@group "VU Nodes"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_reverseEdgeMate (VuP nodeP);

/*---------------------------------------------------------------------------------**//**
@description Query all mask bits on nodeP.
@param nodeP IN node pointer
@return all mask bits from the node
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMask   vu_getCompleteMask
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Query mask bits on nodeP.
@param nodeP IN node pointer
@param maskBits IN mask bits to query
@return the specified mask bits from the node
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMask   vu_getMask
(
VuP             nodeP,
VuMask          maskBits
);

/*---------------------------------------------------------------------------------**//**
@description Set mask bits on nodeP.
@param nodeP IN OUT node pointer
@param maskBits IN mask bits (logically OR'd) to update
@return the complete node mask after the bits are set
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMask   vu_setMask      /* OUT     All mask bits in the node */
(
VuP             nodeP,
VuMask          maskBits
);

/*---------------------------------------------------------------------------------**//**
@description Set mask bits on a node and its edge mate.
@param nodeP IN OUT node pointer
@param maskBits IN mask bits (logically OR'd) to update
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setMaskAroundEdge    /* OUT     All mask bits in the node */
(
VuP             nodeP,
VuMask          maskBits
);

/*---------------------------------------------------------------------------------**//**
@description Clear mask bits on nodeP.
@param nodeP IN OUT node pointer
@param maskBits IN mask bits (logically OR'd) to update
@return the complete node mask after the bits are cleared
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMask   vu_clrMask
(
VuP             nodeP,
VuMask          maskBits
);

/*---------------------------------------------------------------------------------**//**
@description Set or clear mask bits on nodeP.
@param nodeP IN OUT node pointer
@param maskBits IN mask bits (logically OR'd) to update
@param value IN 0 or 1 to indicate clear or set mask bits
@return the complete node mask after the new bits are written
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMask   vu_writeMask
(
VuP             nodeP,
VuMask          maskBits,
int             value
);

/*---------------------------------------------------------------------------------**//**
@description Access the user data field of a node.
@param nodeP IN node pointer
@return the user data field, as a void pointer
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void *   vu_getUserDataP         /* OUT     The userDataP value in the VuP */
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Stores a pointer in the user data field of a single node.
@param nodeP IN OUT node pointer
@param userDataP IN value to store
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setUserDataP
(
VuP             nodeP,
void *          userDataP
);

/*---------------------------------------------------------------------------------**//**
@description Access the user data field of a node, casting it to an int.
@param nodeP IN node pointer
@return value IN the user data field, cast to an int
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_getUserDataPAsInt
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Stores an integer in the user data field of a single node.
@param nodeP IN OUT node pointer
@param userInt IN value to cast as pointer and store
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setUserDataPAsInt
(
VuP             nodeP,
int             userInt
);


/*---------------------------------------------------------------------------------**//**
@description Access the user data1 field of a node
@param nodeP IN node pointer
@return value IN the user data field
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP ptrdiff_t vu_getUserData1
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Stores an integer in the user data field of a single node.
@param nodeP IN OUT node pointer
@param userInt IN value to store
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setUserData1
(
VuP             nodeP,
ptrdiff_t       value
);

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Access the internal data field of a node.
@param nodeP IN node pointer
@return value IN the internal data field, cast to an int
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_getInternalDataPAsInt
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Store an integer in the internal data pointer field of a single node.
@param nodeP IN OUT node pointer
@param value IN value to cast to pointer and store
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setInternalDataPAsInt
(
VuP             nodeP,
int             value
);

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Access the internal data field of a node.
@param nodeP IN node pointer
@return value IN the internal data field
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void *vu_getInternalDataP
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Store a pointer in the internal data pointer field of a single node.
@param nodeP IN OUT node pointer
@param value IN value to store
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setInternalDataP
(
VuP             nodeP,
void*           value
);

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Copy internal dataP (or int, as per usage) from sourceP to destP
@param destP destination node.
@param soruceP source node.
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void   vu_copyInternalDataP (VuP destP, VuP sourceP);



/*---------------------------------------------------------------------------------**//**
@nodoc
@description Store an integer in the internal data pointer field of all nodes around a vertex.
@param nodeP IN OUT any node around the vertex
@param value IN value to cast to pointer and store
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setInternalDataPAsIntAroundVertex
(
VuP             nodeP,
int             value
);

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Store an integer in the internal data pointer field of all nodes around a face.
@param nodeP IN any node around the face
@param value IN value to cast to pointer and store
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setInternalDataPAsIntAroundFace
(
VuP             nodeP,
int             value
);

/*---------------------------------------------------------------------------------**//**
@description Count the edges (nodes) around a vertex.
@remarks This is the same as ~mvu_countEdgesAroundVertex.
@param nodeP IN any node at the vertex
@return the number of edges around the vertex loop
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_vertexLoopSize
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Count the edges (nodes) around a face.
@remarks This is the same as ~mvu_countEdgesAroundFace.
@param nodeP IN any node in the face
@return the number of edges around the face loop
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_faceLoopSize
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Count the edges (nodes) in the graph.
@remarks graph nodes "on the graph stack" are not included in this count.
@remarks this count is obtained by visiting all the nodes in the graph.  It is NOT a simple field access.
@param graph IN graph to inspect.
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_countNodesInGraph (VuSetP graph);

/*---------------------------------------------------------------------------------**//**
@description Count the number of nodes that have different values of a specified mask versus their face successor.
@param graph IN graph to inspect.
@param mask IN mask to compare.
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_countMaskChangesAroundFaces (VuSetP graph, VuMask mask);
/*---------------------------------------------------------------------------------**//**
@description Clear the user data fields in all nodes of the graph.
@param graphP IN OUT graph header
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_clearUserDataPInGraph
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Return a pointer to the extra data part of a node.
@remarks This pointer leads directly to the data part.  The size of this block is fixed at the time the graph is initialized.
@param nodeP IN node pointer
@return pointer to extra data at tail of node structure
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void *   vu_getExtraDataPointer
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Query the size (in bytes) of the extra data attached to each node of the graph.
@remarks This size is established at the time the graph is created.
@param graphP IN graph header
@return Byte count for extra data
@group "VU Node Data Fields"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_getExtraDataSize
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Set a mask on all nodes around a face.
@param startP IN any node on the face
@param mask IN mask to apply
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void   vu_markFace
(
VuP             startP,
VuMask          mask
);

/*---------------------------------------------------------------------------------**//**
@description Set a mask on all nodes around a vertex.
@param startP IN any node on the vertex
@param mask IN mask to apply
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void   vu_markVertex
(
VuP             startP,         /* Any start node on the vertex whose area is needed */
VuMask          mask            /* Mask to install along the way */
);

/*---------------------------------------------------------------------------------**//**
@description Count the edges (nodes) around a vertex.
@param startP IN any node at the vertex
@return the number of edges around the vertex loop
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   vu_countEdgesAroundVertex
(
VuP             startP
);

/*---------------------------------------------------------------------------------**//**
@description Count the edges (nodes) around a face.
@param startP IN any node in the face
@return the number of edges around the face loop
@group "VU Edges"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   vu_countEdgesAroundFace
(
VuP             startP
);

/*---------------------------------------------------------------------------------**//**
@description Count the edges around a vertex that have a specified mask.
@param startP IN any node at the vertex
@param mask IN mask to test
@return the number of edges that are in the vertex loop containing startP and that have the indicated mask set
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_countMaskAroundVertex
(
VuP             startP,
VuMask          mask
);

/*---------------------------------------------------------------------------------**//**
@description Count the edges around a face that have a specified mask.
@param startP IN any node in the face
@param mask IN mask to test
@return the number of edges that are in the face loop containing startP and that have the indicated mask set
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_countMaskAroundFace
(
VuP             startP,
VuMask          mask
);

/*---------------------------------------------------------------------------------**//**
@description Count the edges around a vertex that have a specified mask on their edge mate.
@param startP IN any node at the vertex
@param mask IN mask to test on edge mates
@return the number of edges that are in the vertex loop containing startP and that have the indicated mask set on their edge mate
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_countMateMaskAroundVertex
(
VuP             startP,
VuMask          mask
);

/*---------------------------------------------------------------------------------**//**
@description Count the edges around a face that have a specified mask on their edge mate.
@param startP IN any node in the face
@param mask IN mask to test on edge mates
@return the number of edges that are in the face loop containing startP and that have the indicated mask set on their edge mate
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_countMateMaskAroundFace
(
VuP             startP,
VuMask          mask
);

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to all nodes.
@param arrayP OUT array of node pointers
@param graphP IN graph header
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectAllNodes
(
VuArrayP        arrayP,
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to all nodes with a mask present
@param arrayP OUT array of node pointers
@param graphP IN graph header
@param mask IN target mask
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectMaskedNodes
(
VuArrayP        arrayP,
VuSetP          graphP,
VuMask          mask
);

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to all nodes with a mask not present
@param arrayP OUT array of node pointers
@param graphP IN graph header
@param mask IN target mask
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectUnMaskedNodes
(
VuArrayP        arrayP,
VuSetP          graphP,
VuMask          mask
);

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to representative nodes of exterior face loops.
@remarks Only exterior nodes will appear in the array; face loops with all interior nodes are not represented.
@param faceArrayP OUT array of node pointers
@param graphP IN graph header
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectExteriorFaceLoops
(
VuArrayP        faceArrayP,
VuSetP          graphP
);

/*------------------------------------------------------------------*//**
@description Advance one step forward in the face loop, then take steps backwards around the vertex loop until a masked edge is found.
@remarks This is a "face step" in a graph in which "real" face edges have their edges masked, and non-masked edges interior to the real
    face are to be ignored.
@param nodeP IN start node for search
@param mask IN mask to find
@group "VU Node Masks"
@return first node found with specified mask
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_searchMaskedFSuccVPred
(
VuP             nodeP,
VuMask          mask
);

/*------------------------------------------------------------------*//**
@nodoc
@deprecated vu_searchMaskedFSuccVPred
@description Advance one step forward in the face loop, then take steps backwards around the vertex loop until a masked edge is found.
@remarks This is a "face step" in a graph in which "real" face edges have their edges masked, and non-masked edges interior to the real
    face are to be ignored.
@param nodeP IN start node for search
@param mask IN mask to find
@group "VU Node Masks"
@return first node found with specified mask
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_SearchMaskedFSuccVPred
(
VuP             nodeP,
VuMask          mask
);

/*------------------------------------------------------------------*//**
@description Search around a face loop for any node that has a specified mask.
@param nodeP IN first node to test
@param mask IN mask to find
@group "VU Node Masks"
@return first node found with specified mask
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_findMaskAroundFace
(
VuP             nodeP,
VuMask          mask
);

/*------------------------------------------------------------------*//**
@description Search around a vertex loop for any node that has a specified mask.
@param nodeP IN first node to test
@param mask IN mask to find
@group "VU Node Masks"
@return first node found with specified mask
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_findMaskAroundVertex
(
VuP             nodeP,
VuMask          mask
);

/*------------------------------------------------------------------*//**
@description Search entire graph for any node that has a specified mask.
@param graph IN graph to search
@param mask IN mask to find
@group "VU Node Masks"
@return first node found with specified mask
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_findMaskInSet
(
VuSetP      graph,
VuMask          mask
);



/*------------------------------------------------------------------*//**
@description Search around a vertex loop for any node that does not have a specified mask.
@param nodeP IN first node to test
@param mask IN mask to find
@group "VU Node Masks"
@return first node found without specified mask
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_findUnMaskedAroundVertex
(
VuP             nodeP,
VuMask          mask
);

/*------------------------------------------------------------------*//**
@description Search around a face loop for any node that does not have a specified mask.
@param nodeP IN first node to test
@param mask IN mask to find
@group "VU Node Masks"
@return first node found without specified mask
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_findUnMaskedAroundFace
(
VuP             nodeP,
VuMask          mask
);

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to representative nodes of interior face loops.
@remarks Only interior nodes will appear in the array; face loops with all exterior nodes are not represented.
@param faceArrayP OUT array of node pointers
@param graphP IN graph header
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectInteriorFaceLoops
(
VuArrayP        faceArrayP,
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to representative nodes of face loops.
        Distrubite the loops into arrays according to mask states.
        Any combination of null and repeated array pointers is allowed.
@param faceArrayUnMaskedP OUT array of node pointers for faces that have no masks.
@param faceArrayMixedP OUT array of node pointers for faces containing both masked and unmasked nodes.
@param faceArrayMaskedP OUT array of node pointers for faces that are fully masked.
@param graphP IN graph header
@param mask   IN test mask
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectMaskedFaceLoops
(
VuArrayP        faceArrayUnMaskedP,
VuArrayP        faceArrayMixedP,
VuArrayP        faceArrayMaskedP,
VuSetP          graphP,
VuMask          mask
);

/*---------------------------------------------------------------------------------**//**
@description Find a specific node around a vertex.
@param nodeP IN start node for search around vertex loop
@param targetP IN target node
@return NULL if not found, targetP if found
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_findNodeAroundVertex
(
VuP             nodeP,
VuP             targetP
);

/*---------------------------------------------------------------------------------**//**
@description Find a specific node around a face.
@param nodeP IN start node for search around face loop
@param targetP IN target node
@return NULL if not found, targetP if found
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_findNodeAroundFace
(
VuP             nodeP,
VuP             targetP
);

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to representative nodes of all vertex loops with interior nodes.
@remarks Only interior nodes will appear in the array; vertex loops with all exterior nodes are not represented.
@param vertexArrayP OUT array of node pointers
@param graphP IN graph header
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectVertexLoops
(
VuArrayP        vertexArrayP,
VuSetP          graphP          /* IN      parent graph */
);

/*---------------------------------------------------------------------------------**//**
@description Allocate a header for a marked edge set.
@param graphP IN OUT graph header
@param mExcluded IN mask for edges that may not be entered into the set
@return pointer to the new, empty edge set
@group "VU Marked Edge Sets"
@see vu_markedEdgeSetFree
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMarkedEdgeSetP vu_markedEdgeSetNew
(
VuSetP graphP,
VuMask mExcluded
);

/*---------------------------------------------------------------------------------**//**
@description Free the header and associated memory of a marked edge set.
@param edgeSetP IN edge set
@group "VU Marked Edge Sets"
@see vu_markedEdgeSetNew
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markedEdgeSetFree
(
VuMarkedEdgeSetP edgeSetP
);

/*---------------------------------------------------------------------------------**//**
@description Conditionally add an edge to the marked edge set.
@remarks The conditions to skip the add are:
<ul>
<li>the edge is already in the set, or</li>
<li>the edge has masks set that were identified as "excluded" in the call to ~mvu_markedEdgeSetNew.</li>
</ul>
@param edgeSetP IN OUT edge set
@param P IN node to test
@group "VU Marked Edge Sets"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markedEdgeSetTestAndAdd
(
VuMarkedEdgeSetP edgeSetP,
VuP             P
);

/*---------------------------------------------------------------------------------**//**
@description Remove an edge from a marked edge set.
@param edgeSetP IN OUT edge set
@return pointer to one of the nodes of the chosen edge
@group "VU Marked Edge Sets"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP      vu_markedEdgeSetChooseAny
(
VuMarkedEdgeSetP edgeSetP
);

/*---------------------------------------------------------------------------------**//**
@description Search for an edge mate in a face marked non-null, i.e., skipping past edges that are marked null.
@remarks This assumes that VU_NULL_FACE bits have been set all around each face that
contains a null edge, i.e., triangles which have an edge at the north/south pole.
@remarks This function assumes meanings of:
<ul>
<li>VU_NULL_EDGE: actual polar edge</li>
<li>VU_NULL_FACE: face which exists in parameter space graph but collapses to a line in real graph.</li>
<li>VU_BOUNDARY_EDGE</li>
</ul>
@remarks These conditions are pretty esoteric.
@param startP IN start node for search
@param extraMask IN (optional) mask which is to be treated the same as a VU_NULL_FACE
@group "VU Node Masks"
@return mate node
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP vu_edgeMateInNonNullFace
(
VuP startP,
VuMask extraMask
);

/*---------------------------------------------------------------------------------**//**
@description Set a marker bit on all edges of all faces that have a trigger bit on any edge.
    That is, at each edge that has a trigger bit set, set the marker bit all around the face.

@remarks The implementation implicitly assumes that the marker bit is <EM>not set</EM> anywhere.
    Pre-set marker bits will cause unpredictable omissions from the marking.
@param graphP IN OUT graph header
@param triggerMask IN mask to trigger full marking of faces
@param markerMask IN mask to apply
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_markMarkedFaces
(
VuSetP graphP,
VuMask  triggerMask,
VuMask  markerMask
);

/*---------------------------------------------------------------------------------**//**
@description Compute the Euler characteristic of the graph.
@remarks The Euler characteristic is defined as the sum of the number of vertices (V) and faces (F)
    minus the number of edges (E), viz
<pre>
        V - E + F
</pre>
@remarks When a 2-manifold graph is "embedded" in a 3D space surface, the Euler characteristic
    is equal to twice the difference of the number of connected components (C) and the number of "holes" or "handles" (H)
    on the surface, viz
<pre>
        2 (C - H)
</pre>
@remarks Most importantly, for a single connected component embedded in the 2D plane (not in 3D), the Euler characteristic is expected
    to be exactly 2.  If the plane embedding has improper sorting of edges around faces and vertices, the sort errors create illogical
    "tunnels" through the graph; each "tunnel" drops the Euler characteristic by 2.
@param graphP IN graph header
@return the computed Euler characteristic
@group "VU Graph Header"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_eulerCharacteristic
(
VuSetP graphP
);

/*---------------------------------------------------------------------------------**//**
@description Count faces, edges, and vertices.
@remark this is a one-pass operation.  It does not count connected components directly,
   but if the graph is properly merged (non-toroidal) V-E+F=2*components
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      vu_countLoops
(
VuSetP graphP,
int &numVertices,
int &numEdges,
int &numFaces
);

#ifdef Build_vu_EulerCharacteristic_WrongCase
/*---------------------------------------------------------------------------------**//**
* @nodoc
* @deprecated vu_eulerCharacteristic
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_EulerCharacteristic
(
VuSetP graphP
);
#endif
/*---------------------------------------------------------------------------------**//**
@description  Find true exterior (negative area) faces.
    Visit adjacent faces recursively to assign parity and exterior masks.
@param pGraph IN OUT graph header
@param trueExteriorMask IN maks to apply to true exterior.
@param parityMask IN mask to apply to faces that are exterior by parity rules.
            trueExteriorMask and parityMask may be identical.
            parityMask can be 0 (i.e. only mark true exterior)
@param boundaryMask IN mask that identifies true boundaries
@return true if all adjacencies matched parity.  false if different paths to same place
    produced different parity.
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    vu_markParity
(
VuSetP pGraph,
VuMask trueExteriorMask,
VuMask parityMask,
VuMask boundaryMask
);

/*---------------------------------------------------------------------------------**//**
@description  Search for connected components.  In each component, find the face
   with most negative area.   (For a merged graph, these are the outer faces of
   the connected components.)
@param graphP IN graph header
@param seedArrayP OUT array of representative (most negative area) faces.
@param bulkArrayP OUT optional array containing all one node from each face of the entire
        graph, with each component terminated by a NULL pointer.
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_collectConnectedComponents
(
VuSetP graphP,
VuArrayP seedArrayP,
VuArrayP bulkArrayP
);





END_BENTLEY_GEOMETRY_NAMESPACE

