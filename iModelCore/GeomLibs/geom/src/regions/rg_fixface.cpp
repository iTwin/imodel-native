/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "Regions/rg_intern.h"
#include <stdio.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


static int s_noisy = 0;

/*------------------------------------------------------------------*//**
*
* @param vertexMask => mask known to be clear at all vertices around
*       this face.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static int      jmdlRG_countDistinctVertexLoopsAroundFace
(
MTGGraph            *pGraph,
MTGNodeId           faceNodeId,
MTGMask             vertexMask
)
    {
    int count = 0;
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, faceNodeId)
        {
        if (!jmdlMTGGraph_getMask (pGraph, currNodeId, vertexMask))
            {
            count++;
            jmdlMTGGraph_setMaskAroundVertex (pGraph, currNodeId, vertexMask);
            }
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, faceNodeId)

    MTGARRAY_FACE_LOOP (currNodeId, pGraph, faceNodeId)
        {
        jmdlMTGGraph_clearMaskAroundVertex (pGraph, currNodeId, vertexMask);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, faceNodeId)
    return count;
    }

/*------------------------------------------------------------------*//**
*
* @param vertexMask => mask known to be clear at all mate faces adjacent to
*       this face.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static int      jmdlRG_countDistinctMateFaceLoopsAroundFace
(
MTGGraph            *pGraph,
MTGNodeId           faceNodeId,
MTGMask             workMask
)
    {
    int count = 0;
    MTGNodeId mateNodeId;
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, faceNodeId)
        {
        mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
        if (!jmdlMTGGraph_getMask (pGraph, mateNodeId, workMask))
            {
            count++;
            jmdlMTGGraph_setMaskAroundFace (pGraph, mateNodeId, workMask);
            }
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, faceNodeId)

    MTGARRAY_FACE_LOOP (currNodeId, pGraph, faceNodeId)
        {
        mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
        jmdlMTGGraph_clearMaskAroundFace (pGraph, mateNodeId, workMask);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, faceNodeId)
    return count;
    }

typedef struct
    {
    int vertexId0;  /* Lower number */
    int vertexId1;  /* Higher number */
    int nodeId;
    } EdgeDescr;

int compareVertexId
(
const EdgeDescr *pDescr0,
const EdgeDescr *pDescr1
)
    {
    if (pDescr0->vertexId0 < pDescr1->vertexId0)
        return -1;
    if (pDescr0->vertexId0 > pDescr1->vertexId0)
        return 1;
    if (pDescr0->vertexId1 < pDescr1->vertexId1)
        return -1;
    if (pDescr0->vertexId1 > pDescr1->vertexId1)
        return 1;
    return 0;
    }
/*------------------------------------------------------------------*//**
* Search (entire) face loop starting at given node for a node at given start
* vertex.
+---------------+---------------+---------------+---------------+------*/
static MTGNodeId   findIndexedVertexIdInFaceLoop
(
MTGGraph            *pGraph,
EmbeddedIntArray    *pNodeIdToVertexIdArray,
MTGNodeId           seedNodeId,
int                 targetVertexId
)
    {
    int currVertexId;
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, seedNodeId)
        {
        jmdlEmbeddedIntArray_getInt (pNodeIdToVertexIdArray, &currVertexId, currNodeId);
        if (currVertexId == targetVertexId)
            return currNodeId;

        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, seedNodeId)
    return MTG_NULL_NODEID;
    }
static double vertexExitAngle
(
RG_Header           *pRG,
MTGNodeId           nodeId
)
    {
    DPoint3d xyz[2];
    double theta;
    jmdlRG_getVertexData (pRG, xyz, 1, NULL, nodeId, 0.0);
    theta = Angle::Atan2 (xyz[1].y, xyz[1].x);
    return theta;
    }

static double edgeLength
(
RG_Header           *pRG,
MTGNodeId           nodeId
)
    {
    RG_EdgeData edgeData;
    double length;
    jmdlRG_getEdgeData (pRG, &edgeData, nodeId);
    length = edgeData.xyz[0].Distance (*(&edgeData.xyz[1]));
    return length;
    }


/*------------------------------------------------------------------*//**
* Check spherical-face conditions on a single vertex from a single outgoing masked node.
* @param nodesOnActiveFace => collection of nodes from the "original" face.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static bool    checkMaskedVertex
(
RG_Header           *pRG,
MTGNodeId           *pOutNodeId0,
MTGNodeId           *pOutNodeId1,
EmbeddedIntArray    *pNodeIdToVertexIdArray,
MTGNodeId           nodeId0,
MTG_MarkSet &nodesOnActiveFace
)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    int vertexId0;
    MTGNodeId nodeId1, nodeId2;
    /* Go around the face to the next node at the same vertex */
    jmdlEmbeddedIntArray_getInt (pNodeIdToVertexIdArray, &vertexId0, nodeId0);
    nodeId1 = findIndexedVertexIdInFaceLoop (pGraph, pNodeIdToVertexIdArray,
                jmdlMTGGraph_getFSucc (pGraph, nodeId0), vertexId0);
    if (nodeId1 != MTG_NULL_NODEID && nodeId1 != nodeId0)
        {
        /* And then come back towards the start node.looking for a mask of this face */
        nodeId2 = jmdlMTGGraph_findMaskAroundVertex (pGraph,
                        jmdlMTGGraph_getVSucc (pGraph, nodeId1), nodesOnActiveFace.GetMask ());
        if (nodeId2 == nodeId0)
            {
            }
        else
            {
            MTGNodeId nextNodeId;
            double currTheta, nextTheta;
            static double s_halfAngle = 1.0e-2;
            static double s_halfAngle_crossFace = 1.0e-4;
            double dtheta;
            double sweep = 2.0 * s_halfAngle;
            /* Find two departing edges, both of this face and with
                    nearby angles, and swap them to improve the parity */
            if (s_noisy > 2)
                {
                GEOMAPI_PRINTF (" Node sequence condition %d %d %d at vertex %d\n",
                            nodeId0, nodeId2, nodeId1, vertexId0);
                MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, nodeId1)
                    {
                    nextNodeId = jmdlMTGGraph_getVSucc (pGraph, currNodeId);
                    currTheta = vertexExitAngle (pRG, currNodeId);
                    nextTheta = vertexExitAngle (pRG, nextNodeId);
                    if (nodesOnActiveFace.IsNodeInSet (currNodeId))
                        GEOMAPI_PRINTF (" [%d]", currNodeId);
                    else
                        GEOMAPI_PRINTF (" %d", currNodeId);
                    GEOMAPI_PRINTF (" (theta %lf)", currTheta);
                    GEOMAPI_PRINTF (" (dtheta %lf)", bsiTrig_getNormalizedAngle (currTheta - nextTheta));
                    GEOMAPI_PRINTF (" (r %lf \n)", edgeLength (pRG, currNodeId));
                    }
                MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, nodeId1)
                }

            /* First pass - only consider swaps within face. */
            MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, nodeId1)
                {
                nextNodeId = jmdlMTGGraph_getVSucc (pGraph, currNodeId);
                currTheta = vertexExitAngle (pRG, currNodeId);
                nextTheta = vertexExitAngle (pRG, nextNodeId);

                if (   nodesOnActiveFace.IsNodeInSet (currNodeId)
                    // && nodesOnActiveFace.IsNodeInSet (currNodeId) *** NEEDS WORK STATIC ANALYSIS - redundant code - what did we mean to test here in addition to nodesOnActiveFace.IsNodeInSet?
                    && Angle::InSweepAllowPeriodShift (nextTheta, currTheta - s_halfAngle, sweep))
                    {
                    MTGNodeId backNodeId = jmdlMTGGraph_getVPred (pGraph, currNodeId);
                    jmdlMTGGraph_vertexTwist (pGraph, backNodeId, currNodeId);
                    jmdlMTGGraph_vertexTwist (pGraph, currNodeId, nextNodeId);
                    *pOutNodeId0 = currNodeId;
                    *pOutNodeId1 = nextNodeId;
                    if (s_noisy)
                        GEOMAPI_PRINTF (" Swapping %d %d %lf %lf\n",
                                    currNodeId, nextNodeId, currTheta, nextTheta);
                    return true;
                    }
                }
            MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, nodeId1)

            /* Second pass -- out-of-face swap, but with tighter tolerance condition */
            MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, nodeId1)
                {
                nextNodeId = jmdlMTGGraph_getVSucc (pGraph, currNodeId);
                currTheta = vertexExitAngle (pRG, currNodeId);
                nextTheta = vertexExitAngle (pRG, nextNodeId);
                dtheta = fabs (bsiTrig_getNormalizedAngle (currTheta - nextTheta));
                if (dtheta < s_halfAngle_crossFace)
                    {
                    MTGNodeId backNodeId = jmdlMTGGraph_getVPred (pGraph, currNodeId);
                    jmdlMTGGraph_vertexTwist (pGraph, backNodeId, currNodeId);
                    jmdlMTGGraph_vertexTwist (pGraph, currNodeId, nextNodeId);
                    *pOutNodeId0 = currNodeId;
                    *pOutNodeId1 = nextNodeId;
                    if (s_noisy)
                        GEOMAPI_PRINTF (" Swapping %d %d %lf %lf\n",
                                    currNodeId, nextNodeId, currTheta, nextTheta);
                    return true;
                    }
                }
            MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, nodeId1)

            }
        }
    return false;
    }

/*------------------------------------------------------------------*//**
* Check spherical-face conditions on a single face.
* @param nodesOnActiveSet => node set that can be cleared and reused.
* @param workMask => mask guaranteed to be clear throughout graph on entry
*       and exit of this function.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static void      jmdlRG_fixFaceTopology
(
RG_Header           *pRG,
EmbeddedIntArray    *pNodeIdToVertexIdArray,
MTGNodeId           nodeId,
MTG_MarkSet       &nodesOnActiveFace,
MTGMask             workMask
)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    int numDoubleEdge, numSingleEdge, numVertex, numFace, numNode;
    nodesOnActiveFace.Clear ();
    nodesOnActiveFace.AddNodesInScope (nodeId, MTG_ScopeFace);
    MTGMask faceMask = nodesOnActiveFace.GetMask ();
    jmdlMTGGraph_countMateMasksAroundFace (&numDoubleEdge, &numSingleEdge, pGraph, nodeId, faceMask);
    numVertex     = jmdlRG_countDistinctVertexLoopsAroundFace (pGraph, nodeId, workMask);
    numFace       = jmdlRG_countDistinctMateFaceLoopsAroundFace (pGraph, nodeId, workMask);
    numNode = numSingleEdge + numDoubleEdge;

    if (numDoubleEdge > 0)
        {
        if (s_noisy)
            GEOMAPI_PRINTF ("(Node %d (edges %d) (vertexLoops %d) (adjacent faces %d) (doubleEdge %d))\n",
                        nodeId,
                        numNode,
                        numVertex,
                        numFace,
                        numDoubleEdge
                        );
        /* Find all the doubles ... */
        MTGARRAY_FACE_LOOP (nodeId0, pGraph, nodeId)
            {
            int numMasked, numUnMasked;
            jmdlMTGGraph_countMasksAroundVertex
                        (
                        pGraph,
                        &numMasked,
                        &numUnMasked,
                        nodeId0,
                        faceMask
                        );
            if (numMasked > 0)
                {
                MTGNodeId fixNodeId0, fixNodeId1;
                if (checkMaskedVertex (pRG,
                            &fixNodeId0,
                            &fixNodeId1,
                            pNodeIdToVertexIdArray, nodeId0, nodesOnActiveFace))
                    {
                    return;
                    }
                }
            }
        MTGARRAY_END_FACE_LOOP (nodeId0, pGraph, nodeId)

        }
    }

/*------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static void     jmdlRG_fixTwistedBananas
(
RG_Header           *pRG
)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    MTGNodeId nodeId1, nodeId2, nodeId3, nodeId4, nodeId5, nodeId6, nodeId7, nodeId3a;
    MTGARRAY_SET_LOOP (nodeId0, pGraph)
        {
        nodeId1 = jmdlMTGGraph_getFSucc (pGraph, nodeId0);
        nodeId2 = jmdlMTGGraph_getFSucc (pGraph, nodeId1);
        if (nodeId2 == nodeId0)
            {
            /* Degree 2 face -- probably no area, but not a problem */
            }
        else
            {
            nodeId3 = jmdlMTGGraph_getVPred (pGraph, nodeId1);
            nodeId3a = jmdlMTGGraph_getVSucc (pGraph, nodeId1);
            if (nodeId3 == nodeId3a)
                {
                /* vertex degree 2 or less */
                }
            else
                {
                nodeId4 = jmdlMTGGraph_getFSucc (pGraph, nodeId3);
                nodeId5 = jmdlMTGGraph_getVSucc (pGraph, nodeId4);
                nodeId6 = jmdlMTGGraph_getVSucc (pGraph, nodeId2);
                if (nodeId5 == nodeId0)
                    {
                    if (s_noisy)
                        {
                        GEOMAPI_PRINTF (" Twisted banana at node sequence %d %d %d %d\n",
                                nodeId0, nodeId1, nodeId3, nodeId4);
                        if (nodeId6 == nodeId4)
                            GEOMAPI_PRINTF (" 6:4 condition!!\n");
                        if (nodeId2 == nodeId4)
                            GEOMAPI_PRINTF (" 2:4 condition!!\n");
                        }
                    nodeId7 = jmdlMTGGraph_getVPred (pGraph, nodeId4);
                    jmdlMTGGraph_vertexTwist (pGraph, nodeId4, nodeId7);
                    jmdlMTGGraph_vertexTwist (pGraph, nodeId4, nodeId0);
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (nodeId0, pGraph)
    }

/*------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static void     jmdlRG_fixSlings
(
RG_Header           *pRG,
EmbeddedIntArray    *pNodeIdToVertexIdArray
)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    MTGNodeId nodeId1;
    int vertexId0, vertexId1;

    MTGARRAY_SET_LOOP (nodeId0, pGraph)
        {
        nodeId1 = jmdlMTGGraph_getFSucc (pGraph, nodeId0);
        jmdlEmbeddedIntArray_getInt (pNodeIdToVertexIdArray, &vertexId0, nodeId0);
        jmdlEmbeddedIntArray_getInt (pNodeIdToVertexIdArray, &vertexId1, nodeId1);
        if (vertexId0 == vertexId1)
            {
            if (s_noisy)
                GEOMAPI_PRINTF (" Same vertex condition nodes %d %d\n", nodeId0, nodeId1);
            }
        }
    MTGARRAY_END_SET_LOOP (nodeId0, pGraph)
    }


/*------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_checkFaces
(
RG_Header           *pRG,
EmbeddedIntArray    *pNodeIdToVertexIdArray
)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    MTGMask vertexMask = jmdlMTGGraph_grabMask (pGraph);
    MTGMask visitMask = jmdlMTGGraph_grabMask (pGraph);
    MTGMask allMasks = vertexMask | visitMask;
    MTG_MarkSet activeFaceNodes (pGraph, MTG_ScopeNode);  // These 'should' be within a face, but the 
                                                           // point of the fixup is to revise face structures, so we
                                                           // can't trust face loop logic within the markset.
    jmdlMTGGraph_clearMaskInSet (pGraph, allMasks);

    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, currNodeId, visitMask))
            {
            jmdlMTGGraph_setMaskAroundFace (pGraph, currNodeId, visitMask);
            jmdlRG_fixFaceTopology (pRG, pNodeIdToVertexIdArray, currNodeId, activeFaceNodes, vertexMask);
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)

    jmdlMTGGraph_dropMask (pGraph, vertexMask);
    jmdlMTGGraph_dropMask (pGraph, visitMask);
    }

/*------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static int     jmdlRG_countTopology
(
RG_Header           *pRG,
char const          *pMessage,
int                 noisy
)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);

    int numVertex = (int)pGraph->CountVertexLoops ();
    int numFace = (int)pGraph->CountFaceLoops ();
    bvector<bvector<MTGNodeId>> component;
    pGraph->CollectConnectedComponents (component);
    int numComponent = (int)component.size ();
    int numEdge = (int)(pGraph->GetActiveNodeCount () / 2);

    int numHole = numComponent - (numVertex - numEdge + numFace) / 2;

    if (noisy)
        {
        GEOMAPI_PRINTF ("%s", pMessage);
        GEOMAPI_PRINTF (" (V %d)\n", numVertex);
        GEOMAPI_PRINTF (" (E %d)\n", numEdge);
        GEOMAPI_PRINTF (" (F %d)\n", numFace);
        GEOMAPI_PRINTF (" (C %d)\n", numComponent);
        GEOMAPI_PRINTF (" (H %d)\n", numHole);
        }
    return numHole;
    }

/*------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRG_fixTopology
(
RG_Header           *pRG
)
    {
    // This function seeks some known special cases of ordering and adjacency errors that result from
    // numerical fuzz in merging.
    // The release form only executes the tests if _countTopology reports that there are holes in the graph.
    // Note that these holes are 3D holes (donut holes!!) -- not merely holes in an area.
    // The test suite does not have cases that exhibit problems.  Hence the bulk of this file does not get executed.
    // Setting (at compile time) s_checkAll to true triggers all the tests and gets file coverage up to 71% of 210 lines.
    // (
    EmbeddedIntArray *pNodeIdToVertexIdArray = jmdlEmbeddedIntArray_grab ();
    int numHole;

    static int s_checkAll = false;
    numHole = jmdlRG_countTopology (pRG, "PRE-FIXUP", s_noisy);
    if (numHole > 0 || s_checkAll)
        {
        if (s_noisy)
            jmdlRG_countTopology (pRG, "PRE-FIXUP", 0);
        jmdlRG_checkFaces (pRG, pNodeIdToVertexIdArray);
        jmdlRG_fixSlings (pRG, pNodeIdToVertexIdArray);
        jmdlRG_fixTwistedBananas (pRG);
        jmdlRG_countTopology (pRG, "POST-FIXUP", s_noisy);
        }


    jmdlEmbeddedIntArray_drop (pNodeIdToVertexIdArray);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
