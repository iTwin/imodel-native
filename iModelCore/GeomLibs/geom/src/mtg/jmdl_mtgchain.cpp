/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/jmdl_mtgchain.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <stdlib.h>
#include <limits.h>
#include "../DeprecatedFunctions.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
 typedef struct _sortKey
    {
    MTGNodeId   nodeId;
    int         pointIndex;
    double      dotValue;
    }MTG_SortKey;

struct MTG_LexicalSortKey
    {
    MTGNodeId   nodeId;
    int         lowVertexIndex;
    int         highVertexIndex;
    double      angle;
    // save vertex indices with possible reversal for (low, high) relationship !!!
    MTG_LexicalSortKey (MTGNodeId _nodeId, int vertexIndex1, int vertexIndex2)
        {
        nodeId = _nodeId;
        if (vertexIndex1 < vertexIndex2)
            {
            lowVertexIndex  = vertexIndex1;
            highVertexIndex = vertexIndex2;
            }
        else
            {
            lowVertexIndex  = vertexIndex2;
            highVertexIndex = vertexIndex1;
            }
        angle = 0.0;
        }
    };

static int sDebug = 0;
/*----------------------------------------------------------------------+
|FUNC           compareId                                               |
|AUTHOR         RaghavanKunigahalli                     8/96            |
+----------------------------------------------------------------------*/
static int compareId
(
const void    *pElem1,
const void    *pElem2
)
    {
    MTG_LexicalSortKey *pFirst = (MTG_LexicalSortKey*) pElem1;
    MTG_LexicalSortKey *pSecond = (MTG_LexicalSortKey*) pElem2;

    if (pFirst->lowVertexIndex < pSecond->lowVertexIndex)
        return  -1;
    else if (pFirst->lowVertexIndex > pSecond->lowVertexIndex)
        return  1;
    else
        {
        if (pFirst->highVertexIndex < pSecond->highVertexIndex)
            return  -1;
        else if (pFirst->highVertexIndex > pSecond->highVertexIndex)
            return  1;
        else
            return  0;
        }
    }

/*----------------------------------------------------------------------+
|FUNC           compareAngle                                            |
|AUTHOR         EarlinLutz                              12/96           |
+----------------------------------------------------------------------*/
static int compareAngle
(
const void    *pElem1,
const void    *pElem2
)
    {
    MTG_LexicalSortKey *pFirst = (MTG_LexicalSortKey*) pElem1;
    MTG_LexicalSortKey *pSecond = (MTG_LexicalSortKey*) pElem2;

    if (pFirst->angle < pSecond->angle)
        return  -1;
    else if (pFirst->angle > pSecond->angle)
        return  1;
    else
        {
        return 0;
        }
    }

/*----------------------------------------------------------------------+
|FUNC           compareDist                                             |
|AUTHOR         RaghavanKunigahalli                     8/96            |
+----------------------------------------------------------------------*/
static int compareDist
(
const void    *pElem1,
const void    *pElem2
)
    {
    MTG_SortKey *pFirst = (MTG_SortKey*) pElem1;
    MTG_SortKey *pSecond = (MTG_SortKey*) pElem2;

    return pFirst->dotValue < pSecond->dotValue ? -1: pFirst->dotValue == pSecond->dotValue? 0: 1;
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlMTGChain_samePoint                                  |
|AUTHOR         RaghavanKunigahalli                     8/96            |
+----------------------------------------------------------------------*/
static bool    jmdlMTGChain_samePoint
(
const DPoint3d *pPoint1,
const DPoint3d *pPoint2,
double epsilon
)
    {
    if ((fabs(pPoint1->x - pPoint2->x) <= epsilon)
        && (fabs(pPoint1->y - pPoint2->y) <= epsilon)
        && (fabs(pPoint1->z - pPoint2->z) <= epsilon))
        return  true;
    else
        return  false;
    }


/**
* @param pFacetHeader
* @param pNodeIdArray
* @param pVertexIdArray
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGChain_mergeNodes
(
MTGFacets *     pFacetHeader,
EmbeddedIntArray      *pNodeIdArray,
EmbeddedIntArray      *pVertexIdArray
)
    {
    int pivotNodeId, pivotVertexId, testNodeId, testVertexId;
    size_t i, j = 0, count = pNodeIdArray->size ();
    for (i = 0; j < count && i < count - 1;)
        {
        pivotVertexId = pVertexIdArray->at (i);
        pivotNodeId = pNodeIdArray->at (i);
        for (j = i + 1; i < j && j < count;)
            {
            testVertexId = pVertexIdArray->at (j);
            if (pivotVertexId == testVertexId)
                {
                testNodeId = pNodeIdArray->at (j);
                jmdlMTGGraph_vertexTwist ((jmdlMTGFacets_getGraph (pFacetHeader)), pivotNodeId, testNodeId);
                j++;
                }
            else
                {
                i = j;
                }
            }
        }
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlMTGChain_loadPivotInfo                                      |
|AUTHOR         RaghavanKunigahalli                     9/96            |
+----------------------------------------------------------------------*/
static void  jmdlMTGChain_loadPivotInfo
(
MTG_SortKey     **ppPivotKey,
const DPoint3d  **ppPivotPoint,
bvector <MTG_SortKey> *pDotArray,
MTGFacets *     pFacetHeader,
int             pivotIndex
)
    {
    *ppPivotKey = pDotArray->data () + (size_t) pivotIndex;
    *ppPivotPoint = pFacetHeader->vertexArrayHdr.data () + (size_t)(*ppPivotKey)->pointIndex;
    }

/**
Compute a vertex tolerance based on input tolerance and range of points
* @param pFacetHeader
* @param absTol
* @param relTol
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static double jmdlMTGChain_getTolerance
(
DPoint3dP pXYZArray,
int       numXYZ,
double absTol,
double relTol
)
    {
    static double s_defaultRelTol  = 1.0e-6;
    static double s_defaultAbsTol = 1.0e-14;

    if (absTol <= 0.0)
        absTol = 0.0;

    if (relTol <= 0.0)
        relTol = 0.0;

    if (absTol == 0.0 && relTol == 0.0)
        {
        relTol = s_defaultRelTol;
        absTol = s_defaultAbsTol;
        }

    double largestCoord = bsiDPoint3d_getLargestCoordinate(pXYZArray, numXYZ);
    return absTol + largestCoord * relTol;
    }


/**
* @param pFacetHeader
* @param pNodeIdArray
* @param pVertexIdArray
* @param applyMask  => true to apply the mask
* @param maskToApply
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGChain_identifyMatchedVerticesExt
(
MTGFacets * pFacetHeader,
EmbeddedIntArray  *pNodeIdArray,
EmbeddedIntArray  *pVertexIdArray,
bool        applyMask,
int         maskToApply,
double      absTol,
double      relTol
)
    {
    int label, vertexCount, currIndex, nodeCount = 0;
    double largestCoord, epsilon, epsilon2;
    bool    done;
    MTGNodeId vSuccId;
    DPoint3d    randomVector;
    const DPoint3d *pPivotPoint = NULL, *pTestPoint = NULL, *pCurrPoint = NULL;
    MTG_SortKey *pTestKey = NULL, *pPivotKey = NULL, sortInfo;

    static double s_defaultRelTol  = 1.0e-6;
    static double s_defaultAbsTol = 1.0e-14;

    if (absTol <= 0.0)
        absTol = 0.0;

    if (relTol <= 0.0)
        relTol = 0.0;

    if (absTol == 0.0 && relTol == 0.0)
        {
        relTol = s_defaultRelTol;
        absTol = s_defaultAbsTol;
        }

    
    bvector<int> nodeIdArray;
    bvector<int> vertexIdArray;

    vertexCount = (int)pFacetHeader->vertexArrayHdr.size ();

    largestCoord = bsiDPoint3d_getLargestCoordinate(pFacetHeader->vertexArrayHdr.data (), vertexCount);
    epsilon = absTol + largestCoord * relTol;
    epsilon2 = 1.1 * epsilon;
    randomVector.x = 0.133333333334;
    randomVector.y = 1.414213562373;
    randomVector.z = 1.666666666667;

    bvector <MTG_SortKey> dotArray;

    MTGARRAY_SET_LOOP (currNodeId, (jmdlMTGFacets_getGraph (pFacetHeader)))
        {
        if (!applyMask || jmdlMTGGraph_getMask ((jmdlMTGFacets_getGraph (pFacetHeader)), currNodeId, maskToApply))
            {
            nodeCount++;
            sortInfo.nodeId = currNodeId;
            jmdlMTGGraph_getLabel ((jmdlMTGFacets_getGraph (pFacetHeader)), &currIndex, currNodeId, pFacetHeader->vertexLabelOffset);
            sortInfo.pointIndex = currIndex;
            pCurrPoint = pFacetHeader->vertexArrayHdr.data () + (size_t)currIndex;
            sortInfo.dotValue = pCurrPoint->DotProduct (randomVector);
            dotArray.push_back (sortInfo);
            }

        if (applyMask && !jmdlMTGGraph_getMask ((jmdlMTGFacets_getGraph (pFacetHeader)), currNodeId, maskToApply))
            {
            done = false;
            vSuccId = jmdlMTGGraph_getVSucc ((jmdlMTGFacets_getGraph (pFacetHeader)), currNodeId);
            do
                {
                if (jmdlMTGGraph_getMask ((jmdlMTGFacets_getGraph (pFacetHeader)), vSuccId, maskToApply))
                    {
                    jmdlMTGGraph_getLabel ((jmdlMTGFacets_getGraph (pFacetHeader)), &label, vSuccId, pFacetHeader->vertexLabelOffset);
                    jmdlMTGGraph_setLabel ((jmdlMTGFacets_getGraph (pFacetHeader)), currNodeId, pFacetHeader->vertexLabelOffset, label);
                    done = true;
                    }
                vSuccId = jmdlMTGGraph_getVSucc ((jmdlMTGFacets_getGraph (pFacetHeader)), vSuccId);
                } while (!done || vSuccId != currNodeId);
            }

        }
    MTGARRAY_END_SET_LOOP (currNodeId, (jmdlMTGFacets_getGraph (pFacetHeader)))

    qsort (dotArray.data (), nodeCount, sizeof (MTG_SortKey),
                            compareDist);

    for (int pivotIndex = 0; pivotIndex < nodeCount; pivotIndex++)
        {
        jmdlMTGChain_loadPivotInfo(&pPivotKey, &pPivotPoint, &dotArray, pFacetHeader, pivotIndex);
        if (pPivotKey->nodeId != MTG_NULL_NODEID)
            {
            // The pivot is a new cluster ...
            nodeIdArray.push_back (pPivotKey->nodeId);
            vertexIdArray.push_back (pPivotKey->pointIndex);
            for (int testIndex = pivotIndex + 1; testIndex < nodeCount; testIndex++)
                {
                pTestKey = dotArray.data () + (size_t)testIndex;
                if (pTestKey->dotValue - pPivotKey->dotValue > epsilon2)
                    break;
                if (pTestKey->nodeId != MTG_NULL_NODEID)
                    {
                    pTestPoint = pFacetHeader->vertexArrayHdr.data () + (size_t)pTestKey->pointIndex;
                    if (jmdlMTGChain_samePoint(pPivotPoint, pTestPoint, epsilon))
                        {
                        // Get us in the same group with the pivot ...
                        nodeIdArray.push_back (pTestKey->nodeId);
                        vertexIdArray.push_back (pPivotKey->pointIndex);
                        pTestKey->nodeId = MTG_NULL_NODEID;
                        }
                    }
                }
            } // testIndex loop
        } // pivotIndex loop

    if (NULL != pNodeIdArray)
        pNodeIdArray->swap (nodeIdArray);
    if (NULL != pVertexIdArray)
        pVertexIdArray->swap (vertexIdArray);
    }


/**
* @param pFacetHeader   => facet header to examine.
* @param pNodeIdArray
* @param pVertexIdArray
* @param bool        applyMask
* @param int        maskToApply
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGChain_identifyMatchedVertices
(
MTGFacets * pFacetHeader,
EmbeddedIntArray  *pNodeIdArray,
EmbeddedIntArray  *pVertexIdArray,
bool        applyMask,
int         maskToApply
)
    {
    static double s_defaultAbsTol = 0.0;
    static double s_defaultRelTol = 1.0e-10;
    jmdlMTGChain_identifyMatchedVerticesExt
                (
                pFacetHeader,
                pNodeIdArray,
                pVertexIdArray,
                applyMask,
                maskToApply,
                s_defaultAbsTol,
                s_defaultRelTol
                );

    }

/**
* @param pFacetHeader    <= MTG Graph resulting from compression
*                                           due to coincinding points
* @param pSourcePoints array of alternating start and end points
* @param int        sourcePointCount         the number of points multiple of 2
* @param abstol
* @param relTol,
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_formChainGraphExt
(
MTGFacets * pFacetHeader,
DPoint3d    *pSourcePoints,
int         sourcePointCount,
double      abstol,
double      reltol
)
    {
    int id0, id1, i;
    bool    boolstat = true;

    for (i = 0; i < sourcePointCount; i+=2)
        {
        boolstat &= jmdlMTGGraph_createEdge ((jmdlMTGFacets_getGraph (pFacetHeader)), &id0, &id1);
        boolstat &= jmdlMTGGraph_setLabel ((jmdlMTGFacets_getGraph (pFacetHeader)), id0, pFacetHeader->vertexLabelOffset, i);
        boolstat &= jmdlMTGGraph_setLabel ((jmdlMTGFacets_getGraph (pFacetHeader)), id1, pFacetHeader->vertexLabelOffset, (i + 1));
        
        pFacetHeader->vertexArrayHdr.push_back (pSourcePoints[i]);
        pFacetHeader->vertexArrayHdr.push_back (pSourcePoints[i+1]);
        }

    if (boolstat)
        {
        bvector <int> nodeIdArray;
        bvector <int> vertexIdArray;
        jmdlMTGChain_identifyMatchedVerticesExt (pFacetHeader, &nodeIdArray, &vertexIdArray, 0, 0, abstol, reltol);
        jmdlMTGChain_mergeNodes (pFacetHeader, &nodeIdArray, &vertexIdArray);
        }

    return  boolstat;
    }

/**
* @param pFacetHeader    <= MTG Graph resulting from compression
*                                           due to coincinding points
* @param pSourcePoints array of alternating start and end points
* @param int        sourcePointCount         the number of points multiple of 2
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_formChainGraph
(
MTGFacets * pFacetHeader,
DPoint3d    *pSourcePoints,
int         sourcePointCount
)
    {
    static double s_defaultAbsTol = 0.0;
    static double s_defaultRelTol = 1.0e-10;
    return jmdlMTGGraph_formChainGraphExt (pFacetHeader, pSourcePoints, sourcePointCount,
                        s_defaultAbsTol, s_defaultRelTol);
    }

/**
* Delete parallel edges from the chain graph.
   Assumed input state -- vertex loops have been formed based on toleranced comparison.
   However, vertex INDICES have NOT been consolidated. "parallel edge" condition is to be
   detected purely by tolerancing.
* @param pFacetHeader   => facet header to examine.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void jmdlMTGChain_purgeDuplicateEdges
(
MTGFacets *  pFacets,
double       tolerance
)
    {
    MTGGraphP pGraph = jmdlMTGFacets_getGraph (pFacets);
    EmbeddedIntArray *pBaseNodeIdArray = jmdlEmbeddedIntArray_grab ();
    MTGMask vertexLoopMask =  jmdlMTGGraph_grabMask (pGraph);
    jmdlMTGGraph_clearMaskInSet (pGraph, vertexLoopMask);
    // Find one seed node at each vertex ....
    MTGARRAY_SET_LOOP (nodeIdA, pGraph)
        {
        if (!jmdlMTGGraph_getMask (jmdlMTGFacets_getGraph (pFacets), nodeIdA, vertexLoopMask))
            {
            jmdlMTGGraph_setMaskAroundVertex (pGraph, nodeIdA, vertexLoopMask);
            jmdlEmbeddedIntArray_empty (pBaseNodeIdArray);
            DPoint3d xyzB1, xyzC1;
            // Collect all the nodes around the loop ..
            MTGARRAY_VERTEX_LOOP (nodeIdB, pGraph, nodeIdA)
                {
                jmdlEmbeddedIntArray_addInt (pBaseNodeIdArray, nodeIdB);
                }
            MTGARRAY_END_VERTEX_LOOP (nodeIdB, pGraph, nodeIdA)
            int numNode     = jmdlEmbeddedIntArray_getCount (pBaseNodeIdArray);
            int *pBaseIndex = jmdlEmbeddedIntArray_getPtr (pBaseNodeIdArray, 0);

            for (int iB = 0; iB < numNode; iB++)
                {
                MTGNodeId nodeIdB0 = pBaseIndex[iB];
                // Skip if this edge was deleted earlier in the sweep ..
                if (nodeIdB0 == MTG_NULL_NODEID)
                    continue;
                MTGNodeId nodeIdB1 = jmdlMTGGraph_getEdgeMate (pGraph, nodeIdB0);
                jmdlMTGFacets_getNodeCoordinates (pFacets, &xyzB1, nodeIdB1);
                for (int iC = iB + 1; iC < numNode; iC++)
                    {
                    MTGNodeId nodeIdC0 = pBaseIndex[iC];
                    if (nodeIdC0 == MTG_NULL_NODEID)
                        continue;
                    MTGNodeId nodeIdC1 = jmdlMTGGraph_getEdgeMate (pGraph, nodeIdC0);
                    jmdlMTGFacets_getNodeCoordinates (pFacets, &xyzC1, nodeIdC1);
                    if (xyzB1.Distance (xyzC1) <= tolerance)
                        {
                        jmdlMTGGraph_dropEdge (pGraph, nodeIdC0);
                        pBaseIndex[iC] = MTG_NULL_NODEID;
                        }
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (vertexSeedNodeId, pGraph)
    jmdlMTGGraph_dropMask (pGraph, vertexLoopMask);
    jmdlEmbeddedIntArray_drop (pBaseNodeIdArray);
    }

/**
* Extracts multiple chains from an input MTG_graph
* @param pFacetHeader   => facet header to examine.
* @param pChainPoints <= polyline formed by chaining segments in PsourcePoints together
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGChain_extractAsGPA
(
GraphicsPointArray  *pChainPoints,
MTGFacets *  pFacetHeader
)
    {
    int             pass, chainPointIndex;
    bool            boolstat = true;
    MTGNodeId      chainNodeId;
    MTGMask         visitedMask;
    const DPoint3d  *pVertexPtr;

    visitedMask =  jmdlMTGGraph_grabMask ((jmdlMTGFacets_getGraph (pFacetHeader)));
    jmdlMTGGraph_clearMaskInSet ((jmdlMTGFacets_getGraph (pFacetHeader)), visitedMask);

    for (pass = 0;pass < 2 ;pass++ )
        {
        MTGARRAY_SET_LOOP (currNodeId, (jmdlMTGFacets_getGraph (pFacetHeader)))
            {
            if (!jmdlMTGGraph_getMask ((jmdlMTGFacets_getGraph (pFacetHeader)), currNodeId, visitedMask) && ( pass > 0 || currNodeId == jmdlMTGGraph_getVSucc ((jmdlMTGFacets_getGraph (pFacetHeader)), currNodeId)))
                {
                chainNodeId = currNodeId;

                do
                    {
                    jmdlMTGGraph_getLabel ((jmdlMTGFacets_getGraph (pFacetHeader)), &chainPointIndex, chainNodeId, pFacetHeader->vertexLabelOffset);
                    pVertexPtr = pFacetHeader->vertexArrayHdr.data () + (size_t)chainPointIndex;
                    jmdlGraphicsPointArray_addDPoint3d (pChainPoints, pVertexPtr);
                    jmdlMTGGraph_setMaskAroundEdge ((jmdlMTGFacets_getGraph (pFacetHeader)), chainNodeId, visitedMask);
                    chainNodeId = jmdlMTGGraph_getFSucc((jmdlMTGFacets_getGraph (pFacetHeader)), chainNodeId);
                    } while (
                           !jmdlMTGGraph_getMask ((jmdlMTGFacets_getGraph (pFacetHeader)), chainNodeId, visitedMask)
                            );

                jmdlMTGGraph_getLabel ((jmdlMTGFacets_getGraph (pFacetHeader)), &chainPointIndex, chainNodeId, pFacetHeader->vertexLabelOffset);
                pVertexPtr = pFacetHeader->vertexArrayHdr.data () + (size_t)chainPointIndex;
                jmdlGraphicsPointArray_addDPoint3d (pChainPoints, pVertexPtr);
                jmdlMTGGraph_setMask ((jmdlMTGFacets_getGraph (pFacetHeader)), chainNodeId, visitedMask);

                jmdlGraphicsPointArray_markBreak (pChainPoints);
                }
            }
        MTGARRAY_END_SET_LOOP (currNodeId, (jmdlMTGFacets_getGraph (pFacetHeader)))
        }

   jmdlMTGGraph_dropMask (jmdlMTGFacets_getGraph (pFacetHeader), visitedMask);
    return boolstat;
    }

/**
* Extracts multiple chains from an input array storing line
* segments in a sequential manner.
*NOTE:  The elements in matchPoints and scratchArea are utilized for
*storing and manipulating intermediate information during extraction
*process.
* @param pChainPoints <= polyline formed by chaining segments
                                            in PsourcePoints together
* @param pSourcePoints array of alternating start and end points
* @param int        sourcePointCount         the number of points multiple of 2
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGChain_segmentArrayToGPA
(
GraphicsPointArray  *pChainPoints,
DPoint3d    *pSourcePoints,
int         sourcePointCount
)
    {
    int             offset;
    bool            boolstat;
    MTGFacets *     pFacetHeader;

    pFacetHeader = jmdlMTGFacets_grab ();
    offset = jmdlMTGGraph_defineLabel ((jmdlMTGFacets_getGraph (pFacetHeader)), 0, MTG_LabelMask_VertexProperty, 0);
    pFacetHeader->vertexLabelOffset = offset;
    static double s_absTol = 0.0;
    static double s_relTol = 1.0e-10;

    double abstol = jmdlMTGChain_getTolerance (pSourcePoints, sourcePointCount, s_absTol, s_relTol);

    jmdlMTGGraph_formChainGraphExt (pFacetHeader, pSourcePoints, sourcePointCount, abstol, 0.0);
    jmdlMTGChain_purgeDuplicateEdges (pFacetHeader, abstol);
    boolstat = jmdlMTGChain_extractAsGPA (pChainPoints, pFacetHeader);

    jmdlMTGFacets_drop (pFacetHeader);
    return boolstat;

    }


template <typename T>
static bool TryGet (bvector <T> &source, T *data, int i)
    {
    size_t ii = (size_t)i;
    if (ii >= source.size ())
        return false;
    *data = source[i];
    return true;
    }

template <typename T>
static bool TryPop (bvector <T> &source, T &data)
    {
    if (source.size () == 0)
        return false;
    data = source.back ();
    return true;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGChain_flipMaskBit                                        |
|                                                                       |
| author        RaghavanKunigahalli                      01/97          |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGChain_flipMaskBit
(
MTGGraph *      pGraph,
MTGMask mask,
bvector <int> &visitedNodes,
MTGNodeId      nodeId
)
    {
    int visited;
    if (TryGet <int> (visitedNodes, &visited, nodeId) && visited == MTG_NOT_VISITED)
        {
        if (jmdlMTGGraph_getMask(pGraph, nodeId, mask))
            jmdlMTGGraph_clearMask(pGraph, nodeId, mask);
        else
            jmdlMTGGraph_setMask (pGraph, nodeId, mask);
        }
    }


/**
* @param pGraph
* @param mask
* @param startNodeId
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGChain_flipComponentOrientation
(
MTGGraph *      pGraph,
MTGMask mask,
MTGNodeId      startNodeId
)
    {
    int currComponentNumber = 1, visited, edgeMateId, fSuccId, stackTopId;
    size_t nodeIdCount = (size_t)jmdlMTGGraph_getNodeIdCount(pGraph);

    bvector<int> visitedNodes;
    for (size_t i = 0; i < nodeIdCount; i++)
        visitedNodes.push_back (MTG_NOT_VISITED);

    bvector<int> stack;

    MTGARRAY_VERTEX_LOOP (currVertId, pGraph, startNodeId)
        {
        jmdlMTGChain_flipMaskBit (pGraph, mask, visitedNodes, currVertId);
        stack.push_back (currVertId);
        visitedNodes[currVertId] = currComponentNumber;
        }
    MTGARRAY_END_VERTEX_LOOP (currVertId, pGraph, startNodeId)

    while (TryPop (stack, stackTopId))
        {
        edgeMateId = jmdlMTGGraph_getEdgeMate (pGraph, stackTopId);
        jmdlMTGChain_flipMaskBit (pGraph, mask, visitedNodes, edgeMateId);
        visitedNodes[edgeMateId] = currComponentNumber;
        if (jmdlMTGGraph_isValidNodeId (pGraph, (fSuccId = jmdlMTGGraph_getFSucc (pGraph, stackTopId))))
            {
            if (TryGet<int> (visitedNodes, &visited, fSuccId)
                && visited == MTG_NOT_VISITED)
                {
                MTGARRAY_VERTEX_LOOP (currVertId, pGraph, fSuccId)
                    {
                    jmdlMTGChain_flipMaskBit (pGraph, mask, visitedNodes, currVertId);
                    stack.push_back (currVertId);
                    visitedNodes[currVertId] = currComponentNumber;
                    }
                MTGARRAY_END_VERTEX_LOOP (currVertId, pGraph, fSuccId)
                }
            }
        }
    return  SUCCESS;
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlMTGChain_angleBetweenEdgeNeighborhoods                      |
|AUTHOR         EarlinLutz                              12/96           |
+----------------------------------------------------------------------*/
static double angleBetweenEdgeNeighborhoods
(
MTGFacets *             pFacetHeader,
MTG_LexicalSortKey      *pKey0,
MTG_LexicalSortKey      *pKey1,
bvector<DVec3d>         &vertexNormalArray
)
    {
    MTGNodeId node0Id  = pKey0->nodeId;
    MTGNodeId node1Id  = pKey1->nodeId;
    int baseVertex      = pKey0->lowVertexIndex;
    int endVertex       = pKey0->highVertexIndex;

    DPoint3d basePoint, endPoint;
    DVec3d normal0, normal1, tangent;
    if (   TryGet<DVec3d> (vertexNormalArray, &normal0, node0Id)
        && TryGet<DVec3d>  (vertexNormalArray, &normal1, node1Id)
        && TryGet<DPoint3d>  (pFacetHeader->vertexArrayHdr, &basePoint, baseVertex)
        && TryGet<DPoint3d>  (pFacetHeader->vertexArrayHdr, &endPoint, endVertex)
        )
        {
        tangent.DifferenceOf (endPoint, basePoint);
        return normal0.SignedAngleTo (normal1, tangent);
        }
    return 0.0;
    }

/**
* @param pFacetHeader
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGChain_twistEdgeStars
(
MTGFacets * pFacetHeader
)
    {
    MTGGraph *  pGraph = (jmdlMTGFacets_getGraph (pFacetHeader));
    int  currIndex1, currIndex2;
    size_t count = 0;
    MTGNodeId  fSuccId;
    MTG_LexicalSortKey  *pItem0, *pItem1;
    int vertexIdOffset = pFacetHeader->vertexLabelOffset;
    bool    boolstat = true;
    bvector < MTG_LexicalSortKey> sortArray;
    
    // Find all mtg nodes sitting at lower-numbered vertex of their edge.
    // Collect these as representatives of a face incident to the edge.
    MTGARRAY_SET_LOOP (currNodeId, (jmdlMTGFacets_getGraph (pFacetHeader)))
        {
        fSuccId = jmdlMTGGraph_getFSucc ((jmdlMTGFacets_getGraph (pFacetHeader)), currNodeId);
        jmdlMTGGraph_getLabel ((jmdlMTGFacets_getGraph (pFacetHeader)), &currIndex1, currNodeId, vertexIdOffset);
        jmdlMTGGraph_getLabel ((jmdlMTGFacets_getGraph (pFacetHeader)), &currIndex2, fSuccId, vertexIdOffset);
        if (currIndex1 < currIndex2)
            {
            count++;
            sortArray.push_back (MTG_LexicalSortKey (currNodeId, currIndex1, currIndex2));
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, (jmdlMTGFacets_getGraph (pFacetHeader)))

    qsort (sortArray.data (), (int)count, sizeof (MTG_LexicalSortKey), compareId);

    // Compute face normals for each face loop
    bvector <DVec3d> vertexNormalArray;
    bvector <DPoint3d>faceLoopArray;
    bvector<int> faceStartArray;

    size_t numNodeId = (size_t)jmdlMTGGraph_getNodeIdCount (jmdlMTGFacets_getGraph (pFacetHeader));
    vertexNormalArray.resize (numNodeId);
    DVec3d zeroVector = DVec3d::From (0,0,0);
    for (size_t i = 0; i < (size_t) numNodeId; i++)
        vertexNormalArray.push_back (zeroVector);

    jmdlMTGFacets_getGraph (pFacetHeader)->CollectFaceLoops (faceStartArray);
    const DPoint3d *pCoordinateArray;
    DVec3d normal;

    for (int startId : faceStartArray)
        {
        if (jmdlMTGFacets_getFaceCoordinates ((jmdlMTGFacets_getGraph (pFacetHeader)),
                        &faceLoopArray, (&pFacetHeader->vertexArrayHdr), startId, vertexIdOffset)
            )
            {
            pCoordinateArray = faceLoopArray.data ();
            size_t nVertex = faceLoopArray.size ();
            bsiGeom_polygonNormal (&normal, NULL, pCoordinateArray, (int)nVertex);
            MTGARRAY_FACE_LOOP (currNodeId, pGraph, startId)
                {
                vertexNormalArray[currNodeId] = normal;
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, startId)
            }
        }


    // Find blocks of nodes around common edges. Merge the edge stars.
    for (size_t i0 = 0, i1 = 0; i0 < count; i0 = i1)
        {
        // Find out how many successive nodes are along the same edge, and compute
        // their angle with respect to the edge ....
        pItem0 = sortArray.data () + i0;
        pItem0->angle = 0.0;
        for (i1 = i0 + 1; i1 < count; i1++)
            {
            pItem1 = sortArray.data () + i1;
            if (   pItem0->lowVertexIndex != pItem1->lowVertexIndex
                || pItem0->highVertexIndex != pItem1->highVertexIndex)
                {
                break;
                }
            pItem1->angle = angleBetweenEdgeNeighborhoods
                                                (
                                                pFacetHeader,
                                                pItem0,
                                                pItem1,
                                                vertexNormalArray
                                                );
            }

        size_t n = i1 - i0;
        if (n > 1)
            {
            // Sort by angle around the common edge ...
            qsort ((void*)pItem0, (int)n, sizeof (MTG_LexicalSortKey), compareAngle);
            size_t j;
            // And splice them all together.
            // REMARK: This assumes the edges were all dangling in the first place.
            // If not, they have to be spread apart?????
            for (j = i0 + 1; j < i1; j++)
                {
                // This could be lagged so only one lookup per pass...
                pItem0 = sortArray.data () + j - 1;
                pItem1 = sortArray.data () + j;
                jmdlMTGGraph_edgeTwist ((jmdlMTGFacets_getGraph (pFacetHeader)), pItem0->nodeId, pItem1->nodeId);
                }
            }
        }

    return boolstat;
    }




/**
* @param pGraph
* @param pMinLabel
* @param pMaxLabel
* @param offset
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGChain_findLabelRange
(
MTGGraph *  pGraph,
int         *pMinLabel,
int         *pMaxLabel,
int         offset
)
    {
    if (pMinLabel && pMaxLabel)
        {
        int currLabel;
        *pMaxLabel = INT_MIN;
        *pMinLabel = INT_MAX;
        MTGARRAY_SET_LOOP (currNodeId, pGraph)
            {
            jmdlMTGGraph_getLabel (pGraph, &currLabel, currNodeId, offset);
            if (currLabel > *pMaxLabel)
                *pMaxLabel = currLabel;
            if (currLabel < *pMinLabel)
                *pMinLabel = currLabel;
            }
        MTGARRAY_END_SET_LOOP (currNodeId, pGraph)
        }
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlMTGChain_renumberLabel                              |
|AUTHOR         RaghavanKunigahalli                     9/96            |
+----------------------------------------------------------------------*/
static bool    jmdlMTGChain_renumberLabel
(
MTGGraph *  pGraph,
EmbeddedIntArray  *pReferenceArray,
int         offset,
int         maxLabelAllowed
)
    {
    int newIndex, newCount=0;
    int minLabel, maxLabel, currLabel;

    jmdlMTGChain_findLabelRange (pGraph, &minLabel, &maxLabel, offset);
    if (maxLabel > maxLabelAllowed)
        return  false;

    pReferenceArray->clear ();
    //pReferenceArray->resize ((size_t)(maxLabel + 1));
    for (size_t i = 0; i < (size_t)(maxLabel + 1); i++)
        pReferenceArray->push_back (-1);

    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        jmdlMTGGraph_getLabel (pGraph, &currLabel, currNodeId, offset);
        if (currLabel >= 0)
            {
            newIndex = pReferenceArray->at ((size_t)currLabel);
            if (newIndex == -1)
                {
                newIndex = newCount++;
                pReferenceArray->at ((size_t)currLabel) = newIndex;
                }

            jmdlMTGGraph_setLabel (pGraph, currNodeId, offset, newIndex);
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)
    return true;
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlMTGChain_switchRenumberedArray                              |
|AUTHOR         RaghavanKunigahalli                     9/96            |
+----------------------------------------------------------------------*/
static void jmdlMTGChain_shuffleDPoint3dArray
(
EmbeddedDPoint3dArray     *pArray,
EmbeddedIntArray            *pRefArray
)
    {
    bvector <DPoint3d> newVertexArray;
    jmdlVArrayInt_shuffleArray (&newVertexArray, pArray, pRefArray);
    pArray->swap (newVertexArray);
    }


/**
* @param  pFacetHeader
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGChain_renumberVertices
(
MTGFacets * pFacetHeader
)
    {
    bvector<int> vertexRefArray, normalRefArray;

    int vertCount, normCount;

    vertCount = (int)pFacetHeader->vertexArrayHdr.size ();

    switch (pFacetHeader->normalMode)
        {
        case MTG_Facets_NoData:
            break;

        case MTG_Facets_VertexOnly:
            if (jmdlMTGChain_renumberLabel ((jmdlMTGFacets_getGraph (pFacetHeader)), &vertexRefArray, pFacetHeader->vertexLabelOffset, vertCount))
                {
                jmdlMTGChain_shuffleDPoint3dArray (&pFacetHeader->vertexArrayHdr, &vertexRefArray);
                }

            break;

        case MTG_Facets_NormalPerVertex:

            if (jmdlMTGChain_renumberLabel (jmdlMTGFacets_getGraph (pFacetHeader), &vertexRefArray, pFacetHeader->vertexLabelOffset, vertCount))
                {
                jmdlMTGChain_shuffleDPoint3dArray (&pFacetHeader->vertexArrayHdr, &vertexRefArray);
                jmdlMTGChain_shuffleDPoint3dArray (&pFacetHeader->normalArrayHdr, &vertexRefArray);
                }
            break;

        case MTG_Facets_SeparateNormals:
            if (jmdlMTGChain_renumberLabel (jmdlMTGFacets_getGraph (pFacetHeader), &vertexRefArray, pFacetHeader->vertexLabelOffset, vertCount))
                {
                jmdlMTGChain_shuffleDPoint3dArray (&pFacetHeader->vertexArrayHdr, &vertexRefArray);
                }

            normCount = (int)pFacetHeader->normalArrayHdr.size ();

            if (jmdlMTGChain_renumberLabel ((jmdlMTGFacets_getGraph (pFacetHeader)), &normalRefArray, pFacetHeader->normalLabelOffset, normCount))
                {
                jmdlMTGChain_shuffleDPoint3dArray (&pFacetHeader->normalArrayHdr, &normalRefArray);
                }
            break;
        }
    }


/**
* @param pFacetHeader
* @param pComponentFacet
* @param pStartArray
* @param pComponentArray
* @param componentNumber
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGChain_getConnectedSubgraph
(
const   MTGFacets           *pFacetHeader,
        MTGFacets           *pComponentFacet,
        EmbeddedIntArray    *pStartArray,
        EmbeddedIntArray    *pComponentArray,
        int                 componentNumber
)
    {
    int compId = -1;
    if (!pFacetHeader || !pComponentFacet)
        return  false;
    jmdlMTGFacets_copy (pComponentFacet, pFacetHeader);

    MTGARRAY_SET_LOOP (currNodeId, (jmdlMTGFacets_getGraph (pComponentFacet)))
        {
        if (   TryGet <int> (*pComponentArray, &compId, currNodeId)
            && compId != componentNumber)
            jmdlMTGGraph_dropEdge ((jmdlMTGFacets_getGraph (pComponentFacet)), currNodeId);
        }
    MTGARRAY_END_SET_LOOP (currNodeId, (jmdlMTGFacets_getGraph (pComponentFacet)))

    jmdlMTGChain_renumberVertices (pComponentFacet);

    return  true;
    }



/*---------------------------------------------------------------------------------**//**
Container for MTGFacets with markup for joining curve fragments into chains.
* @bsimethod                                    Earlin.Lutz                     06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
struct _MTGFragmentSorter
    {
private:
    // Each curve fragment represented by (only) its start and endpoints.
    // Use a facet structure to carry the points and graph.
    MTGFacets *mpChains;
    // In the graph, each edge is labeled with (NONNEGATIVE) integer index of its parent edge ...
    int mParentIndexLabelOffset;
    // and one end is marked as the tail ...
    MTGMask mTailMask;

    // Look up the parent and tail data at a node.
    // Encode the two as a single integer.
    int DerefMTGNodeIdToSignedLabel (MTGNodeId nodeId)
        {
        int parentId;
        if (!jmdlMTGGraph_getLabel (jmdlMTGFacets_getGraph (mpChains), &parentId, nodeId, mParentIndexLabelOffset))
            return 0;
        int signedParentId = 1 + abs (parentId);
        if (!jmdlMTGGraph_getMask (jmdlMTGFacets_getGraph (mpChains), nodeId, mTailMask))
            signedParentId = - signedParentId;
        return signedParentId;
        }

public:
    // CONSTRUCTOR
    _MTGFragmentSorter (int dummy)
        {
        mpChains = jmdlMTGFacets_grab ();
        mParentIndexLabelOffset = jmdlMTGGraph_defineLabel (jmdlMTGFacets_getGraph (mpChains), 1000, MTG_LabelMask_EdgeProperty, -1);
        jmdlMTGFacets_setNormalMode (mpChains, MTG_Facets_VertexOnly, 0, 0);
        mTailMask = jmdlMTGGraph_grabMask (jmdlMTGFacets_getGraph (mpChains));
        jmdlMTGGraph_clearMaskInSet  (jmdlMTGFacets_getGraph (mpChains), mTailMask);
        }
    // DESTRUCTOR
    ~_MTGFragmentSorter ()
        {
        // clean up and drop the facets ...
        jmdlMTGGraph_dropMask (jmdlMTGFacets_getGraph (mpChains), mTailMask);
        jmdlMTGFacets_drop (mpChains);
        }
    // Define a new fragment.
    void AddFragment (int parentIndex, DPoint3dCR xyz0, DPoint3dCR xyz1)
        {
        DPoint3d xyz[2];
        xyz[0] = xyz0;
        xyz[1] = xyz1;
        // Add an isolated edge ...
        MTGNodeId nodeId0 = jmdlMTGFacets_addCoordinateChainExt (mpChains, xyz, NULL, 2);
        MTGNodeId nodeId1 = jmdlMTGGraph_getFSucc (jmdlMTGFacets_getGraph (mpChains), nodeId0);
        // Label both ends
        jmdlMTGGraph_setLabel (jmdlMTGFacets_getGraph (mpChains), nodeId0, mParentIndexLabelOffset, parentIndex);
        jmdlMTGGraph_setLabel (jmdlMTGFacets_getGraph (mpChains), nodeId1, mParentIndexLabelOffset, parentIndex);
        // and mark the tail ....
        jmdlMTGGraph_setMask (jmdlMTGFacets_getGraph (mpChains), nodeId0, mTailMask);
#ifdef COMPILE_PRINTF
        if (sDebug)
            {
            printf ("<edge>\n");
            printf ("   (nodeId %d) (masks %8lx) (xyz %lg %lg %lg)\n", nodeId0,
                                jmdlMTGGraph_getMask (jmdlMTGFacets_getGraph (mpChains), nodeId0, -1),
                                xyz0.x, xyz0.y, xyz0.z);
            printf ("   (nodeId %d) (masks %8lx) (xyz %lg %lg %lg)\n", nodeId1,
                                jmdlMTGGraph_getMask (jmdlMTGFacets_getGraph (mpChains), nodeId1, -1),
                                xyz1.x, xyz1.y, xyz1.z);
            printf ("</edge>\n");
            }
#endif
        }
    // Get coordinates out of nodeId and append to final linestring of pArray.
    // 
    void AppendCoordinates (bvector<bvector<DPoint3d>> *pArray, MTGNodeId nodeId, bool beginningOfNewLinestring)
        {
        DPoint3d xyz;
        if (nullptr != pArray)
            {
            if (beginningOfNewLinestring)
                pArray->push_back (bvector<DPoint3d> ());
            if (mpChains->NodeToVertexCoordinates (nodeId, xyz))
                pArray->back ().push_back (xyz);
            }
        }
    // List fragments in cycle order.
    // Fragment index is converted to one base, negated if used in reverse direction.
    // Loops or chains are separated by 0.
    void SortAndExtractLoopsAndChains (EmbeddedIntArray *pSignedOneBasedLoops,
            EmbeddedIntArray *pSignedOneBasedChains,
            bvector<bvector<DPoint3d>> *pLoopCoordinates,
            bvector<bvector<DPoint3d>> *pChainCoordinates
            )
        {
        MTGGraph *pGraph = jmdlMTGFacets_getGraph (mpChains);
        bvector <int> nodeIdArray, vertexIdArray;

#ifdef COMPILE_PRINTF
        if (sDebug)
            {
            printf ("RAW GRAPH\n");
            jmdlMTGGraph_printFaceLoops (pGraph);
            }
#endif
        jmdlMTGChain_identifyMatchedVertices (mpChains, &nodeIdArray, &vertexIdArray, 0, 0);
        jmdlMTGChain_mergeNodes (mpChains, &nodeIdArray, &vertexIdArray);

#ifdef COMPILE_PRINTF
        if (sDebug)
            {
            printf ("SPLICED GRAPH\n");
            jmdlMTGGraph_printFaceLoops (pGraph);
            }
#endif
        MTGMask visitMask = jmdlMTGGraph_grabMask (pGraph);
        MTGMask extractMask = jmdlMTGGraph_grabMask (pGraph);
        MTGMask barrierMask = jmdlMTGGraph_grabMask (pGraph);

        jmdlMTGGraph_clearMaskInSet (pGraph, visitMask | barrierMask | extractMask);
        // Set a barrier mask at each vertex with other than 2 edges ....
        int numBarrier = 0;
        MTGARRAY_SET_LOOP (seedNodeId, pGraph)
            {
            if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, visitMask))
                {
                int numAroundVertex = (int)pGraph->CountNodesAroundVertex (seedNodeId);
                jmdlMTGGraph_setMaskAroundVertex (pGraph, seedNodeId, visitMask);
                if (sDebug)
                    {
                    printf ("(node %d) (vertexDegree %d)\n", seedNodeId, numAroundVertex);
                    }
                if (numAroundVertex != 2)
                    {
                    jmdlMTGGraph_setMaskAroundVertex (pGraph, seedNodeId, barrierMask);
                    numBarrier++;
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (seedNodeId, jmdlMTGFacets_getGraph (mpChains))

        // Visit chains bounded by barriers ...
        if (numBarrier > 0)
            {
            MTGMask chainBarrierMask = barrierMask | extractMask;
            MTGARRAY_SET_LOOP (seedNodeId, pGraph)
                {
                if (    jmdlMTGGraph_getMask (pGraph, seedNodeId, barrierMask)
                    && !jmdlMTGGraph_getMask (pGraph, seedNodeId, extractMask))
                    {
                    MTGNodeId currNodeId = seedNodeId;
                    if (sDebug)
                        printf ("(chain seed %d)\n", currNodeId);
                    AppendCoordinates (pLoopCoordinates, currNodeId, true);
                    do
                        {
                        int parentIndex = DerefMTGNodeIdToSignedLabel (currNodeId);
                        if (pSignedOneBasedChains)
                            jmdlEmbeddedIntArray_addInt (pSignedOneBasedChains, parentIndex);
                        if (sDebug)
                            {
                            printf ("(chain node %d parent %d)\n", currNodeId, parentIndex);
                            }
                        jmdlMTGGraph_setMaskAroundEdge (pGraph, currNodeId, extractMask);
                        currNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId);
                        AppendCoordinates (pLoopCoordinates, currNodeId, false);
                        } while (!jmdlMTGGraph_getMask (pGraph, currNodeId, chainBarrierMask));
                    if (pSignedOneBasedChains)
                        jmdlEmbeddedIntArray_addInt (pSignedOneBasedChains, 0);
                    }
                }
            MTGARRAY_END_SET_LOOP (seedNodeId, jmdlMTGFacets_getGraph (mpChains))
            }

        // All remaining nodes are in loops with no barriers.
        MTGARRAY_SET_LOOP (seedNodeId, pGraph)
            {
            // Any unvisited node is a start ...
            if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, extractMask))
                {
                MTGNodeId currNodeId = seedNodeId;
                if (sDebug)
                    printf ("(loop seed %d)\n", currNodeId);
                AppendCoordinates (pLoopCoordinates, currNodeId, true);
                do
                    {
                    int parentIndex = DerefMTGNodeIdToSignedLabel (currNodeId);
                    if (sDebug)
                        {
                        printf ("(loop node %d parent %d)\n", currNodeId, parentIndex);
                        }
                    if (pSignedOneBasedLoops)
                        jmdlEmbeddedIntArray_addInt (pSignedOneBasedLoops, parentIndex);

                    jmdlMTGGraph_setMaskAroundEdge (pGraph, currNodeId, extractMask);
                    currNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId);
                    AppendCoordinates (pLoopCoordinates, currNodeId, false);
                    } while (!jmdlMTGGraph_getMask (pGraph, currNodeId, extractMask));

                if (pSignedOneBasedLoops)
                    jmdlEmbeddedIntArray_addInt (pSignedOneBasedLoops, 0);
                }
            }
        MTGARRAY_END_SET_LOOP (seedNodeId, jmdlMTGFacets_getGraph (mpChains))

        jmdlMTGGraph_dropMask (jmdlMTGFacets_getGraph (mpChains), barrierMask);
        jmdlMTGGraph_dropMask (jmdlMTGFacets_getGraph (mpChains), extractMask);
        jmdlMTGGraph_dropMask (jmdlMTGFacets_getGraph (mpChains), visitMask);
        }
    };

/*---------------------------------------------------------------------------------**//**
Allocate a sorter for chained fragments.
@return pointer to the sorter.
@bsimethod                                    Earlin.Lutz                     05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGFragmentSorterP bsiMTGFragmentSorter_new
(
)
    {
    return new _MTGFragmentSorter (1);
    }

/*---------------------------------------------------------------------------------**//**
Free a sorter for chained fragments.
@bsimethod                                    Earlin.Lutz                     05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiMTGFragmentSorter_free
(
MTGFragmentSorterP pSorter
)
    {
   if (pSorter)
        delete pSorter;
    }

/*---------------------------------------------------------------------------------**//**
@description Add a fragment for later end-to-end sorting.
@param pSorter IN OUT evolving sort structure.
@param parentId IN caller's non-negative id of the curve between the endpoints.
@param xyz0 IN start coordinate.
@param xyz1 IN   end coordinate.
@bsimethod                                    Earlin.Lutz                     05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiMTGFragmentSorter_addFragment
(
MTGFragmentSorterP pSorter,
int parentId,
DPoint3dCP pXYZ0,
DPoint3dCP pXYZ1
)
    {
   if (pSorter)
        {
        pSorter->AddFragment (parentId, *pXYZ0, *pXYZ1);
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Add a fragment for later end-to-end sorting.
@param pSorter IN OUT evolving sort structure.
@param parentId IN caller's non-negative id of the curve between the endpoints.
@param xyz0 IN start coordinate.
@param xyz1 IN   end coordinate.
@bsimethod                                    Earlin.Lutz                     05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiMTGFragmentSorter_sortAndExtractSignedLoopsAndChains
(
MTGFragmentSorterP pSorter,
EmbeddedIntArray *pLoopArray,
EmbeddedIntArray *pChainArray
)
    {
   if (pSorter)
        {
        pSorter->SortAndExtractLoopsAndChains (pLoopArray, pChainArray, NULL, NULL);
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Extract chain and loop data.
@param pSorter IN evolving sort structure.
@param pLoopArray OUT filled with "signed one based" indices of fragments traversed by completely isolated loops.
@param pChainArray OUT filled with "signed one based" indices of fragments traversed by chains that are not part of loops.
@param pLoopArray OUT filled with coordinates of completely isolated loops.
@param pChainArray OUT filled with coordinates of chains not parts of loops.
@remark The loop and chain coordinate arrays may be the same pointer.
@bsimethod                                    Earlin.Lutz                     05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiMTGFragmentSorter_sortAndExtractSignedLoopsAndChains
(
MTGFragmentSorterP pSorter,
bvector<int> *pLoopArray,
bvector<int> *pChainArray,
bvector<bvector<DPoint3d>> *pLoopCoordinates,
bvector<bvector<DPoint3d>> *pChainCoordinates
)
    {
    if (pSorter)
        {
        pSorter->SortAndExtractLoopsAndChains (pLoopArray, pChainArray, pLoopCoordinates, pChainCoordinates);
        }
    }



END_BENTLEY_GEOMETRY_NAMESPACE
