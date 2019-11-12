/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#include <assert.h>
#define VU_NOCHECK 0
#define VU_CHECKNEW 1000
/* Set this to 1000 to trigger complete validity checks
   at every join and split.  This will be pretty slow, eh!! */
static int      vu_checkMode = VU_NOCHECK;


/*-------------------------------------------------------------------*//**
@nodoc "VU Internals"
@description Swap (twist) the "next" pointers of two vu nodes.
@remarks This is extremely dangerous and is to be done only by vu internal code.
@param P IN first node for swap.
@param Q IN second node for swap.
@bsimethod                                                  EarlinLutz      10/04
+----------------------------------------------------------------------*/
Public void     vu_ntwist
(
VuP     P,
VuP     Q
)
    {
    VuP             R = VU_NEXT (P);
    VU_NEXT (P) = VU_NEXT (Q);
    VU_NEXT (Q) = R;
    }

#define COMPILE_PANIC
#ifdef COMPILE_PANIC
/*======================================================================+

   Emergency Error Detection Code Section

+======================================================================*/
/*---------------------------------------------------------------------------------**//**
@description A useful place to set a breakpoint.  Does nothing.
@param node0P IN first node at point of error detection.
@param node1P IN first node at point of error detection.
@group "VU Debugging"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_panicPair
(
VuP             node0P,
VuP             node1P
)
    {
    }

/*-------------------------------------------------------------------*//**
@description Check the fundamental invariant of a single cylic loop:
   each vu has exactly one predecessor in the V and F loops.
@remarks Call ~mvu_panicPair for each mismatch.
@param graphP IN graph header
@group "VU Debugging"
@bsimethod                                                    Earlin.Lutz     10/04
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void            vu_checkPredecessors
(
VuSetP graphP
)
    {
    VuMask          mVert = vu_grabMask (graphP);
    VuMask          mFace = vu_grabMask (graphP);
    /* A zero for mVert or mFace indicates no predecessor yet found */
    vu_clearMaskInSet (graphP, mVert | mFace);
    /* Set the mVert and mFace bits from the predecessors */
    VU_SET_LOOP (currP, graphP)
        {
        VuP             vertP = VU_VSUCC (currP);
        VuP             faceP = VU_FSUCC (currP);
        if (VU_GETMASK (vertP, mVert))
            vu_panicPair (currP, vertP);
        if (VU_GETMASK (faceP, mFace))
            vu_panicPair (currP, faceP);
        VU_SETMASK (vertP, mVert);
        VU_SETMASK (faceP, mFace);
        }
    END_VU_SET_LOOP (currP, graphP)
    /* Check that all mVert and mFace bits are set */
        VU_SET_LOOP (currP, graphP)
        {
        if (!VU_GETMASK (currP, mVert))
            vu_panicPair (currP, currP);
        if (!VU_GETMASK (currP, mFace))
            vu_panicPair (currP, currP);
        }
    END_VU_SET_LOOP (currP, graphP);

    vu_returnMask (graphP, mFace);
    vu_returnMask (graphP, mVert);
    }

static int      test_Counter = -1;
static int      test_stopId = -1;

/*---------------------------------------------------------------------------------**//**
@description Check that the VUs in a set are coupled properly via their 'next' pointers.
@remarks Call ~mvu_panicPair for each mismatch.
@param graphP IN graph header.
@group "VU Debugging"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_checkNextInSet
(
VuSetP graphP
)
    {
    int             n = graphP->nvu;
    /* We expect that global node list to be numbered
       backwards .. */
    VuP             P = graphP->lastNodeP;
    int             error = 0;
    if (++test_Counter == test_stopId)
        {
        vu_panicPair (0, 0);
        }
    if (!P)
        {
        }
    else if (graphP->lastNodeP->id != 0
             || VU_NEXT (graphP->lastNodeP)->id != n - 1)
        {
        error = 1;
        }
    else
        {
        P = VU_NEXT (P);
        n--;
        while (error == 0 && n)
            {
            if (P && VU_ID (P) == n)
                {
                n--;
                P = VU_NEXT (P);
                }
            else
                {
                error = 2;
                }
            }
        }
    if (error)
        vu_panicPair (0, 0);
    if (!error)
        vu_checkPredecessors (graphP);
    }


#define FEDGE(P) P->fs->vs->fs->vs
#define VEDGE(P) P->vs->fs->vs->fs
/*---------------------------------------------------------------------------------**//**
@description Function called by various vu internals when severe error conditions are detected.
@param graphP IN graph header.
@group "VU Debugging"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_panic
(
VuSetP graphP
)
    {
    int             opcode = 0;
    VU_THROW_MESSAGE(VU_MESSAGE_ANNOUNCE_PANIC, graphP, NULL, NULL, NULL, SUCCESS );
    while (opcode)
        {
        switch (opcode--)
            {
            case 10:
                vu_checkNextInSet (graphP);
                break;
            case 9:
                break;
            case 8:
                break;
            case 7:
                break;
            }
        }
    }
#endif



/*---------------------------------------------------------------------------------
@description Fill in data fields in a function definition packet.
@return Returns packetP (so you can call the fill function right at the point where you pass the packet along as an argument)
@param packetP IN data packet to fill.
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
static VuMessagePacketP vu_fillFunctionPacket
(
VuMessagePacketP        packetP,                /* <= pacekt to be filled */
VuMessageFunction       functionP,              /* => primary function */
void                    *userDataP,             /* => primary data */
VuMessageFunction       auxFunctionP,           /* => secondary function */
void                    *auxDataP               /* => secondary data */
)
    {
    packetP->functionP          = functionP;
    packetP->userDataP          = userDataP;
    packetP->auxFunctionP       = auxFunctionP;
    packetP->auxDataP           = auxDataP;
    return packetP;
    }

/*======================================================================+

   Major Public Code Section

+======================================================================*/
/*---------------------------------------------------------------------------------**//**
@description Set the application function callback to be called for important transitions in a VU graph.
@param graphP IN OUT graph header
@param functionP IN message function
@param userDataP IN pointer to pass back on function calls
@group "VU Debugging"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setCMessageFunction
(
VuSetP          graphP,
VuMessageFunction functionP,
void            *userDataP
)
    {
    vu_fillFunctionPacket (&graphP->mPrimitiveData.messagePacket, functionP, userDataP, NULL, NULL);
    }



/*---------------------------------------------------------------------------------*//**
@description Swap vertex loop successors and face loop predecessors of two nodes.
@param graphP IN OUT graph header
@param node0P IN first node of swap
@param node1P IN second node of swap
@group "VU Nodes"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_vertexTwist
(
VuSetP          graphP,
VuP             node0P,
VuP             node1P
)
    {
    VuP             vertSucc0P = VU_VSUCC (node0P);     /* vertex successor of node0P */
    VuP             faceSucc0P = VU_EDGE_MATE (vertSucc0P);     /* face predecessor of node0P */
    VuP             vertSucc1P = VU_VSUCC (node1P);     /* vertex successor of node1P */
    VuP             faceSucc1P = VU_EDGE_MATE (vertSucc1P);     /* face predecessor of node1P */
#ifdef VERIFY_PROPERTIES
    if (FEDGE (node0P) != node0P
        || VEDGE (node0P) != node0P
        || FEDGE (node1P) != node1P
        || VEDGE (node1P) != node1P
        )
        vu_panicPair (node0P, node1P);
#endif
    VU_THROW_MESSAGE (VU_MESSAGE_ANNOUNCE_PRE_VTWIST, graphP, node0P, node1P, NULL, SUCCESS);
    /* Swap SUCCESSORS in vertex loops: */
    VU_VSUCC (node0P) = vertSucc1P;
    VU_VSUCC (node1P) = vertSucc0P;
    /* Swap PREDECESSORS in face loops. */
    VU_FSUCC (faceSucc0P) = node1P;
    VU_FSUCC (faceSucc1P) = node0P;
    VU_THROW_MESSAGE (VU_MESSAGE_ANNOUNCE_POST_VTWIST, graphP, node0P, node1P, NULL, SUCCESS);
    }

void _VuSet::VertexTwist (VuP node0P, VuP node1P)
    {
    vu_vertexTwist (this, node0P, node1P);
    }


/*---------------------------------------------------------------------------------**//**
@description Test if a graph is empty, e.g., if it has no nodes.
@param graphP IN graph to test.
@group "VU Graph Header"
@return true if graph is empty.
@see vu_reinitializeVuSet
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool           vu_emptyGraph
(
VuSetP          graphP
)
    {
    return (graphP->lastNodeP ? 0 : 1);
    }



/*---------------------------------------------------------------------------------**//**
@description Count the number of times (0, 1 or 2) a mask appears on the nodes of the edge, and conditionally delete the edge by count test.
@param graphP IN OUT graph to modify
@param mask IN mask to count
@param free0 IN true to free edges with mask not appearing at all.
@param free1 IN true to free edges with mask appearing once
@param free2 IN true to free edges with mask appearing twice
@group "VU Edges"
@see vu_freeMarkedEdges
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_freeEdgesByMaskCount
(
VuSetP          graphP,
VuMask          mask,
bool            free0,
bool            free1,
bool            free2
)
    {
    VuMask      mDelete = vu_grabMask  (graphP);
    VuMask      mVisited = vu_grabMask (graphP);
    bool    freeFlag[3];
    VuP mateP;
    int numMask;

    freeFlag[0] = free0;
    freeFlag[1] = free1;
    freeFlag[2] = free2;

    vu_clearMaskInSet (graphP, mVisited | mDelete);

    VU_SET_LOOP (currP, graphP)
        {
        if (!vu_getMask (currP, mVisited))
            {
            mateP = vu_edgeMate (currP);
            vu_setMask (currP, mVisited);
            vu_setMask (mateP, mVisited);
            numMask = 0;
            if (vu_getMask (currP, mask))
                numMask++;
            if (vu_getMask (mateP, mask))
                numMask++;
            if (freeFlag[numMask])
                {
                vu_setMask (currP, mDelete);
                vu_setMask (mateP, mDelete);
                }
            }
        }
    END_VU_SET_LOOP (currP, graphP)

    vu_freeMarkedEdges (graphP, mDelete);
    vu_returnMask (graphP, mDelete);
    vu_returnMask (graphP, mVisited);
    }

/*---------------------------------------------------------------------------------**//**
@description Heal, or merge, two edges into one.
@param graphP IN OUT graph to modify.
@param nodeP IN one of the nodes at the degree-2 vertex being excised between the two edges to be healed.
*       On return, this node and its vertex successor are a sling.
@return false if vertex condition is not satisfied.
@group "VU Edges"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    vu_healEdge
(
VuSetP graphP,
VuP    nodeP
)
    {
    VuP del0P = nodeP;
    VuP del1P = VU_VSUCC(del0P);

    VuP base0P = VU_FPRED (del0P);
    VuP base1P = VU_FPRED (del1P);

    VuP end0P  = VU_FSUCC (del0P);
    VuP end1P  = VU_FSUCC( del1P);

    bool    boolstat = false;
/*
    |base0     del0        end0|
    +------------+-------------+
    |end1      del1       base1|
*/
    if (del0P != del1P && vu_vsucc (del1P) == del0P)
        {
        /* All vertex successors are unchanged. */
        /* Face successors at base nodes skip over deleted nodes.. */
        VU_FSUCC(base0P) = end0P;
        VU_FSUCC(base1P) = end1P;
        /* As a formality, reconnect del0P and del1P as a sling */
        VU_FSUCC(del0P) = del0P;
        VU_FSUCC(del1P) = del1P;

        boolstat = true;
        }

    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
@description Heal, or merge, two side-by-side edges into one.
@param graphP IN OUT graph to modify.
@param nodeP IN one of the nodes at the degree-2 face being excised.
*       On return, this node and its vertex successor are a simple pair that has been pulled out.
*       The two "outside" vu edges are now mates !!!
*       The caller is responsible for deleting the floating edge.
@return false if the face has other than 2 edges.
@group "VU Edges"
@bsimethod                                                    Earlin.Lutz     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    vu_healNullFace
(
VuSetP graph,
VuP    nodeA
)
    {
    VuP nodeB = vu_fsucc(nodeA);
    if (nodeB == nodeA
        || vu_fsucc(nodeB) != nodeA)
        return false;
//      |                                      |
//      | nodeA1                         nodeB0|
//        -------------------------------------
//      * nodeA                          nodeB *
//       --------------------------------------
//      | nodeA0                         nodeB1|
//      |                                      |
    VuP nodeA1 = vu_vsucc (nodeA);
    VuP nodeB1 = vu_vsucc(nodeB);
    VuP nodeA0 = vu_fsucc (nodeB1);
    VuP nodeB0 = vu_fsucc (nodeA1);

    /* All face successors are unchanged. */
    /* vertex successors mutate */
    VU_VSUCC(nodeA) = nodeA;
    VU_VSUCC (nodeB) = nodeB;
    VU_VSUCC(nodeB0) = nodeB1;
    VU_VSUCC(nodeA0) = nodeA1;

    return true;
    }

Public GEOMDLLIMPEXP int    vu_exciseNullFaces
(
VuSetP graph,
VuMask mask
)
    {
    VuMask deleteMask = vu_grabMask (graph);
    VuMask visitMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, deleteMask | visitMask);
    int numDelete = 0;
    VU_SET_LOOP (seed, graph)
        {
        if (!vu_getMask (seed, visitMask))
            {
            int numEdge = 0;
            VU_FACE_LOOP (edge, seed)
                {
                vu_setMask (edge, visitMask);
                numEdge++;
                }
            END_VU_FACE_LOOP (edge, seed)
            if (numEdge == 2)
                {
                VuP other = vu_fsucc (seed);
                if (mask == 0
                    || (vu_getMask (seed, mask) && vu_getMask (other, mask))
                    )
                    {
                    vu_healNullFace (graph, seed);
                    vu_setMask (seed, deleteMask);
                    vu_setMask (other, deleteMask);
                    BeAssert (vu_vsucc (seed) == seed);
                    BeAssert (vu_vsucc (other) == other);
                    numDelete++;
                    }
                }
            }
        }
    END_VU_SET_LOOP (seed, graph)
    if (numDelete > 0)
        vu_freeMarkedEdges (graph, deleteMask);
    vu_returnMask (graph, visitMask);
    vu_returnMask (graph, deleteMask);
    return numDelete;
    }

/*---------------------------------------------------------------------------------**//**
@description Allocate two vu nodes that define an edge with two distinct vertices.
@param graphP IN OUT graph header
@param outnode1PP OUT newly created node
@param outnode2PP OUT newly created node
@group "VU Edges"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_makePair
(
VuSetP          graphP,
VuP           *outnode1PP,
VuP           *outnode2PP
)
    {
    *outnode1PP = vu_newVuP (graphP);
    *outnode2PP = vu_newVuP (graphP);
    VU_VSUCC (*outnode2PP) = *outnode2PP;
    VU_VSUCC (*outnode1PP) = *outnode1PP;
    VU_FSUCC (*outnode2PP) = *outnode1PP;
    VU_FSUCC (*outnode1PP) = *outnode2PP;
    VU_THROW_MESSAGE (VU_MESSAGE_ANNOUNCE_MAKE_PAIR, graphP, *outnode1PP, *outnode2PP, NULL, SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
@description Make two vu nodes that define a sling edge with one vertex.
@param graphP IN OUT graph header
@param outnode1PP OUT newly created node
@param outnode2PP OUT newly created node
@group "VU Edges"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_makeSling
(
VuSetP          graphP,
 VuP           *outnode1PP,
 VuP           *outnode2PP
)
    {
    *outnode1PP = vu_newVuP (graphP);
    *outnode2PP = vu_newVuP (graphP);
    VU_FSUCC (*outnode2PP) = *outnode2PP;
    VU_FSUCC (*outnode1PP) = *outnode1PP;
    VU_VSUCC (*outnode2PP) = *outnode1PP;
    VU_VSUCC (*outnode1PP) = *outnode2PP;
    VU_THROW_MESSAGE (VU_MESSAGE_ANNOUNCE_MAKE_SLING, graphP, *outnode1PP, *outnode2PP, NULL, SUCCESS);
    }
/*---------------------------------------------------------------------------------**//**
@description Create a new vertex, typically within an existing edge but possibly as an isolated sling if no edges is given.
@remarks If P is not NULL, insert a new vertex within its existing edge.   The insertion is done as a primitive operation that preserves
   the vertex and face incidences of all preexisting vertex uses.  If P is NULL, make a loop and set the marker bits so outnode1PP is
   considered the inside of a boundary and outnode2PP the outside.
@remarks This is useful in building up loops of many nodes when given coordinates in an array.  Initialize P=0, and then repeatedly
   add points to the loop, with no need to add the first point any differently from any other.
@param graphP       IN OUT  graph header
@param P            IN OUT  base node of edge to split
@param outnode1PP   IN      new node on same side as P
@param outnode2PP   IN      new node on opposite side
@group "VU Edges"
@see vu_splitEdgeAtPoint, vu_splitEdgeAtDPoint3d
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_splitEdge
(
VuSetP          graphP,
VuP             P,
VuP            *outnode1PP,
VuP            *outnode2PP,
VuMask          mask1,
VuMask          mask2
)
    {
    if (vu_checkMode >= VU_CHECKNEW)
        vu_checkNextInSet (graphP);

    if (P)
        {
        VuP             Q = VU_EDGE_MATE (P);
        VU_THROW_MESSAGE (VU_MESSAGE_ANNOUNCE_PRE_SPLIT_EDGE, graphP, P, Q, NULL, SUCCESS);
        *outnode1PP = vu_newVuP (graphP);
        *outnode2PP = vu_newVuP (graphP);
        VU_FSUCC (*outnode1PP) = VU_FSUCC (P);
        VU_FSUCC (P) = *outnode1PP;
        VU_FSUCC (*outnode2PP) = VU_FSUCC (Q);
        VU_FSUCC (Q) = *outnode2PP;
        VU_VSUCC (*outnode1PP) = *outnode2PP;
        VU_VSUCC (*outnode2PP) = *outnode1PP;
        VU_SETMASK (*outnode1PP, VU_GETMASK (P, graphP->mPrimitiveData.mCopyOnSplit));
        VU_SETMASK (*outnode2PP, VU_GETMASK (Q, graphP->mPrimitiveData.mCopyOnSplit));
        if (graphP->mPrimitiveData.bUserDataIsEdgeProperty)
            {
            VU_USER_ID (*outnode1PP) = VU_USER_ID (P);
            VU_USER_ID (*outnode2PP) = VU_USER_ID (Q);
            }
        if (graphP->mPrimitiveData.bUserData1IsEdgeProperty)
            {
            vu_setUserData1 (*outnode1PP, vu_getUserData1 (P));
            vu_setUserData1 (*outnode2PP, vu_getUserData1 (Q));
            }
        VU_THROW_MESSAGE (VU_MESSAGE_ANNOUNCE_POST_SPLIT_EDGE, graphP, *outnode1PP, *outnode2PP, NULL, SUCCESS);
        }
    else
        {
        vu_makeSling (graphP, outnode1PP, outnode2PP);
        VU_SETMASK (*outnode1PP, mask1);
        VU_SETMASK (*outnode2PP, mask2);
        }
    }

Public GEOMDLLIMPEXP void            vu_splitEdge
(
VuSetP          graphP,
VuP             P,
VuP            *outnode1PP,
VuP            *outnode2PP
)
    {
    vu_splitEdge (graphP, P, outnode1PP, outnode2PP, graphP->mPrimitiveData.mNewLoopInterior, graphP->mPrimitiveData.mNewLoopExterior);
    }

/*---------------------------------------------------------------------------------**//**
@description Copy from P to Q those mask bits that are designated in the graph for copying around vertex loops.
 @param graphP IN OUT graph header
 @param P IN source node
 @param Q IN OUT destination node
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_copyMaskAroundVertex
(
VuSetP graphP,
VuP P,
VuP Q
)
    {
    VU_SETMASK (Q, VU_GETMASK (P, graphP->mPrimitiveData.mCopyAroundVertex));
    }

/*---------------------------------------------------------------------------------**//**
@description If graph says the "user data" field of nodes is vertex data, copy the user data from P to Q.   Otherwise do nothing.
@param graphP IN OUT graph header
@param P IN source node
@param Q IN OUT destination node
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_copyConditionalVertexData
(
VuSetP graphP,
VuP P,
VuP Q
)
    {
    if (graphP->mPrimitiveData.bUserDataIsVertexProperty)
        VU_USER_ID(Q) = VU_USER_ID(P);
    if (graphP->mPrimitiveData.bUserData1IsVertexProperty)
        vu_setUserData1 (Q, vu_getUserData1 (P));
    }

/*---------------------------------------------------------------------------------**//**
@description Create a new edge and insert it from vertex P to vertex Q.
@remarks The nodes of the new edge receive mask bits according to the parent vu graph.
@param graphP IN OUT graph header
@param P IN OUT preexisting node.  If NULL, new edge dangles in space at this end.
@param Q IN OUT preexisting node.  If NULL, new edge dangles in space at this end.
@param outnode1PP OUT node of new edge, at P end.
@param outnode2PP OUT node of new edge, at Q end.
@group "VU Edges"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_join
(
VuSetP graphP,
VuP P,
VuP Q,
VuP *outnode1PP,
VuP *outnode2PP
)
    {
#ifdef VERIFY_PROPERTIES
    if (vu_checkMode >= VU_CHECKNEW)
        vu_checkNextInSet (graphP);

    if (VU_FSUCC (P) == Q || VU_FSUCC (Q) == P)
        vu_panic (graphP);
#endif

    vu_makePair (graphP, outnode1PP, outnode2PP);

    if (P)
        {
        vu_vertexTwist (graphP, *outnode1PP, P);
        vu_copyMaskAroundVertex (graphP, P, *outnode1PP);
        vu_copyCoordinates (*outnode1PP, P);
        vu_copyConditionalVertexData (graphP, P, *outnode1PP);
        }

    if (Q)
        {
        vu_vertexTwist (graphP, *outnode2PP, Q);
        vu_copyMaskAroundVertex (graphP, Q, *outnode2PP);
        vu_copyCoordinates (*outnode2PP, Q);
        vu_copyConditionalVertexData (graphP, Q, *outnode2PP);
        }

    if (graphP->mPrimitiveData.edgeInsertionFunction)
        {
        (*graphP->mPrimitiveData.edgeInsertionFunction) (graphP, P);
        (*graphP->mPrimitiveData.edgeInsertionFunction) (graphP, Q);
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Excise both ends of the edge starting at nodeP from their vertices.
@param graphP IN OUT graph header
@param nodeP IN OUT node to detach from its vertex loops.
@group "VU Edges"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_detachEdge
(
VuSetP          graphP,
VuP             nodeP
)
    {
    VuP farP = VU_FSUCC(nodeP);
    VuP mateP = VU_VSUCC(farP);
    VuP predP = VU_FSUCC(mateP);
    vu_vertexTwist (graphP, nodeP, predP);
    vu_vertexTwist (graphP, mateP, farP);
    }

/*---------------------------------------------------------------------------------**//**
@description Get any available mask bit from the graph header.
@remarks The application must return the mask via ~mvu_returnMask
@param graphP IN OUT graph header
@return the borrowed mask
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuMask vu_grabMask
(
VuSetP graphP
)
{
    return graphP->GrabMask ();
    }

/*---------------------------------------------------------------------------------**//**
@description Return (drop) a mask bit previously grabbed (borrowed) via ~mvu_grabMask.
@param graphP IN OUT graph header
@param m IN the mask to drop
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_returnMask
(
VuSetP graphP,
VuMask m
)
    {
    graphP->DropMask (m);
    }



/*---------------------------------------------------------------------------------**//**
@description Clear a mask bit in all nodes in a graph.
@param graphP IN OUT graph header
@param m IN mask to clear
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_clearMaskInSet
(
VuSetP          graphP,         /* Set in which the mask is to be cleared */
VuMask          m               /* Mask to be cleared */
)
    {
    VU_SET_LOOP (currP, graphP)
        {
        VU_CLRMASK (currP, m);
        }
    END_VU_SET_LOOP (currP, graphP)
    }

/*---------------------------------------------------------------------------------**//**
@description Set a mask bit in all nodes in a graph.
@param graphP IN OUT graph header
@param m IN mask to set
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_setMaskInSet
(
VuSetP          graphP,         /* Set in which the mask is to be set */
VuMask          m               /* Mask to be cleared */
)
    {
    VU_SET_LOOP (currP, graphP)
        {
        VU_SETMASK (currP, m);
        }
    END_VU_SET_LOOP (currP, graphP)
    }

/*---------------------------------------------------------------------------------**//**
@description toggle a mask bit in all nodes in a graph.
@param graphP IN OUT graph header
@param m IN mask to change
@group "VU Node Masks"
@bsimethod                                                    BentleySystems  05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_toggleMaskInSet
(
VuSetP          graphP,         /* Set in which the mask is to be set */
VuMask          m               /* Mask to be toggled */
)
    {
    VU_SET_LOOP (currP, graphP)
        {
        if (vu_getMask (currP, m))
            {
            vu_clrMask (currP, m);
            }
        else
            vu_setMask (currP, m);
        }
    END_VU_SET_LOOP (currP, graphP)
    }



/*---------------------------------------------------------------------------------**//**
@description Copy value of one mask bit to another throughout the graph.
@param graphP IN OUT graph header
@param oldMask mask to read
@param newMask mask to set or clear
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_copyMaskInSet
(
VuSetP          graphP,         /* Set in which the mask is to be set */
VuMask          oldMask,        /* Mask to read */
VuMask          newMask         /* Mask to write */
)
    {
    VU_SET_LOOP (currP, graphP)
        {
        if (vu_getMask (currP, oldMask))
            vu_setMask (currP, newMask);
        else
            vu_clrMask (currP, newMask);
        }
    END_VU_SET_LOOP (currP, graphP)
    }
/*---------------------------------------------------------------------------------**//**
@description Set a given mask bit in all nodes around the vertex loop starting at vertexP.
@param vertexP IN any node in the vertex loop
@param m IN the mask bits
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_setMaskAroundVertex
(
VuP             vertexP,
VuMask          m
)
    {
    VU_VERTEX_LOOP (currP, vertexP)
        {
        VU_SETMASK (currP, m);
        }
    END_VU_VERTEX_LOOP (currP, vertexP)
    }

/*---------------------------------------------------------------------------------**//**
@description Set a given mask bit in nodes around the face loop starting at startP.
@param startP IN any node at the face loop
@param m IN the mask bits
@group "VU Node Masks"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_setMaskAroundFace
(
VuP             startP,
VuMask          m
)
    {
    VU_FACE_LOOP (currP, startP)
        {
        VU_SETMASK (currP, m);
        }
    END_VU_FACE_LOOP (currP, startP)
    }

/*---------------------------------------------------------------------------------**//**
@description Assign a unique index to each VU in the current graph.
@remarks These ids are considered volatile and may change if nodes are added to the graph.
@remarks In canonical indices, each edge pair (nodeP and vu_fsucc(nodeP)) is an even-odd sequential pair, i.e. (0,1), (2,3), etc.
@param graphP IN graph header
@return The number of nodes in the graph, or -1 if numbers could not be assigned.
@group "VU Graph Header"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_assignCanonicalEdgeIndices
(
VuSetP          graphP
)
    {
    int i;
    int count;
    /* Verify that the entire graph is self-contained, i.e. all successor fields
        are in the same master loop */
    /* Set all ids to 0.  Check that vs, fs lead to 0 ids.
       Set all ids to -1.  Check that vs, fs lead to -1 ids.
       All nodes are left with -1 at end -- this will be important to
        identify unvisited edges during re-indexing!!!!
    */
    for (i = 0; i > -2; i--)
        {
        /* Set flag value */
        VU_SET_LOOP (currP, graphP)
            {
            VU_ID (currP) = 0;
            }
        END_VU_SET_LOOP (currP, graphP)
        /* Check the flag over the successor pointers */
        VU_SET_LOOP (currP, graphP)
            {
            if (VU_ID (VU_FSUCC(currP)) != i)
                return -1;
            if (VU_ID (VU_VSUCC(currP)) != i)
                return -1;
            }
        END_VU_SET_LOOP (currP, graphP)
        }

    /* Assign canonical to edges */
    count = 0;
    VU_SET_LOOP (currP, graphP)
        {
        if (VU_ID(currP) < 0)
            {
            VU_ID(currP) = count++;
            VU_ID(VU_EDGE_MATE(currP)) = count++;
            }
        }
    END_VU_SET_LOOP(currP, graphP)

    return count;
    }

/*---------------------------------------------------------------------------------**//**
@description Get the unique index assigned to a node by ~mvu_assignCanonicalEdgeIndices.
@param nodeP IN node to inspect
@return node index
@group "VU Nodes"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_getIndex     /* <= index of the node. */
(
VuP             nodeP                   /* => node to read */
)
    {
    return VU_ID(nodeP);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
