/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "Regions/rg_intern.h"
#include <stdio.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#define DEBUG_MERGE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void    jmdlRGGapList_init
(
RG_GapList  *pGapList,
RG_Header   *pRG,
MTGMask     mask,
double      vertexSize,
double      minVertexVertexGap,
double      maxVertexVertexGap,
double      minVertexEdgeGap,
double      maxVertexEdgeGap
)
    {
    memset (pGapList, 0, sizeof (*pGapList));
    pGapList->pRG = pRG;
    pGapList->mask = mask;

    pGapList->minVertexVertexGap = minVertexVertexGap;
    pGapList->maxVertexVertexGap = minVertexVertexGap;

    pGapList->minVertexEdgeGap = minVertexEdgeGap;
    pGapList->maxVertexEdgeGap = minVertexEdgeGap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void    jdmlRGGapList_releaseMemory
(
RG_GapList  *pGapList
)
    {
    }

/*---------------------------------------------------------------------------------**//**
* Read in an int array, seeking a negative entry or end of array.
* Primary return is the interval i0 <= i < i1 bounding a block of contiguous
*   non-negative entries.
* @param pReadIndex <=> on input, first index to read from.  On output, advanced
*               to first index for next pass.
* @param pI0 <= first
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    getNonNegativeBlock
(
EmbeddedIntArray *pBlockedIndexArray,
int         *pReadIndex,
int         *pI0,
int         *pI1
)
    {
    int i0 = *pReadIndex;
    int value;
    int i1 = i0;
    for (;jmdlEmbeddedIntArray_getInt (pBlockedIndexArray, &value, i1) && value >= 0; i1++)
        {
        }
    *pI0 = i0;
    *pI1 = i1;
    *pReadIndex = i1 + 1;
    return i1 > i0;
    }


/*---------------------------------------------------------------------------------**//**
* Search for and record perpendiculars from a particular gap vertex.
* @param pSegmentXYZArray <=> accumulating list of (start, end) pairs for gap edges.
* @param maxVertexVertex => do not add gap edge if space point and
*                   a near vertex of the edge are this close ---
*                   there will be a vertex-vertex gap edge anyway.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    searchGapPerpendicular
(
RG_GapList *pGapList,
EmbeddedDPoint3dArray *pSegmentXYZArray,
const DPoint3d *pXYZ,
MTGNodeId   nodeId,
double minGap,
double maxGap,
double maxVertexVertex
)
    {
    double paramOfProjection;
    double minDistSquared;
    DPoint3d xyzPerp;
    DPoint3d xyzVertex[2];
    double dist;
    RG_EdgeData edgeData;

    jmdlRG_getEdgeData (pGapList->pRG, &edgeData, nodeId);
    jmdlRGEdge_getXYZ (&edgeData, &xyzVertex[0], 0);
    jmdlRGEdge_getXYZ (&edgeData, &xyzVertex[1], 1);
    /* Don't bother if close to an endpoint ... */
    if (  pXYZ->DistanceXY (xyzVertex[0]) <= maxVertexVertex
       || pXYZ->DistanceXY (xyzVertex[1]) <= maxVertexVertex
       )
        {
        }
    else if (jmdlRG_getClosestXYPointOnEdge (pGapList->pRG,
                &paramOfProjection, &minDistSquared, &xyzPerp, NULL,
                pXYZ,
                nodeId))
        {
        dist = sqrt (minDistSquared);
        if (    dist >= minGap
            && dist <= maxGap
            &&  paramOfProjection > 0.0
            &&  paramOfProjection < 1.0
            )
            {
            /* GEOMAPI_PRINTF (" gap detect: %lf %lf --> node %d\n", pXYZ->x, pXYZ->y, nodeId); */
            jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, pXYZ);
            jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyzPerp);
            jmdlRG_debugSegment (pGapList->pRG, pXYZ, &xyzPerp, 1, 1, RG_NORMALDRAW);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Search for and record "gap" vertex pairs.
*
* @param vertexSize => tolerance for direct vertex clustering (without creating
*       gap edges)
* @param vertexVertexMin => min vertex-vertex gap length
* @param vertexVertexMax => max vertex-vertex gap length
* @param vertexVertexMin => min vertex-edge gap length
* @param vertexEdgeMax => max vertex-edge gap length
* @param vertexSize => size for initial vertex clustering
* @param boxFactor => when building a box (rather than closing gap)
*                   make the box this factor of the gap size.
* @param maxDiagFraction => but limit box to this fraction of range diagonal.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRGGapList_addGapEdgesToGraph
(
RG_Header       *pRG,
MTGMask         mask,
double          vertexSize,
double          vertexVertexMin,
double          vertexVertexMax,
double          vertexEdgeMin,
double          vertexEdgeMax,
double          boxFactor,
double          maxDiagBoxFraction
)
    {
    bvector<DPoint3d> xyz0, xyz1;
    
    EmbeddedIntArray *pBlockIndexArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pEdgeCandidateArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedDPoint3dArray *pSegmentXYZArray = jmdlEmbeddedDPoint3dArray_grab ();
    EmbeddedDPoint3dArray *pNodeXYZArray = jmdlEmbeddedDPoint3dArray_grab ();
    double dCurr;
    double s_relTol = 1.0e-14;
    DPoint3d xyz;
    DPoint3d xyzSegment[2];
    DRange3d searchRange;
    double diagonalSize;
    double maxBoxSize;
    int readIndex, i0, i1;
    MTGNodeId nodeId;
    double vertexEdgeRangeExtension = vertexEdgeMax + vertexSize;
    DRange3d geometryRange;
    RG_GapList  gapListHeader;
    static int s_addVE = 1;
    static int s_addVV = 1;
    int i;

    jmdlRGGapList_init (&gapListHeader, pRG, mask,
                vertexSize,
                vertexVertexMin, vertexVertexMax,
                vertexEdgeMin,   vertexEdgeMax
                );

    jmdlRG_getRange (pRG, &geometryRange);
    diagonalSize = geometryRange.low.Distance (geometryRange.high);
    maxBoxSize = maxDiagBoxFraction * diagonalSize;

    if (vertexEdgeMax > 0)
        {
        if (vertexVertexMin < vertexSize)
            vertexVertexMin = vertexSize;

        jmdlVArrayDPoint3d_identifyMatchedVertices
                (
                pRG->pVertexArray,
                NULL,
                pBlockIndexArray,
                vertexSize,
                s_relTol
                );

        for (readIndex = 0; getNonNegativeBlock (pBlockIndexArray, &readIndex, &i0, &i1);)
            {
            searchRange.Init ();
            for (i = i0; i < i1; i++)
                {
                int vertexIndex;
                jmdlEmbeddedIntArray_getInt (pBlockIndexArray, &vertexIndex, i);
                jmdlEmbeddedDPoint3dArray_getDPoint3d (pRG->pVertexArray, &xyz, vertexIndex);
                searchRange.Extend (xyz);
                }
            xyz.Interpolate (searchRange.low, 0.5, searchRange.high);
            searchRange.Extend (vertexEdgeRangeExtension);

            jmdlRG_collectXYEdgeRangeHits
                            (pRG, pEdgeCandidateArray, &searchRange);
            jmdlEmbeddedDPoint3dArray_addDPoint3d (pNodeXYZArray, &xyz);

            for (i = 0; jmdlEmbeddedIntArray_getInt (pEdgeCandidateArray, &nodeId, i); i++)
                {
                if (s_addVE)
                    searchGapPerpendicular
                            (
                            &gapListHeader,
                            pSegmentXYZArray,
                            &xyz,
                            nodeId,
                            vertexEdgeMin,
                            vertexEdgeMax,
                            vertexVertexMax
                            );
                }
            }


        if (s_addVV)
            {
            /* Find close vertices ... */
            jmdlEmbeddedIntArray_empty (pBlockIndexArray);
            jmdlVArrayDPoint3d_identifyMatchedVertices
                    (
                    pNodeXYZArray,
                    NULL,
                    pBlockIndexArray,
                    vertexVertexMax,
                    s_relTol
                    );
            /*
                Add all combinations within each close vertex cluster ..
                For n > 3, should form convex hull
            */
            for (readIndex = 0; getNonNegativeBlock (pBlockIndexArray, &readIndex, &i0, &i1);)
                {
                int numVertex = i1 - i0;
                if (numVertex > 1)
                    {
                    static int s_addMode = 2;
                    int j, jIndex, iIndex;
                    if (s_addMode == 1) /* Rectangle */
                        {
                        DRange3d range;
                        DPoint3d xyzA;
                        double dx, dy;
                        DPoint3d xyzRectangle[5];
                        range.Init ();
                        for (i = i0; i < i1;  i++)
                            {
                            jmdlEmbeddedIntArray_getInt (pBlockIndexArray, &iIndex, i);
                            jmdlEmbeddedDPoint3dArray_getDPoint3d (pNodeXYZArray, &xyzA, iIndex);
                            range.Extend (xyzA);
                            }
                        xyzRectangle[0] = range.low;
                        xyzRectangle[2] = range.high;
                        dx = boxFactor * vertexVertexMax;
                        if (dx > maxBoxSize)
                            dx = maxBoxSize;
                        dy = dx;
                        xyzRectangle[0].x -= dx;
                        xyzRectangle[0].y -= dy;;

                        xyzRectangle[2].x += dx;
                        xyzRectangle[2].y += dy;

                        xyzRectangle[1] = xyzRectangle[0];
                        xyzRectangle[1].x = xyzRectangle[2].x;

                        xyzRectangle[3] = xyzRectangle[0];
                        xyzRectangle[3].y = xyzRectangle[2].y;

                        jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyzRectangle[0]);
                        jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyzRectangle[1]);

                        jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyzRectangle[1]);
                        jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyzRectangle[2]);

                        jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyzRectangle[2]);
                        jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyzRectangle[3]);

                        jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyzRectangle[3]);
                        jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyzRectangle[0]);
                        }
                    else if (s_addMode == 2)    /* Convex hull */
                        {
                        int num00;
                        xyz0.clear ();
                        xyz1.clear ();
                        DPoint3d xyzA;
                        num00 = 0;
                        for (i = i0; i < i1;  i++)
                            {
                            jmdlEmbeddedIntArray_getInt (pBlockIndexArray, &iIndex, i);
                            jmdlEmbeddedDPoint3dArray_getDPoint3d (pNodeXYZArray, &xyzA, iIndex);
                            xyz0.push_back (xyzA);
                            }
                        DPoint3dOps::ConvexHullXY (xyz0, xyz1);
                        size_t num1 = xyz1.size ();
                        if (num1 > 2)
                            {
                            xyz1[num1++] = xyz1[0];
                            }
                        for (j = 1; j < num1; j++)
                            {
                            dCurr = xyz1[j-1].DistanceXY (xyz1[j]);
                            if (dCurr > vertexSize)
                                {
                                jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyz1[j-1]);
                                jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyz1[j]);
                                }
                            }

                        }
                    else
                        {
                        for (i = i0; i < i1;  i++)
                            {
                            jmdlEmbeddedIntArray_getInt (pBlockIndexArray, &iIndex, i);
                            jmdlEmbeddedDPoint3dArray_getDPoint3d (pNodeXYZArray, &xyzSegment[0], iIndex);
                            for (j = i + 1; j < i1; j++)
                                {
                                jmdlEmbeddedIntArray_getInt (pBlockIndexArray, &jIndex, j);
                                jmdlEmbeddedDPoint3dArray_getDPoint3d (pNodeXYZArray, &xyzSegment[1], jIndex);
                                dCurr = xyzSegment[0].DistanceXY (xyzSegment[1]);
                                if (dCurr > vertexSize)
                                    {
                                    jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyzSegment[0]);
                                    jmdlEmbeddedDPoint3dArray_addDPoint3d (pSegmentXYZArray, &xyzSegment[1]);
                                    }
                                }
                            }
                        }
                    }
                }
            }

        /* Add the segments to the graph */
        for (
            i = 0;
                jmdlEmbeddedDPoint3dArray_getDPoint3d (pSegmentXYZArray, &xyzSegment[0], i)
            &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pSegmentXYZArray, &xyzSegment[1], i+1);
            i += 2
            )
            {
            dCurr = xyzSegment[0].DistanceXY (xyzSegment[1]);
            jmdlRG_addMaskedLinear (pRG, NULL, xyzSegment, 2, -1, mask, mask);
            jmdlRG_debugSegment (pRG,
                                &xyzSegment[0],
                                &xyzSegment[1],
                                1, 2, RG_NORMALDRAW);
            }




        }

    jdmlRGGapList_releaseMemory (&gapListHeader);
    jmdlEmbeddedDPoint3dArray_drop (pNodeXYZArray);
    jmdlEmbeddedDPoint3dArray_drop (pSegmentXYZArray);
    jmdlEmbeddedIntArray_drop (pEdgeCandidateArray);
    jmdlEmbeddedIntArray_drop (pBlockIndexArray);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Consolidate specified nodes to a single vertex.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         void            consolidateVertexCoordinates
(
RG_Header           *pRG,
EmbeddedIntArray    *pNodeIdArray
)
    {
    int i;
    int vertexId;
    MTGNodeId nodeId;
    DPoint3d xyz, xyzTick;
    int numNode = jmdlEmbeddedIntArray_getCount (pNodeIdArray);
    double tickSize;
    static int s_outputTrigger = 5;
    static double s_tickFraction = 0.05;
    bvector<RG_EdgeData> edgeDataArray ((size_t)numNode);
    RG_EdgeData edgeData;

    int numCurve = 0;
    int numLine = 0;
    int iLine0, iCurve0;
    int vertexIndex;



    if (numNode >= s_outputTrigger)
        {
        jmdlEmbeddedIntArray_getInt (pNodeIdArray, &nodeId, 0);
        jmdlRG_getVertexData (pRG, &xyz, 0, &vertexId, nodeId, 0.0);

        tickSize = s_tickFraction * (pRG->graphRange.high.x - pRG->graphRange.low.x);
        xyzTick.x = xyz.x + tickSize;
        xyzTick.y = xyz.y + tickSize;
        xyzTick.z = xyz.z;
        jmdlRG_debugSegment (pRG, &xyz, &xyzTick, 1, 1, RG_NORMALDRAW);
        }


    /* Collect all the edge data, with lines first */
    for (i = 0; jmdlEmbeddedIntArray_getInt (pNodeIdArray, &nodeId, i); i++)
        {
        jmdlRG_getEdgeData (pRG, &edgeData, nodeId);
        if (jmdlRGEdge_getCurveId (&edgeData) == RG_NULL_CURVEID)
            edgeDataArray[numLine++] = edgeData;
        else
            edgeDataArray[numNode - (++numCurve)] = edgeData;
        }
    iLine0 = 0;
    iCurve0 = numNode - numCurve;

    if (numCurve == 0 && numLine > 1)
        {
        /* Arbitrarily move all lines ends to one spot. */
        jmdlRGEdge_getXYZ (&edgeDataArray[iLine0], &xyz, 0);
        for (i = iLine0; i < iLine0 + numLine; i++)
            {
            vertexIndex = jmdlRGEdge_getVertexId (&edgeDataArray[i], 0);
            jmdlRG_moveVertex (pRG, vertexIndex, &xyz);
            }
        }
    else if (numCurve == 1 && numLine > 0)
        {
        /* Arbitrarily move all lines ends to one spot. */
        jmdlRGEdge_getXYZ (&edgeDataArray[iCurve0], &xyz, 0);
        for (i = iLine0; i < iLine0 + numLine; i++)
            {
            vertexIndex = jmdlRGEdge_getVertexId (&edgeDataArray[i], 0);
            jmdlRG_moveVertex (pRG, vertexIndex, &xyz);
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* Search for and close small gaps by moving vertex coordinates.
*
* @param vertexSize => tolerance for direct vertex clustering (without creating
*       gap edges)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_closeSimpleGaps
(
RG_Header       *pRG,
double          vertexSize
)
    {
    MTGGraph *pGraph    = pRG->pGraph;
    EmbeddedIntArray *pVertexToBaseVertexArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pBlockArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pNodeIdListArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pVertexToNodeIdListArray = jmdlEmbeddedIntArray_grab ();
    int i0, i1, readIndex;
    int numVertex = jmdlEmbeddedDPoint3dArray_getCount (pRG->pVertexArray);
    int numNodeId = jmdlMTGGraph_getNodeIdCount (pGraph);
    double s_relTol = 1.0e-14;
    int i;
    int baseVertex, currVertex;
    MTGNodeId baseNodeId, currNodeId;

    if (vertexSize > 0)
        {
        /* Find close vertices ... */
        jmdlVArrayDPoint3d_identifyMatchedVertices
                (
                pRG->pVertexArray,
                NULL,
                pBlockArray,
                vertexSize,
                s_relTol
                );

        jmdlEmbeddedIntArray_setConstant (pVertexToBaseVertexArray,  -1,              numVertex);
        jmdlEmbeddedIntArray_setConstant (pVertexToNodeIdListArray, MTG_NULL_NODEID, numVertex);
        jmdlEmbeddedIntArray_setConstant (pNodeIdListArray,          MTG_NULL_NODEID, numNodeId);

        /* Index from each vertex to a base vertex in its cluster. */
        for (readIndex = 0; getNonNegativeBlock (pBlockArray, &readIndex, &i0, &i1);)
            {
            jmdlEmbeddedIntArray_getInt (pBlockArray, &baseVertex, i0);
            for (i = i0; i < i1; i++)
                {
                jmdlEmbeddedIntArray_getInt (pBlockArray, &currVertex, i);
                jmdlEmbeddedIntArray_setInt (pVertexToBaseVertexArray, baseVertex, currVertex);
                }
            }

        /* null terminated linked list from node id to other nodes with same base vertex.
            Each base vertex goes to its node list.
            Only a base vertex has a node id.
        */
        MTGARRAY_SET_LOOP (currNodeIdA, pGraph)
            {
            jmdlRG_getVertexData (pRG, NULL, 0, &currVertex, currNodeIdA, 0.0);
            jmdlEmbeddedIntArray_getInt (pVertexToBaseVertexArray, &baseVertex, currVertex);
            jmdlEmbeddedIntArray_getInt (pVertexToNodeIdListArray, &baseNodeId, baseVertex);

            jmdlEmbeddedIntArray_setInt (pVertexToNodeIdListArray, currNodeIdA, baseVertex);
            jmdlEmbeddedIntArray_setInt (pNodeIdListArray, baseNodeId, currNodeIdA);
            }
        MTGARRAY_END_SET_LOOP (currNodeIdA, pGraph)

        for (i = 0; jmdlEmbeddedIntArray_getInt (pVertexToNodeIdListArray, &baseNodeId, i); i++)
            {
            if (baseNodeId != MTG_NULL_NODEID)
                {
                jmdlEmbeddedIntArray_empty (pBlockArray);
                jmdlEmbeddedIntArray_addInt (pBlockArray, baseNodeId);
                for (currNodeId = baseNodeId;
                    jmdlEmbeddedIntArray_getInt (pNodeIdListArray, &currNodeId, currNodeId)
                    && currNodeId != MTG_NULL_NODEID;
                    )
                    {
                    jmdlEmbeddedIntArray_addInt (pBlockArray, currNodeId);
                    }
                consolidateVertexCoordinates (pRG, pBlockArray);
                }
            }
        }

    jmdlEmbeddedIntArray_drop (pVertexToBaseVertexArray);
    jmdlEmbeddedIntArray_drop (pBlockArray);
    jmdlEmbeddedIntArray_drop (pNodeIdListArray);
    jmdlEmbeddedIntArray_drop (pVertexToNodeIdListArray);
    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
