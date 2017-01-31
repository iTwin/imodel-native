/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/unfold.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
#include <Mtg/unfold.fdf> // This is wrong...should not include from msj!
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/
static int      s_noisy = 0;

/*----------------------------------------------------------------------+
|                                                                       |
|   Public Global variables                                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   External variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|    Macro Definitions                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Private Type Definitions                                            |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*//**
* Initialize a transform with x along specified edge, z with given normal,
* and y perpendicular.
* @param pTransform <= the transform (optional)
* @param pInverse <= inverse transform (optional; must be different from pTransform)
* @param edgeNodeId => base of edge.
* @param pZVector => vector to serve as z column of the transform.
* @param normalizeXY => true to have x, y directions unit vectors.  If false,
*           both are sized to the length of the edge.  The zvector is always left
*           unchanged.
* @return true if the matrix is invertible.
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGFold_alignTransformToEdgeAndNormal
(
MTGFacets       *pFacets,
Transform      *pTransform,
Transform      *pInverse,
MTGNodeId       tailNodeId,
const DPoint3d  *pNormal,
bool            normalizeXY
)
    {
    DPoint3d tailXYZ, headXYZ;
    DPoint3d edgeVector;
    DPoint3d yVector;
    Transform transform, inverse;
    bool    inverted = false;

    MTGNodeId headNodeId = jmdlMTGGraph_getFSucc (&pFacets->graphHdr, tailNodeId);
    if (   jmdlMTGFacets_getNodeCoordinates (pFacets, &tailXYZ, tailNodeId)
        && jmdlMTGFacets_getNodeCoordinates (pFacets, &headXYZ, headNodeId)
        )
        {
        if (normalizeXY)
            {
            edgeVector.NormalizedDifference (headXYZ, tailXYZ);
            yVector.NormalizedCrossProduct (*pNormal, edgeVector);
            }
        else
            {
            double edgeLength;
            edgeVector.DifferenceOf (headXYZ, tailXYZ);
            edgeLength = edgeVector.Magnitude ();
            bsiDPoint3d_sizedCrossProduct (&yVector, pNormal, &edgeVector, edgeLength);
            }

        bsiTransform_initFromOriginAndVectors (&transform, &tailXYZ, &edgeVector, &yVector, pNormal);
        inverted = inverse.InverseOf (transform);
        if (pTransform)
            *pTransform = transform;
        if (pInverse)
            *pInverse = inverse;
        }
    return inverted;
    }

/*----------------------------------------------------------------------*//**
* Initialize a transform with x along specified edge, y in the plane containing the
* farthest point of the face.
* @param pTransform <= the transform (optional)
* @param pInverse <= inverse transform (optional; must be different from pTransform)
* @param edgeNodeId => base of edge.
* @param normalizeXY => true to have all directions of the transfor normalized.
*       If false, all columns are the length of the edge.
* @return true if the matrix is invertible.
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGFold_alignTransformToFace
(
MTGFacets       *pFacets,
Transform       *pTransform,
Transform       *pInverse,
MTGNodeId       tailNodeId,
bool            normalizeXY
)
    {
    DPoint3d tailXYZ, headXYZ, currXYZ, farVector, closeXYZ;
    DPoint3d xVector, yVector, zVector;
    Transform transform, inverse;
    bool    inverted = false;
    double ddMax, ddCurr;
    MTGNodeId currNodeId;


    MTGNodeId headNodeId = jmdlMTGGraph_getFSucc (&pFacets->graphHdr, tailNodeId);
    if (   jmdlMTGFacets_getNodeCoordinates (pFacets, &tailXYZ, tailNodeId)
        && jmdlMTGFacets_getNodeCoordinates (pFacets, &headXYZ, headNodeId)
        )
        {
        DRay3d edgeRay;
        double transformLength;
        bsiDRay3d_initFromDPoint3dStartEnd (&edgeRay, &tailXYZ, &headXYZ);
        /* Search for farthest node */
        for (ddMax = 0, currNodeId = jmdlMTGGraph_getFSucc (&pFacets->graphHdr, headNodeId);
                        currNodeId != tailNodeId;
                        currNodeId = jmdlMTGGraph_getFSucc (&pFacets->graphHdr, currNodeId))
            {
            jmdlMTGFacets_getNodeCoordinates (pFacets, &currXYZ, currNodeId);
            bsiDRay3d_projectPoint (&edgeRay, &closeXYZ, NULL, &currXYZ);

            ddCurr = closeXYZ.DistanceSquared (currXYZ);
            if (ddCurr > ddMax)
                {
                ddMax = ddCurr;
                farVector.DifferenceOf (currXYZ, closeXYZ);
                }
            }

        if (normalizeXY)
            {
            transformLength = 1.0;
            xVector.Normalize (edgeRay.direction);
            yVector.Normalize (farVector);
            zVector.NormalizedCrossProduct (xVector, yVector);
            }
        else
            {
            double edgeLength = edgeRay.direction.Magnitude ();
            xVector = edgeRay.direction;
            bsiDPoint3d_scaleToLength (&yVector, &yVector, edgeLength);
            bsiDPoint3d_sizedCrossProduct (&zVector, &xVector, &yVector, edgeLength);
            }

        bsiTransform_initFromOriginAndVectors (&transform, &tailXYZ, &xVector, &yVector, &zVector);
        inverted = inverse.InverseOf (transform);
        if (pTransform)
            *pTransform = transform;
        if (pInverse)
            *pInverse = inverse;
        }
    return inverted;
    }
#ifdef CompileAll
/*-------------------------------------------------------------------*//**
* Simultaneously walk around faces in a source and destination graph.
* For each destination node which has a null vertex index, create a new
* vertex and assign coordinates via a transformation of the corresponding
* the source graph vertex.
* @param pNormal = normal vector for dest facets (usually null)
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGFold_assignTransformedCoordinatesAroundFaces
(
MTGFacets   *pDestFacets,
const MTGFacets   *pSourceFacets,
const Transform  *pTransform,
MTGNodeId   sourceSeedNodeId,
MTGNodeId   destSeedNodeId,
const DPoint3d  *pNormal
)
    {
    const MTGGraph *pSourceGraph = &pSourceFacets->graphHdr;
    MTGGraph *pDestGraph = &pDestFacets->graphHdr;
    MTGNodeId sourceNodeId = sourceSeedNodeId;
    DPoint3d sourceXYZ, destXYZ;
    int sourceVertexOffset = pSourceFacets->vertexLabelOffset;
    int destVertexOffset   = pDestFacets->vertexLabelOffset;
    int sourceVertexIndex = -1;
    int destVertexIndex   = -1;

    MTGARRAY_FACE_LOOP (destNodeId, pDestGraph, destSeedNodeId)
        {
        if (   jmdlMTGGraph_getLabel (pDestGraph,   &destVertexIndex,
                                                destNodeId, destVertexOffset)
            && jmdlMTGGraph_getLabel (pSourceGraph, &sourceVertexIndex,
                                                sourceNodeId, sourceVertexOffset)
            && jmdlMTGFacets_getNodeCoordinates (pSourceFacets, &sourceXYZ, sourceNodeId)
            && destVertexIndex < 0
            )
            {
            pTransform->Multiply (&destXYZ, &sourceXYZ, 1);
            destVertexIndex = jmdlMTGFacets_addVertex (pDestFacets, &destXYZ, NULL);
            }
        sourceNodeId = jmdlMTGGraph_getFSucc (pSourceGraph, sourceNodeId);
        }
    MTGARRAY_END_FACE_LOOP (destNodeId, pDestGraph, destSeedNodeId)
    return true;
    }
#endif
/* Pending nodes are recorded in single linked lists.
   Both the tail and head of the SLL are recorded in node pairs,
   so insertion is easy at either end, described as "base" and "top".
   This allows constant time insertion at either end, constant time
    removal at the top.

   In the "pState" nodeId pairs, nodeId[0] is the base, nodeId[1] is the top.
*/

/*-------------------------------------------------------------------*//**
* Init the array used for storing multiple stack pointers.
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static void jmdlMTGMultiStack_initArray
(
EmbeddedIntArray      *pArray,
const MTGGraph  *pGraph
)
    {
    int numNode = jmdlMTGGraph_getNodeIdCount (pGraph);
    jmdlEmbeddedIntArray_setConstant (pArray, MTG_NULL_NODEID, numNode);
    }

/*-------------------------------------------------------------------*//**
* Init the head and tail pointers for a single stack.
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static void jmdlMTGMultiStack_initStack
(
MTGNodeIdPair   *pState
)
    {
    pState->nodeId[0] = pState->nodeId[1] = MTG_NULL_NODEID;
    }

#ifdef CompileAll
/*-------------------------------------------------------------------*//**
* Add at the base of a stack.
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGMultiStack_addAtBase
(
EmbeddedIntArray      *pArray,
MTGNodeIdPair   *pState,
MTGNodeId       nodeId
)
    {
    MTGNodeId oldLinkNodeId;
    bool    status;
    if (s_noisy)
        printf ("  AddAtBase %d ", nodeId);

    if (!jmdlEmbeddedIntArray_getInt (pArray, &oldLinkNodeId, nodeId))
        {
        /* Bad node index */
        if (s_noisy)
            printf ("  (Bad node -- reject)\n");
        status = false;
        }
    else if (oldLinkNodeId != MTG_NULL_NODEID)
        {
        /* Node already enqueued */
        if (s_noisy)
            printf ("  (already in queue -- reject)\n");
        status = false;
        }
    else if (pState->nodeId[0] == MTG_NULL_NODEID)
        {
        /* Empty queue.  Simple insert. */
        pState->nodeId[0] = pState->nodeId[1] = nodeId;
        if (s_noisy)
            printf ("  (insert to empty queue)\n");
        status = true;
        }
    else
        {
        MTGNodeId oldBaseNodeId = pState->nodeId[0];
        if (s_noisy)
            printf ("  (insert)\n");
        /* Link from nodeId to the old tail ... */
        jmdlEmbeddedIntArray_setInt (pArray, nodeId, oldBaseNodeId);
        /* And change the tail: */
        pState->nodeId[0] = nodeId;
        status = true;
        }
    return status;
    }
#endif
/*-------------------------------------------------------------------*//**
* Add at the top of a stack.
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGMultiStack_addAtTop
(
EmbeddedIntArray      *pArray,
MTGNodeIdPair   *pState,
MTGNodeId       nodeId,
int             tag
)
    {
    MTGNodeId oldLinkNodeId;
    bool    status;

    if (s_noisy)
        printf ("  AddAtTop of (%d) %d ", tag, nodeId);

    if (!jmdlEmbeddedIntArray_getInt (pArray, &oldLinkNodeId, nodeId))
        {
        /* Bad node index */
        if (s_noisy)
            printf ("  (Bad node -- reject)\n");
        status = false;
        }
    else if (oldLinkNodeId != MTG_NULL_NODEID)
        {
        /* Node already enqueued */
        if (s_noisy)
            printf ("  (already in queue -- reject)\n");
        status = false;
        }
    else if (pState->nodeId[1] == MTG_NULL_NODEID   )
        {
        /* Empty queue.  Simple insert. */
        pState->nodeId[0] = pState->nodeId[1] = nodeId;
        if (s_noisy)
            printf ("  (insert to empty queue)\n");
        status = true;
        }
    else
        {
        MTGNodeId oldTopNodeId = pState->nodeId[1];
        if (s_noisy)
            printf ("  (insert)\n");
        /* Link from old head to nodeId ... */
        jmdlEmbeddedIntArray_setInt (pArray, oldTopNodeId, nodeId);
        /* And change the head: */
        pState->nodeId[1] = nodeId;
        status = true;
        }
    return status;
    }

/*-------------------------------------------------------------------*//**
* Pop from a stack.
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static bool        jmdlMTGMultiStack_pop
(
EmbeddedIntArray      *pArray,
MTGNodeIdPair   *pState,
MTGNodeId       *pNodeId
)
    {
    MTGNodeId nodeId = pState->nodeId[1];
    MTGNodeId nextNodeId;

    if (nodeId == MTG_NULL_NODEID)
        {
        /* Empty stack ... */
        }
    else
        {
        if (jmdlEmbeddedIntArray_getInt (pArray, &nextNodeId, nodeId))
            {
            jmdlEmbeddedIntArray_setInt (pArray, MTG_NULL_NODEID, nodeId);
            pState->nodeId[1] = nextNodeId;
            if (nextNodeId == MTG_NULL_NODEID)
                {
                pState->nodeId[0] = MTG_NULL_NODEID;
                }
            }
        else
            {
            if (s_noisy)
                printf ("Bad Pop ???\n");
            /* This shouldn't happen ... */
            jmdlMTGMultiStack_initStack (pState);
            }
        }
    if (s_noisy)
        printf ("Pop %d\n", nodeId);

    return (*pNodeId = nodeId) != MTG_NULL_NODEID;
    }

#define NUM_FOLD_STACK 3
typedef struct
    {
      MTGFacets     *pSourceFacets;
      MTGFacets     *pDestFacets;
      MTGGraph      *pSourceGraph;          /* exactly &pSourceFacets->graphHdr */
      MTGGraph      *pDestGraph;            /* exactly &pDestFacets->graphHdr */
      EmbeddedIntArray    *pSourceToDestArray;
      EmbeddedIntArray    *pSourceQueues;
      MTGNodeIdPair stackLimits[NUM_FOLD_STACK];
      MTGMask       visitMask;
      DPoint3d      destOrigin;
      DPoint3d      destDir;
      DPoint3d      destNormal;
      MTGMask       newLeftMask;
      MTGMask       newRightMask;

      int sourceComponentIndexLabelOffset;
      int sourceInternalIndexLabelOffset;
      int destComponentIndexLabelOffset;
      int destInternalIndexLabelOffset;
    } MTGFoldContext;

/*-------------------------------------------------------------------*//**
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static bool        jmdlMTGFold_initContext
(
      MTGFoldContext *pContext,
      MTGFacets     *pSourceFacets,
      MTGFacets     *pDestFacets,
      MTGMask       visitMask,
const DPoint3d      *pDestOrigin,
const DPoint3d      *pDestDir,
const DPoint3d      *pDestNormal
)
    {
    int i;
    pContext->pSourceFacets = pSourceFacets;
    pContext->pDestFacets   = pDestFacets;
    pContext->pSourceGraph = &pSourceFacets->graphHdr;
    pContext->pDestGraph = &pDestFacets->graphHdr;
    pContext->pSourceToDestArray = jmdlEmbeddedIntArray_grab ();
    pContext->pSourceQueues = jmdlEmbeddedIntArray_grab ();
    /*
        Source nodeId's are queued for processing.  Several
            disjoint queues are maintained within this array;
            high/low priority queueing allows the unfolding to
            proceed on primary and secondary directions in quad grids.
            Each queue has first, last nodeId's as a node pair.
    */
    jmdlMTGMultiStack_initArray (pContext->pSourceQueues, pContext->pSourceGraph);

    for (i = 0; i < NUM_FOLD_STACK; i++)
        jmdlMTGMultiStack_initStack (&pContext->stackLimits[i]);

    pContext->visitMask = visitMask;
    pContext->newLeftMask    = MTG_PRIMARY_EDGE_MASK | MTG_EXTERIOR_MASK;
    pContext->newRightMask   = MTG_PRIMARY_EDGE_MASK | MTG_EXTERIOR_MASK;
    pContext->destOrigin    = *pDestOrigin;
    pContext->destDir       = *pDestDir;
    pContext->destNormal    = *pDestNormal;

    pContext->sourceComponentIndexLabelOffset
                = jmdlMTGGraph_getLabelOffset (pContext->pSourceGraph, MTGReservedLabelTag_ComponentIndex);
    pContext->sourceInternalIndexLabelOffset
                = jmdlMTGGraph_getLabelOffset (pContext->pSourceGraph, MTGReservedLabelTag_InternalIndex);
    pContext->destComponentIndexLabelOffset
                = jmdlMTGGraph_getLabelOffset (pContext->pDestGraph, MTGReservedLabelTag_ComponentIndex);
    pContext->destInternalIndexLabelOffset
                = jmdlMTGGraph_getLabelOffset (pContext->pDestGraph, MTGReservedLabelTag_InternalIndex);
    return true;
    }

/*-------------------------------------------------------------------*//**
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static void    jmdlMTGFold_releaseMem
(
      MTGFoldContext *pContext
)
    {
    jmdlEmbeddedIntArray_drop (pContext->pSourceQueues);
    jmdlEmbeddedIntArray_drop (pContext->pSourceToDestArray);
    }

/*-------------------------------------------------------------------*//**
* Pop from (multiple) stacks until an unvisited seed node is found.
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static bool        jmdlMTGFold_popSeed
(
      MTGFoldContext *pContext,
      MTGNodeId     *pSeedNodeId
)
    {
    MTGNodeId sourceNodeId;
    int stackId;
    MTGMask skipMask = pContext->visitMask | MTG_EXTERIOR_MASK;
    *pSeedNodeId = MTG_NULL_NODEID;

    for (stackId = 0; stackId < NUM_FOLD_STACK; stackId++)
        {
        while (jmdlMTGMultiStack_pop
                    (pContext->pSourceQueues, &pContext->stackLimits[stackId], &sourceNodeId))
            {
            if (!jmdlMTGGraph_getMask (pContext->pSourceGraph, sourceNodeId, skipMask))
                {
                *pSeedNodeId = sourceNodeId;
                return true;
                }
            }
        }
    return false;
    }

/*-------------------------------------------------------------------*//**

* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static bool        jmdlMTGFold_closeDestFace
(
MTGFoldContext  *pContext,
MTGNodeId       sourceNodeId0,
MTGNodeId       destNodeId0,
Transform    *pTransform
)
    {
    MTGNodeId destNodeId1 = jmdlMTGGraph_getFSucc (pContext->pDestGraph, destNodeId0);
    MTGNodeId sourceNodeId1 = jmdlMTGGraph_getFSucc (pContext->pSourceGraph, sourceNodeId0);
    MTGNodeId sourceNodeId2, destNodeId2;
    MTGNodeId sourceMateId;
    MTGNodeId newDestNodeId1, newDestNodeId2;
    int     destVertexIndex1, destVertexIndex2;
    int     nullNormalIndex = -1;
    DPoint3d sourceXYZ, destXYZ2, vector01, vector12, destXYZ1;
    static int s_pushOrder = 2;
    bool    pushAtEnd = true;

    /* For each edge of the original face, add a corresponding edge to the dest facets: */
    for (sourceNodeId2 = jmdlMTGGraph_getFSucc (pContext->pSourceGraph, sourceNodeId1),
         jmdlMTGFacets_getNodeVertexIndex (pContext->pDestFacets, &destVertexIndex1, destNodeId1);
         sourceNodeId1 != sourceNodeId0;
         sourceNodeId1 = sourceNodeId2,
         sourceNodeId2 = jmdlMTGGraph_getFSucc (pContext->pSourceGraph, sourceNodeId1),
         destNodeId1 = newDestNodeId2,
         destVertexIndex1 = destVertexIndex2
         )
        {
        jmdlMTGFacets_getNodeCoordinates (pContext->pSourceFacets, &sourceXYZ,      sourceNodeId2);
        jmdlMTGFacets_getNodeCoordinates (pContext->pSourceFacets, &destXYZ1,    destNodeId1);
        if (sourceNodeId2 == sourceNodeId0)
            {
            destNodeId2 = destNodeId0;
            jmdlMTGFacets_getNodeVertexIndex
                    (pContext->pDestFacets, &destVertexIndex2, destNodeId0);
            }
        else
            {
            destNodeId2 = MTG_NULL_NODEID;
            destXYZ2 = sourceXYZ;
            pTransform->Multiply (destXYZ2);
            destVertexIndex2 = jmdlMTGFacets_addVertex (pContext->pDestFacets, &destXYZ2, NULL);
            }

        jmdlMTGFacets_addIndexedEdge (pContext->pDestFacets,
                        &newDestNodeId1, &newDestNodeId2,
                        destNodeId1, destNodeId2,
                        pContext->newLeftMask, pContext->newRightMask,
                        destVertexIndex1, destVertexIndex2,
                        nullNormalIndex,  nullNormalIndex);

        sourceMateId = jmdlMTGGraph_getEdgeMate (pContext->pSourceGraph, sourceNodeId1);
        jmdlEmbeddedIntArray_setInt (pContext->pSourceToDestArray, newDestNodeId2, sourceMateId);
        if (s_pushOrder == 0)
            {
            int stackId = 0;
            if (jmdlMTGGraph_getMask (pContext->pSourceGraph, sourceMateId, MTG_EXTERIOR_MASK))
                stackId = 1;
            jmdlMTGMultiStack_addAtTop (pContext->pSourceQueues,
                                    &pContext->stackLimits[stackId],
                                    sourceMateId,
                                    stackId);
            pushAtEnd = false;
            }
        else if (s_pushOrder == 1)
            {
            int stackId = 0;
            vector01.DifferenceOf (destXYZ2, pContext->destOrigin);
            vector12.DifferenceOf (destXYZ1, destXYZ2);
            if (jmdlMTGGraph_getMask (pContext->pSourceGraph, sourceMateId, MTG_EXTERIOR_MASK))
                {
                stackId = 2;
                }
            else if (bsiDPoint3d_tripleProduct (&vector01, &vector12, &pContext->destNormal) <= 0.0)
                {
                stackId = 1;
                }

            jmdlMTGMultiStack_addAtTop (pContext->pSourceQueues,
                                    &pContext->stackLimits[stackId],
                                    sourceMateId,
                                    stackId);
            pushAtEnd = false;
            }
        }

    /* All destination edges are created as exterior.  Move this face to interior. */
    jmdlMTGGraph_clearMaskAroundFace (pContext->pDestGraph, destNodeId0, MTG_EXTERIOR_MASK);

    if (pushAtEnd)
        {
        /* Net preference:
                Stack 0: Neighbors of quad source, farthest from start first.
                Stack 1: Neighbors of non-quad source
                Stack 2: exterior
        */
        MTGNodeId leftNodeId, rightNodeId;
        MTGNodeId savedNodeId;
        int stackId = 0;
        int defaultStackId = 1;
        int primaryStackId = 0;
        if (jmdlMTGGraph_countNodesAroundFace (pContext->pSourceGraph, sourceNodeId0) == 4)
            defaultStackId = 1;
        for (savedNodeId = sourceNodeId0,
             leftNodeId  = jmdlMTGGraph_getFPred (pContext->pSourceGraph, sourceNodeId0),
             rightNodeId = jmdlMTGGraph_getFSucc (pContext->pSourceGraph, sourceNodeId0);
            leftNodeId != rightNodeId && leftNodeId != savedNodeId;
            savedNodeId = rightNodeId,
            leftNodeId = jmdlMTGGraph_getFPred (pContext->pSourceGraph, leftNodeId),
            rightNodeId = jmdlMTGGraph_getFSucc (pContext->pSourceGraph, rightNodeId))
            {

            sourceMateId = jmdlMTGGraph_getEdgeMate (pContext->pSourceGraph, leftNodeId);
            stackId = defaultStackId;
            if (jmdlMTGGraph_getMask (pContext->pSourceGraph, sourceMateId, MTG_EXTERIOR_MASK))
                stackId = 2;
            jmdlMTGMultiStack_addAtTop (pContext->pSourceQueues,
                                        &pContext->stackLimits[stackId],
                                        sourceMateId,
                                        stackId);

            sourceMateId = jmdlMTGGraph_getEdgeMate (pContext->pSourceGraph, rightNodeId);
            stackId = defaultStackId;
            if (jmdlMTGGraph_getMask (pContext->pSourceGraph, sourceMateId, MTG_EXTERIOR_MASK))
                stackId = 2;
            jmdlMTGMultiStack_addAtTop (pContext->pSourceQueues,
                                        &pContext->stackLimits[stackId],
                                        sourceMateId,
                                        stackId);
            }
        if (leftNodeId == rightNodeId)
            {
            sourceMateId = jmdlMTGGraph_getEdgeMate (pContext->pSourceGraph, leftNodeId);
            stackId = primaryStackId;
            if (jmdlMTGGraph_getMask (pContext->pSourceGraph, sourceMateId, MTG_EXTERIOR_MASK))
                stackId = 2;
            jmdlMTGMultiStack_addAtTop (pContext->pSourceQueues,
                                        &pContext->stackLimits[stackId],
                                        sourceMateId,
                                        stackId);
            }
        }
    return true;
    }

static void copyLabelsAroundFace
(
MTGFoldContext *pContext,
MTGNodeId      sourceNodeId0,
int sourceLabelOffset,
MTGNodeId       destNodeId0,
int destLabelOffset
)
    {
    if (sourceLabelOffset >= 0 && destLabelOffset >= 0
        && jmdlMTGGraph_countNodesAroundFace (pContext->pSourceGraph, sourceNodeId0)
            == jmdlMTGGraph_countNodesAroundFace (pContext->pDestGraph, destNodeId0)
        )
        {
        // Concurrent walk around two face loops.
        MTGNodeId destNodeId = destNodeId0;
        MTGARRAY_FACE_LOOP (sourceNodeId, pContext->pSourceGraph, sourceNodeId0)
            {
            int destLabelValue;
            jmdlMTGGraph_getLabel (pContext->pSourceGraph, &destLabelValue, sourceNodeId, sourceLabelOffset);
            jmdlMTGGraph_setLabel (pContext->pDestGraph, destNodeId, destLabelOffset, destLabelValue);
            destNodeId = jmdlMTGGraph_getFSucc (pContext->pDestGraph, destNodeId);
            }
        MTGARRAY_END_FACE_LOOP (sourceNodeId, pContext->pSourceGraph, sourceNodeId0)
        }
    }

/*-------------------------------------------------------------------*//**
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static bool        jmdlMTGFold_placeFace
(
MTGFoldContext *pContext,
MTGNodeId      seedNodeId
)
    {
    Transform inversePickupTransform, putdownTransform, product;
    bool    boolstat = false;
    MTGNodeId sourceNodeId0, destNodeId0, destNodeId1, destMateId;

    sourceNodeId0 = seedNodeId;

    if (jmdlEmbeddedIntArray_getInt (pContext->pSourceToDestArray, &destNodeId0, sourceNodeId0))
        {
        destNodeId1    = jmdlMTGGraph_getFSucc (pContext->pDestGraph, destNodeId0);
        destMateId      = jmdlMTGGraph_getVSucc (pContext->pDestGraph, destNodeId1 );
        if (   jmdlMTGFold_alignTransformToFace (pContext->pSourceFacets, NULL, &inversePickupTransform,
                                    sourceNodeId0, true)
            && jmdlMTGFold_alignTransformToEdgeAndNormal (pContext->pDestFacets, &putdownTransform, NULL,
                                    destNodeId0, &pContext->destNormal, true)
            )
            {
            product.InitProduct (putdownTransform, inversePickupTransform);

            jmdlMTGFold_closeDestFace (pContext, sourceNodeId0, destNodeId0, &product);
            jmdlMTGGraph_setMaskAroundFace (pContext->pSourceGraph, sourceNodeId0,
                        pContext->visitMask);
            if (jmdlMTGGraph_getMask (pContext->pSourceGraph, sourceNodeId0, MTG_EXTERIOR_MASK))
                jmdlMTGGraph_setMaskAroundFace (pContext->pDestGraph, destNodeId0, MTG_EXTERIOR_MASK);

            copyLabelsAroundFace (pContext,
                                sourceNodeId0, pContext->sourceComponentIndexLabelOffset,
                                destNodeId0, pContext->destComponentIndexLabelOffset);
            copyLabelsAroundFace (pContext,
                                sourceNodeId0, pContext->sourceInternalIndexLabelOffset,
                                destNodeId0, pContext->destInternalIndexLabelOffset);
            }
        }
    return boolstat;
    }

/*-------------------------------------------------------------------*//**
* Select a start node.  If a node is given, taken it unconditionally.
* If not, select any arbitrary unvisited node.
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static MTGNodeId  jmdlMTGFold_selectStartNode
(
MTGFoldContext  *pContext,
MTGNodeId       seedNodeId
)
    {
    MTGNodeId secondaryNodeId = MTG_NULL_NODEID;
    MTGNodeId mateNodeId;

    if (seedNodeId != MTG_NULL_NODEID)\
        return seedNodeId;

    MTGARRAY_SET_LOOP (currNodeId, pContext->pSourceGraph)
        {
        if (!jmdlMTGGraph_getMask (pContext->pSourceGraph, currNodeId, pContext->visitMask))
            {
            mateNodeId = jmdlMTGGraph_getEdgeMate (pContext->pSourceGraph, currNodeId);
            if (  jmdlMTGGraph_getMask (pContext->pSourceGraph, currNodeId, MTG_EXTERIOR_MASK)
               || jmdlMTGGraph_getMask (pContext->pSourceGraph, mateNodeId, MTG_EXTERIOR_MASK))
                {
                secondaryNodeId = currNodeId;
                }
            else
                return currNodeId;
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pContext->SourceGraph)
    return secondaryNodeId;
    }


/*-------------------------------------------------------------------*//**
* Place an initial edge into the destination graph, and load the
* recursion queues to continue.
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
static bool        jmdlMTGFold_placeInitialEdge
(
MTGFoldContext *pContext,
MTGNodeId     sourceNodeId0
)
    {
    DPoint3d sourceXYZ[2];
    DPoint3d destXYZ[2];
    MTGNodeId sourceNodeId1;
    MTGNodeId destNodeId0, destNodeId1;
    double edgeLength;
    DPoint3d destEdgeVector;
    bool    boolstat = false;

    sourceNodeId1 = jmdlMTGGraph_getEdgeMate(pContext->pSourceGraph, sourceNodeId0);
    if  (  !jmdlMTGGraph_getMask (pContext->pSourceGraph, sourceNodeId0, MTG_EXTERIOR_MASK)
        && jmdlMTGFacets_getNodeCoordinates (pContext->pSourceFacets, &sourceXYZ[0], sourceNodeId0)
        && jmdlMTGFacets_getNodeCoordinates (pContext->pSourceFacets, &sourceXYZ[1], sourceNodeId1)
        )
        {
        edgeLength = sourceXYZ[0].Distance (sourceXYZ[1]);
        bsiDPoint3d_scaleToLength (&destEdgeVector, &pContext->destDir, edgeLength);
        destXYZ[0] = pContext->destOrigin;
        destXYZ[1].SumOf (destXYZ[0], destEdgeVector);
        destNodeId0 = jmdlMTGFacets_addCoordinateChainExt (pContext->pDestFacets, destXYZ, NULL, 2);
        destNodeId1 = jmdlMTGGraph_getFSucc (pContext->pDestGraph, destNodeId0);
        jmdlEmbeddedIntArray_setInt (pContext->pSourceToDestArray, destNodeId0, sourceNodeId0);
        jmdlEmbeddedIntArray_setInt (pContext->pSourceToDestArray, destNodeId1, sourceNodeId1);

        /* If source side is interior push on high priority queue. */
        if (!jmdlMTGGraph_getMask (pContext->pSourceGraph, sourceNodeId0, MTG_EXTERIOR_MASK))
            jmdlMTGMultiStack_addAtTop (pContext->pSourceQueues, &pContext->stackLimits[0], sourceNodeId0, 0);

        if (!jmdlMTGGraph_getMask (pContext->pSourceGraph, sourceNodeId1, MTG_EXTERIOR_MASK))
            jmdlMTGMultiStack_addAtTop (pContext->pSourceQueues, &pContext->stackLimits[0], sourceNodeId1, 0);


        /* Mark dest edge exterior -- interior status is confered as faces are completed */
        jmdlMTGGraph_setMask (pContext->pDestGraph, destNodeId0, MTG_EXTERIOR_MASK);
        jmdlMTGGraph_setMask (pContext->pDestGraph, destNodeId1, MTG_EXTERIOR_MASK);

        boolstat = true;
        }
    return boolstat;
    }

/*-------------------------------------------------------------------*//**
* Search the source facets from a starting edge.  As each face is reached,
* add a transformed copy to the destination facets.
* @param destOrigin => origin for data in the destination facets.
* @param destDir => edge direction in the destination facets.
* @param destNormal => out-of-plane normal in destination facets.
* @param visitMask => mask to apply in source facets indicating nodes are visited.
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFold_appendUnfolded
(
      MTGFacets     *pSourceFacets,
      MTGFacets     *pDestFacets,
      MTGNodeId     seedNodeId,
const DPoint3d      *pDestOrigin,
const DPoint3d      *pDestDir,
const DPoint3d      *pDestNormal,
      MTGMask       visitMask
)
    {
    MTGNodeId currSeedNodeId;
    MTGFoldContext context;

    if (s_noisy)
        {
        printf ("\n\nSOURCE -- unfold from %d\n", seedNodeId);
        jmdlMTGFacets_printFaceLoops (pSourceFacets);
        jmdlMTGFacets_printVertexLoops (pSourceFacets);
        }

    jmdlMTGFold_initContext (&context, pSourceFacets, pDestFacets, visitMask,
                    pDestOrigin, pDestDir, pDestNormal);

    seedNodeId = jmdlMTGFold_selectStartNode (&context, seedNodeId);

    if (seedNodeId != MTG_NULL_NODEID)
        {
        jmdlMTGFold_placeInitialEdge (  &context, seedNodeId);

        for (;jmdlMTGFold_popSeed (&context, &currSeedNodeId);)
            {
            if (s_noisy > 1)
                {
                printf (" DESTINATION at pop seed %d\n", currSeedNodeId);
                jmdlMTGFacets_printFaceLoops (pDestFacets);
                }
            jmdlMTGFold_placeFace (&context, currSeedNodeId);
            }
        }
    jmdlMTGFold_releaseMem (&context);

    if (s_noisy)
        {
        printf (" FINAL UNFOLDED\n");
        jmdlMTGFacets_printFaceLoops (pDestFacets);
        jmdlMTGFacets_printVertexLoops (pDestFacets);
        }

    return true;
    }

static bool    getTailVerticesRange
(
MTGFacets   *pFacets,
DRange3d    *pRange,
int         numSkip
)
    {
    int numTotal = jmdlEmbeddedDPoint3dArray_getCount (&pFacets->vertexArrayHdr);
    bsiDRange3d_init (pRange);
    if (numTotal > numSkip)
        {
        bsiDRange3d_extendByDPoint3dArray (pRange,
                jmdlEmbeddedDPoint3dArray_getPtr (&pFacets->vertexArrayHdr, numSkip),
                numTotal - numSkip
                );
        return true;
        }
    return false;
    }

static bool    translateTailVertices
(
MTGFacets   *pFacets,
const DPoint3d  *pTranslation,
int         numSkip
)
    {
    int numTotal = jmdlEmbeddedDPoint3dArray_getCount (&pFacets->vertexArrayHdr);
    int i;
    DPoint3d *pBuffer = jmdlEmbeddedDPoint3dArray_getPtr (&pFacets->vertexArrayHdr, 0);

    for (i = numSkip; i < numTotal; i++)
        pBuffer[i].Add (*pTranslation);

    return numTotal > numSkip;
    }

/*-------------------------------------------------------------------*//**
* Unfold facets, using as many seed nodes as possible.
*   (Using jmdlMTGFold_appendUnfolded, which only unfolds from a single seed node.)
* @param destOrigin => origin for data in the destination facets.
* @param destDir => edge direction in the destination facets.
* @param destNormal => out-of-plane normal in destination facets.
* @param layoutSpace => global coordinates space between right of first placement
*           range box and left of second.
* @bsihdr                                       EarlinLutz      10/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFold_appendAllUnfolded
(
      MTGFacets     *pSourceFacets,
      MTGFacets     *pDestFacets,
const DPoint3d      *pDestOrigin,
const DPoint3d      *pDestDir,
const DPoint3d      *pDestNormal,
      double        layoutSpace
)
    {
    MTGGraph *pGraph = jmdlMTGFacets_getGraph (pSourceFacets);
    MTGMask visitMask = jmdlMTGGraph_grabMask (pGraph);
    int oldNumVertex;
    DRange3d newRange, compositeRange;
    DPoint3d newBasePoint;
    DPoint3d translation;
    DPoint3d zeroPoint;
    MTGMask visitOrExteriorMask = visitMask | MTG_EXTERIOR_MASK;
    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
    bsiDRange3d_init (&compositeRange);

    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, visitOrExteriorMask))
            {

            oldNumVertex = jmdlMTGFacets_getVertexCount (pDestFacets);
            zeroPoint.Zero ();
            jmdlMTGFold_appendUnfolded
                        (
                        pSourceFacets,
                        pDestFacets,
                        seedNodeId,
                        &zeroPoint,
                        pDestDir,
                        pDestNormal,
                        visitMask
                        );

            if (getTailVerticesRange (pDestFacets, &newRange, oldNumVertex))
                {
                if (oldNumVertex == 0)
                    {
                    if (pDestOrigin)
                        newBasePoint = *pDestOrigin;
                    else
                        newBasePoint.Zero ();
                    }
                else
                    {
                    newBasePoint.Init (
                                        compositeRange.high.x + layoutSpace,
                                        compositeRange.low.y,
                                        compositeRange.low.z
                                        );
                    }
                translation.DifferenceOf (newBasePoint, newRange.low);
                translateTailVertices (pDestFacets, &translation, oldNumVertex);
                newRange.low.Add (translation);
                newRange.high.Add (translation);
                bsiDRange3d_extendByRange (&compositeRange, &newRange);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pGraph)

    jmdlMTGGraph_dropMask (pGraph, visitMask);
    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
