/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "Regions/rg_intern.h"
#include <stdio.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#define UNVISITED_DEPTH (-1)
#define STARTING_DEPTH (0)

static bool    sameMarkState
(
MTG_MarkSet *pMarkSet,
MTGNodeId   nodeId0,
MTGNodeId   nodeId1
)
    {
    bool    stat0 = jmdlMTGMarkSet_isNodeInSet (pMarkSet, nodeId0);
    bool    stat1 = jmdlMTGMarkSet_isNodeInSet (pMarkSet, nodeId1);

    if (stat0)
        {
        return stat1 ? true : false;
        }
    else
        return stat1 ? false : true;
    }

/*------------------------------------------------------------------*//**
* Invariants:
*<ul>
*<li>Assigned depth is uniform within a face.
*<li>The edge mate of a node on the stack has an assigned depth.
*</ul>
*---------------------------------------------------------------------*/
static void recordAndStackEdgeMatesAroundFace
(
MTGGraph    *pGraph,
EmbeddedIntArray *pStack0,  /* For face reached without state change */
EmbeddedIntArray *pStack1,  /* For faces reached with state change */
EmbeddedIntArray *pNodeIdToDepthArray,
MTG_MarkSet *pMarkSet,
MTGNodeId       seedNodeId,
int             seedDepth,
int             noisy
)
    {
    MTGNodeId mateNodeId;
    int mateDepth;
    /* Assign depth to this face ... */
    if (noisy)
        GEOMAPI_PRINTF (" Face at depth %d:", seedDepth);
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, seedNodeId)
        {
        jmdlEmbeddedIntArray_setInt (pNodeIdToDepthArray, seedDepth, currNodeId);
        if (noisy)
            GEOMAPI_PRINTF(" %d", currNodeId);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, seedNodeId)

    if (noisy)
        GEOMAPI_PRINTF ("\n Unvisited mates:");


    MTGARRAY_FACE_LOOP (currNodeId, pGraph, seedNodeId)
        {
        mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
        if (jmdlEmbeddedIntArray_getInt (pNodeIdToDepthArray, &mateDepth, mateNodeId))
            {
            if (mateDepth == UNVISITED_DEPTH)
                {
                if (   jmdlMTGGraph_getMask (pGraph, mateNodeId, RG_MTGMASK_NULL_FACE)
                    || sameMarkState (pMarkSet, currNodeId, mateNodeId))
                    {
                    if (noisy)
                        GEOMAPI_PRINTF ("(=%d)", mateNodeId);
                    jmdlEmbeddedIntArray_addInt (pStack0, mateNodeId);
                    }
                else
                    {
                    jmdlEmbeddedIntArray_addInt (pStack1, mateNodeId);
                    if (noisy)
                        GEOMAPI_PRINTF ("(+%d)", mateNodeId);
                    }
                }
            }
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, seedNodeId)

        if (noisy)
        GEOMAPI_PRINTF ("\n");
    }

static void runDepthAssignmentSearch
(
RG_Header           *pRG,
EmbeddedIntArray    *pNodeIdToDepthArray,
MTG_MarkSet         *pMarkSet,
EmbeddedIntArray    *pStack0,   /* Stack (probably pre-seeded) for first-priority nodes */
EmbeddedIntArray    *pStack1,   /* Stack (possibily pre-seeded) for second priority nodes */
int                 noisy
)
    {
    MTGGraph *pGraph = pRG->pGraph;
    int currDepth;
    int mateDepth;
    MTGNodeId mateNodeId, currNodeId;
    EmbeddedIntArray *pTempStack;
    int swapCount = 0;

    while (  jmdlEmbeddedIntArray_getCount (pStack0) > 0
          || jmdlEmbeddedIntArray_getCount (pStack1) > 0
          )
        {
        /* Always remember
            .... edges pulled from stack are on the "inside" loops that may need
                    to be visited.  The visited path is back across the edge mate.
            .... the MATE of a stacked node must already have a depth assigned.
            .... if this is first visit to the stacked face, its depth
                    can be same or one deeper than mate.
        */
        while (jmdlEmbeddedIntArray_popInt (pStack0, &currNodeId))
            {
            if (noisy)
                GEOMAPI_PRINTF ("     Pop %d\n", currNodeId);
            /* Look BACKWARDS to the labeled data on the other side of the edge. */
            if (   jmdlEmbeddedIntArray_getInt (pNodeIdToDepthArray, &currDepth, currNodeId)
                && currDepth == UNVISITED_DEPTH
                && jmdlEmbeddedIntArray_getInt (pNodeIdToDepthArray, &mateDepth,
                                (mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId)))
                )
                {
                currDepth =
                       jmdlMTGGraph_getMask (pGraph, currNodeId, RG_MTGMASK_NULL_FACE)
                    || sameMarkState (pMarkSet, currNodeId, mateNodeId)
                    ? mateDepth
                    : mateDepth + 1;
                if (noisy)
                    GEOMAPI_PRINTF ("         Mark from %d at depth %d\n", currNodeId, currDepth);
                recordAndStackEdgeMatesAroundFace
                        (
                        pGraph,
                        pStack0,
                        pStack1,
                        pNodeIdToDepthArray,
                        pMarkSet,
                        currNodeId,
                        currDepth,
                        noisy
                        );
                }
            }
        /* OK, we have fully flooded the faces reachable from a seed in stack 0
                without changing mark state.  During the flood, further faces reachable
                with a mark state change were pushed onto stack 1.  Switch stacks
                and repeat.
        */
        pTempStack = pStack0;
        pStack0 = pStack1;
        pStack1 = pStack0;
        if (noisy)
            GEOMAPI_PRINTF(" stack swap # %d\n", ++swapCount);
        }
    }

/*------------------------------------------------------------------*//**
* Search a graph in which a mark set identifies distinguished faces.
* Mark geometrically exterior faces as depth 0.  For all other faces,
* the depth is the smallest number of transition edges which must be crossed to
* reach that face.
*
* @param pNodeIdDepthArray <=> array of node depths.
* @param pMarkSet => Set membership defines depth transitions.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRG_setMarksetDepthByInwardSearch
(
RG_Header           *pRG,
EmbeddedIntArray    *pNodeIdToDepthArray,
MTG_MarkSet         *pMarkSet
)
    {
    int noisy = jmdlRG_getNoisy ();
    static int s_noisy = 0;
    MTGGraph *pGraph = pRG->pGraph;
    int maxNode = jmdlMTGGraph_getNodeIdCount (pGraph);
    int seedDepth;
    EmbeddedIntArray *pStack0 = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pStack1 = jmdlEmbeddedIntArray_grab ();

    if (s_noisy)
        noisy = s_noisy;

    if (noisy)
        GEOMAPI_PRINTF (" **** Marked set depth search\n");

    jmdlEmbeddedIntArray_setConstant (pNodeIdToDepthArray, UNVISITED_DEPTH, maxNode);
    jmdlRG_buildBridgeEdges (pRG);

    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        if (  jmdlEmbeddedIntArray_getInt (pNodeIdToDepthArray, &seedDepth, seedNodeId)
           && seedDepth == UNVISITED_DEPTH
           && jmdlRG_faceIsTrueExterior (pRG, seedNodeId)
           )
            {
            if (noisy)
                GEOMAPI_PRINTF (" **** True exterior face recursion seed %d\n", seedNodeId);

            recordAndStackEdgeMatesAroundFace
                    (
                    pGraph,
                    pStack0,
                    pStack1,
                    pNodeIdToDepthArray,
                    pMarkSet,
                    seedNodeId,
                    STARTING_DEPTH,
                    noisy
                    );
            runDepthAssignmentSearch
                    (
                    pRG,
                    pNodeIdToDepthArray,
                    pMarkSet,
                    pStack0,
                    pStack1,
                    noisy
                    );
            }
        }
    MTGARRAY_END_SET_LOOP (newSeedNodeId, pGraph)
    jmdlRG_dropBrideEdges (pRG);

    jmdlEmbeddedIntArray_drop (pStack1);
    jmdlEmbeddedIntArray_drop (pStack0);
    }

/*------------------------------------------------------------------*//**
* Given a depth as determined by inward parity search, return the
* depth by the compressed depth definition that odd depth greater than 1
* is an island and any even depth greater than 1 is a canal.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public int     jmdlRG_depthToCompressedDepth
(
int depth
)
    {
    int compressedDepth;
    if (depth == 1)
        {
        compressedDepth = RG_COMPRESSED_DEPTH_PRIMARY;
        }
    else if (depth > 1)
        {
        if (depth & 0x01)
            compressedDepth = RG_COMPRESSED_DEPTH_ISLAND;
        else
            compressedDepth = RG_COMPRESSED_DEPTH_CANAL;
        }
    else
        {
        compressedDepth = RG_COMPRESSED_DEPTH_EXTERIOR;
        }
    return compressedDepth;
    }

/*------------------------------------------------------------------*//**
* Compute classifications (unused, primary, canal, island) on both sides
* of an edge.
* @return true if classification completed.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public bool         jmdlRG_getCompressedDepthsOnEdge
(
RG_Header           *pRG,
int                 *pNodeCompressedDepth,
int                 *pMateCompressedDepth,
EmbeddedIntArray    *pNodeIdToDepthArray,
MTGNodeId           nodeId
)
    {
    MTGNodeId mateNodeId = jmdlRG_skipNullFacesToEdgeMate (pRG, nodeId);
    int nodeDepth, mateDepth;
    int nodeCompressedDepth = 0, mateCompressedDepth = 0;
    bool    boolstat = false;

    if (   jmdlEmbeddedIntArray_getInt (pNodeIdToDepthArray, &nodeDepth, nodeId)
        && jmdlEmbeddedIntArray_getInt (pNodeIdToDepthArray, &mateDepth, mateNodeId))
        {
        nodeCompressedDepth = jmdlRG_depthToCompressedDepth (nodeDepth);
        mateCompressedDepth = jmdlRG_depthToCompressedDepth (mateDepth);
        boolstat = true;
        }

    if (pNodeCompressedDepth)
        *pNodeCompressedDepth = nodeCompressedDepth;
    if (pMateCompressedDepth)
        *pMateCompressedDepth = mateCompressedDepth;

    return boolstat;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
