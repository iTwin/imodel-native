/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <vector>
#include <algorithm>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool cb_VuVertexSortKey_LT_VuVertexSortKey (VuVertexSortKey const &keyA, VuVertexSortKey const &keyB)
{
    return keyA.sortCoordinate < keyB.sortCoordinate;
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    keysInSameCluster
(
VuSetP pGraph,
VuVertexSortKey *pKeyA,
VuVertexSortKey *pKeyB,
double    abstol
)
    {
    double dd = pKeyA->xy.DistanceSquared (pKeyB->xy);
    return dd < abstol * abstol;
    }

/**
* install integer label around vertex.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void markNode
(
VuSetP pGraph,
VuP     pSeed,
int     value
)
    {
    VU_VERTEX_LOOP (pCurr, pSeed)
        {
        vu_setUserDataPAsInt (pCurr, value);
        }
    END_VU_VERTEX_LOOP (pCurr, pSeed)
    }
#ifdef CompileAll
/**
* qsort style comparison function -- compare sort coordinate
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static int cb_qsort_compareSortCoordinates
(
const VuVertexSortKey *pA,
const VuVertexSortKey *pB
)
    {
    if (pA->sortCoordinate < pB->sortCoordinate)
        return -1;
    if (pA->sortCoordinate > pB->sortCoordinate)
        return 1;
    return 0;
    }
#endif
/* METHOD(default,none,constructVertexClusterArrays) */
/**
* @description Identify clusters of nearly-identical nodes.
* @remarks Only xy-coordinates are compared.
* @param pGraph                     IN OUT  graph to examine
* @param pClusterArray              OUT     array of NULL-deliminted blocks of nodes whose xy-coordinates lie within tolerance
* @param absTol                     IN      absolute tolerance for xy-coordinate comparison
* @param bSaveClusterIndexInNode    IN      whether to fill nodes' userId field with cluster index
* @param bReassignXYZ               IN      whether to assign common coordinates to the nodes within each cluster
* @group "VU Coordinates"
* @see vu_consolidateClusterCoordinates, vu_consolidateClusterCoordinatesAndSortByAngle
* @bsimethod                                    EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_collectClusters
(
VuSetP          pGraph,
VuArrayP        pClusterArray,
double          absTol,
bool            bSaveClusterIndexInNode,
bool            bReassignXYZ
)
    {
    int         pivotIndex, testIndex, nodeCount = 0;
    double      epsilon, epsilon2;
    VuVertexSortKey   sortInfo;
    DPoint3d    sortVector;
    DPoint2d    xy;
    int         numCluster;

    VuMask                  visitMask = vu_grabMask (pGraph);
    DPoint3d xyz;

    sortVector.Init (0.13363333323, 1.41423, 0.0);
    sortVector.Normalize ();
    epsilon = absTol;
    epsilon2 = sqrt (3.0) * epsilon;   // l_infinity vertex tol mapped to linear l_2 tol along random vector

    // MEMORY MANAGEMENT -- use the (one and only) cached mClusterSortArray that is ALWAYS preallocated (and will have lots of reserved space as reused)
    bvector<VuVertexSortKey> &clusterSortArray = pGraph->mClusterSortArray;
    clusterSortArray.clear ();
    vu_clearMaskInSet (pGraph, visitMask);
    VU_SET_LOOP (pSeedNode, pGraph)
        {
        if (!vu_getMask (pSeedNode, visitMask))
            {
            vu_markVertex (pSeedNode, visitMask);
            vu_getDPoint2d (&xy, pSeedNode);
            sortInfo.pNode = pSeedNode;
            sortInfo.xy = xy;
            sortInfo.sortCoordinate = xy.x * sortVector.x + xy.y * sortVector.y;
            sortInfo.nextInCluster = -1;
            clusterSortArray.push_back (sortInfo);
            nodeCount++;
            }
        }
    END_VU_SET_LOOP (pSeedNode, pGraph)

    std::sort (clusterSortArray.begin (), clusterSortArray.end (), cb_VuVertexSortKey_LT_VuVertexSortKey);

    /* Sorting along the single direction does a good but not complete job of bringing
        geometrically clustered points together in the sort array.
        Do linear search "within epsilon" in the sort order to complete the filtering.
        Push nodeId's back to NULL as nodes are gathered into clusters. */
#define NOT_YET_CLUSTERED(_pSortBuffer_,_index_) (_pSortBuffer_.at(_index_).nextInCluster == -1)
    numCluster = 0;
    for (pivotIndex = 0; pivotIndex < nodeCount; pivotIndex++)
        {
        if (NOT_YET_CLUSTERED (clusterSortArray, pivotIndex))
            {
            /* The nodeId is still there, therefore this is a the start of a new cluster. */
            double testLimit = clusterSortArray.at(pivotIndex).sortCoordinate + epsilon2;

            /* Link to self as singleton linked list */
            /* REMARK: vu_arrayAdd and vuArrayAddNull are noops if array pointer is null.
                    Don't bother testing for null pClusterArray.
            */
            clusterSortArray.at (pivotIndex).nextInCluster = pivotIndex;
            vu_arrayAdd (pClusterArray, clusterSortArray.at (pivotIndex).pNode);
            if (bSaveClusterIndexInNode)
                markNode (pGraph, clusterSortArray.at (pivotIndex).pNode, numCluster);
            vu_getDPoint3d (&xyz, clusterSortArray.at (pivotIndex).pNode);

            for (testIndex = pivotIndex + 1;
                 testIndex < nodeCount && clusterSortArray.at (testIndex).sortCoordinate <= testLimit;
                 testIndex++
                 )
                {
                if (NOT_YET_CLUSTERED(clusterSortArray, testIndex))
                    {
                    if (keysInSameCluster (pGraph, &clusterSortArray.at (pivotIndex), &clusterSortArray.at (testIndex), epsilon))
                        {
                        clusterSortArray.at (testIndex).nextInCluster = clusterSortArray.at (pivotIndex).nextInCluster;
                        clusterSortArray.at (pivotIndex).nextInCluster = testIndex;
                        vu_arrayAdd (pClusterArray, clusterSortArray.at (testIndex).pNode);
                        if (bSaveClusterIndexInNode)
                            markNode (pGraph, clusterSortArray.at (testIndex).pNode, numCluster);
                        if (bReassignXYZ)
                            vu_setDPoint3dAroundVertex (clusterSortArray.at (testIndex).pNode, &xyz);
                        }
                    }
                }
            vu_arrayAddNull (pClusterArray);
            numCluster++;
            }
        }

    vu_returnMask (pGraph, visitMask);
    }


/* METHOD(default,none,consolidateClusters) */
/**
* @description Identify clusters of nearly-identical nodes and assign common coordinates to the nodes within each cluster.
* @remarks Only xy-coordinates are compared.
* @remarks This function just calls ~mvu_collectClusters (pGraph, NULL, abstol, false, true).
* @param pGraph     IN OUT  graph to examine
* @param abstol     IN      absolute tolerance for xy-coordinate comparison
* @group "VU Coordinates"
* @see vu_collectClusters, vu_consolidateClusterCoordinatesAndSortByAngle
* @bsimethod                                    EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_consolidateClusterCoordinates
(
VuSetP  pGraph,
double abstol
)
    {
    vu_collectClusters (pGraph, NULL, abstol, false, true);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static bool lexicalCompare_cluster0_cluster1
(
VuEdgeSortKey const &coedgeA,
VuEdgeSortKey const &coedgeB
)
    {
    if (coedgeA.cluster0 < coedgeB.cluster0)
        return true;
    if (coedgeA.cluster0 > coedgeB.cluster0)
        return false;
    if (coedgeA.cluster1 < coedgeB.cluster1)
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static bool lexicalCompare_cluster0_theta
(
VuEdgeSortKey const &coedgeA,
VuEdgeSortKey const &coedgeB
)
    {
    if (coedgeA.cluster0 < coedgeB.cluster0)
        return true;
    if (coedgeA.cluster0 > coedgeB.cluster0)
        return false;
    if (coedgeA.theta < coedgeB.theta)
        return true;
    return false;
    }
typedef bvector<VuEdgeSortKey> VuEdgeSortArray;

void EdgeSortArray_Sort_cluster0_cluster1 (bvector<VuEdgeSortKey> &data)
    {
    std::sort (data.begin (), data.end (), lexicalCompare_cluster0_cluster1);
    }
void EdgeSortArray_Sort_cluster0_theta (bvector<VuEdgeSortKey> &data)
    {
    std::sort (data.begin (), data.end (), lexicalCompare_cluster0_theta);
    }


/**
* Build an array of <node,id,node,id,theta>
* Sling edges are ignored.
* @param bDeleteSlings IN true to find and delete slings.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void collectClusteredEdgeData
(
VuSetP  pGraph,
VuArrayP pClusterArray,
VuEdgeSortArray &sortArray,
VuMask bDeleteSlings
)
    {
    int numNode = 0;
    VuP pVertexSeed;
    VuEdgeSortKey data;
    int clusterRef, numClusterRef, numCluster;
    VuMask slingMask = vu_grabMask (pGraph);

    VU_SET_LOOP (pCurr, pGraph)
        {
        numNode++;
        vu_clrMask (pCurr, slingMask);
        }
    END_VU_SET_LOOP (pCurr, pGraph)

    sortArray.reserve (numNode);

    /* Save cluster id in all nodes. */
    numClusterRef = vu_arraySize (pClusterArray);
    numCluster = 0;
    for (clusterRef = 0; clusterRef < numClusterRef; clusterRef++, numCluster++)
        {
        for (;clusterRef < numClusterRef
                && NULL != (pVertexSeed = vu_arrayGetVuP (pClusterArray, clusterRef));
                clusterRef++)
            {
            vu_setInternalDataPAsIntAroundVertex (pVertexSeed, numCluster);
            }

        }

    VU_SET_LOOP (pCurr, pGraph)
        {
        VuP pMate = vu_edgeMate (pCurr);
        int currClusterIndex = vu_getInternalDataPAsInt (pCurr);
        int mateClusterIndex = vu_getInternalDataPAsInt (pMate);
        double dx, dy;
        data.cluster0 = currClusterIndex;
        data.cluster1 = mateClusterIndex;
        data.pNode0   = pCurr;
        data.pNode1   = pMate;
        vu_getDXY (&dx, &dy, pCurr);
        data.theta = Angle::Atan2 (dy, dx);
        if (bDeleteSlings && currClusterIndex == mateClusterIndex)
            {
            vu_setMask (pCurr, slingMask);
            vu_setMask (pMate, slingMask);
            }
        else
            sortArray.push_back (data);
        }
    END_VU_SET_LOOP (pCurr, pGraph)

    if (bDeleteSlings)
        vu_freeMarkedEdges (pGraph, slingMask);

    vu_returnMask (pGraph, slingMask);
    }

static void sortVertexLoops
(
VuSetP  pGraph,
VuEdgeSortArray &sortArray,
int mergeType
)
    {
    size_t iA, iB;
    VuEdgeSortKey edgeA, edgeB;
    size_t numCoedge = sortArray.size ();
    VuMask nullFaceMask = vu_grabMask (pGraph);
    int numDelete = 0;

    /* Turn all edges into isolated components */
    VU_SET_LOOP (pCurr, pGraph)
        {
        VuP pPred = vu_vpred (pCurr);
        vu_vertexTwist (pGraph, pCurr, pPred);
        }
    END_VU_SET_LOOP (pCurr, pGraph)

    EdgeSortArray_Sort_cluster0_cluster1 (sortArray);
    for (iA = 0; iA < numCoedge; iA++)
        {
        edgeA = sortArray[iA];
        }

    vu_clearMaskInSet (pGraph, nullFaceMask);

    /* Rebuild null face bundles */
    for (iA = 0; iA < numCoedge;)
        {
        VuP pNodeC0, pNodeC1;
        VuP pNodeA0, pNodeA1;
        edgeA = sortArray[iA];
        /*
           When we are preserving duplicate edges (i.e. creating null faces)
            A refers to edgeA as recorded in the array.  This is the same throughout the inner loop.
            B refers to edgeB as recorded in the array. This changes each pass through the inner loop.
            C refers to the exposed nodes of the the growing bundle.
                C1 is always A1.
                C0 is moves ahead to B0 at each pass.
            When we are deleting duplicate edges in pairs
                A is the first of two in sequence
                B is the second of two in sequence
                C doesn't matter
                After marking A, B we jump out and start over.  If there is a third, it becomes A.
            When we are deleting all after the first of a bundle
                A,C is remains the first in bundle
                B moves through the bundle.
        */
        pNodeC0 = pNodeA0 = edgeA.pNode0;
        pNodeC1 = pNodeA1 = edgeA.pNode1;

        if (   vu_getMask (pNodeA0, nullFaceMask)
            || vu_getMask (pNodeA1, nullFaceMask)
            )
            {
            /* Skip this edge -- it is already bundled. */
            iA++;
            }
        else
            {

            /* Pin edges together within blocks with identical start and end cluster */
            for (iB = iA + 1;
                // Watch the commas....
                iB < numCoedge
                && (edgeB = sortArray[iB],
                       (edgeA.cluster0 == edgeB.cluster0 && edgeA.cluster1 == edgeB.cluster1)
                   )
                ; iB++)
                {
                VuP pNodeB0 = edgeB.pNode0;
                VuP pNodeB1 = vu_fsucc (pNodeB0);
                /* B should be an isolated edge.  Check both ends to be sure ---
                        but if it's not something is really wrong. */
                if (vu_getMask (pNodeB0, nullFaceMask)
                    || vu_getMask (pNodeB1, nullFaceMask))
                    {
                    }
                else if (vu_vsucc (pNodeB0) == pNodeB0 && vu_vsucc (pNodeB1) == pNodeB1)
                    {
                    if (mergeType == VUUNION_PARITY)
                        {
                        /* We have found a parallel pair.
                           We are supposed to delete such.
                           Mark both edges null.
                           break out of the inner loop.
                           If there are more in parallel, they will alternately act as A and B.
                        */
                        vu_setMask (pNodeA0, nullFaceMask);
                        vu_setMask (pNodeA1, nullFaceMask);
                        vu_setMask (pNodeB0, nullFaceMask);
                        vu_setMask (pNodeB1, nullFaceMask);
                        numDelete += 2;
                        break;
                        }
                    else if (mergeType == VUUNION_KEEP_ONE_AMONG_DUPLICATES)
                        {
                        /* Mark B for deletion, leave A as the bundle */
                        vu_setMask (pNodeB0, nullFaceMask);
                        vu_setMask (pNodeB1, nullFaceMask);
                        numDelete += 1;
                        }
                    else
                        {
                        /* Paste B onto the evolving bundle.  Far side becomes
                            the exposed bundle node on the paste side.  Near side of A
                            remains exposed on the other side.
                            We do NOT increment the delete count -- this null really is
                                a face. (Unlike the previous 2 cases where the null marking
                                is really by EDGE.)
                        */
                        vu_vertexTwist (pGraph, pNodeC0, pNodeB0);
                        vu_vertexTwist (pGraph, pNodeB1, pNodeC1);
                        vu_setMask (pNodeC0, nullFaceMask);
                        vu_setMask (pNodeB1, nullFaceMask);
                        pNodeC0 = pNodeB0;
                        }
                    }
                }
            iA = iB;
            }
        }

    /* At each cluster, sort by theta.  The null face nodes are still there, just let them go.
       In delete mode, all there will be more marked null -- they all get skipped.
    */
    //bvector<VuEdgeSortKey>::difference_type numEntry = clusterSortArray.begin() - clusterSortArray.end ();
    for (iA = 0; iA < numCoedge; iA++)
        {
        edgeA = sortArray[iA];
        }
    EdgeSortArray_Sort_cluster0_theta (sortArray);
    for (iA = 0; iA < numCoedge; iA++)
        {
        edgeA = sortArray[iA];
        }

    for (iA = 0; iA < numCoedge;)
        {
        VuP pNodeC;
        edgeA = sortArray[iA];

        /* Node C is the exposed sector where subsequent edges (or bundles) are inserted to the vertex. */
        pNodeC = edgeA.pNode0;

        if (vu_getMask (pNodeC, nullFaceMask))
            pNodeC = NULL;

        for (iB = iA + 1;
            // Watch the commas....
               iB < numCoedge
            && (edgeB = sortArray[iB],
                edgeA.cluster0 == edgeB.cluster0)
            ; iB++)
            {
            VuP pNodeB = edgeB.pNode0;
            if (!vu_getMask (pNodeB, nullFaceMask))
                {
                if (pNodeC)
                    {
                    vu_vertexTwist (pGraph, pNodeB, pNodeC);
                    }
                pNodeC = pNodeB;
                }
            }
        iA = iB;
        }

    if (numDelete > 0)
        vu_freeMarkedEdges (pGraph, nullFaceMask);

    vu_returnMask (pGraph, nullFaceMask);
    }

/**
* @description Identify clusters of nearly-identical nodes, assign common coordinates to the nodes within each cluster, and sort duplicate
*       edges by angle and mergeType.
* @remarks Only xy-coordinates are compared.
* @param pGraph     IN OUT  graph to examine
* @param mergeType  IN      rule to apply to cull duplicate edges, e.g. VUUNION_PARITY, VUUNION_KEEP_ONE_AMONG_DUPLICATES or otherwise for no culling
* @param abstol     IN      absolute tolerance for xy-coordinate comparison
* @group "VU Coordinates"
* @see vu_collectClusters, vu_consolidateClusterCoordinates
* @bsimethod                                    EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_consolidateClusterCoordinatesAndSortByAngle
(
VuSetP      pGraph,
VuMergeType mergeType,
double      abstol
)
    {
    VuArrayP pClusterArray = vu_grabArray (pGraph);

    vu_collectClusters (pGraph, pClusterArray, abstol, false, true);
    VuEdgeSortArray &edgeArray = pGraph->mEdgeSortArray;
    edgeArray.clear ();

    collectClusteredEdgeData (pGraph, pClusterArray, edgeArray, true);

    sortVertexLoops (pGraph, edgeArray, mergeType);

    vu_returnArray (pGraph, pClusterArray);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
