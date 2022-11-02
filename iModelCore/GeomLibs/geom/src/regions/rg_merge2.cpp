/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
#include "Regions/rg_intern.h"
#include <stdio.h>
#include    <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define DEBUG_MERGE

static int s_noisy = 0;

typedef struct
    {
    int cluster0;
    int cluster1;
    int nodeId;
    size_t parentIndex;
    } VVNStruct;




void PrintEdgeData (char const *name, RG_EdgeData const &data)
    {
    GEOMAPI_PRINTF (" (%s (nodes %d %d) (verts %d %d) (xyz0 %g,%g) (xyz1 %g,%g) (D %g)\n",
            name,
            data.nodeId[0], data.nodeId[1],
            data.vertexIndex[0], data.vertexIndex[1],
            data.xyz[0].x, data.xyz[0].y, data.xyz[1].x, data.xyz[1].y,
            data.xyz[0].DistanceXY (data.xyz[1])
            );
    }


bool Get (bvector<VVNStruct> *source, size_t i, VVNStruct &dest)
    {
    if (i < source->size ())
        {
        dest = source->at (i);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Collect array of <x,y,nodeId> tuples with each nodeId in the MTGGraph represented.
* @return maximum node id recorded.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void detachAllEdges
(
MTGGraph    *pGraph
)
    {
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        jmdlMTGGraph_yankEdgeFromVertex (pGraph, currNodeId);
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)
    }


/*---------------------------------------------------------------------------------**//**
* Collect array of <x,y,nodeId> tuples with each nodeId in the MTGGraph represented.
* @return maximum node id recorded.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int collectXYNodeId
(
RG_Header   *pRG,
EmbeddedDPoint3dArray *pXYNodeIdArray
)
    {
    int maxNodeId;
    DPoint3d xyNodeId;
    DPoint3d startPoint;
    RG_EdgeData edgeData;
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    maxNodeId = -1;
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if  (  jmdlRG_getEdgeData (pRG, &edgeData, currNodeId)
            && jmdlRG_evaluateEdgeData (pRG, &startPoint, &edgeData, 0.0)
            )
            {
            xyNodeId.x = startPoint.x;
            xyNodeId.y = startPoint.y;
            xyNodeId.z = (double)currNodeId;
            jmdlEmbeddedDPoint3dArray_addDPoint3d (pXYNodeIdArray, &xyNodeId);
            if (currNodeId > maxNodeId)
                maxNodeId = currNodeId;
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)
    return maxNodeId;
    }

/*---------------------------------------------------------------------------------**//**
* Find xy clusters in the xyNodeId array.
* Build an index from nodeId to cluster.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int buildNodeIdToClusterIndexArray
(
RG_Header *pRG,
EmbeddedIntArray *pNodeIdToVertexIndexArray,
EmbeddedIntArray *pBlockedIndexArray,
EmbeddedDPoint3dArray *pXYNodeIdArray,
int             maxNodeId,
double          tolerance,
int             noisy
)
    {
    DPoint3d xyNodeId;
    int numThisCluster, clusterIndex, blockIndex, numCluster, numBlockedIndex;
    int *pBlockedIndexBuffer;
    int xyNodeIdIndex;

    numCluster = jmdlVArrayDPoint3d_identifyMatchedVerticesXY
                            (pXYNodeIdArray, NULL, pBlockedIndexArray, tolerance, 0.0);
    pBlockedIndexBuffer = jmdlEmbeddedIntArray_getPtr (pBlockedIndexArray, 0);
    numBlockedIndex = jmdlEmbeddedIntArray_getCount (pBlockedIndexArray);


    jmdlEmbeddedIntArray_empty (pNodeIdToVertexIndexArray);
    jmdlEmbeddedIntArray_setConstant (pNodeIdToVertexIndexArray, -1, maxNodeId);

    clusterIndex = 0;
    numThisCluster = 0;
    memset (&xyNodeId, 0, sizeof (xyNodeId));
    for (
        blockIndex = 0;
        jmdlEmbeddedIntArray_getInt (pBlockedIndexArray, &xyNodeIdIndex, blockIndex);
        blockIndex++
        )
        {

        if (xyNodeIdIndex < 0)
            {
            if (noisy > 2)
                {
                GEOMAPI_PRINTF (" ================= (endOfCluster %d) (x %lf) (y %lf)\n",
                            clusterIndex, xyNodeId.x, xyNodeId.y);
                }
            numThisCluster = 0;
            clusterIndex++;
            }
        else
            {
            int nodeId;
            jmdlEmbeddedDPoint3dArray_getDPoint3d (pXYNodeIdArray, &xyNodeId, xyNodeIdIndex);
            nodeId = (int)xyNodeId.z;
            numThisCluster++;
            if (noisy > 2)
                {
                GEOMAPI_PRINTF (" (blockIndex %d) (xyzIndex %d) (nodeId %d)\n",
                        blockIndex, xyNodeIdIndex, nodeId);
                }
            jmdlEmbeddedIntArray_setInt (pNodeIdToVertexIndexArray, clusterIndex, nodeId);
            }
        }

    return clusterIndex;
    }

/*---------------------------------------------------------------------------------**//**
* Find xy clusters in the xyNodeId array.
* Build an index from nodeId to cluster.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    buildVVNArray
(
RG_Header *pRG,
bvector<VVNStruct> *pVVNArrayHeader,
EmbeddedIntArray *pNodeIdToVertexIndexArray,
int             maxNodeId,
MTGMask         deleteMask,
double          clusterSize,
int             noisy
)
    {
    MTGNodeId nodeId0, nodeId1;
    VVNStruct vvn;
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    double quickLength;
    static double s_tolFactor = 4.0;
    static int s_numChords = 8;
    double lengthTol = s_tolFactor * clusterSize;
    bool    loopEdge;
    pVVNArrayHeader->clear ();

    for (nodeId0 = 0; nodeId0 <= maxNodeId; nodeId0++)
        {
        vvn.nodeId = nodeId0;
        jmdlEmbeddedIntArray_getInt (pNodeIdToVertexIndexArray, &vvn.cluster0, nodeId0);
        if (vvn.cluster0 >= 0)
            {
            nodeId1 = jmdlMTGGraph_getEdgeMate (pGraph, nodeId0);
            jmdlEmbeddedIntArray_getInt (pNodeIdToVertexIndexArray, &vvn.cluster1, nodeId1);
            loopEdge = false;

            if (noisy > 4)
                {
                if (vvn.cluster0 == vvn.cluster1)
                GEOMAPI_PRINTF (" Edge: (node %d) (cluster %d) (node %d) (cluster %d)\n",
                        nodeId0, vvn.cluster0, nodeId1, vvn.cluster1);
                }

            if (vvn.cluster0 == vvn.cluster1)
                {
                quickLength = jmdlRG_quickChordLength (pRG, nodeId0, s_numChords);
                if (quickLength > lengthTol)
                    {
                    loopEdge = true;
                    }
                else
                    {
                    if (noisy > 4)
                        GEOMAPI_PRINTF ("    **** SHORT EDGE TO BE DELETED ****\n");
                    jmdlMTGGraph_yankEdgeFromVertex (pGraph, nodeId0);
                    jmdlMTGGraph_yankEdgeFromVertex (pGraph, nodeId1);
                    jmdlMTGGraph_setMaskAroundEdge (pGraph, nodeId0, deleteMask);
                    }
                }
            // Umm.. not sure about multiple loops with common tangent.
            if (vvn.cluster0 < vvn.cluster1)
                {
                pVVNArrayHeader->push_back (vvn);
                }


            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Compare (for qsort) 2 records of 3 integers, (lowVertex, highVertex, nodeId)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool cb_vvnCompare
(
const VVNStruct &vvn0,
const VVNStruct &vvn1
)
    {
    if (vvn0.cluster0 < vvn1.cluster0)
        return true;
    if (vvn0.cluster0 > vvn1.cluster0)
        return false;
    if (vvn0.cluster1 < vvn1.cluster1)
        return true;
    if (vvn0.cluster1 > vvn1.cluster1)
        return false;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Sort the VVN array so common start/end cluster records appear together.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    sortVVNArray
(
bvector<VVNStruct> &vvnData,
int noisy
)
    {
    std::sort (vvnData.begin (), vvnData.end (), cb_vvnCompare);

    if (noisy > 4)
        {
        for(VVNStruct vvn : vvnData)
            {
            GEOMAPI_PRINTF (" VVN: %d %d %d\n",
                    vvn.cluster0,
                    vvn.cluster1,
                    vvn.nodeId
                    );
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @return true if given VertexVertexNode data connects the same edges.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    identicalEdgeConnectivity
(
RG_Header       *pRG,
VVNStruct       &vvn0,
VVNStruct       &vvn1
)
    {
    return    vvn0.cluster0 == vvn1.cluster0
           && vvn0.cluster1 == vvn1.cluster1;
    }

/*---------------------------------------------------------------------------------**//**
* @description Test if two edges are geometrically close.   The similarity condition
*           is that they (1) have the same end vertex clusters and (2) 5 points
*           evaluated on edge 0 are within tolerance of edge1.  If you can make
*           a pair of splines that nearly touch in this way and report no intersections from
*           prior intersection calculations, you will fool this function.
* @return true if given VertexVertexNode data connects the same edges AND
*               has identical geometry.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    identicalEdgeGeometry
(
RG_Header                       *pRG,
VVNStruct       *pVVN0,
VVNStruct       *pVVN1,
double          geometryTolerance,
int             noisy
)
    {
#define NUM_TEST_FRACTION 5
    static double testFraction[NUM_TEST_FRACTION] = {0.1236, 0.50234, 0.3982343, 0.64323, 0.802343};
    int i;
    DPoint3d point0, point1;
    double fraction0, fraction1;
    RG_EdgeData edgeData0, edgeData1;
    double tolSquared, dSquared;
    tolSquared = geometryTolerance;

    if (   identicalEdgeConnectivity (pRG, *pVVN0, *pVVN1)
        && jmdlRG_getEdgeData (pRG, &edgeData0, pVVN0->nodeId)
        && jmdlRG_getEdgeData (pRG, &edgeData1, pVVN1->nodeId)
        )
        {
        if (   jmdlRGEdge_getCurveId (&edgeData0) == RG_NULL_CURVEID
            && jmdlRGEdge_getCurveId (&edgeData1) == RG_NULL_CURVEID)
            return true;

        if (noisy)
            {
            GEOMAPI_PRINTF ("VVN0: (clusters %d %d) (node %d)\n", pVVN0->cluster0, pVVN0->cluster1, pVVN0->nodeId);
            GEOMAPI_PRINTF ("VVN1: (clusters %d %d) (node %d)\n", pVVN1->cluster0, pVVN1->cluster1, pVVN1->nodeId);
            PrintEdgeData ("Edge0", edgeData0);
            PrintEdgeData ("Edge1", edgeData1);
            }
        for (i = 0; i < NUM_TEST_FRACTION; i++)
            {
            fraction0 = testFraction [i];

            if (!jmdlRG_evaluateEdgeData (pRG, &point0, &edgeData0, fraction0))
                return false;

            if (!jmdlRG_getClosestXYPointOnEdge (pRG, &fraction1, &dSquared, &point1, NULL,
                                            &point0, pVVN1->nodeId))
                return false;

            if (dSquared > tolSquared)
                return false;
            }

        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* Sort the VVN array and initialize each structure as its own parent.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    assignParentsInSortedVVNArray
(
RG_Header                       *pRG,
bvector<VVNStruct> &vvnData,
double tolerance,
int noisy
)
    {
    int numParallelCandidates = 0;
    int numIsolated = 0;
    size_t numVVN = vvnData.size ();

    for (size_t i0 = 0; i0 < numVVN; i0++)
        vvnData[i0].parentIndex = i0;

    for (size_t i0 = 0; i0 < numVVN; i0++)
        {
        if (vvnData[i0].parentIndex == i0)
            {
            size_t i1;
            for (
                i1 = i0 + 1;
                i1 < numVVN && identicalEdgeConnectivity (pRG, vvnData[i0], vvnData[i1]);
                i1++
                )
                {
                if (noisy)
                    GEOMAPI_PRINTF (" Topologically parallel edges (nodeId %d) (nodeId %d) (vertex %d) (vertex %d)\n",
                            vvnData[i0].nodeId,
                            vvnData[i1].nodeId,
                            vvnData[i0].cluster0,
                            vvnData[i0].cluster1
                            );
                numParallelCandidates++;
                if (identicalEdgeGeometry (pRG, &vvnData[i0], &vvnData[i1], tolerance, noisy))
                    {
                    vvnData[i1].parentIndex = i0;
                    }
                }
            if (i0 == i1 + 1)
                numIsolated++;
            }
        }
    if (noisy)
            GEOMAPI_PRINTF ("(Parallel %d Isolated %d)\n", numParallelCandidates, numIsolated);
    }

static void joinDuplicateEdges
(
RG_Header               *pRG,
bvector<VVNStruct>     *pVVNArray,
MTGMask                 nullFaceMask
)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    MTGNodeId nodeIdParentStart, nodeIdParentEnd, nodeIdChildStart, nodeIdChildEnd;
    detachAllEdges (pGraph);
    VVNStruct child, parent;
    for (size_t childIndex = 0; Get (pVVNArray, childIndex, child); childIndex++)
        {
        size_t parentIndex = child.parentIndex;
        if (   childIndex != parentIndex
            && Get (pVVNArray, parentIndex, parent)
            && parent.parentIndex == parentIndex
            )
            {
            nodeIdParentStart = parent.nodeId;
            nodeIdChildStart  = child.nodeId;
            nodeIdParentEnd   = jmdlMTGGraph_getFSucc (pGraph, nodeIdParentStart);
            nodeIdChildEnd    = jmdlMTGGraph_getFSucc (pGraph, nodeIdChildStart);
            /* Splice the vertex loops together.  The zero area face is inside
                at the child start */
            jmdlMTGGraph_vertexTwist (pGraph, nodeIdChildStart, nodeIdParentStart);
            jmdlMTGGraph_vertexTwist (pGraph, nodeIdChildEnd, nodeIdParentEnd);
            jmdlMTGGraph_setMaskAroundFace (pGraph, nodeIdChildStart, nullFaceMask);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         void            convertXYNodeIdIndexToNodeId
(
EmbeddedDPoint3dArray *pXYNodeIdArray,
EmbeddedIntArray    *pBlockedIndexArray
)
    {
    int numIndex = jmdlEmbeddedIntArray_getCount (pBlockedIndexArray);
    int *pIndex  = jmdlEmbeddedIntArray_getPtr (pBlockedIndexArray, 0);
    int numXYNodeId = jmdlEmbeddedDPoint3dArray_getCount (pXYNodeIdArray);
    DPoint3d *pXYNodeId  = jmdlEmbeddedDPoint3dArray_getPtr (pXYNodeIdArray, 0);
    int i, k;
    MTGNodeId nodeId;
    for (i = 0; i < numIndex; i++)
        {
        k = pIndex[i];
        if (0 <= k && k <= numXYNodeId)
            {
            nodeId = (int) pXYNodeId[k].z;
            pIndex[i] = nodeId;
            }
        }
    }

typedef struct
    {
    MTGNodeId nodeId;
    double theta;
    DPoint3d xyz0;  // edge start
    DPoint3d xyz1;  // 1st derivative -- tangent
    } VertexSortData;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int vsd_compare
(
const VertexSortData *p0,
const VertexSortData *p1
)
    {
    if (p0->theta < p1->theta)
        return -1;
    if (p0->theta > p1->theta)
        return 1;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
*  @return distance to mid-edge point.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static double vsd_init
(
RG_Header           *pRG,
VertexSortData      *pVSD,
DPoint3d            *pXYZ,
MTGNodeId           nodeId,
double              fraction
)
    {
    double thetaA;
    DPoint3d midEdgeXYZ;
    DPoint3d xyz[3];
    RG_EdgeData edgeData;
    pVSD->nodeId = nodeId;
    jmdlRG_getVertexData (pRG, xyz, 1, NULL, nodeId, 0.0);
    pVSD->xyz0 = xyz[0];
    pVSD->xyz1 = xyz[1];
    pVSD->theta = Angle::Atan2 (xyz[1].y, xyz[1].x);

    jmdlRG_getEdgeData (pRG, &edgeData, nodeId);
    if (s_noisy)
        PrintEdgeData ("vsd_init", edgeData);
    jmdlRG_evaluateEdgeData (pRG, &midEdgeXYZ, &edgeData, fraction);
    static double s_zeroDerivativeTolerance = 1.0e-8;
    static double s_fractionA = 1.0e-2;
    if (xyz[1].MagnitudeXY () < s_zeroDerivativeTolerance)
        {
        DPoint3d xyzA;
        jmdlRG_evaluateEdgeData (pRG, &xyzA, &edgeData, s_fractionA);
        DVec3d chordA = DVec3d::FromStartEnd (xyz[0], xyzA);
        thetaA = Angle::Atan2 (chordA.y, chordA.x);
        pVSD->theta = thetaA;
        }
    if (pXYZ)
        *pXYZ = midEdgeXYZ;
    return midEdgeXYZ.DistanceXY (xyz[0]);
    }


static void sortByIntersectionPoints
(
RG_Header           *pRG,
VertexSortData      *pSortData,
int                 numSort,
double              radius,
EmbeddedDPoint3dArray *pXYZArray,
bvector<double>   *pParamArray
)
    {
    DPoint3d xyzCenter, xyzCurr;
    DVec3d vector;
    int numIntersect;
    int numFail;
    int i;
    jmdlEmbeddedDPoint3dArray_empty (pXYZArray);
    pParamArray->clear();
    jmdlRG_getVertexData (pRG, &xyzCenter, 0, NULL, pSortData[0].nodeId, 0.0);
    numFail = 0;
    for (i = 0; i < numSort; i++)
        {
        jmdlRG_edgeCircleXYIntersection (pRG, pParamArray, pXYZArray,
                        pSortData[i].nodeId, &xyzCenter, radius);

        numIntersect = jmdlEmbeddedDPoint3dArray_getCount (pXYZArray);
        if (numIntersect != 1)
            {
            numFail++;
            }
        else
            {
            jmdlEmbeddedDPoint3dArray_getDPoint3d (pXYZArray, &xyzCurr, 0);
            vector.DifferenceOf (xyzCurr, xyzCenter);
            pSortData[i].theta = Angle::Atan2 (vector.y, vector.x);
            }
        }
    qsort (pSortData, numSort, sizeof (VertexSortData), (VBArray_SortFunction)vsd_compare);
    }

static bool    anglesWithinTolerance
(
double theta0,
double theta1,
double tol
)
    {
    return Angle::InSweepAllowPeriodShift (theta0, theta1 - tol, 2.0 * tol);
    }


/*---------------------------------------------------------------------------------**//**
Conditionally swap a pair of SortData records if not in a left-turn relation according to
a circle intersection condition.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void swapByIntersectionPoints
(
RG_Header           *pRG,
VertexSortData      *pSortData,
int                 i,
int                 j,
double              fraction,
EmbeddedDPoint3dArray *pXYZArray,
bvector<double>   *pParamArray
)
    {
    double sOffsetFraction = 0.0; // was 1.0e-6;
    // Look at the tangent magnitudes.
    // Make a circle which is a small fraction of the smaller tangent magnitude.
    // Intersect curves with the circle.
    size_t n0, n1;
    DPoint3d xyz[2][2];
    DVec3d vector[2];
    double param [2];
    DPoint3d xyzTarget[2];
    double a0, a1, a, r;
    jmdlRG_getVertexData (pRG, xyz[0], 1, NULL, pSortData[i].nodeId, sOffsetFraction);
    jmdlRG_getVertexData (pRG, xyz[1], 1, NULL, pSortData[j].nodeId, sOffsetFraction);
    a0 = xyz[0][1].Magnitude ();
    a1 = xyz[1][1].Magnitude ();
    a = a0 < a1 ? a0 : a1;
    r = fraction * a;
    jmdlRG_edgeCircleXYIntersection (pRG, pParamArray, pXYZArray,
                pSortData[i].nodeId, &xyz[0][0], r);

    if ((n0 = pParamArray->size ()) == 1)
        {
        param[0] = pParamArray->at (0);
        jmdlEmbeddedDPoint3dArray_getDPoint3d (pXYZArray, &xyzTarget[0], 0);
        jmdlRG_edgeCircleXYIntersection (pRG, pParamArray, pXYZArray,
                    pSortData[j].nodeId, &xyz[1][0], r);

        if ((n1 = pParamArray->size ()) == 1)
            {
            double cp;
            param[1] = pParamArray->at (0);
            jmdlEmbeddedDPoint3dArray_getDPoint3d (pXYZArray, &xyzTarget[1], 0);
            vector[0].DifferenceOf (xyzTarget[0], *(&xyz[0][0]));
            vector[1].DifferenceOf (xyzTarget[1], *(&xyz[1][0]));
            cp = vector[0].CrossProductXY (vector[1]);
            if (cp < 0.0)
                {
                VertexSortData temp = pSortData[j];
                pSortData[j] = pSortData[i];
                pSortData[i] = temp;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int modularAdd
(
int i0, // Assumed positive.
int i,  // Assumed positive.
int n   // Assumed positive.
)
    {
    int k = i0 + i;
    while (k >= n)
        k -= n;
    return k;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void shuffleByIntersectionPoints
(
RG_Header           *pRG,
VertexSortData      *pSortData,
int                 numSort,
double              fraction,
EmbeddedDPoint3dArray *pXYZArray,
bvector<double>   *pParamArray,
double              angleTol
)
    {
    int i, j, firstBlockStart, currBlockStart, k, numInBlock, m0, m1;

    // Find a place where there is a definite break ....
    firstBlockStart = 0;
    for (i = 0; i < numSort; i++)
        {
        j = modularAdd (i, 1, numSort);
        if (!anglesWithinTolerance (pSortData[i].theta, pSortData[j].theta, angleTol))
            {
            firstBlockStart = i;
            break;
            }
        }

    // Brute force insertion sort within each block of similar angles, starting at firstBlockStart.
    // k is offset from firstBlockStart, interpreted cyclically.
    for (k = 0; k < numSort;)
        {
        // Count similar angles:
        currBlockStart = i = modularAdd (firstBlockStart, k, numSort);
        for (numInBlock = 1; k + numInBlock < numSort; numInBlock++, i = j)
            {
            j = modularAdd (i, 1, numSort);
            if (!anglesWithinTolerance (pSortData[i].theta, pSortData[j].theta, angleTol))
                break;
            }

        // Insertion sort -- pull "leftmost" forward with cyclic block
        for (m0 = 0; m0 + 1 < numInBlock; m0++)
            {
            i = modularAdd (currBlockStart, m0, numSort);
            for (m1 = 1; m1 < numInBlock; m1++)
                {
                j = modularAdd (currBlockStart, m1, numSort);
                swapByIntersectionPoints (pRG, pSortData, i, j, fraction, pXYZArray, pParamArray);
                }
            }
        k+= numInBlock;
        }
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int countAngleDups
(
VertexSortData      *pSortData,
int                 numSort,
double              angleTol
)
    {
    double sweepAngle = 2.0 * angleTol;
    int i,j;
    int numDup = 0;
    for (i = 0, j = numSort - 1; i < numSort; j = i++)
        {
        if (Angle::InSweepAllowPeriodShift (pSortData[i].theta,
                    pSortData[j].theta - angleTol,
                    sweepAngle
                    ))
            numDup++;
        }
    return numDup;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void vsd_sort
(
RG_Header           *pRG,
VertexSortData      *pSortData,
int                 numSort,
double              refDist,
bvector<double>   *pParamArray,
EmbeddedDPoint3dArray *pXYZArray
)
    {
    int numDup;
    static double s_fraction = 0.455;
    static double s_angleTol = 1.0e-5;
    double radius;

    qsort (pSortData, numSort, sizeof (VertexSortData), (VBArray_SortFunction)vsd_compare);
    numDup = countAngleDups (pSortData, numSort, s_angleTol);

    if (numDup > 0)
        {
        radius = s_fraction * refDist;
        sortByIntersectionPoints (pRG, pSortData, numSort, radius, pXYZArray, pParamArray);
        numDup = countAngleDups (pSortData, numSort, s_angleTol);
        if (numDup > 0)
            shuffleByIntersectionPoints (pRG, pSortData, numSort, 0.1, pXYZArray, pParamArray, s_angleTol);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         void            sortAndJoinAroundVertex
(
RG_Header           *pRG,
int                 *pNodeIdBuffer,
size_t              numNodeId,
MTGMask             nullFaceMask,
MTGMask             deleteMask,
bvector<double>   *pParamArray,
EmbeddedDPoint3dArray *pXYZArray
)
    {
    ScopedArray <VertexSortData> sortData (numNodeId);
    VertexSortData *pSortData = sortData.GetData ();
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    MTGNodeId currNodeId, activeEdgeNodeId;
    double currDist, minDist = DBL_MAX;
    MTGMask skipMask = deleteMask | nullFaceMask;
    int numActive = 0;
    int i;

#define NUM_HISTORY_ENTRY 8
    static int numHistory[NUM_HISTORY_ENTRY] = {0, 0,   0, 0,   0, 0,   0, 0};


    for (i = 0; i < numNodeId; i++)
        {
        currNodeId = pNodeIdBuffer[i];
        if (!jmdlMTGGraph_getMask (pGraph, currNodeId, skipMask))
            {
            currDist = vsd_init (pRG, &pSortData[numActive], NULL, currNodeId, 0.5);
            if (numActive == 0 || currDist < minDist)
                minDist = currDist;
            numActive++;
            }
        }


    if (numActive < NUM_HISTORY_ENTRY)
        {
        numHistory[numActive] += 1;
        }
    else
        {
        numHistory[NUM_HISTORY_ENTRY - 1] += 1;
        }

    if (numActive == 1)
        {

        }
    else if (numActive == 2)
        {
        MTGNodeId nodeId0 = pSortData[0].nodeId;
        MTGNodeId nodeId1 = pSortData[1].nodeId;
        jmdlMTGGraph_vertexTwist (pGraph, nodeId0, nodeId1);
        }
    else if (numActive > 2)
        {
        vsd_sort (pRG, pSortData, numActive, minDist, pParamArray, pXYZArray);
        activeEdgeNodeId = pSortData[0].nodeId;
        for (i = 1; i < numActive; i++)
            {
            currNodeId = pSortData[i].nodeId;
            jmdlMTGGraph_vertexTwist (pGraph, activeEdgeNodeId, currNodeId);
            activeEdgeNodeId = currNodeId;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         void            joinDistinctEdges
(
RG_Header           *pRG,
EmbeddedIntArray    *pBlockedNodeIdArray,
MTGMask             nullFaceMask,
MTGMask             deleteMask
)
    {
    size_t numIndex = pBlockedNodeIdArray->size ();
    int *pIndex  = pBlockedNodeIdArray->data ();
    bvector<double> paramArray;
    bvector<DPoint3d> xyzArray;

    size_t i0, i1;
    for (i0 = 0; i0 < numIndex; i0 = i1 + 1)
        {
        /* Count to first position after this block of nodeId's */
        for (i1 = i0; i1 < numIndex && pIndex[i1] >= 0; i1++)
            {
            }
        sortAndJoinAroundVertex (pRG, pIndex + i0, i1 - i0, nullFaceMask, deleteMask,
                        &paramArray, &xyzArray);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_clusterByXY
(
RG_Header                       *pRG,
double                          tolerance,
MTGMask                         nullFaceMask,
int                             noisy
)
    {
    EmbeddedDPoint3dArray *pXYNodeIdArray = jmdlEmbeddedDPoint3dArray_grab ();
    EmbeddedIntArray *pNodeIdToVertexIndexArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pBlockedIndexArray = jmdlEmbeddedIntArray_grab ();
    static double s_toleranceFactor = 1.0;

    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    MTGMask deleteMask = jmdlMTGGraph_grabMask (pGraph);
    int maxNodeId;
    int numCluster;
    bvector<VVNStruct> vvnArrayHeader;

    if (s_noisy > noisy)
        noisy = s_noisy;

    if (noisy)
        GEOMAPI_PRINTF (" XY clustering tolerance %lf\n", tolerance);

    maxNodeId = collectXYNodeId (pRG, pXYNodeIdArray);

    numCluster = buildNodeIdToClusterIndexArray
            (pRG, pNodeIdToVertexIndexArray, pBlockedIndexArray, pXYNodeIdArray, maxNodeId,
                    s_toleranceFactor * tolerance, noisy);

    if (noisy)
        GEOMAPI_PRINTF (" Total clusters found = %d\n", numCluster);

    buildVVNArray (pRG, &vvnArrayHeader,
            pNodeIdToVertexIndexArray, maxNodeId,
            deleteMask,
            tolerance,
            noisy);
    sortVVNArray (vvnArrayHeader, noisy);
    assignParentsInSortedVVNArray (pRG, vvnArrayHeader, tolerance, noisy);
    joinDuplicateEdges (pRG, &vvnArrayHeader, nullFaceMask);
    convertXYNodeIdIndexToNodeId (pXYNodeIdArray, pBlockedIndexArray);
    joinDistinctEdges (pRG, pBlockedIndexArray, nullFaceMask, deleteMask);

    jmdlEmbeddedDPoint3dArray_drop (pXYNodeIdArray);
    jmdlEmbeddedIntArray_drop (pNodeIdToVertexIndexArray);

    jmdlEmbeddedIntArray_drop (pBlockedIndexArray);
    jmdlMTGGraph_dropMask (pGraph, deleteMask);
    return false;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
