/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/regions/rg_loops.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "Regions/rg_intern.h"
#include <Mtg/mtgprint.fdf>
#include <stdio.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE





/*----------------------------------------------------------------------+
|                                                                       |
|       global function definitions                                     |
|                                                                       |
+----------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
* Combine a local noisy flag with jmdlRG_getNoisy ().
+--------------------------------------------------------------------*/
static     int getNoisy
(
int suggestedNoisy
)
    {
    return suggestedNoisy | jmdlRG_getNoisy ();
    }



/**
* The "successor" of an edge in an extended face is reached by (1) moving to its true
* face successor and (2) continuing around that vertex for 0 or more steps until reaching
* an edge whose mask is NOT set on the edge mate.
*
* @instance pGraph      => graph to search
* @param nodeId         => node whose successor is needed.
* @param mask           => mask which must be cleared on the edge mate of the successor.
* @returns successor node
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public MTGNodeId jmdlRG_extendedFaceSuccessor
(
const MTGGraph     *pGraph,
MTGNodeId           nodeId,
MTGMask     mask
)
    {
    MTGNodeId baseNodeId = jmdlMTGGraph_getFSucc (pGraph, nodeId);
    MTGNodeId edgeMateId;

    MTGARRAY_VERTEX_PREDLOOP (currNodeId, pGraph, baseNodeId)
        {
        edgeMateId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
        if (!jmdlMTGGraph_getMask (pGraph, edgeMateId, mask))
            return currNodeId;
        }
    MTGARRAY_END_VERTEX_PREDLOOP (currNodeId, pGraph, baseNodeId)
    return MTG_NULL_NODEID;

    }

/*---------------------------------------------------------------------------------**//**
* Explore the "extended" connected component reachable from startNodeId.
*
* @instance pGraph => containing graph
* @param pStack => array to use for search stack.
* @param pSequenceArray => complete list of nodes in faces.
* @param startNodeId => node to start search.
* @bsimethod                                                    EarlinLutz      07/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void jmdlMTG_exploreExtendedFaceComponent
(
MTGGraph            *pGraph,
EmbeddedIntArray            *pStack,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pSequenceArray,
MTGNodeId           seedNodeId,
MTGMask     candidateMask,
MTGMask     floodMask,
MTGMask     loopMask
)
    {
    int loopCount = 0;
    int currNodeId, mateNodeId;
    int candidateNodeId;
    MTGMask candidateOrFloodMask = candidateMask | floodMask;
    MTGMask candidateOrLoopMask  = candidateMask | loopMask;
    MTGMask testMask;
    jmdlEmbeddedIntArray_empty (pStack);
    jmdlEmbeddedIntArray_addInt(pStack, seedNodeId);


    while (jmdlEmbeddedIntArray_popInt ( pStack, &currNodeId))
        {
        if (!jmdlMTGGraph_getMask (pGraph, currNodeId, floodMask))
            {

            /* Keep the flood going ... */
            jmdlMTGGraph_setMask (pGraph, currNodeId, floodMask);
            candidateNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId);
            testMask = jmdlMTGGraph_getMask (pGraph, candidateNodeId, candidateOrFloodMask);
            if (    (testMask & candidateMask)
                && !(testMask & floodMask))
                jmdlEmbeddedIntArray_addInt(pStack, candidateNodeId);

            candidateNodeId = jmdlMTGGraph_getVSucc (pGraph, currNodeId);
            testMask = jmdlMTGGraph_getMask (pGraph, candidateNodeId, candidateOrFloodMask);
            if (    (testMask & candidateMask)
                && !(testMask & floodMask))
                jmdlEmbeddedIntArray_addInt(pStack, candidateNodeId);

            /* And walk the face if first visitor: */
            testMask = jmdlMTGGraph_getMask (pGraph, currNodeId, candidateOrLoopMask);
            if (  (testMask & candidateMask) && !(testMask & loopMask))
                {
                mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
                if (!jmdlMTGGraph_getMask (pGraph, mateNodeId, candidateMask))
                    {
                    if (loopCount == 0)
                        {
                        if (pStartArray)
                            jmdlEmbeddedIntArray_addInt(pStartArray, MTG_MARKER_START_COMPONENT);
                        if (pSequenceArray)
                            jmdlEmbeddedIntArray_addInt(pSequenceArray, MTG_MARKER_START_COMPONENT);
                        }
                    loopCount++;

                    if (pSequenceArray)
                        jmdlEmbeddedIntArray_addInt(pSequenceArray, MTG_MARKER_START_FACE);
                    if (pStartArray)
                        jmdlEmbeddedIntArray_addInt(pStartArray, currNodeId);

                    for (;   currNodeId != MTG_NULL_NODEID
                          && !jmdlMTGGraph_getMask (pGraph, currNodeId, loopMask);)
                        {
                        jmdlMTGGraph_setMask (pGraph, currNodeId, loopMask);
                        if (pSequenceArray)
                            jmdlEmbeddedIntArray_addInt(pSequenceArray, currNodeId);
                        currNodeId = jmdlRG_extendedFaceSuccessor
                                    (pGraph, currNodeId, candidateMask);
                        }

                    if (pSequenceArray)
                        jmdlEmbeddedIntArray_addInt(pSequenceArray, MTG_MARKER_END_FACE);

                    }
                }
            }
        }

    if (loopCount > 0)
        {
        if (pStartArray)
            jmdlEmbeddedIntArray_addInt(pStartArray, MTG_MARKER_END_COMPONENT);
        if (pSequenceArray)
            jmdlEmbeddedIntArray_addInt(pSequenceArray, MTG_MARKER_END_COMPONENT);
        }
    }

static void jmdlRG_printNodeTag
(
int value
)
    {
    if      (MTG_MARKER_START_COMPONENT == value)
        {
        GEOMAPI_PRINTF ("\n<Component>");
        }
    else if (MTG_MARKER_END_COMPONENT == value)
        {
        GEOMAPI_PRINTF ("\n</Component>");
        }
    else if (MTG_MARKER_START_FACE == value)
        {
        GEOMAPI_PRINTF ("\n\t<Face>");
        }
    else if (MTG_MARKER_END_FACE == value)
        {
        GEOMAPI_PRINTF (" </Face>");
        }
    else
        {
        GEOMAPI_PRINTF (" %d", value);
        }
    }

static void showFace
(
MTGGraph *pGraph,
MTGNodeId seedNodeId,
const char * pName
)
    {
    GEOMAPI_PRINTF (" %s %d ", pName, seedNodeId);
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, seedNodeId)
        {
        GEOMAPI_PRINTF (" %d", currNodeId);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, seedNodeId)
    GEOMAPI_PRINTF ("\n");
    }


/**
* Search the graph for face loops, where faces have an extended definition based
* on a mask.   The definition assumes that the mask is atomic on each face -- either
* set throughout or clear throughout.
*
* The "successor" of an edge in the extended face is reached by (1) moving to its true
* face successor and (2) continuing around that vertex for 0 or more steps until reaching
* an edge whose mask is NOT set on the edge mate.
*
* @instance pGraph      => graph to search
* @param pStartArray    <=> Array giving an arbitraryily chosen start node on each face.
*                               Faces for a single connected component are grouped together
*                               and are bracketed by MTG_MARKER_START_COMPONENT and MTG_MARKER_END_COMPONENT.
*                           Hence the sequence is:
*                           SC f f f f EC SC f f f f EC ....
*                           where each f is the start node of a face and each SC and EC are the
*                           marker values.
*
* @param pSequenceArray <=> Array giving the complete sequence of nodes around each face.  THe
*                           faces for a connected component are clustered together;  the entire cluster
*                           bracketed by MTG_MARKER_START_COMPONENT and MTG_MARKER_END_COMPONENT
*                           Each face's nodes are bracketed by MTG_MAREKR_START_FACE and MTG_MARKER_END_FACE
*                           Hence the overall sequence is:
*                               SC SF f f f EF SF f f f EF EC    SC ...SF f f f EF .... EC
* @see
* @return the number of components (contiguous blocks of face indices).
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public int jmdlMTG_collectAndNumberExtendedFaceLoops
(
MTGGraph            *pGraph,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pSequenceArray,
MTG_MarkSet         *pMarkSet
)
    {
    EmbeddedIntArray *pStack;
    int componentCount;

    MTGMask    mask = pMarkSet->GetMask ();
    int         seedIndex;
    MTGNodeId  seedNodeId;
    MTGMask     floodMask = jmdlMTGGraph_grabMask (pGraph);
    MTGMask     loopMask  = jmdlMTGGraph_grabMask (pGraph);
    MTGMask     bothTempMasks = floodMask | loopMask;
    static int s_noisy = 0;
    int noisy = getNoisy (s_noisy);

    jmdlEmbeddedIntArray_empty (pSequenceArray);
    jmdlEmbeddedIntArray_empty (pStartArray);

    pStack = jmdlEmbeddedIntArray_grab();
    componentCount = 0;

    /* Clear both temp masks throughout the MarkSet */
    jmdlMTGGraph_clearMaskInSet (pGraph, bothTempMasks);

    jmdlMTGMarkSet_initIteratorIndex (pMarkSet, &seedIndex);
    if (noisy > 1)
        GEOMAPI_PRINTF("\n *************** Face Exploration ******************* \n");
    for (;jmdlMTGMarkSet_getNextNode (pMarkSet, &seedIndex, &seedNodeId); )
        {
        if (noisy > 1)
            GEOMAPI_PRINTF (" Seed %d from mark set index %d \n", seedNodeId, seedIndex);
        if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, floodMask))
            {
            jmdlMTG_exploreExtendedFaceComponent (pGraph,
                        pStack, pStartArray, pSequenceArray,
                        seedNodeId, mask, floodMask, loopMask);
            componentCount++;
            }
        }

    if (noisy > 0)
        {
        int faceNodeId;
        int i;
        if (pStartArray)
            {
            GEOMAPI_PRINTF(" EXTENDED FACE START NODES ");
            for (i = 0; jmdlEmbeddedIntArray_getInt (pStartArray, &faceNodeId, i);i++)
                {
                jmdlRG_printNodeTag (faceNodeId);
                }
            }

        if (pSequenceArray)
            {
            GEOMAPI_PRINTF(" EXTENDED FACE FULL SEQUENCES ");
            for (i = 0; jmdlEmbeddedIntArray_getInt (pSequenceArray, &faceNodeId, i);i++)
                {
                jmdlRG_printNodeTag (faceNodeId);
                }
            }
        }

    jmdlMTGGraph_dropMask (pGraph, floodMask);
    jmdlMTGGraph_dropMask (pGraph, loopMask);
    jmdlEmbeddedIntArray_drop (pStack);

    return componentCount;
    }

/*------------------------------------------------------------------*//**
* @param pMarkSet <=> This face is added to this mark set.
* @param pUnvisitedNeighbors <=> stack to receive all unvisited neighbors.
* @param pHoleArray <=> scratch array for hole search.
* @param markAndStepToEdgeMate => if true, mark this face (but do not record it),
*               then step to the edge mate as the primary face (i.e. face to be recorded
*               and whose whose neighbors are marked.)
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void        inwardSearch_markupFaceAndPushNeighbors
(
RG_Header           *pRG,
EmbeddedIntArray    *pNodesReached,
EmbeddedIntArray    *pUnvisitedNeighborsAcrossSimpleEdges,
EmbeddedIntArray    *pUnvisitedNeighborsAcrossHoleEdges,
MTGNodeId           seedNodeId,
MTGMask             floodMask,
bool                markAndStepToEdgeMate,
int                 noisy,
bool                parityWithinComponent,
bool                vertexContactSufficient
)
    {
    MTGGraph *pGraph = pRG->pGraph;
    MTGNodeId mateNodeId;

    if (markAndStepToEdgeMate)
        {
        jmdlMTGGraph_setMaskAroundFace (pGraph, seedNodeId, floodMask);
        seedNodeId = jmdlMTGGraph_getEdgeMate (pGraph, seedNodeId);
        }

    if (jmdlMTGGraph_getMask (pGraph, seedNodeId, floodMask))
        return;

    if (noisy)
        GEOMAPI_PRINTF ("  Record face %d: ", seedNodeId);

    jmdlMTGGraph_setMaskAroundFace (pGraph, seedNodeId, floodMask);

    if (pNodesReached)
        {
#ifdef ADD_ALL_EDGES
        MTGARRAY_FACE_LOOP (currNodeId, pGraph, seedNodeId)
            {
            if (!jmdlMTGGraph_getMask (pGraph, currNodeId, RG_MTGMASK_BRIDGE_EDGE))
                {
                if (noisy)
                    GEOMAPI_PRINTF (" %d: ", currNodeId);
                jmdlEmbeddedIntArray_addInt (pNodesReached, currNodeId);
                }
            }
        MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, seedNodeId)
#else
        int numOuterFound = 0;
        MTGNodeId succNodeId;
        /* We want to record one edge per original face loop, in the
            state without bridges.
            To get just one outer, keep a counter.
            To get just one representative of each hole, record
                from the incoming bridge edge.

        */
        MTGARRAY_FACE_LOOP (currNodeId, pGraph, seedNodeId)
            {
            if (jmdlRG_isBridgeEdge (pRG, currNodeId))
                {
                /* Bridges go inward, bridges go outward.
                   The inward ones are followed by one of the hole edges,
                   marked as a negative area. */
                succNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId);
                if (jmdlRG_faceIsNegativeArea (pRG, succNodeId))
                    {
                    if (noisy)
                        GEOMAPI_PRINTF (" (inner %d) ", succNodeId);
                    jmdlEmbeddedIntArray_addInt (pNodesReached, succNodeId);
                    }
                }
            else if (jmdlRG_faceIsNegativeArea (pRG, currNodeId))
                {
                /* Ignore it --- we'll catch it from the incoming bridge */
                }
            else
                {
                if (numOuterFound++ == 0)
                    {
                    if (noisy)
                        GEOMAPI_PRINTF (" (outer %d) ", currNodeId);
                    jmdlEmbeddedIntArray_addInt (pNodesReached, currNodeId);
                    }
                }
            }
        MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, seedNodeId)
#endif
        }

    if (noisy)
        GEOMAPI_PRINTF ("\n");
    if (pUnvisitedNeighborsAcrossSimpleEdges || pUnvisitedNeighborsAcrossHoleEdges)
        {
        MTGARRAY_FACE_LOOP (currNodeId, pGraph, seedNodeId)
            {
            mateNodeId = jmdlMTGGraph_getEdgeMate (pRG->pGraph, currNodeId);
            if (!jmdlMTGGraph_getMask (pGraph, mateNodeId, floodMask))
                {
                if (jmdlRG_isBridgeEdge (pRG, currNodeId))
                    {
                    /* Ignore.  Well, we shouldn't even get here because the face
                        is already marked with flood. */
                    }
                else if (jmdlRG_faceIsNegativeArea (pRG, currNodeId))
                    {
                    if (noisy)
                        GEOMAPI_PRINTF ("      push hole mate %d\n", mateNodeId);
                    if (pUnvisitedNeighborsAcrossHoleEdges)
                        jmdlEmbeddedIntArray_addInt
                            (pUnvisitedNeighborsAcrossHoleEdges, mateNodeId);
                    }
                else
                    {
                    if (noisy)
                        GEOMAPI_PRINTF ("      push simple mate %d\n", mateNodeId);
                    if (pUnvisitedNeighborsAcrossSimpleEdges)
                        jmdlEmbeddedIntArray_addInt
                            (pUnvisitedNeighborsAcrossSimpleEdges, mateNodeId);
                    }
                }
            }
        MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, seedNodeId)
        }
    }

/*------------------------------------------------------------------*//**
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void        inwardSearch_go
(
RG_Header           *pRG,
EmbeddedIntArray    *pEvenParityNodeArray,
EmbeddedIntArray    *pOddParityNodeArray,
EmbeddedIntArray    *pEvenParityStack,
EmbeddedIntArray    *pOddParityStack,
MTGMask             floodMask,
bool                parityWithinComponent,
bool                vertexContactSufficient
)
    {
    MTGNodeId seedNodeId;
    int noisy = jmdlRG_getNoisy ();

    for (;;)
        {
        if (jmdlEmbeddedIntArray_popInt (pEvenParityStack, &seedNodeId))
            {
            if (noisy)
                showFace (pRG->pGraph, seedNodeId, "Even pop");
            inwardSearch_markupFaceAndPushNeighbors
                (
                pRG,
                pEvenParityNodeArray,
                parityWithinComponent
                            ? pOddParityStack
                            : pEvenParityStack,
                pOddParityStack,
                seedNodeId, floodMask, false, noisy,
                parityWithinComponent, vertexContactSufficient
                );
            }
        else if (jmdlEmbeddedIntArray_popInt (pOddParityStack, &seedNodeId))
            {
            if (noisy)
                showFace (pRG->pGraph, seedNodeId, "Odd pop");
            inwardSearch_markupFaceAndPushNeighbors
                (
                pRG,
                pOddParityNodeArray,
                parityWithinComponent
                            ? pEvenParityStack
                            : pOddParityStack,
                pEvenParityStack,
                seedNodeId, floodMask, false, noisy,
                parityWithinComponent, vertexContactSufficient
                );
            }
        else
            break;
        }
    }



/*------------------------------------------------------------------*//**
* Search the graph for face loops.   For each loop, report a seed node
* and an integer parity indicator.
*
* @param pEvenParityNodeArray <=> array to accumulate one node per face (outer
*                   and holes) on even parity faces.
*                   May be NULL.
* @param pOddParityNodeArray <=> array to accumulate one node per face (outer
*                   and holes) on odd parity faces.
*                   May be NULL.
*
* @param seedNodeId => start node for search.
*               Search type depends on face status:
*                   Any negative area face -- Cross each edge of the face.  The face on the
*                       other side is even in a recursive search.
*                   Any positive area face -- This face is even parity.  Cross each edge
*                               of each hole in the face;  at each such face start a recursive search
*                               with odd parity.
*                   MTG_NULL_NODEID -- From each true exterior face, cross each edge.
*                               The first face reached is an even parity seed for recursive
*                               search.
* @param    parityWithinComponent => if true, face-to-face steps within component
*               are considered parity switches.  If false, only step through hole boundary changes
*               parity.
* @param    vertexContactSufficient => if false, when stepping into an island only
*               faces with edge contact are enabled.  If true vertex contact is sufficient.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRG_collectAllNodesOnInwardParitySearchExt
(
RG_Header           *pRG,
EmbeddedIntArray    *pEvenParityNodeArray,
EmbeddedIntArray    *pOddParityNodeArray,
MTGNodeId           seedNodeId,
bool                parityWithinComponent,
bool                vertexContactSufficient
)
    {
    int noisy = jmdlRG_getNoisy ();

    MTGGraph *pGraph = pRG->pGraph;
    EmbeddedIntArray *pEvenParityStack = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pOddParityStack = jmdlEmbeddedIntArray_grab ();
    MTGMask floodMask = jmdlMTGGraph_grabMask (pGraph);
    jmdlMTGGraph_clearMaskInSet (pGraph, floodMask);

    if (noisy)
        GEOMAPI_PRINTF (" **** Parity Search from %d\n",
                        seedNodeId);

    if (seedNodeId == MTG_NULL_NODEID)
        {
        jmdlRG_buildBridgeEdges (pRG);
        MTGARRAY_SET_LOOP (newSeedNodeId, pGraph)
            {
            if (!jmdlMTGGraph_getMask (pGraph, newSeedNodeId, floodMask)
                && jmdlRG_faceIsTrueExterior (pRG, newSeedNodeId))
                {
                jmdlMTGGraph_setMaskAroundFace (pGraph, newSeedNodeId, floodMask);

                MTGARRAY_FACE_LOOP (currP, pGraph, newSeedNodeId)
                    {
                    inwardSearch_markupFaceAndPushNeighbors
                        (
                        pRG,
                        pEvenParityNodeArray,
                        parityWithinComponent
                            ? pOddParityStack
                            : pEvenParityStack,
                        pOddParityStack,
                        jmdlMTGGraph_getEdgeMate (pGraph, currP),
                        floodMask, false,
                        noisy, parityWithinComponent, vertexContactSufficient
                        );
                    }
                MTGARRAY_END_FACE_LOOP (currP, pGraph, newSeedNodeId)
                inwardSearch_go
                    (
                    pRG,
                    pEvenParityNodeArray, pOddParityNodeArray,
                    pEvenParityStack, pOddParityStack,
                    floodMask, parityWithinComponent, vertexContactSufficient
                    );
                }
            }
        MTGARRAY_END_SET_LOOP (newSeedNodeId, pGraph)
        jmdlRG_dropBrideEdges (pRG);
        }
    else
        {
        // TR 168457 Naively, we want to walk around this face and set the flod mask on
        // all mates.  However, if there is a non-separating edge that has
        // both sides on this face, it introduces the flood mask onto the seed face itself.
        // So follow up by taking the flood mask off of this face !!!
        jmdlMTGGraph_setEdgeMateMaskAroundFace (pRG->pGraph, seedNodeId, floodMask);
        jmdlMTGGraph_clearMaskAroundFace (pRG->pGraph, seedNodeId, floodMask);

        if (jmdlRG_faceIsNegativeArea (pRG, seedNodeId))
            {
            /* Step across to interior neighbor */
            seedNodeId = jmdlMTGGraph_getEdgeMate (pRG->pGraph, seedNodeId);
            jmdlMTGGraph_setEdgeMateMaskAroundFace (pRG->pGraph, seedNodeId, floodMask);
            }


        /* Connect to interior */
        jmdlRG_buildBridgeEdges (pRG);
        /* Record this face and push neighbors for continuation.  Bridge edges
            get us into holes.  Masks we just dropped around the exterior keep us
            from going out.
        */
        inwardSearch_markupFaceAndPushNeighbors
            (
            pRG,
            pEvenParityNodeArray, pEvenParityStack, pOddParityStack,
            seedNodeId, floodMask, false, noisy,  parityWithinComponent, vertexContactSufficient
            );
        inwardSearch_go
            (
            pRG,
            pEvenParityNodeArray, pOddParityNodeArray,
            pEvenParityStack, pOddParityStack,
            floodMask,  parityWithinComponent, vertexContactSufficient
            );
        jmdlRG_dropBrideEdges (pRG);
        }

    jmdlMTGGraph_dropMask (pGraph, floodMask);
    jmdlEmbeddedIntArray_drop (pEvenParityStack);
    jmdlEmbeddedIntArray_drop (pOddParityStack);

    }

Public void     jmdlRG_collectAllNodesOnInwardParitySearch
(
RG_Header           *pRG,
EmbeddedIntArray    *pEvenParityNodeArray,
EmbeddedIntArray    *pOddParityNodeArray,
MTGNodeId           seedNodeId
)
    {
    jmdlRG_collectAllNodesOnInwardParitySearchExt
                    (
                    pRG, pEvenParityNodeArray,
                    pOddParityNodeArray,
                    seedNodeId, false, false
                    );
    }
/*---------------------------------------------------------------------------------**//**
* Find and record hole candidates in the face starting at the seed node.
* @param pRG => regions with hole arrays available.
* @param pStack <=> stack where holes are pushed.
* @param seedNodeId => any node on face to test.
* @param holeCheckMask => If this is set, ignore the face.  Set this mask as holes are
*       extracted.
* @param noisy => enables console output.
* @bsimethod                                                    EarlinLutz      07/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void testForHoleCandidates
(
RG_Header           *pRG,
EmbeddedIntArray            *pStack,
EmbeddedIntArray            *pHoleArray,
MTGNodeId           seedNodeId,
MTGMask             holeCheckMask,
int                 noisy
)
    {
    MTGNodeId candidateNodeId = MTG_NULL_NODEID;
    int i;

    if (jmdlMTGGraph_getMask (pRG->pGraph, seedNodeId, holeCheckMask))
        {
        /* Been here,.... */
        }
    else
        {
        jmdlMTGGraph_setMaskAroundFace (pRG->pGraph, seedNodeId, holeCheckMask);

        jmdlEmbeddedIntArray_empty (pHoleArray);
        jmdlRG_resolveHoleNodeId (pRG, pHoleArray, seedNodeId);

        for (i = 0;
            jmdlEmbeddedIntArray_getInt (pHoleArray, &candidateNodeId, i);
            i++
            )
            {
            if (noisy > 0)
                GEOMAPI_PRINTF ("    Push hole candidate %d\n", candidateNodeId);
            jmdlEmbeddedIntArray_addInt (pStack, candidateNodeId);
            }
        }
    }

/*==================================================================*/
/*    RG VERSION -- USES FACE/HOLE DATA FROM THE RG HEADER          */
/*==================================================================*/

/**
* The "successor" of an edge in an extended face is reached by (1) moving to its true
* face successor and (2) continuing around that vertex for 0 or more steps until reaching
* an edge whose mask is NOT set on the edge mate.
*
* @instance pGraph      => graph to search
* @param nodeId         => node whose successor is needed.
* @param mask           => mask which must be cleared on the edge mate of the successor.
* @returns successor node
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static MTGNodeId jmdlRG_extendedFaceSuccessorWithHoleTest
(
RG_Header           *pRG,
MTGGraph            *pGraph,
EmbeddedIntArray            *pStack,
EmbeddedIntArray            *pHoleArray,
MTGNodeId           nodeId,
MTGMask     mask,
MTGMask     holeCheckMask,
int         noisy
)
    {
    MTGNodeId baseNodeId = jmdlMTGGraph_getFSucc (pGraph, nodeId);
    MTGNodeId edgeMateId;

    MTGARRAY_VERTEX_PREDLOOP (currNodeId, pGraph, baseNodeId)
        {
        edgeMateId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
        if (!jmdlMTGGraph_getMask (pGraph, edgeMateId, mask))
            return currNodeId;
        testForHoleCandidates (pRG, pStack, pHoleArray, currNodeId, holeCheckMask, noisy);
        }
    MTGARRAY_END_VERTEX_PREDLOOP (currNodeId, pGraph, baseNodeId)
    return MTG_NULL_NODEID;

    }

/*---------------------------------------------------------------------------------**//**
* Explore the "extended" connected component reachable from startNodeId.
*
* @instance pGraph => containing graph
* @param pStack => array to use for search stack.
* @param pSequenceArray => complete list of nodes in faces.
* @param startNodeId => node to start search.
* @bsimethod                                                    EarlinLutz      07/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void jmdlRG_exploreExtendedFaceComponent
(
RG_Header           *pRG,
EmbeddedIntArray            *pStack,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pSequenceArray,
MTGNodeId           seedNodeId,
MTGMask     candidateMask,
MTGMask     floodMask,
MTGMask     loopMask,
MTGMask     holeCheckMask,
int         noisy
)
    {
    int loopCount = 0;
    int currNodeId, mateNodeId;
    int candidateNodeId;
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    MTGMask candidateOrFloodMask = candidateMask | floodMask;
    MTGMask candidateOrLoopMask  = candidateMask | loopMask;
    EmbeddedIntArray   *pHoleArray = jmdlEmbeddedIntArray_grab ();

    MTGMask testMask;

    jmdlEmbeddedIntArray_empty (pStack);
    jmdlEmbeddedIntArray_addInt(pStack, seedNodeId);


    while (jmdlEmbeddedIntArray_popInt ( pStack, &currNodeId))
        {
        if (!jmdlMTGGraph_getMask (pGraph, currNodeId, floodMask))
            {
            if (noisy > 1)
                GEOMAPI_PRINTF ("    Pop candididate %d\n", currNodeId);
            /* Test vsucc, fsucc, and holes as possible seeds */

            candidateNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId);
            testMask = jmdlMTGGraph_getMask (pGraph, candidateNodeId, candidateOrFloodMask);
            if (    (testMask & candidateMask)
                && !(testMask & floodMask))
                jmdlEmbeddedIntArray_addInt(pStack, candidateNodeId);

            candidateNodeId = jmdlMTGGraph_getVSucc (pGraph, currNodeId);
            testMask = jmdlMTGGraph_getMask (pGraph, candidateNodeId, candidateOrFloodMask);
            if (    (testMask & candidateMask)
                && !(testMask & floodMask))
                jmdlEmbeddedIntArray_addInt(pStack, candidateNodeId);


            /* And walk the face if first visitor: */
            testMask = jmdlMTGGraph_getMask (pGraph, currNodeId, candidateOrLoopMask);
            if (  (testMask & candidateMask) && !(testMask & loopMask))
                {
                jmdlMTGGraph_setMask (pGraph, currNodeId, floodMask);
                mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
                if (!jmdlMTGGraph_getMask (pGraph, mateNodeId, candidateMask))
                    {
                    if (loopCount == 0)
                        {
                        if (pStartArray)
                            jmdlEmbeddedIntArray_addInt(pStartArray, MTG_MARKER_START_COMPONENT);
                        if (pSequenceArray)
                            jmdlEmbeddedIntArray_addInt(pSequenceArray, MTG_MARKER_START_COMPONENT);
                        }
                    loopCount++;

                    /* Dump the face */
                    if (pSequenceArray)
                        jmdlEmbeddedIntArray_addInt(pSequenceArray, MTG_MARKER_START_FACE);
                    if (pStartArray)
                        jmdlEmbeddedIntArray_addInt(pStartArray, currNodeId);

                    for (;   currNodeId != MTG_NULL_NODEID
                          && !jmdlMTGGraph_getMask (pGraph, currNodeId, loopMask);)
                        {

                        jmdlMTGGraph_setMask (pGraph, currNodeId, loopMask);
                        /* Add holes as candidates */
                        testForHoleCandidates (pRG, pStack, pHoleArray, currNodeId, holeCheckMask, noisy);
                        if (pSequenceArray)
                            jmdlEmbeddedIntArray_addInt(pSequenceArray, currNodeId);
                        currNodeId = jmdlRG_extendedFaceSuccessorWithHoleTest
                                    (
                                    pRG,
                                    pGraph,
                                    pStack,
                                    pHoleArray,
                                    currNodeId,
                                    candidateMask,
                                    holeCheckMask,
                                    noisy
                                    );
                        }

                    if (pSequenceArray)
                        jmdlEmbeddedIntArray_addInt(pSequenceArray, MTG_MARKER_END_FACE);

                    }
                }
            }
        }

    if (loopCount > 0)
        {
        if (pStartArray)
            jmdlEmbeddedIntArray_addInt(pStartArray, MTG_MARKER_END_COMPONENT);
        if (pSequenceArray)
            jmdlEmbeddedIntArray_addInt(pSequenceArray, MTG_MARKER_END_COMPONENT);
        }

    jmdlEmbeddedIntArray_drop (pHoleArray);
    }



/**
* Search the graph for face loops, where faces have an extended definition based
* on a mask.   The definition assumes that the mask is atomic on each face -- either
* set throughout or clear throughout.
*
* The "successor" of an edge in the extended face is reached by (1) moving to its true
* face successor and (2) continuing around that vertex for 0 or more steps until reaching
* an edge whose mask is NOT set on the edge mate.
*
* @instance pGraph      => graph to search
* @param pStartArray    <=> Array giving an arbitraryily chosen start node on each face.
*                               Faces for a single connected component are grouped together
*                               and are bracketed by MTG_MARKER_START_COMPONENT and MTG_MARKER_END_COMPONENT.
*                           Hence the sequence is:
*                           SC f f f f EC SC f f f f EC ....
*                           where each f is the start node of a face and each SC and EC are the
*                           marker values.
*
* @param pSequenceArray <=> Array giving the complete sequence of nodes around each face.  THe
*                           faces for a connected component are clustered together;  the entire cluster
*                           bracketed by MTG_MARKER_START_COMPONENT and MTG_MARKER_END_COMPONENT
*                           Each face's nodes are bracketed by MTG_MAREKR_START_FACE and MTG_MARKER_END_FACE
*                           Hence the overall sequence is:
*                               SC SF f f f EF SF f f f EF EC    SC ...SF f f f EF .... EC
* @see
* @return the number of components (contiguous blocks of face indices).
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public int jmdlRG_collectAndNumberExtendedFaceLoops
(
RG_Header           *pRG,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pSequenceArray,
MTG_MarkSet         *pMarkSet
)
    {
    EmbeddedIntArray *pStack;
    int componentCount;
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);

    MTGMask    mask = pMarkSet->GetMask ();
    int         seedIndex;
    MTGNodeId  seedNodeId;
    MTGMask     floodMask = jmdlMTGGraph_grabMask (pGraph);
    MTGMask     holeCheckMask = jmdlMTGGraph_grabMask (pGraph);
    MTGMask     loopMask  = jmdlMTGGraph_grabMask (pGraph);
    MTGMask     allTempMasks = floodMask | loopMask | holeCheckMask;
    static int s_noisy = 0;
    int noisy = getNoisy (s_noisy);

    jmdlEmbeddedIntArray_empty (pSequenceArray);
    jmdlEmbeddedIntArray_empty (pStartArray);

    pStack = jmdlEmbeddedIntArray_grab();
    componentCount = 0;

    jmdlMTGGraph_clearMaskInSet (pGraph, allTempMasks);

    jmdlMTGMarkSet_initIteratorIndex (pMarkSet, &seedIndex);
    if (noisy > 1)
        GEOMAPI_PRINTF("\n *************** Face Exploration ******************* \n");
    for (;jmdlMTGMarkSet_getNextNode (pMarkSet, &seedIndex, &seedNodeId); )
        {
        if (noisy > 1)
            GEOMAPI_PRINTF (" Seed %d from mark set index %d \n", seedNodeId, seedIndex);
        if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, floodMask))
            {
            jmdlRG_exploreExtendedFaceComponent (pRG,
                        pStack, pStartArray, pSequenceArray,
                        seedNodeId, mask, floodMask, loopMask, holeCheckMask,
                        noisy);
            componentCount++;
            }
        }

    if (noisy > 0)
        {
        int faceNodeId;
        int i;
        if (pStartArray)
            {
            GEOMAPI_PRINTF(" EXTENDED FACE START NODES ");
            for (i = 0; jmdlEmbeddedIntArray_getInt (pStartArray, &faceNodeId, i);i++)
                {
                jmdlRG_printNodeTag (faceNodeId);
                }
            }

        if (pSequenceArray)
            {
            GEOMAPI_PRINTF(" EXTENDED FACE FULL SEQUENCES ");
            for (i = 0; jmdlEmbeddedIntArray_getInt (pSequenceArray, &faceNodeId, i);i++)
                {
                jmdlRG_printNodeTag (faceNodeId);
                }
            }
        }

    jmdlMTGGraph_dropMask (pGraph, holeCheckMask);
    jmdlMTGGraph_dropMask (pGraph, floodMask);
    jmdlMTGGraph_dropMask (pGraph, loopMask);
    jmdlEmbeddedIntArray_drop (pStack);

    return componentCount;
    }

typedef enum
    {
    RGFloodEvent_EnterComponent,
    RGFloodEvent_EnterFace,
    RGFloodEvent_ExitFace,
    RGFloodEvent_ExitComponent
    } RGFloodEvent;

typedef void (*RGC_BooleanMarkupFunction)
    (
    RG_Header *pRG,
    RGFloodEvent event,
    EmbeddedIntArray *pStack,
    struct SimpleBooleanMarkupContext *pContext,
    int         noisy
    );


static bool       floodForBooleanMarkup
(
RG_Header           *pRG,
MTGGraph            *pGraph,
RGC_BooleanMarkupFunction   markupFunc,
EmbeddedIntArray            *pStack,
MTGNodeId           seedNodeId,
MTGMask             faceVisited,
MTGMask             edgeVisited,
SimpleBooleanMarkupContext *pContext,
int                 noisy
)
    {
    MTGNodeId currNodeId, mateNodeId, topNodeId;
    bool    boolstat = true;
    static int s_noisyTrigger = 1;

    jmdlEmbeddedIntArray_empty (pStack);
    jmdlEmbeddedIntArray_addInt(pStack, seedNodeId);
    jmdlMTGGraph_setMaskAroundFace (pGraph, seedNodeId, faceVisited);

    markupFunc (pRG, RGFloodEvent_EnterComponent, pStack, pContext, noisy);
    currNodeId = seedNodeId;

    for (;;)
        {
        mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
        jmdlMTGGraph_setMask (pGraph, currNodeId, edgeVisited);
        jmdlMTGGraph_setMask (pGraph, mateNodeId, edgeVisited);
        if (noisy > s_noisyTrigger)
                GEOMAPI_PRINTF ("    probe edge %d mate %d\n", currNodeId, mateNodeId);

        if (!jmdlMTGGraph_getMask (pGraph, mateNodeId, faceVisited))
            {
            /* Jump into the new face. */
            if (noisy > s_noisyTrigger)
                GEOMAPI_PRINTF ("    enter and mark face %d\n",  mateNodeId);
            jmdlMTGGraph_setMaskAroundFace (pGraph, mateNodeId, faceVisited);
            jmdlEmbeddedIntArray_addInt(pStack, mateNodeId);
            markupFunc (pRG, RGFloodEvent_EnterFace, pStack, pContext, noisy);
            currNodeId = mateNodeId;
            }
        else
            {
            currNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId);
            if (currNodeId == seedNodeId)
                {
                // hmm.. Unexpected, but ok if the seed started at an outside dangler.
                break;
                }
            if (noisy > s_noisyTrigger)
                GEOMAPI_PRINTF ("     face step to currNode.fSucc = %d\n",  currNodeId);
            if (!jmdlEmbeddedIntArray_getInt (pStack, &topNodeId, -1))
                {
                /* Stack should never be empty ... entry to current face
                   is on top */
                boolstat = false;
                break;
                }
            else
                {
                if (noisy > s_noisyTrigger)
                    GEOMAPI_PRINTF ("            top of stack = %d\n",  topNodeId);
                if (topNodeId == currNodeId)
                    {
                    /* We are done with the current face.
                        Exit back to previous face */
                    markupFunc (pRG, RGFloodEvent_ExitFace, pStack, pContext, noisy);
                    /* Don't even need to check if pop succeeded -- previous getInt
                        verified that there is something there. */
                    jmdlEmbeddedIntArray_popInt (pStack, &topNodeId);
                    /* We should only run into a visted edge on return
                       to the face entry, as recorded on the stack. */
                    currNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
                    /* These two conditions should occur at the same time!!! */
                    if (currNodeId == seedNodeId)
                        {
                        break;
                        }
                    currNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId);
                    }
                }
            }
        }
    if (jmdlEmbeddedIntArray_getCount (pStack) != 1)
        {
        /* This should not happen ... */
        boolstat = false;
        }
    markupFunc (pRG, RGFloodEvent_ExitComponent, pStack, pContext, noisy);
    return boolstat;
    }


/*------------------------------------------------------------------*//**
* Face-by-face DFS through the fully merged graph structure.
*<ul>
*<li>Only begin at true exterior edges.</li>
*<li>Announce each face entry and exit through a callback.</li>
*</ul>
* @param transitionFunction => Callback function of form
*<pre>
*           F(pRG,
*</pre>
*<ul>
*<li>Begin connected component</li>
*<li>Enter face across edge</li>
*<li>Exit face across edge</li>
*<li>End connected component</li>
*<li>
*</ul>
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static bool        markup_floodFromAllSeeds
(
RG_Header           *pRG,
RGC_BooleanMarkupFunction markupFunc,
SimpleBooleanMarkupContext                *pUserData
)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    MTGMask faceVisited = jmdlMTGGraph_grabMask (pGraph);
    MTGMask edgeVisited = jmdlMTGGraph_grabMask (pGraph);
    EmbeddedIntArray *pStack = jmdlEmbeddedIntArray_grab ();
    bool    boolstat = true;
    static int s_noisy = 0;
    int noisy = getNoisy (s_noisy);


    jmdlMTGGraph_clearMaskInSet (pGraph, faceVisited | edgeVisited);
    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, faceVisited)
            && jmdlRG_faceIsTrueExterior (pRG, seedNodeId))
            {
            if (noisy)
                    GEOMAPI_PRINTF (" flood loop seed %d\n", seedNodeId);
            MTGARRAY_FACE_LOOP (entryNodeId, pGraph, seedNodeId)
                {
                if (!jmdlMTGGraph_getMask (pGraph, jmdlMTGGraph_getEdgeMate (pGraph, entryNodeId), faceVisited))
                    {
                    boolstat = floodForBooleanMarkup
                            (
                            pRG,
                            pGraph,
                            markupFunc,
                            pStack,
                            entryNodeId,
                            faceVisited,
                            edgeVisited,
                            pUserData,
                            noisy
                            );
                    if (!boolstat)
                        break;
                    }
                }
            MTGARRAY_END_FACE_LOOP (entryNodeId, pGraph, seedNodeId)
            }
        if (!boolstat)
            break;
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pGraph)

    jmdlEmbeddedIntArray_drop (pStack);
    jmdlMTGGraph_dropMask (pGraph, faceVisited);
    jmdlMTGGraph_dropMask (pGraph, edgeVisited);
    return boolstat;
    }

struct SimpleBooleanMarkupContext 
    {
    RGBoolSelect selectorAB;
    RGBoolSelect selectorC;
    MTG_MarkSet *pMarkSet;
    bvector<int> groupParityArray;
    int         highestGroupA;
    int         highestGroupB;
    int         countA;
    int         countB;
    int         countC;     
    bool        reverseSense;   // reverse the sense of the tests.
    
    SimpleBooleanMarkupContext (
            RGBoolSelect _selectorAB,
            RGBoolSelect _selectorC,
            MTG_MarkSet *_pMarkSet,
            int _highestGroupA,
            int _highestGroupB,
            bool _reverseSense)
        : selectorAB (_selectorAB),
          selectorC  (_selectorC),
          pMarkSet (_pMarkSet),
          highestGroupA (_highestGroupA),
          highestGroupB (_highestGroupB),
          reverseSense (_reverseSense)
        {
        }
    ~SimpleBooleanMarkupContext ()
        {
        }
        
    bool IsValidGroupId (int g)
        {
        return g >= 0 && g < (int)groupParityArray.size ();
        }

    int     ReadAndToggleParity (int groupId)
        {
        int value, newValue;
        if (IsValidGroupId (groupId))
            {
            size_t g = (size_t)groupId;
            value = groupParityArray[g];
            newValue = value == 0 ? 1 : 0;
            groupParityArray[g] = newValue;
            return newValue == 0 ? -1 : 1;
            }
        return 0;
        }

    // make {numGroup} parity values 0 ...
    void InitGroupParityArray (int numGroup)
        {
        countA = 0;
        countB = 0;
        countC = 0;
        groupParityArray.clear ();
        for (ptrdiff_t i = 0; i < numGroup; i++)
            groupParityArray.push_back (0);
        }
        

/*------------------------------------------------------------------*//**
* Callback to handle face entry and exit.
*
* @param groupId => the group
* @param direction => positive for push, negative for pop.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
void RecordStateChange
(
int groupId,
int direction
)
    {
    int parityChange = ReadAndToggleParity (groupId);
    //GEOMAPI_PRINTF ("   (g d p %d %d %d)\n", groupId, direction, parityChange);
    if (parityChange == 0)
        {
        }
    else if (groupId <= highestGroupA)
        {
        countA += parityChange;
        }
    else if (groupId <= highestGroupB)
        {
        countB += parityChange;
        }
    else
        {
        countC += parityChange;
        }
    }
};


static bool BinaryClassification (RGBoolSelect selector, bool inA, bool inB, int totalCount, int countedIntersectionTrigger)
    {
    bool classification = true;
    switch (selector)
        {
        case RGBoolSelect_Union:
            classification = inA || inB;
            break;

        case RGBoolSelect_Intersection:
            classification = inA && inB;
            break;

        case RGBoolSelect_Difference:
            classification = inA && !inB;
            break;

        case RGBoolSelect_Parity:
            classification = (inA != inB);
            break;

        case RGBoolSelect_CountedIntersection:
            classification = totalCount >= countedIntersectionTrigger;
            break;

        case RGBoolSelect_GlobalParity:
            classification = (totalCount & 0x01) != 0;
            break;
        case RGBoolSelect_Left:
            classification = inA;
            break;
        }
    return classification;
    }
/*------------------------------------------------------------------*//**
* Evaluate the current face state in the markup.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static bool        markup_evaluateState
(
SimpleBooleanMarkupContext *pMarkup
)
    {
    bool    classification = false;
    bool    inA = pMarkup->countA > 0;
    bool    inB = pMarkup->countB > 0;
    bool    inC = pMarkup->countC > 0;
    int countAB = pMarkup->countA + pMarkup->countB;
    int countABC = countAB + pMarkup->countC;
    classification = BinaryClassification (pMarkup->selectorAB, inA, inB, countAB, pMarkup->highestGroupA);
    // Remark: global parity and counted intersection don't make sense for selectorC.
    classification = BinaryClassification (pMarkup->selectorC, classification, inC, countABC, pMarkup->highestGroupA);
    if (pMarkup->reverseSense)
        classification = (classification != 0) ? 0 : 1;
    return classification;
    }


static char const * markup_stringName
(
int event
)
    {
    switch (event)
        {
        case RGFloodEvent_EnterComponent:
            return "    EnterComponent ";
        case RGFloodEvent_EnterFace:
            return "       Enter Face: ";
        case RGFloodEvent_ExitFace:
            return "       Exit Face: ";
        case RGFloodEvent_ExitComponent:
            return "    ExitComponent ";
        default:
            return "   ??? Unknown markup event ";
        }
    }

/*------------------------------------------------------------------*//**
* Callback to handle face entry and exit.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void markup_announce
(
RG_Header *pRG,
RGFloodEvent event,
EmbeddedIntArray *pStack,
SimpleBooleanMarkupContext *pMarkup,
int noisy
)
    {
    MTGNodeId currNodeId;
    int count;
    if (noisy > 0)
        {
        count = jmdlEmbeddedIntArray_getCount (pStack);
        currNodeId = MTG_NULL_NODEID;
        if (count > 0)
            jmdlEmbeddedIntArray_getInt (pStack, &currNodeId, -1);
        GEOMAPI_PRINTF("    %s count %d topNode %d\n",
                    markup_stringName (event), count, currNodeId);

        }
    }

/*------------------------------------------------------------------*//**
* Callback to handle face entry and exit.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void markup_extendBooleanEvaluation
(
RG_Header *pRG,
RGFloodEvent event,
EmbeddedIntArray *pStack,
SimpleBooleanMarkupContext *pMarkup,
int noisy
)
    {
    MTGNodeId currNodeId;
    int groupId;
    jmdlEmbeddedIntArray_getInt(pStack, &currNodeId, -1);
    markup_announce (pRG, event, pStack, pMarkup, noisy);
    /* Synch the groupid stack */
    switch (event)
        {
        case RGFloodEvent_EnterComponent:
            /* No action -- this node is outside. */
            break;
        case RGFloodEvent_EnterFace:
            if (jmdlRG_getGroupId (pRG, &groupId, currNodeId))
                {
                pMarkup->RecordStateChange (groupId, 1);
                }
            break;
        case RGFloodEvent_ExitFace:
            if (jmdlRG_getGroupId (pRG, &groupId, currNodeId))
                {
                pMarkup->RecordStateChange (groupId, -1);
                }
            break;
        case RGFloodEvent_ExitComponent:
            // Check if group parity array is all zeros?
            break;
        }

    if (   currNodeId != MTG_NULL_NODEID
        && event == RGFloodEvent_EnterFace
        && markup_evaluateState (pMarkup)
        )
        {
        jmdlMTGMarkSet_addNode (pMarkup->pMarkSet, currNodeId);
        }
    }

/*------------------------------------------------------------------*//**
* Check a particular face for regularization into or out of mark set.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void regularizeFace
(
RG_Header *pRG,
MTGNodeId nodeIdA,
MTG_MarkSet *pMarkSet,
double areaTol
)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);

    MTGNodeId nodeIdB = jmdlMTGGraph_getFSucc (pGraph, nodeIdA);
    MTGNodeId nodeIdC = jmdlMTGGraph_getFSucc (pGraph, nodeIdB);

    // If this is a 2-node face AND we are at the lower numbered node ....
    if (nodeIdA == nodeIdC && nodeIdA < nodeIdB)
        {
        MTGNodeId nodeIdAv = jmdlMTGGraph_getVSucc (pGraph, nodeIdA);
        MTGNodeId nodeIdBv = jmdlMTGGraph_getVSucc (pGraph, nodeIdB);
        int switchDir = 0;
        if (!jmdlMTGMarkSet_isNodeInSet (pMarkSet, nodeIdA))
            {
            if (  jmdlMTGMarkSet_isNodeInSet (pMarkSet, nodeIdAv)
               && jmdlMTGMarkSet_isNodeInSet (pMarkSet, nodeIdBv))
                switchDir = 1;
            }
        else
            {
            if (  !jmdlMTGMarkSet_isNodeInSet (pMarkSet, nodeIdAv)
               && !jmdlMTGMarkSet_isNodeInSet (pMarkSet, nodeIdBv))
                    switchDir = -1;
            }

        if (switchDir != 0)
            {
            double area;
            // To a debugger in the future.  We're going to make the decision
            //   based on actual area.  However, the null face mask is probably valid.
            //   You can probably just check the mask instead of going through the
            //   area calculation.
            // (Do you care?  A mask check is typically much faster than an area calculation,
            //    but the topological conditions allow only a small number of faces
            //    to reach this far, and those always have only two edges, so the
            //    compute time is almost irrelevant.   The choice is really matter
            //    of how sure the caller is that the mask is set.)
            //**MTGMask nullMask =
            //**    jmdlMTGGraph_getMask (pRG->pGraph, nodeIdA, RG_MTGMASK_NULL_FACE);
            jmdlRG_getFaceSweepProperties (pRG, &area, NULL, NULL, nodeIdA);
            if (fabs (area) < areaTol)
                {
                if (switchDir < 0)
                    jmdlMTGMarkSet_removeNode (pMarkSet, nodeIdA);
                if (switchDir > 0)
                    jmdlMTGMarkSet_addNode (pMarkSet, nodeIdA);
                }
            }
        }
    }

/*------------------------------------------------------------------*//**
* Search for zero-area faces which can be added or removed
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
void regularizeMarkSet
(
RG_Header *pRG,
MTG_MarkSet *pMarkSet
)
    {
    double distanceTol = jmdlRG_getTolerance (pRG);
    double areaTol;
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    DRange3d range;
    jmdlRG_getRange (pRG, &range);
    // A face with width distanceTol and length of diagonal is near zero ...
    areaTol = range.low.Distance (range.high) * distanceTol;
    MTGARRAY_SET_LOOP (nodeIdA, pGraph)
        {
        regularizeFace (pRG, nodeIdA, pMarkSet, areaTol);
        }
    MTGARRAY_END_SET_LOOP (nodeIdA, pGraph)
    }

/*------------------------------------------------------------------*//**
* Find the maximum group id in the graph.
* Initialize parity array to zero for each group id from 0 to max.
* @return false if there are edges with no groupid.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static bool        markup_initGroupParityArray
(
RG_Header               *pRG,
SimpleBooleanMarkupContext *pMarkup
)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    MTGMask edgeVisited = jmdlMTGGraph_grabMask (pGraph);
    bool    boolstat = true;
    int groupId;
    int maxGroupId = -1;


    jmdlMTGGraph_clearMaskInSet (pGraph, edgeVisited);
    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, edgeVisited))
            {
            jmdlMTGGraph_setMaskAroundEdge (pGraph, seedNodeId, edgeVisited);
            if (jmdlRG_getGroupId (pRG, &groupId, seedNodeId))
                {
                if (groupId > maxGroupId)
                    maxGroupId = groupId;
                }
            else
                {
                boolstat = false;
                }
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pGraph)
    jmdlMTGGraph_dropMask (pGraph, edgeVisited);

    if (boolstat)
        {
        pMarkup->InitGroupParityArray (maxGroupId + 1);
        }

    return boolstat;
    }

void printNodeXY
(
MTGGraph *pGraph,
MTGNodeId nodeId,
RG_Header   *pRG
)
    {
    DPoint3d xyz0, xyz1, delta;
    int groupId;
    if (    jmdlRG_getVertexDPoint3d (pRG, &xyz0, nodeId)
        &&  jmdlRG_getVertexDPoint3d (pRG, &xyz1, jmdlMTGGraph_getFSucc (pGraph, nodeId))
       )
        {
        delta.DifferenceOf (xyz1, xyz0);
        double degrees = Angle::RadiansToDegrees(Angle::Atan2 (delta.y, delta.x));
        double length = delta.MagnitudeXY ();
        if (jmdlRG_getGroupId (pRG, &groupId, nodeId))
            GEOMAPI_PRINTF (" %2dG",groupId);
        GEOMAPI_PRINTF (" %lf %lf (%.17g @ %.17g)", xyz0.x, xyz0.y, length, degrees);
        }
    }

/*------------------------------------------------------------------*//**
* Collect a markset with faces that are "in" according to
* one of several bool    operations, with boundaries distinguished
* by groupId value attached to the curve of each edge.
* @param highestGroupA => highest group id which is considered
*           part of the "left" operand.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRG_collectBooleanFaces
(
RG_Header           *pRG,
RGBoolSelect        selectAB,
RGBoolSelect        selectC,
int                 highestOperandA,
int                 highestOperandB,
MTG_MarkSet         *pMarkSet,
bool                reverseSense
)
    {
    SimpleBooleanMarkupContext context (selectAB, selectC, pMarkSet, highestOperandA, highestOperandB, reverseSense);
    static bool    s_noisy = false;
    if (s_noisy)
        jmdlRG_setNoisy (1000);  
    bool    result = false;

    jmdlMTGMarkSet_empty (pMarkSet);
    if (s_noisy)
        jmdlMTGGraph_printFaceLoopsExt (pRG->pGraph, (MTGNodeFunc)printNodeXY, pRG);
    if (markup_initGroupParityArray (pRG, &context))
        {
        jmdlRG_buildBridgeEdges (pRG);

        if (s_noisy)
            jmdlMTGGraph_printFaceLoopsExt (pRG->pGraph, (MTGNodeFunc)printNodeXY, pRG);


        result = markup_floodFromAllSeeds (pRG, markup_extendBooleanEvaluation, &context);
        jmdlRG_dropBrideEdges (pRG);
        regularizeMarkSet (pRG, pMarkSet);
        }


    if (s_noisy)
        jmdlMTGGraph_printFaceLoopsExt (pRG->pGraph, (MTGNodeFunc)printNodeXY, pRG);
    if (s_noisy)
        jmdlRG_setNoisy (0);          
    return result;
    }
    
Public bool     jmdlRG_collectBooleanFaces
(
RG_Header           *pRG,
RGBoolSelect        select,
int                 highestOperandA,
MTG_MarkSet         *pMarkSet,
bool                reverseSense
)
    {
    return jmdlRG_collectBooleanFaces (pRG, select, RGBoolSelect_Left, highestOperandA, INT_MAX, pMarkSet, reverseSense);
    }
/*------------------------------------------------------------------*//**
* @doctext
*<h1>Boolean Searches in Merged Regions</h1>
*<h2>External Usage</h2>
*<ul>
*   <li>Each region is identified by a unique "groupId"</li>
*   <li>The groupId is stored with the curve table, accessible through jmdlRG_getGroupId</li>
*   <li>Each region is potentially multiple components with holes.</li>
*   <li>Within each region, parity rules are to be applied to resolve
*       holes and crossing boundaries.</li>
*   <li>For binary conditions, there is are implicit unions of "primary" and "secondary" regions,
*               (distinguished by low and high group ids)</li>
*   <li>While boundary geometry into curve tables (e.g. rimsbs)
*       as usual, except with the small additional step of setting up
*       the groupId equal to the region number.
*       (With RIMSBS as the curve table manager, call
*       jmdlRIMSBS_setCurrentGroupId(pContext, componentNumber)
*       before starting each component.)</li>
*   <li>Merge the graph and create hole arrays as usual.</li>
*   <li>
*       Booleans supported are:
*       <ul>
*           <li>RGBoolSelect_Union -- Union of all regions.</li>
*           <li>RGBoolSelect_Intersection -- Primary intersect secondary.</li>
*           <li>RGBoolSelect_Difference -- primary minus secondary.</li>
*           <li>RGBoolSelect_Parity -- primary exclusiveOr with secondary.</li>
*           <li>RGBoolSelect_CountedIntersection -- Intersection of at least {highestGroupA} regions</li>
*           <li>RGBoolSelect_GlobalParity -- Parity among all regions.</li>
*       </ul>
*       </li>
*   <li>Call
*       <code>jmdlRG_collectBooleanFaces (pRG, select, highestGroupA, pMarkSet)</code>
*       to generate the mark set for the bool.</li>
*</ul>
+---------------+---------------+---------------+---------------+------*/

/*------------------------------------------------------------------*//**
* Walk around the given face looking across edges for a mate
* marked as a hole edge.  Return first found, or MTG_NULL_NODEID if no such found.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public MTGNodeId  jmdlRG_stepOutOfHole
(
RG_Header *pRG,
MTGNodeId seedNodeId
)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    MTGNodeId mateNodeId;
    if (jmdlRG_faceIsHoleLoop (pRG, seedNodeId))
        return MTG_NULL_NODEID;

    MTGARRAY_FACE_LOOP (currNodeId, pGraph, seedNodeId)
        {
        mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
        if (jmdlRG_faceIsHoleLoop (pRG, mateNodeId))
            {
            return mateNodeId;
            }
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, seedNodeId)
    return MTG_NULL_NODEID;
    }


/*------------------------------------------------------------------*//**
* Return the entire face-hole node array in flattened form with MTG_NULL_NODEID separators:
*   outerA,innerA1,innerA2,MTG_NULL_NODEID,outerB,innerB1,.....
* @param pRG IN region header
* @param pLoopArray OUT array to receive node sequences.
* @param pMarkSet IN optional mark set.
* @param bTargetMarkState IN true/false to collect only if the outer face is in/notIn the mark set.
* @param bIncludeMarkedSimpleLoops IN true to add singleton entries (nodeID+MTG_NULL_NODEID) for all
*    faces that do not have holes.
* @bsihdr                                       EarlinLutz      06/08
+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_collectOuterAndInnerFaces
(
RG_Header *pRG,
EmbeddedIntArray *pLoopArray,
MTG_MarkSet *pMarkSet,
bool    bTargetMarkState,
bool    bIncludeMarkedSimpleLoops
)
    {
    // Collect outer and inner loops as a flat list.
    MTGNodeId outerNodeId0 = MTG_NULL_NODEID;
    bool    bOuterNodeActive = false;
    MTGNodeId outerNodeId, holeNodeId;
    int outerCount = 0;
    int i;
    jmdlEmbeddedIntArray_empty (pLoopArray);
    // For each (outer,inner) pair ...
    for (i = 0;
               jmdlEmbeddedIntArray_getInt (pRG->pFaceHoleNodeIdArray, &outerNodeId, i)
            && jmdlEmbeddedIntArray_getInt (pRG->pFaceHoleNodeIdArray, &holeNodeId, i + 1);
        i += 2)
        {
        // Change of outerNodeId triggers check if active ..
        if (outerNodeId0 != outerNodeId)
            {
            bOuterNodeActive = true;
            if (NULL != pMarkSet)
                {
                bOuterNodeActive = jmdlMTGMarkSet_isNodeInSet (pMarkSet, outerNodeId);
                if (!bTargetMarkState)
                    bOuterNodeActive = !bOuterNodeActive;
                }

            outerNodeId0 = outerNodeId;
            if (bOuterNodeActive)
                {
                if (outerCount != 0)
                    jmdlEmbeddedIntArray_addInt (pLoopArray, MTG_NULL_NODEID);
                outerCount++;
                jmdlEmbeddedIntArray_addInt (pLoopArray, outerNodeId);
                }
            }
        if (bOuterNodeActive)
            jmdlEmbeddedIntArray_addInt (pLoopArray, holeNodeId);
        }
    if (outerCount != 0)
        jmdlEmbeddedIntArray_addInt (pLoopArray, MTG_NULL_NODEID);

    if (pMarkSet != NULL && bIncludeMarkedSimpleLoops)
        {
        MTGGraph *pGraph = jmdlRG_getGraph (pRG);
        MTGMask visitMask = jmdlMTGGraph_grabMask (pGraph);
        int iteratorIndex;
        MTGNodeId seedNodeId;
        int ib;
        jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
        // Set the mask on the faces already identify by the hole pairing ...
        for (ib = 0; jmdlEmbeddedIntArray_getInt (pLoopArray, &seedNodeId, ib); ib++)
            {
            if (seedNodeId != MTG_NULL_NODEID)
                jmdlMTGGraph_setMaskAroundFace (pGraph, seedNodeId, visitMask);
            }

        jmdlMTGMarkSet_initIteratorIndex (pMarkSet, &iteratorIndex);
        while (jmdlMTGMarkSet_getNextNode (pMarkSet, &iteratorIndex, &seedNodeId))
            {
            if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, visitMask))
                {
                jmdlMTGGraph_setMaskAroundFace (pGraph, seedNodeId, visitMask);
                jmdlEmbeddedIntArray_addInt (pLoopArray, seedNodeId);
                jmdlEmbeddedIntArray_addInt (pLoopArray, MTG_NULL_NODEID);
                }
            }
        }
    }

//! "flood" from a seed node to "reachable" faces.
//! @param [in] barrierMask mask for uncrossable edges.  This is tested on both sides.
//! @param [inout] pMarkSet face set already accepted.   Flooding will not reenter these faces.  Flood faces are added to this set.
//! @param [inout] addedNodes New nodes are added here.
//! @param [in] seedNode
void jmdlMTG_floodToBoundary (MTGGraph *graph, 
MTG_MarkSet &markset,
MTGMask barrierMask,
MTGNodeId seedNodeId
)
    {
    bvector<MTGNodeId> seedNodes;
    seedNodes.push_back (seedNodeId);
    while (seedNodes.size () > 0)
        {
        MTGNodeId faceNodeId = seedNodes.back ();
        seedNodes.pop_back ();
        if (!markset.IsNodeInSet (faceNodeId))
            {
            markset.AddNode (faceNodeId);
            MTGARRAY_FACE_LOOP (currNodeId, graph, faceNodeId)
                {
                if (!graph->GetMaskAt (currNodeId, barrierMask))
                    {
                    MTGNodeId mateId = graph->EdgeMate (currNodeId);
                    if (!graph->GetMaskAt (mateId, barrierMask)
                        && !markset.IsNodeInSet (mateId))
                        {
                        seedNodes.push_back (mateId);
                        }
                    }
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, graph, faceNodeId)
            }
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE
