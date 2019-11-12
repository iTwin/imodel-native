/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#include <math.h>
/******************************************************************************
**
** Miscellaneous search and filter operations on subsets of nodes in a vu graph
**
*******************************************************************************/

/**********************************************************************
** A VuMarkedEdgeSet is an abstract data type for managing sets of
** edges.   Applications deal with the set via the VuMarkedEdgeSetP
** pointers and use the service routines for all accesses into the
** set.
**
***********************************************************************/
typedef struct vuMarkedEdgeSet
{
    VuArrayP        arrayP;     /* flexible array to hold VuP's in the set.
                                   This is allocated from the array cache in
                                   graphP */
    VuMask          mExcluded;  /* Bits for node types that are excluded. */
    VuMask          mMember;    /* Bit mask for marking members. */
    VuMask          mExcludedOrMember;  /* OR of mExcluded and mMeber */
    VuSetP          graphP;     /* The parent vu graph */
    } VuMarkedEdgeSet;

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_markedEdgeSetInit                                    |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void    vu_markedEdgeSetInit
(
VuMarkedEdgeSetP edgeSetP,
VuSetP          graphP,
VuMask          mExcluded
)
    {
    edgeSetP->mExcluded = mExcluded;
    edgeSetP->mMember = vu_grabMask (graphP);
    edgeSetP->mExcludedOrMember = edgeSetP->mMember | edgeSetP->mExcluded;
    edgeSetP->arrayP = vu_grabArray (graphP);
    edgeSetP->graphP = graphP;
    vu_clearMaskInSet (graphP, edgeSetP->mMember);
    vu_arrayClear (edgeSetP->arrayP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_markedEdgeSetDeinit                                  |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void    vu_markedEdgeSetDeinit
(
VuMarkedEdgeSetP edgeSetP
)
    {
    vu_returnMask (edgeSetP->graphP, edgeSetP->mMember);
    vu_returnArray (edgeSetP->graphP, edgeSetP->arrayP);
    }

/*======================================================================+
|                                                                       |
|   Major Public Code Section                                           |
|                                                                       |
+======================================================================*/
/*---------------------------------------------------------------------------------**//**
@description Step forward around a face loop.
@param nodeP IN node to query
@returns next node around face
@group "VU Nodes"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_fsucc
(
VuP             nodeP
)
    {
    return VU_FSUCC(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Step around a face loop a variable number of times.
@param nodeP IN node to query
@param numStep IN signed number of steps -- negative means step backwards.
@returns final node after steps
@group "VU Nodes"
@bsimethod                                                    BentleySystems  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_fsucc (VuP nodeP, int numStep)
    {
    if (numStep >= 0)
        {
        while (numStep-- > 0)
          nodeP = VU_FSUCC(nodeP);
        }
    else
        {
        while (numStep++ < 0)
          nodeP = vu_fpred (nodeP);
        }
    return nodeP;
    }



/*---------------------------------------------------------------------------------**//**
@description Step forward around a vertex loop.
@param nodeP IN node to query
@returns next node around vertex
@group "VU Nodes"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_vsucc
(
VuP             nodeP
)
    {
    return VU_VSUCC(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Step to a node's predecessor in its face loop.
@param nodeP IN node to query
@returns previous node around face
@group "VU Nodes"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_fpred
(
VuP             nodeP
)
    {
    return VU_FPRED(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Step to a node's predecessor in its vertex loop.
@param nodeP IN node to query
@returns previous node around vertex
@group "VU Nodes"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_vpred
(
VuP             nodeP
)
    {
    return VU_VPRED(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Step to the node on the opposite side and at the opposite end of an edge.
@param nodeP IN node to query
@returns the edge mate of the node
@group "VU Nodes"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_edgeMate (VuP nodeP)
    {
    return VU_EDGE_MATE(nodeP);
    }
/*---------------------------------------------------------------------------------**//**
@description Step to the node on the opposite side and at the opposite end of an edge, in the reverse direction of the face.
@param nodeP IN node to query
@returns the trailing edge mate of the node
@group "VU Nodes"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_reverseEdgeMate (VuP nodeP)
    {
    return VU_FSUCC (VU_VSUCC (nodeP));
    }


/*---------------------------------------------------------------------------------**//**
@description Query all mask bits on nodeP.
@param nodeP IN node pointer
@return all mask bits from the node
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMask   vu_getCompleteMask
(
VuP             nodeP
)
    {
    return VU_MASK(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Query mask bits on nodeP.
@param nodeP IN node pointer
@param maskBits IN mask bits to query
@return the specified mask bits from the node
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMask   vu_getMask
(
VuP             nodeP,
VuMask          maskBits
)
    {
    return VU_GETMASK(nodeP,maskBits);
    }

/*---------------------------------------------------------------------------------**//**
@description Set mask bits on nodeP.
@param nodeP IN OUT node pointer
@param maskBits IN mask bits (logically OR'd) to update
@return the complete node mask after the bits are set
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMask   vu_setMask      /* <= All mask bits in the node */
(
VuP             nodeP,
VuMask          maskBits
)
    {
    return VU_SETMASK(nodeP,maskBits);
    }

/*---------------------------------------------------------------------------------**//**
@description Set mask bits on a node and its edge mate.
@param nodeP IN OUT node pointer
@param maskBits IN mask bits (logically OR'd) to update
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setMaskAroundEdge    /* <= All mask bits in the node */
(
VuP             nodeP,
VuMask          maskBits
)
    {
    VuP mateP = VU_EDGE_MATE(nodeP);
    VU_SETMASK (nodeP, maskBits);
    VU_SETMASK (mateP, maskBits);
    }

/*---------------------------------------------------------------------------------**//**
@description Clear mask bits on nodeP.
@param nodeP IN OUT node pointer
@param maskBits IN mask bits (logically OR'd) to update
@return the complete node mask after the bits are cleared
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMask   vu_clrMask
(
VuP             nodeP,
VuMask          maskBits
)
    {
    return VU_CLRMASK(nodeP,maskBits);
    }

/*---------------------------------------------------------------------------------**//**
@description Set or clear mask bits on nodeP.
@param nodeP IN OUT node pointer
@param maskBits IN mask bits (logically OR'd) to update
@param value IN 0 or 1 to indicate clear or set mask bits
@return the complete node mask after the new bits are written
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMask   vu_writeMask
(
VuP             nodeP,
VuMask          maskBits,
int             value
)
    {
    return VU_WRITEMASK(nodeP, maskBits, value);
    }

/*---------------------------------------------------------------------------------**//**
@description Access the user data field of a node.
@param nodeP IN node pointer
@return the user data field, as a void pointer
@group "VU Node Data Fields"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void *   vu_getUserDataP         /* <= The userDataP value in the VuP */
(
VuP             nodeP
)
    {
    return nodeP->userId.asPointer;
    }

/*---------------------------------------------------------------------------------**//**
@description Stores a pointer in the user data field of a single node.
@param nodeP IN OUT node pointer
@param userDataP IN value to store
@group "VU Node Data Fields"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setUserDataP
(
VuP             nodeP,
void *          userDataP
)
    {
    nodeP->userId.asPointer = userDataP;
    }

/*---------------------------------------------------------------------------------**//**
@description Access the user data field of a node, casting it to an int.
@param nodeP IN node pointer
@return value IN the user data field, cast to an int
@group "VU Node Data Fields"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_getUserDataPAsInt
(
VuP             nodeP
)
    {
    return nodeP->userId.asInt;
    }

/*---------------------------------------------------------------------------------**//**
@description Stores an integer in the user data field of a single node.
@param nodeP IN OUT node pointer
@param userInt IN value to cast as pointer and store
@group "VU Node Data Fields"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setUserDataPAsInt
(
VuP             nodeP,
int             userInt
)
    {
    nodeP->userId.asInt = userInt;
    }

/*---------------------------------------------------------------------------------**//**
@description Access the user data1 field of a node
@param nodeP IN node pointer
@return value IN the user data field
@group "VU Node Data Fields"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP ptrdiff_t vu_getUserData1
(
VuP             nodeP
)
    {
    return nodeP->userId1;
    }

/*---------------------------------------------------------------------------------**//**
@description Stores an integer in the user data field of a single node.
@param nodeP IN OUT node pointer
@param userInt IN value to store
@group "VU Node Data Fields"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setUserData1
(
VuP             nodeP,
ptrdiff_t       value
)
    {
    nodeP->userId1 = value;
    }



Public GEOMDLLIMPEXP void vu_setInternalDataP (VuP nodeP, void *value)      {nodeP->internalDataP.asPointer = value;}
Public GEOMDLLIMPEXP void vu_setInternalDataPAsInt (VuP nodeP, int value)   {nodeP->internalDataP.asInt = value;}

Public GEOMDLLIMPEXP void * vu_getInternalDataP (VuP nodeP)                 {return nodeP->internalDataP.asPointer;}
Public GEOMDLLIMPEXP int    vu_getInternalDataPAsInt (VuP nodeP)            {return nodeP->internalDataP.asInt;}
Public GEOMDLLIMPEXP void   vu_copyInternalDataP (VuP destP, VuP sourceP)   {destP->internalDataP = sourceP->internalDataP;}


/*---------------------------------------------------------------------------------**//**
@nodoc
@description Store an integer in the internal data pointer field of all nodes around a vertex.
@param nodeP IN OUT any node around the vertex
@param value IN value to cast to pointer and store
@group "VU Node Data Fields"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setInternalDataPAsIntAroundVertex
(
VuP             nodeP,
int             value
)
    {
    VU_VERTEX_LOOP (currP, nodeP)
        {
        vu_setInternalDataPAsInt (currP, value);
        }
    END_VU_VERTEX_LOOP (currP, nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Store an integer in the internal data pointer field of all nodes around a face.
@param nodeP IN any node around the face
@param value IN value to cast to pointer and store
@group "VU Node Data Fields"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setInternalDataPAsIntAroundFace
(
VuP             nodeP,
int             value
)
    {
    VU_FACE_LOOP (currP, nodeP)
        {
        vu_setInternalDataPAsInt (currP, value);
        }
    END_VU_FACE_LOOP (currP, nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Count the edges (nodes) around a vertex.
@remarks This is the same as ~mvu_countEdgesAroundVertex.
@param nodeP IN any node at the vertex
@return the number of edges around the vertex loop
@group "VU Edges"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_vertexLoopSize
(
VuP             nodeP
)
    {
    int count = 0;
    VU_VERTEX_LOOP (currP, nodeP)
        {
        count++;
        }
    END_VU_VERTEX_LOOP (currP, nodeP)
    return count;
    }
/*---------------------------------------------------------------------------------**//**
@description Count the edges (nodes) around a face.
@remarks This is the same as ~mvu_countEdgesAroundFace.
@param nodeP IN any node in the face
@return the number of edges around the face loop
@group "VU Edges"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_faceLoopSize
(
VuP             nodeP
)
    {
    int count = 0;
    VU_FACE_LOOP (currP, nodeP)
        {
        count++;
        }
    END_VU_FACE_LOOP (currP, nodeP)
    return count;
    }

/*---------------------------------------------------------------------------------**//**
@description Count the edges (nodes) in the graph.
@remarks 
@param graph IN graph to inspect.
@group "VU Edges"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_countNodesInGraph (VuSetP graph)
    {
    int count = 0;
    VU_SET_LOOP (currP, graph)
        {
        count++;
        }
    END_VU_SET_LOOP (currP, graph)
    return count;
    }

int vu_countMaskChangesAroundFaces (VuSetP graph, VuMask mask)
    {
    int n = 0;
    VU_SET_LOOP (seed, graph)
        {
        if (vu_getMask (seed, mask) != vu_getMask (vu_fsucc (seed), mask))
            n++;
        }
    END_VU_SET_LOOP (seed, graph)
    return n;
    }

/*---------------------------------------------------------------------------------**//**
@description Clear the user data fields in all nodes of the graph.
@param graphP IN OUT graph header
@group "VU Node Data Fields"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_clearUserDataPInGraph
(
VuSetP          graphP
)
    {
    VU_SET_LOOP (currP, graphP)
        {
        vu_setUserDataP (currP, NULL);
        }
    END_VU_SET_LOOP (currP, graphP)
    }


/*---------------------------------------------------------------------------------**//**
@description Set a mask on all nodes around a face.
@param startP IN any node on the face
@param mask IN mask to apply
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void   vu_markFace
(
VuP             startP,
VuMask          mask
)
    {
    VU_FACE_LOOP (P, startP)
        {
        VU_SETMASK( P, mask );
        }
    END_VU_FACE_LOOP (P, startP)
    }

/*---------------------------------------------------------------------------------**//**
@description Set a mask on all nodes around a vertex.
@param startP IN any node on the vertex
@param mask IN mask to apply
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void   vu_markVertex
(
VuP             startP,         /* Any start node on the vertex whose area is needed */
VuMask          mask            /* Mask to install along the way */
)
    {
    VU_VERTEX_LOOP (P, startP)
        {
        VU_SETMASK( P, mask );
        }
    END_VU_VERTEX_LOOP (P, startP)
    }

/*---------------------------------------------------------------------------------**//**
@description Count the edges (nodes) around a vertex.
@param startP IN any node at the vertex
@return the number of edges around the vertex loop
@group "VU Edges"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   vu_countEdgesAroundVertex
(
VuP             startP
)
    {
    int n = 0;
    VU_VERTEX_LOOP (P, startP)
        {
        n++;
        }
    END_VU_VERTEX_LOOP (P, startP)
    return n;
    }

/*---------------------------------------------------------------------------------**//**
@description Count the edges (nodes) around a face.
@param startP IN any node in the face
@return the number of edges around the face loop
@group "VU Edges"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   vu_countEdgesAroundFace
(
VuP             startP
)
    {
    int n = 0;
    VU_FACE_LOOP (P, startP)
        {
        n++;
        }
    END_VU_FACE_LOOP (P, startP)
    return n;
    }

/*---------------------------------------------------------------------------------**//**
@description Count the edges around a vertex that have a specified mask.
@param startP IN any node at the vertex
@param mask IN mask to test
@return the number of edges that are in the vertex loop containing startP and that have the indicated mask set
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_countMaskAroundVertex
(
VuP             startP,
VuMask          mask
)
    {
    int n = 0;
    VU_VERTEX_LOOP (currP, startP)
        {
        if (VU_GETMASK( currP, mask))
                n++;
        }
    END_VU_VERTEX_LOOP (currP, startP)
    return n;
    }

/*---------------------------------------------------------------------------------**//**
@description Count the edges around a face that have a specified mask.
@param startP IN any node in the face
@param mask IN mask to test
@return the number of edges that are in the face loop containing startP and that have the indicated mask set
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_countMaskAroundFace
(
VuP             startP,
VuMask          mask
)
    {
    int n = 0;
    VU_FACE_LOOP (currP, startP)
        {
        if (VU_GETMASK( currP, mask))
                n++;
        }
    END_VU_FACE_LOOP (currP, startP)
    return n;
    }

/*---------------------------------------------------------------------------------**//**
@description Count the edges around a vertex that have a specified mask on their edge mate.
@param startP IN any node at the vertex
@param mask IN mask to test on edge mates
@return the number of edges that are in the vertex loop containing startP and that have the indicated mask set on their edge mate
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_countMateMaskAroundVertex
(
VuP             startP,
VuMask          mask
)
    {
    int n = 0;
    VU_VERTEX_LOOP (currP, startP)
        {
        if (VU_GETMASK(VU_EDGE_MATE(currP), mask))
                n++;
        }
    END_VU_VERTEX_LOOP (currP, startP)
    return n;
    }

/*---------------------------------------------------------------------------------**//**
@description Count the edges around a face that have a specified mask on their edge mate.
@param startP IN any node in the face
@param mask IN mask to test on edge mates
@return the number of edges that are in the face loop containing startP and that have the indicated mask set on their edge mate
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_countMateMaskAroundFace
(
VuP             startP,
VuMask          mask
)
    {
    int n = 0;
    VU_FACE_LOOP (currP, startP)
        {
        if (VU_GETMASK(VU_EDGE_MATE(currP), mask))
                n++;
        }
    END_VU_FACE_LOOP (currP, startP)
    return n;
    }

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to all nodes.
@param arrayP OUT array of node pointers
@param graphP IN graph header
@group "VU Node Arrays"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectAllNodes
(
VuArrayP        arrayP,
VuSetP          graphP
)
    {
    vu_arrayClear (arrayP);
    VU_SET_LOOP (currP, graphP)
        {
        vu_arrayAdd (arrayP, currP);
        }
    END_VU_SET_LOOP (currP, graphP)
    }

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to all nodes with a mask present
@param arrayP OUT array of node pointers
@param graphP IN graph header
@param mask IN target mask
@group "VU Node Arrays"
@bsimethod                                                    Earlin.Lutz     05/08
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectMaskedNodes
(
VuArrayP        arrayP,
VuSetP          graphP,
VuMask          mask
)
    {
    vu_arrayClear (arrayP);
    VU_SET_LOOP (currP, graphP)
        {
        if (vu_getMask (currP, mask))
            vu_arrayAdd (arrayP, currP);
        }
    END_VU_SET_LOOP (currP, graphP)
    }

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to all nodes with a mask not present
@param arrayP OUT array of node pointers
@param graphP IN graph header
@param mask IN target mask
@group "VU Node Arrays"
@bsimethod                                                    Earlin.Lutz     05/08
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectUnMaskedNodes
(
VuArrayP        arrayP,
VuSetP          graphP,
VuMask          mask
)
    {
    vu_arrayClear (arrayP);
    VU_SET_LOOP (currP, graphP)
        {
        if (!vu_getMask (currP, mask))
            vu_arrayAdd (arrayP, currP);
        }
    END_VU_SET_LOOP (currP, graphP)
    }

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to representative nodes of exterior face loops.
@remarks Only exterior nodes will appear in the array; face loops with all interior nodes are not represented.
@param faceArrayP OUT array of node pointers
@param graphP IN graph header
@group "VU Node Arrays"
@bsimethod                                                    Earlin.Lutz     05/08
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectExteriorFaceLoops
(
VuArrayP        faceArrayP,
VuSetP          graphP
)
    {
    VuMask          mVisited = vu_grabMask (graphP);
    vu_arrayClear (faceArrayP);
    vu_clearMaskInSet (graphP, mVisited);
    VU_SET_LOOP (P, graphP)
        {
        if (!VU_GETMASK (P, mVisited))
            {
            if (VU_GETMASK (P, VU_EXTERIOR_EDGE))
                vu_arrayAdd (faceArrayP, P);
            VU_FACE_LOOP (Q, P)
                {
                VU_SETMASK (Q, mVisited);
                }
            END_VU_FACE_LOOP (Q, P)
            }
        }
    END_VU_SET_LOOP (P, graphP)
    vu_returnMask (graphP, mVisited);
    }

/*------------------------------------------------------------------*//**
@description Advance one step forward in the face loop, then take steps backwards around the vertex loop until a masked edge is found.
@remarks This is a "face step" in a graph in which "real" face edges have their edges masked, and non-masked edges interior to the real
    face are to be ignored.
@param nodeP IN start node for search
@param mask IN mask to find
@group "VU Node Masks"
@return first node found with specified mask
@bsimethod                                      EDL 01/01
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_searchMaskedFSuccVPred
(
VuP             nodeP,
VuMask          mask
)
    {
    VuP startP = VU_FSUCC(nodeP);
    VuP currP;
    if (VU_GETMASK (startP, mask))
        return startP;

    for ( currP = VU_VPRED (startP); currP != startP; currP = VU_VPRED (currP))
        {
        if (VU_GETMASK (currP, mask))
            return currP;
        }
    return NULL;
    }

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
@bsimethod                                      EDL 01/01
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_SearchMaskedFSuccVPred
(
VuP             nodeP,
VuMask          mask
)
    {
    return vu_searchMaskedFSuccVPred (nodeP, mask);
    }



/*------------------------------------------------------------------*//**
@description Search entire graph for any node that has a specified mask.
@param graph IN graph to search
@param mask IN mask to find
@group "VU Node Masks"
@return first node found with specified mask
@bsimethod                                                    BentleySystems  01/01
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_findMaskInSet
(
VuSetP      graph,
VuMask          mask
)
    {
    VU_SET_LOOP (node, graph)
        {
        if (vu_getMask (node, mask))
            return node;
        }
    END_VU_SET_LOOP (node, graph)
    return nullptr;
    }

/*------------------------------------------------------------------*//**
@description Search around a face loop for any node that has a specified mask.
@param nodeP IN first node to test
@param mask IN mask to find
@group "VU Node Masks"
@return first node found with specified mask
@bsimethod                                      EDL 01/01
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_findMaskAroundFace
(
VuP             nodeP,
VuMask          mask
)
    {
    VU_FACE_LOOP (currP, nodeP)
        {
        if (VU_GETMASK (currP, mask))
            return currP;
        }
    END_VU_FACE_LOOP (currP, nodeP)
    return NULL;
    }

/*------------------------------------------------------------------*//**
@description Search around a vertex loop for any node that has a specified mask.
@param nodeP IN first node to test
@param mask IN mask to find
@group "VU Node Masks"
@return first node found with specified mask
@bsimethod                                      EDL 01/01
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_findMaskAroundVertex
(
VuP             nodeP,
VuMask          mask
)
    {
    VU_VERTEX_LOOP (currP, nodeP)
        {
        if (VU_GETMASK (currP, mask))
            return currP;
        }
    END_VU_VERTEX_LOOP (currP, nodeP)
    return NULL;
    }

/*------------------------------------------------------------------*//**
@description Search around a vertex loop for any node that does not have a specified mask.
@param nodeP IN first node to test
@param mask IN mask to find
@group "VU Node Masks"
@return first node found without specified mask
@bsimethod                                      EDL 01/01
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_findUnMaskedAroundVertex
(
VuP             nodeP,
VuMask          mask
)
    {
    VU_VERTEX_LOOP (currP, nodeP)
        {
        if (!VU_GETMASK (currP, mask))
            return currP;
        }
    END_VU_VERTEX_LOOP (currP, nodeP)
    return NULL;
    }

/*------------------------------------------------------------------*//**
@description Search around a face loop for any node that does not have a specified mask.
@param nodeP IN first node to test
@param mask IN mask to find
@group "VU Node Masks"
@return first node found without specified mask
@bsimethod                                      EDL 01/01
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP     vu_findUnMaskedAroundFace
(
VuP             nodeP,
VuMask          mask
)
    {
    VU_FACE_LOOP (currP, nodeP)
        {
        if (!VU_GETMASK (currP, mask))
            return currP;
        }
    END_VU_FACE_LOOP (currP, nodeP)
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to representative nodes of interior face loops.
@remarks Only interior nodes will appear in the array; face loops with all exterior nodes are not represented.
@param faceArrayP OUT array of node pointers
@param graphP IN graph header
@group "VU Node Arrays"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectInteriorFaceLoops
(
VuArrayP        faceArrayP,
VuSetP          graphP
)
    {
    VuMask          mVisited = vu_grabMask (graphP);
    vu_arrayClear (faceArrayP);
    vu_clearMaskInSet (graphP, mVisited);
    VU_SET_LOOP (P, graphP)
        {
        if (!VU_GETMASK (P, mVisited))
            {
            if (!VU_GETMASK (P, VU_EXTERIOR_EDGE))
                vu_arrayAdd (faceArrayP, P);
            VU_FACE_LOOP (Q, P)
                {
                VU_SETMASK (Q, mVisited);
                }
            END_VU_FACE_LOOP (Q, P)
            }
        }
    END_VU_SET_LOOP (P, graphP)
    vu_returnMask (graphP, mVisited);
    }

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
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectMaskedFaceLoops
(
VuArrayP        faceArrayUnMaskedP,
VuArrayP        faceArrayMixedP,
VuArrayP        faceArrayMaskedP,
VuSetP          graphP,
VuMask          mask
)
    {
    VuMask          mVisited = vu_grabMask (graphP);
    VuP node0P, node1P;

    if (NULL != faceArrayUnMaskedP)
        vu_arrayClear (faceArrayUnMaskedP);
    if (NULL != faceArrayMixedP)
        vu_arrayClear (faceArrayMixedP);
    if (NULL != faceArrayMaskedP)
        vu_arrayClear (faceArrayMaskedP);

    vu_clearMaskInSet (graphP, mVisited);
    VU_SET_LOOP (P, graphP)
        {
        if (!VU_GETMASK (P, mVisited))
            {
            node0P = NULL;
            node1P = NULL;

            VU_FACE_LOOP (Q, P)
                {
                VU_SETMASK (Q, mVisited);
                if (VU_GETMASK (Q, mask))
                    node1P = Q;
                else
                    node0P = Q;
                }
            END_VU_FACE_LOOP (Q, P)
            // There is at least one node on the face.
            // Therefore once either pointer is verified NULL the other must be non-null.
            if (NULL == node0P)
                vu_arrayAdd (faceArrayMaskedP, node1P);
            else if (NULL == node1P)
                vu_arrayAdd (faceArrayUnMaskedP, node0P);
            else
                vu_arrayAdd (faceArrayMixedP, node1P);
            }
        }
    END_VU_SET_LOOP (P, graphP)
    vu_returnMask (graphP, mVisited);
    }

/*---------------------------------------------------------------------------------**//**
@description Find a specific node around a vertex.
@param nodeP IN start node for search around vertex loop
@param targetP IN target node
@return NULL if not found, targetP if found
@group "VU Node Arrays"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_findNodeAroundVertex
(
VuP             nodeP,
VuP             targetP
)
    {
    VU_VERTEX_LOOP (currP, nodeP)
        {
        if (currP == targetP)
            return currP;
        }
    END_VU_VERTEX_LOOP (currP, nodeP)
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
@description Find a specific node around a face.
@param nodeP IN start node for search around face loop
@param targetP IN target node
@return NULL if not found, targetP if found
@group "VU Node Arrays"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_findNodeAroundFace
(
VuP             nodeP,
VuP             targetP
)
    {
    VU_FACE_LOOP (currP, nodeP)
        {
        if (currP == targetP)
            return currP;
        }
    END_VU_FACE_LOOP (currP, nodeP)
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
@description Collect pointers to representative nodes of all vertex loops with interior nodes.
@remarks Only interior nodes will appear in the array; vertex loops with all exterior nodes are not represented.
@param vertexArrayP OUT array of node pointers
@param graphP IN graph header
@group "VU Node Arrays"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_collectVertexLoops
(
VuArrayP        vertexArrayP,
VuSetP          graphP          /* => parent graph */
)
    {
    VuMask          mVisited = vu_grabMask (graphP);
    VuMask          mDontStartHere = mVisited | VU_EXTERIOR_EDGE;
    vu_arrayClear (vertexArrayP);
    vu_clearMaskInSet (graphP, mVisited);
    VU_SET_LOOP (startP, graphP)
        {
        if (!VU_GETMASK (startP, mDontStartHere))
            {
            vu_arrayAdd (vertexArrayP, startP);
            VU_VERTEX_LOOP (currP, startP)
                {
                VU_SETMASK (currP, mVisited);
                }
            END_VU_VERTEX_LOOP (currP, startP)
            }
        }
    END_VU_SET_LOOP (startP, graphP)
    vu_returnMask (graphP, mVisited);
    }

/*---------------------------------------------------------------------------------**//**
@description Allocate a header for a marked edge set.
@param graphP IN OUT graph header
@param mExcluded IN mask for edges that may not be entered into the set
@return pointer to the new, empty edge set
@group "VU Marked Edge Sets"
@see vu_markedEdgeSetFree
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMarkedEdgeSetP vu_markedEdgeSetNew
(
VuSetP graphP,
VuMask mExcluded
)
    {
    VuMarkedEdgeSetP edgeSetP = (VuMarkedEdgeSetP)BSIBaseGeom::Malloc (sizeof(VuMarkedEdgeSet));
    vu_markedEdgeSetInit (edgeSetP, graphP, mExcluded);
    return edgeSetP;
    }

/*---------------------------------------------------------------------------------**//**
@description Free the header and associated memory of a marked edge set.
@param edgeSetP IN edge set
@group "VU Marked Edge Sets"
@see vu_markedEdgeSetNew
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markedEdgeSetFree
(
VuMarkedEdgeSetP edgeSetP
)
    {
    vu_markedEdgeSetDeinit (edgeSetP);
    BSIBaseGeom::Free(edgeSetP);
    }

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
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markedEdgeSetTestAndAdd
(
VuMarkedEdgeSetP edgeSetP,
VuP             P
)
    {
    VuP             Q = VU_EDGE_MATE (P);
    if (!VU_GETMASK (P, edgeSetP->mExcludedOrMember)
        && !VU_GETMASK (Q, edgeSetP->mExcludedOrMember))
        {
        VU_SETMASK (P, edgeSetP->mMember);
        VU_SETMASK (Q, edgeSetP->mMember);
        vu_arrayAdd (edgeSetP->arrayP, P);
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Remove an edge from a marked edge set.
@param edgeSetP IN OUT edge set
@return pointer to one of the nodes of the chosen edge
@group "VU Marked Edge Sets"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP      vu_markedEdgeSetChooseAny
(
VuMarkedEdgeSetP edgeSetP
)
{
    VuP             P = vu_arrayRemoveLast (edgeSetP->arrayP);
    if (P)
        {
        VU_CLRMASK (P, edgeSetP->mMember);
        VU_CLRMASK (VU_EDGE_MATE (P), edgeSetP->mMember);
        }
    return P;
    }

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
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP vu_edgeMateInNonNullFace
(
VuP startP,
VuMask extraMask
)
    {
    VuP baseP = VU_EDGE_MATE(startP);
    VuP testP;
    int passesAllowed= 5;
    VuMask nullFaceMask = VU_NULL_FACE | extraMask;
    VuMask barredExitMask = VU_NULL_EDGE | VU_BOUNDARY_EDGE;
    while ( VU_GETMASK( baseP , nullFaceMask ) )
        {
        /* Search for a non-null edge other than where entered */
        for(testP = VU_FSUCC(baseP) ;
                testP != baseP && VU_GETMASK(testP,barredExitMask) ;
                testP = VU_FSUCC(testP)
                )
            {}
        if( testP != baseP )
            {
            baseP = VU_EDGE_MATE(testP);
            }
        if(passesAllowed-- <= 0 ) return NULL;
        /* For the curious: If you replace baseP by startP everywhere
        inside this loop, the returned edges are a (small) circle one
        latitude line away from the pole.  Interesting bug.
        */
        }
    return baseP;
    }

/*---------------------------------------------------------------------------------**//**
@description Set a marker bit on all edges of all faces that have a trigger bit on any edge.
    That is, at each edge that has a trigger bit set, set the marker bit all around the face.

@remarks The implementation implicitly assumes that the marker bit is <EM>not set</EM> anywhere.
    Pre-set marker bits will cause unpredictable omissions from the marking.
@param graphP IN OUT graph header
@param triggerMask IN mask to trigger full marking of faces
@param markerMask IN mask to apply
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_markMarkedFaces
(
VuSetP graphP,
VuMask  triggerMask,
VuMask  markerMask
)
    {
    VU_SET_LOOP(currP,graphP)
        {
        if(     VU_GETMASK(currP,triggerMask)
            && !VU_GETMASK(currP,markerMask) )
            {
            VU_FACE_LOOP(faceP,currP)
                {
                VU_SETMASK(faceP,markerMask);
                }
            END_VU_FACE_LOOP(faceP,currP)
            }
        }
    END_VU_SET_LOOP(currP,graphP)
    }

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
@bsimethod                                                      Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_eulerCharacteristic
(
VuSetP graph
)
    {
    int numVertices, numEdges, numFaces;
    vu_countLoops (graph, numVertices, numEdges, numFaces);
    return numVertices - numEdges + numFaces;
    }

Public GEOMDLLIMPEXP void vu_countLoops
(
VuSetP graphP,
int &numVertices,
int &numEdges,
int &numFaces
)
    {
    VuMask faceMask = vu_grabMask (graphP);
    VuMask vertexMask = vu_grabMask (graphP);
    int numNodes;

    vu_clearMaskInSet (graphP, faceMask | vertexMask);

    numFaces = numVertices = numNodes = 0;
    VU_SET_LOOP (startP, graphP)
        {
        numNodes++;
        if (!VU_GETMASK (startP, faceMask))
            {
            numFaces++;
            VU_FACE_LOOP (faceP, startP)
                {
                VU_SETMASK (faceP, faceMask);
                }
            END_VU_FACE_LOOP (faceP, startP)
            }

        if (!VU_GETMASK (startP, vertexMask))
            {
            numVertices++;
            VU_VERTEX_LOOP (vertP, startP)
                {
                VU_SETMASK (vertP, vertexMask);
                }
            END_VU_VERTEX_LOOP (vertP, startP)
            }

        }
    END_VU_SET_LOOP (startP, graphP)

    vu_returnMask (graphP, faceMask);
    vu_returnMask (graphP, vertexMask);
    numEdges = numNodes / 2;    /* Should be even */

    //return  (numFaces - numEdges + numVertices);
    }
#ifdef Build_vu_EulerCharacteristic_WrongCase
/*---------------------------------------------------------------------------------**//**
* @nodoc
* @deprecated vu_eulerCharacteristic
* @bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_EulerCharacteristic
(
VuSetP graphP
)
    {
    return vu_eulerCharacteristic (graphP);
    }
#endif
/*---------------------------------------------------------------------------------**//**
@description  Find true exterior (negative area) faces.
    Visit adjacent faces recursively to assign parity and exterior masks.
@param graphP IN OUT graph header
@param trueExteriorMask IN maks to apply to true exterior.
@param parityMask IN mask to apply to faces that are exterior by parity rules.
            trueExteriorMask and parityMask may be identical.
            parityMask can be 0 (i.e. only mark true exterior)
@param boundaryMask IN mask that identifies true boundaries
@return true if all adjacencies matched parity.  false if different paths to same place
    produced different parity.
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    vu_markParity
(
VuSetP graphP,
VuMask trueExteriorMask,
VuMask parityMask,
VuMask boundaryMask
)
    {
    bool    bParityOK = true;
    VuArrayP pStack = vu_grabArray (graphP);
    VuArrayP pComponentSeeds = vu_grabArray (graphP);
    VuMask parityVisitMask = vu_grabMask (graphP);
    vu_clearMaskInSet (graphP, trueExteriorMask | parityVisitMask );
    vu_collectConnectedComponents (graphP, pComponentSeeds, NULL);

    if (trueExteriorMask != 0 && trueExteriorMask != parityMask)
        {
        // Simple inward recursion through non-boundaries ...
        VuP pComponentSeed;
        for (   vu_arrayOpen (pComponentSeeds);
                vu_arrayRead (pComponentSeeds, &pComponentSeed);
                )
            {
            VuP pSeed = NULL;
            vu_arrayClear (pStack);
            vu_arrayAdd (pStack, pComponentSeed);
            while (NULL != (pSeed = vu_arrayRemoveLast (pStack)))
                {
                vu_setMaskAroundFace (pSeed, trueExteriorMask);
                VU_FACE_LOOP (pCurr, pSeed)
                    {
                    VuP pMate = vu_edgeMate (pCurr);
                    if (   !vu_getMask (pCurr, boundaryMask)
                        && !vu_getMask (pMate, boundaryMask)
                        && !vu_getMask (pMate, trueExteriorMask))
                        vu_arrayAdd (pStack, pMate);
                    }
                END_VU_FACE_LOOP (pCurr, pSeed)
                }
            }
        }

    if (parityMask)
        {
        // Complete recursion ...
        VuP pComponentSeed;
        for (vu_arrayOpen (pComponentSeeds);vu_arrayRead (pComponentSeeds, &pComponentSeed);)
            {
            VuP pSeed = NULL;
            vu_arrayClear (pStack);
            vu_arrayAdd (pStack, pComponentSeed);
            vu_setMaskAroundFace (pComponentSeed, parityMask);
            while (NULL != (pSeed = vu_arrayRemoveLast (pStack)))
                {
                // Seed is already consistently marked ....
                VuMask seedParity = vu_getMask (pSeed, parityMask);
                // Look for neighbors separated by boundaries ...
                VU_FACE_LOOP (pCurr, pSeed)
                    {
                    VuP pMate = vu_edgeMate (pCurr);
                    VuMask mateParity = vu_getMask (pMate, parityMask);
                    VuMask mateVisited = vu_getMask (pMate, parityVisitMask);
                    bool    bIsBoundary =  vu_getMask (pCurr, boundaryMask)
                                        || vu_getMask (pMate, boundaryMask);
                    if (!mateVisited)
                        {
                        // The mate edge is a new face.
                        // Always mark it visited. Only mark the parity if needed.
                        vu_arrayAdd (pStack, pMate);
                        vu_setMaskAroundFace (pMate, parityVisitMask);
                        if (bIsBoundary)
                            {
                            if (!seedParity)
                                vu_setMaskAroundFace (pMate, parityMask);
                            }
                        else
                            {
                            if (seedParity)
                                vu_setMaskAroundFace (pMate, parityMask);
                            }
                        }
                    else
                        {
                        bool    bSameParity = mateParity == seedParity;
                        if (bIsBoundary)
                            {
                            if (bSameParity)
                                bParityOK = false;
                            }
                        else
                            {
                            if (!bSameParity)
                                bParityOK = false;
                            }
                        }
                    }
                END_VU_FACE_LOOP (pCurr, pSeed)
                }
            }
        }
    vu_returnMask (graphP, parityVisitMask);
    vu_returnArray (graphP, pComponentSeeds);
    vu_returnArray (graphP, pStack);
    return bParityOK;
    }


/*---------------------------------------------------------------------------------**//**
@description  Flood search through a single connected component, assuming pre-cleared visit mask.
@param graphP IN graph to search
@param arrayP OUT array to receive (without clearing) one node pointer per face.  May be null.
@param stackP IN OUT scratch array to be used as search stack.  This may NOT be null.
@param seedP IN start node.
@param visitMask IN mask identifying visited nodes.
@returns face with most negative area.
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
static VuP vu_collectFacesInComponent
(
VuSetP graphP,
VuArrayP arrayP,
VuArrayP stackP,
VuP    seedP,
VuMask visitMask
)
    {
    VuP faceP;
    VuP minAreaFaceP = NULL;
    double minArea = DBL_MAX;
    double currArea = 0.0;
    vu_arrayClear (stackP);
    vu_arrayAdd (stackP, seedP);
    while (NULL != (faceP = vu_arrayRemoveLast (stackP)))
        {
        if (!vu_getMask (faceP, visitMask))
            {
            if (NULL != arrayP)
                vu_arrayAdd (arrayP, faceP);
            currArea = vu_area (faceP);
            if (currArea < minArea)
                {
                minArea = currArea;
                minAreaFaceP = faceP;
                }
            vu_setMaskAroundFace (faceP, visitMask);
            VU_FACE_LOOP (currP, faceP)
                {
                VuP mateP = vu_edgeMate (currP);
                if (!vu_getMask (mateP, visitMask))
                    {
                    vu_arrayAdd (stackP, mateP);
                    }
                }
            END_VU_FACE_LOOP (currP, faceP)
            }
        }
    return minAreaFaceP;
    }

/*---------------------------------------------------------------------------------**//**
@description  Search for connected components.  In each component, find the face
   with most negative area.   (For a merged graph, these are the outer faces of
   the connected components.)
@param graphP IN graph header
@param seedArrayP OUT array of representative (most negative area) faces.
@param bulkArrayP OUT optional array containing all one node from each face of the entire
        graph, with each component terminated by a NULL pointer.
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     05/08
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_collectConnectedComponents
(
VuSetP graphP,
VuArrayP seedArrayP,
VuArrayP bulkArrayP
)
    {
    VuP outerFaceP;
    VuArrayP stackP = vu_grabArray (graphP);
    VuMask visitMask = vu_grabMask (graphP);
    if (seedArrayP)
        vu_arrayClear (seedArrayP);
    if (bulkArrayP)
        vu_arrayClear (bulkArrayP);
    vu_clearMaskInSet (graphP, visitMask);
    VU_SET_LOOP (seedP, graphP)
        {
        if (!vu_getMask (seedP, visitMask))
            {
            outerFaceP = vu_collectFacesInComponent (graphP, bulkArrayP, stackP, seedP, visitMask);
            // (um .. outerFaceP would only be null if seedP has the mask.  so it's not.)
            if (seedArrayP != NULL)
                vu_arrayAdd (seedArrayP, outerFaceP);
            if (bulkArrayP != NULL)
                vu_arrayAddNull (bulkArrayP);
            }
        }
    END_VU_SET_LOOP (seedP, graphP)

    vu_returnArray (graphP, stackP);
    vu_returnMask (graphP, visitMask);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
