/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/* A stack implementation for VU graphs */

/* Declare these locally so that they do not get exposed in an .fdf file */
extern void     vu_ntwist (VuP     P, VuP      Q);
extern VuP      vu_newVuP (VuSetP  graphP);
extern void vu_recycleNodeLoop (VuP *listPP, VuP *loopPP);
/*---------------------------------------------------------------------------------**//**
@description Delete all nodes on stacked graphs.
@remarks Nodes in the current graph are unaffected.
@param graphP IN OUT graph header
@group "VU Graph Stack"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_stackClearAll
(
VuSetP          graphP
)
    {
    VuP         loopP;

    while (loopP = vu_arrayRemoveLast (&graphP->mGraphStack))
        vu_recycleNodeLoop (&graphP->freeNodeP, &loopP);
    }

/*---------------------------------------------------------------------------------**//**
@description Push all nodes from the current graph onto the graph stack.
@remarks The current graph appears to be empty after the push.
@remarks If there are no current nodes, an empty stack frame is pushed.
@param graphP IN OUT graph header
@return the number of stacked node sets after this push
@group "VU Graph Stack"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackPush
(
VuSetP          graphP
)
    {
    /* Notice this allows pushing an empty graph onto the stack */

    vu_arrayAdd (&graphP->mGraphStack, graphP->lastNodeP);
    graphP->lastNodeP = VU_NULL;

    return  vu_arraySize (&graphP->mGraphStack);
    }

/*---------------------------------------------------------------------------------**//**
@description Push all nodes from the current graph onto the graph stack, and immediately
    assemble a copy of the just-stacked nodes in the graph.
@remarks If there are no current nodes, an empty stack frame is pushed, and the post-copy graph has no nodes.
@param graphP IN OUT graph header
@return the number of stacked node sets after this push
@group "VU Graph Stack"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackPushCopy
(
VuSetP          graphP
)
    {
    int         stackSize;
    VuP         startP, P, Q, R, S;

    /* Duplicate the current graph on the stack. */
    if (vu_emptyGraph (graphP))
        return vu_stackPush (graphP);

    startP = graphP->lastNodeP;
    stackSize = vu_stackPush (graphP);

    /* Allocate duplicate VuP's in the graph. */
    P = startP;
    do
        {
        if (NULL == vu_newVuP (graphP))
            {
            vu_panic (graphP);
            return  stackSize;
            }
        P = VU_NEXT (P);
        } while (P != startP);

    /* Fix all the data in the duplicated graph. */
    Q = graphP->lastNodeP;
    do
        {
        /* copy data */
        VU_MASK (Q)         = VU_MASK (P);
        VU_ID (Q)           = VU_ID (P);

#if defined (needs_work)
    /* This function should let the user pass in a function that will be called to
        do a deep copy of the lptr field in the VU node!!!!! If no function supplied,
        then just copy value.
        This will have to do for now. */
#endif
        vu_copyInternalDataP (Q, P);

        VU_USER_ID (Q)      = VU_USER_ID (P);
        vu_copyCoordinates (Q, P);

        /* fix face loop pointer */
        S = Q;  R = P;
        do
            {
            S = VU_NEXT(S);
            R = VU_NEXT(R);
            } while (R != VU_FSUCC (P));
        VU_FSUCC (Q) = S;

        /* fix vertex loop pointer */
        S = Q;  R = P;
        do
            {
            S = VU_NEXT(S);
            R = VU_NEXT(R);
            } while (R != VU_VSUCC (P));
        VU_VSUCC (Q) = S;


        P = VU_NEXT (P);
        Q = VU_NEXT (Q);
        } while (P != startP);

    return  stackSize;
    }

/*---------------------------------------------------------------------------------**//**
@description Pop all nodes from the top node set of the graph stack into the current graph.
@remarks All popped nodes become accessible by normal traversals such as visiting all nodes in the graph.
@param graphP IN OUT graph header
@return the number of stacked node sets after this pop
@group "VU Graph Stack"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackPop
(
VuSetP          graphP
)
    {
    VuP         fromStackP = vu_arrayRemoveLast (&graphP->mGraphStack);

    if (VU_NULL != fromStackP)
        {
        if (VU_NULL != graphP->lastNodeP)
            vu_ntwist (graphP->lastNodeP, fromStackP);
        else
            graphP->lastNodeP = fromStackP;
        }
    return  vu_arraySize (&graphP->mGraphStack);
    }

/*---------------------------------------------------------------------------------**//**
@description Pop all nodes from the top node set of the graph stack into the current graph, and
    immediately execute the callback operation on the graph.
@param graphP IN OUT graph header
@param operationFunc IN callback operation
@param userDataP IN unused
@return the number of stacked node sets after this pop
@group "VU Graph Stack"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackPopWithOperation
(
VuSetP          graphP,
VuStackOpFuncP  operationFunc,
void            *userDataP
)
    {
    VuP         fromStackP = vu_arrayRemoveLast (&graphP->mGraphStack);

    if (VU_NULL != fromStackP)
        {
        if (VU_NULL != graphP->lastNodeP)
            vu_ntwist (graphP->lastNodeP, fromStackP);
        else
            graphP->lastNodeP = fromStackP;
        }

    if (operationFunc)
        (*operationFunc) (graphP);

    return  vu_arraySize (&graphP->mGraphStack);
    }

/*---------------------------------------------------------------------------------**//**
@description Pop all nodes of a specified number of stacked node sets back into the current graph.
@remarks All popped nodes become accessible by normal traversals such as visiting all nodes in the graph.
@param graphP IN OUT graph header
@param numberOfEntries IN number of pops to execute
@return the number of stacked node sets after this pop
@group "VU Graph Stack"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackPopNEntries
(
VuSetP          graphP,
int             numberOfEntries
)
    {
    int         numberLeftInStack = vu_arraySize (&graphP->mGraphStack);

    while (numberOfEntries-- && numberLeftInStack)
        {
        numberLeftInStack = vu_stackPop (graphP);
        }

    return  numberLeftInStack;
    }

/*---------------------------------------------------------------------------------**//**
@description Pop all nodes of all stacked node sets back into the current graph.
@remarks All nodes become accessible by normal traversals such as visiting all nodes in the graph.
@param graphP IN OUT graph header
@return the number of stacked node sets after this pop: zero
@group "VU Graph Stack"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackPopAll
(
VuSetP          graphP
)
    {
    return  vu_stackPopNEntries (graphP, vu_arraySize (&graphP->mGraphStack));
    }

/*---------------------------------------------------------------------------------**//**
@description Swap node sets between the "current" graph and the top node set in the graph stack.
@param graphP IN OUT graph header
@group "VU Graph Stack"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_stackExchange
(
VuSetP          graphP
)
    {
    VuP         fromStackP = vu_arrayRemoveLast (&graphP->mGraphStack);

    vu_arrayAdd (&graphP->mGraphStack, graphP->lastNodeP);
    graphP->lastNodeP = fromStackP;
    }

/*---------------------------------------------------------------------------------**//**
@description Query the number of stacked node sets.
@param graphP IN graph header
@return number of stacked node sets
@group "VU Graph Stack"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackSize
(
VuSetP          graphP
)
    {
    return  vu_arraySize (&graphP->mGraphStack);
    }
// Oct 2013.
// extractNodeList/restoreNodeList -- allow app to manage its own stack.
// internal stack should just use these, yes?
/*---------------------------------------------------------------------------------**//**
@description return (a pointer to ) the list of all active nodes.  Clear the active set.
    Caller becomes responsible for returning the nodes at application specific time.
@param graphP IN OUT graph header
@return pointer to nodes in (prior) active set.
@group "VU Graph Stack"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public VuP vu_extractNodeList (VuSetP graphP)
    {
    VuP nodeList = graphP->lastNodeP;
    graphP->lastNodeP = NULL;
    return nodeList;
    }

/*---------------------------------------------------------------------------------**//**
@description Reactivate nodes previously obtained from vu_extractNodeList.
@param graphP IN OUT graph header
@param nodeListP IN node pointer previously returned from vu_extractNodeList
@group "VU Graph Stack"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public void vu_restoreNodeList (VuSetP graphP, VuP nodeListP)
    {
    if (VU_NULL != nodeListP)
        {
        if (VU_NULL != graphP->lastNodeP)
            vu_ntwist (graphP->lastNodeP, nodeListP);
        else
            graphP->lastNodeP = nodeListP;
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE
