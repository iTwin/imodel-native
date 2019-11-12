/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// EDL March 2015.
// This function uses the IndexedHeap sorter.
// This looks like an improvement over the older splitMesh_go algorithms:
// 1) as-good-as-or-better partitions in limited GUI testing.
// 2) synergy with bvector-based heaps.
// So I am making it the default in PolyfaceHeader::PartitionByXYRange.
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool PartitionByIndexedRangeHeap (
    PolyfaceHeaderR mesh,
    bvector<PolyfaceHeaderPtr> &submeshArray,
    size_t targetFaceCount,
    size_t targetMeshCount
    )
    {
    static bool s_sortX = true;
    static bool s_sortY = true;
    static bool s_sortZ = false;
    PolyfaceIndexedHeapRangeTreePtr tree = PolyfaceIndexedHeapRangeTree::CreateForPolyface (mesh, s_sortX, s_sortY, s_sortZ);
    size_t numFacet = tree->GetNumRanges ();
    if (numFacet < 2)
        return false;

    int meshCountDepth = 0;
    int faceCountDepth = 0;
    if (targetMeshCount > 1)
        meshCountDepth = (int)ceil (log((double)targetMeshCount) / log (2.0));
    if (targetFaceCount > 1 && targetFaceCount * 2 < numFacet)
        {
        faceCountDepth = (int)ceil (log ((double)numFacet / (double)targetFaceCount) / log (2.0));
        }

    int depth = 0;
    if (faceCountDepth > depth)
        depth = faceCountDepth;
    if (meshCountDepth > depth)
        depth = meshCountDepth;

    bvector<bvector<size_t>> raggedReadIndices;
    tree->CollectReadIndicesByTreeDepth (raggedReadIndices, depth);
    bvector<ptrdiff_t> blockedReadIndex;
    for (bvector<size_t> const &block : raggedReadIndices)
        {
        for (size_t readIndex : block)
            {
            blockedReadIndex.push_back ((ptrdiff_t)readIndex);
            }
        blockedReadIndex.push_back (-1);
        }
    submeshArray.clear ();
    return mesh.CopyPartitions (blockedReadIndex, submeshArray);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::PartitionByXYRange
(
size_t targetFaceCount,
size_t targetMeshCount,
bvector<PolyfaceHeaderPtr> &submeshArray
)
    {
    if (PartitionByIndexedRangeHeap (*this, submeshArray, targetFaceCount, targetMeshCount))
        return true;
    
    bvector<ptrdiff_t> blockedReadIndex;
    submeshArray.clear ();
    return PartitionByXYRange (targetFaceCount, targetMeshCount, blockedReadIndex)
        && CopyPartitions (blockedReadIndex, submeshArray);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::PartitionMaintainFaceOrder
(
size_t targetFaceCount,
size_t targetMeshCount,
bvector<PolyfaceHeaderPtr> &submeshArray
)
    {
    bvector<ptrdiff_t> blockedReadIndex;
    submeshArray.clear ();
    return PartitionMaintainFaceOrder (targetFaceCount, targetMeshCount, blockedReadIndex)
        && CopyPartitions (blockedReadIndex, submeshArray);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::PartitionReadIndicesByNormal(DVec3dCR vector, bvector<bvector<ptrdiff_t>> &readIndices, double sideFaceRadiansTolerance)
    {
    const int forwardIndex = 0;
    const int reverseIndex = 1;
    const int sideIndex = 2;
    readIndices.clear();
    readIndices.push_back(bvector<ptrdiff_t>());
    readIndices.push_back(bvector<ptrdiff_t>());
    readIndices.push_back(bvector<ptrdiff_t>());
    PolyfaceVisitorPtr      visitor = PolyfaceVisitor::Attach(*this);
    ValidatedDVec3d unit = vector.ValidatedNormalize();
    if (!unit.IsValid())
        return false;
    double tolerance = Angle::SmallAngle();
    if (sideFaceRadiansTolerance > 0.0)
        tolerance = sideFaceRadiansTolerance;
    DPoint3d centroid;
    DVec3d normal;
    double area;
    for (visitor->Reset(); visitor->AdvanceToNextFace();)
        {
        size_t readIndex = visitor->GetReadIndex();
        if (visitor->TryGetFacetCentroidNormalAndArea(centroid, normal, area))
            {
            double angleDeviation = normal.AngleFromPerpendicular (unit);    // this is always positive
            if (fabs(angleDeviation) < tolerance)
                readIndices[sideIndex].push_back(readIndex);
            else
                {
                double d = normal.DotProduct (unit);
                if (d > 0.0)
                    readIndices[forwardIndex].push_back(readIndex);
                else 
                    readIndices[reverseIndex].push_back(readIndex);
                }
            }
        }
    return true;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::PartitionByConnectivity (int connectivityType, bvector<PolyfaceHeaderPtr> &submeshArray) const
    {
    bvector<ptrdiff_t> blockedReadIndex;
    submeshArray.clear ();
    return PartitionByConnectivity (connectivityType, blockedReadIndex)
        && CopyPartitions (blockedReadIndex, submeshArray);
    }

static int s_noisy = 0;
/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct FaceComponentData
    {
    int readIndex;
    int componentId;
    } FaceComponentData;


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct EdgeComponentData
    {
    int m_vertexA;
    int m_vertexB;
    size_t m_readIndex;
    int m_id;
    bool m_visible;

EdgeComponentData (int vertexA, int vertexB, size_t readIndex, int id, bool visible)
    {
    m_vertexA = vertexA;
    m_vertexB = vertexB;
    m_readIndex = readIndex;
    m_id = id;
    m_visible = visible;
    }
// qsort comparison for lexical sort on (lowVertexIndex, highVertexIndex)
static bool cb_lt_lowVertexHighVertex
(
EdgeComponentData const &edge0,
EdgeComponentData const &edge1
)
    {
    int low0 = edge0.m_vertexA < edge0.m_vertexB ? edge0.m_vertexA : edge0.m_vertexB;
    int low1 = edge1.m_vertexA < edge1.m_vertexB ? edge1.m_vertexA : edge1.m_vertexB;
    if (low0 < low1)
        return true;
    if (low0 > low1)
        return false;

    int high0 = edge0.m_vertexA > edge0.m_vertexB ? edge0.m_vertexA : edge0.m_vertexB;
    int high1 = edge1.m_vertexA > edge1.m_vertexB ? edge1.m_vertexA : edge1.m_vertexB;
    if (high0 < high1)
        return true;
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool cb_lt_id_readIndex
(
EdgeComponentData const &edge0,
EdgeComponentData const &edge1
)
    {
    if (edge0.m_id < edge1.m_id)
        return true;
    if (edge0.m_id > edge1.m_id)
        return false;
    if (edge0.m_readIndex < edge1.m_readIndex)
        return true;
    return false;
    }

};


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct FacePartitionData
    {
    DRange3d range;
    int readIndex;
    ptrdiff_t nextInLinkedList;
    int axisToInterval[3];
    int binIndex;
    };


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct RecursionState
{
PolyfaceHeaderR m_polyface;
bvector<FacePartitionData> faceDataArray;
bvector<ptrdiff_t> seedArray;
size_t m_targetFaceCount;

size_t TargetFaceCount (){return m_targetFaceCount;}
RecursionState (PolyfaceHeaderR polyface)
    : m_polyface (polyface)
    {
    faceDataArray.reserve (polyface.PointIndex().size() / 4);
    }

bool SetTargets (size_t targetFaceCount, size_t targetMeshCount)
    {
    size_t numFace = faceDataArray.size ();

    if (targetFaceCount > 0 && targetMeshCount <= 1)
        {
        m_targetFaceCount = targetFaceCount;
        }
    else if (targetMeshCount > 1 && targetFaceCount <= 0)
        {
        m_targetFaceCount = (int)(0.998 + (double)numFace / (double) targetMeshCount);
        }
    else if (targetFaceCount > 1 && targetMeshCount >= 2)
        {
        m_targetFaceCount = (int)(0.998 + (double)numFace / (double)targetMeshCount);
        if (targetFaceCount < m_targetFaceCount)
            {
            m_targetFaceCount = targetFaceCount;
            }
        }
    else
        {
        /* Really don't know whats happening here. */
        m_targetFaceCount = numFace / 2;
        return false;
        }
    return true;
    }


bool Load ()
    {
    PolyfaceVisitorPtr      visitor = PolyfaceVisitor::Attach (m_polyface);

    FacePartitionData                faceData;
    double                  d0, eps = 1.0e-10;

    visitor->SetNumWrap (1);
    /* For each face:
        Save range, readIndex.
        Collect all faces in singly linked list in reverse order
    */
    bool ignore = false;
    size_t numNull = 0;
    bvector<DPoint3d>const& points = visitor->Point ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        size_t numThisFace = visitor->NumEdgesThisFace ();
        if (numThisFace < 3)
            continue;
        //TR233436
        // EDL May 2013 This code appears to purge degenerate triangles.
        //  It was done for a TR, so changing it would break something somewhere.
        //  So we leave it.
        //  But I don't approve -- if the degenerate triangle was of interest in the parent mesh,
        //   why does the splitter take it upon itself to declare it invalid?
        ignore = false;
        size_t numSignificantEdges = 0;
        for(size_t i = 0; i < numThisFace; i++)
            {
            d0 = points[i].Distance (points[i+1]);
            if(d0 > eps)
                numSignificantEdges++;
            }
        if(numSignificantEdges < 3)
            ignore = true;
        if (ignore)
            {
            numNull++;
            }
        else
            {
            faceData.readIndex = (int)visitor->GetReadIndex  ();
            faceData.range.Init ();
            faceData.range.Extend (visitor->Point ());
            faceData.nextInLinkedList = faceDataArray.size () - 1; // current tail index.
            faceDataArray.push_back (faceData);
            }
        }
    return faceDataArray.size () > 0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TryGet(FacePartitionData &faceData, ptrdiff_t index)
    {
    if (index < 0 || index >= (ptrdiff_t)faceDataArray.size ())
        return false;
    faceData = faceDataArray[(size_t) index];
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TrySet(FacePartitionData &faceData, ptrdiff_t index)
    {
    if (index >= 0 && index < (ptrdiff_t)faceDataArray.size ())
        {
        faceDataArray[(size_t)index] = faceData;
        return true;
        }
    return false;
    }


/*
Compute the range (and count) for a FacePartitionData linked list.
@param pRange IN composite range
@param pArray IN parent array containing linked list
@param seedIndex IN index of head of linked list.
*/
size_t llRange
(
DRange3dR range,
ptrdiff_t seedIndex
)
    {
    size_t count = 0;
    ptrdiff_t currIndex = seedIndex;
    FacePartitionData faceData;
    range.Init ();

    while (TryGet (faceData, currIndex))
        {
        count++;
        range.Extend (faceData.range);
        currIndex = faceData.nextInLinkedList;
        }

    return count;
    }



};


static int s_maxRecursion = 6;  /* At least two rounds per dimension */


/*
Pull out of bounds indice back to 0..count-1
@param k IN index to adjust.
@param count IN upper limit (non-inclusive)
@return corrected index.
*/
static int restrictIndex
(
int k,
int count
)
    {
    if (k < 0)
        return 0;
    if (k >= count)
        return count - 1;
    return k;
    }


/*
Assign intervals into bins.
*/
static void assignIntervalsToBins
(
int pIntervalToBin[],
int *pNumBin,
int pFacesInInterval[][3],
int numInterval,
int axisId,
size_t targetFaceCount
)
    {
    size_t currCount = 0;
    int i;
    int currBin = 0;
    *pNumBin = 0;
    if (s_noisy > 2)
        printf (" assignIntervalsToBins (numInterval %d) (targetFaceCount %d)\n",
                        numInterval, (int)targetFaceCount);
    for (i = 0; i < numInterval; i++)
        {
        currCount += pFacesInInterval[i][axisId];
        pIntervalToBin[i] = currBin;
        *pNumBin = currBin + 1;
        if (s_noisy > 2)
            printf ("\t\t(i %d) (bin %d) (currCount %d)\n", i, currBin, (int)currCount);
        if (currCount > targetFaceCount || (i == numInterval - 1 && currCount > 0))
            {
            if (s_noisy > 2)
                printf ("\t(bin %d) (count %d)\n", (int)currBin, (int)currCount);
            currBin++;
            currCount = 0;
            }
        }
    }

//
// Subivide range into numInterval bins in each direction.
// Count the number of faces (in linked list) whose range max falls in each bin.
//
static void llCountFacesInIntervals
(
int pFacesInInterval[][3],
int numInterval,
DRange3d *pRange,
RecursionState &rc,
ptrdiff_t seedIndex
)
    {
    FacePartitionData faceData;
    double delta[3];
    double xyz0[3];
    double xyz[3];
    int i, k;
    double diagonal;
    static double s_diagonalFraction = 0.01;
    double dMin;
    memset (pFacesInInterval, 0, numInterval * 3 * sizeof (int));

    xyz0[0] = pRange->low.x;
    xyz0[1] = pRange->low.y;
    xyz0[2] = pRange->low.z;

    delta[0] = (pRange->high.x - pRange->low.x) / numInterval;
    delta[1] = (pRange->high.y - pRange->low.y) / numInterval;
    delta[2] = (pRange->high.z - pRange->low.z) / numInterval;

    diagonal = pRange->low.Distance (pRange->high);

    dMin = s_diagonalFraction * diagonal;

    for (i = 0; i < 3; i++)
        if (delta[i] <= dMin)
            delta[i] = dMin;

    for (ptrdiff_t currIndex = seedIndex;
        rc.TryGet (faceData, currIndex);
        currIndex = faceData.nextInLinkedList
        )
        {
        xyz[0] = faceData.range.high.x;
        xyz[1] = faceData.range.high.y;
        xyz[2] = faceData.range.high.z;
        for (i = 0; i < 3; i++)
            {
            k = (int)((xyz[i] - xyz0[i]) / delta[i]);
            k = restrictIndex (k, numInterval);
            pFacesInInterval[k][i] += 1;
            faceData.axisToInterval[i] = k;
            }
        rc.TrySet (faceData, currIndex);
        }

    if (s_noisy > 2)
        {
        printf (" Faces per interval on initial bucket split\n");
        for (i = 0; i < numInterval; i++)
            {
            printf ("\t%d  (%dX,%dY,%dZ)\n", i,
                    pFacesInInterval[i][0],
                    pFacesInInterval[i][1],
                    pFacesInInterval[i][2]
                    );
            }
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void assignFacesToBins
(
int *pIntervalToBinId,
int numInterval,
ptrdiff_t *pBinToSeed,
int numBin,
RecursionState &rc,
ptrdiff_t seedIndex,
int axisIndex
)
    {
    int bin;
    FacePartitionData faceData;
    int intervalIndex, binIndex;

    for (bin = 0; bin < numBin; bin++)
        {
        pBinToSeed[bin] = -1;
        }

    for (ptrdiff_t currIndex = seedIndex;
        rc.TryGet (faceData, currIndex);
        )
        {
        ptrdiff_t nextIndex = faceData.nextInLinkedList;
        intervalIndex = restrictIndex (faceData.axisToInterval[axisIndex], numInterval);
        binIndex = restrictIndex (pIntervalToBinId[intervalIndex], numBin);
        faceData.nextInLinkedList = pBinToSeed[binIndex];
        rc.TrySet (faceData, currIndex);
        pBinToSeed[binIndex] = currIndex;
        if (s_noisy > 9)
            printf (" face %d interval %d bin %d\n", (int)currIndex, (int)intervalIndex, (int)binIndex);
        currIndex = nextIndex;
        }
    }

/*
Find the axis direction with largest range, possibliy excluding an axis.
*/
static int selectAxisIndex
(
const DRange3d *pRange,
int skipAxis
)
    {
    double dMax, di;
    int    iMax, i;
    iMax = -1;
    dMax = -1.0;
    for (i = 0; i < 3; i++)
        {
        if (i != skipAxis)
            {
            di = fabs (pRange->high.GetComponent (i) - pRange->low.GetComponent (i));
            if (di > dMax)
                {
                dMax = di;
                iMax = i;
                }
            }
        }
    return iMax;
    }
/*
@description Recursive partitioning step.
@param pRC IN context not affected by recursion
@param seedIndex IN index of head of linked list to analyze
@param maxRecursion IN number of recursions allowed.
@param previousAxis IN axis used on parent split.  This split will choose a different axis.
*/
static void splitMesh_go
(
RecursionState &rc,
ptrdiff_t seedIndex,
int maxRecursion,
int previousAxis
)
    {
    DRange3d listRange;
#define MAX_INTERVAL 40
// We divide the range in one axis into INTERVALs.
// The number of intervals is generally larger than number of bins produced;
//    contiguous intervals are combined to make bins of appropriate size.
    int  xyzCounter[MAX_INTERVAL][3];
    int  intervalToBin[MAX_INTERVAL];
    ptrdiff_t  binToSeedFace[MAX_INTERVAL];
    int numInterval;
    int numBin;
    double dNumBin;
    int i;
    int axisIndex;
    if (seedIndex < 0)
        return;

    size_t numFace = rc.llRange (listRange, seedIndex);
    if (s_noisy > 1)
        {
        printf ("splitMesh_go\n");
        printf ("\t(maxRecursion=%d)\n", maxRecursion);
        printf ("\t(seed=%d)\n", (int)seedIndex);
        printf ("\t(numFace=%d)\n", (int)numFace);
        printf("(targetFaceCount %d)\n", (int)rc.TargetFaceCount());
        }
    bvector<ptrdiff_t> seedArray;
    // If list is already short, just record it and quit.
    if (numFace <= rc.TargetFaceCount () || maxRecursion <= 0)
        {
        rc.seedArray.push_back (seedIndex);
        if (s_noisy > 0)
            printf(" !! Accept %d faces from seed %d\n", (int)numFace, (int)seedIndex);
        return;
        }

    numInterval = MAX_INTERVAL;
    llCountFacesInIntervals (xyzCounter, numInterval, &listRange,
                    rc, seedIndex);

    dNumBin = (double)numFace / (double)rc.TargetFaceCount ();
    if (dNumBin >= 4)
        dNumBin = sqrt (dNumBin);
    numBin = (int)dNumBin;
    if (numBin > numInterval)
        numBin = numInterval;

    if (numBin < 2)
        numBin = 2;

    //currTarget = pRC->targetFaceCount / numBin;
    size_t currTarget = (size_t)((double)numFace / (double)numBin);
    if (currTarget > rc.TargetFaceCount ())
        currTarget = rc.TargetFaceCount ();

    axisIndex = selectAxisIndex (&listRange, previousAxis);
    if (s_noisy > 1)
        {
        printf ("\t(axisIndex %d)\n", axisIndex);
        printf ("\t(numBin %d)\n", numBin);
        }
    assignIntervalsToBins (intervalToBin, &numBin, xyzCounter, numInterval, axisIndex, currTarget);
    assignFacesToBins (intervalToBin, numInterval, binToSeedFace, numBin, rc, seedIndex, axisIndex);

    for (i = 0; i < numBin; i++)
        {
        splitMesh_go (rc, binToSeedFace[i], maxRecursion - 1, axisIndex);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void splitMesh_collectPartition
(
RecursionState &rc,
bvector<ptrdiff_t> &blockedReadIndexArray
)
    {
    FacePartitionData faceData;
    int numThisBlock;
    blockedReadIndexArray.clear ();

    for (size_t i = 0; i < rc.seedArray.size (); i++)
        {
        size_t seedIndex = rc.seedArray[i];
        numThisBlock = 0;
        for (size_t currIndex = seedIndex;
            rc.TryGet (faceData, currIndex);
            currIndex = faceData.nextInLinkedList
            )
            {
            blockedReadIndexArray.push_back (faceData.readIndex);
            numThisBlock++;
            }
        if (numThisBlock > 0)
            blockedReadIndexArray.push_back (-1);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::PartitionByXYRange
(
size_t targetFaceCount,
size_t targetMeshCount,
bvector<ptrdiff_t> &blockedReadIndexArray
)
    {
    ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();

    PolyfaceVisitorPtr      visitor = PolyfaceVisitor::Attach (*this);
    RecursionState          rc (*this);

    rc.Load ();
    rc.SetTargets (targetFaceCount, targetMeshCount);


    if (s_noisy > 0)
        printf ("**** Starting split of mesh with %d original faces\n", (int)rc.faceDataArray.size ());

    splitMesh_go (rc, rc.faceDataArray.size () - 1, s_maxRecursion, -1);
    splitMesh_collectPartition (rc, blockedReadIndexArray);

    return true;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::PartitionMaintainFaceOrder
(
size_t targetFaceCount,
size_t targetMeshCount,
bvector<ptrdiff_t> &blockedReadIndexArray
)
    {
    ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();

    PolyfaceVisitorPtr      visitor = PolyfaceVisitor::Attach (*this, false);
    visitor->SetNumWrap (0);
    size_t numFace = 0;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        numFace++;
        }


    size_t myTargetFaceCount = 0;
    if (targetFaceCount > 0 && targetMeshCount <= 1)
        {
        myTargetFaceCount = targetFaceCount;
        }
    else if (targetMeshCount > 1 && targetFaceCount <= 0)
        {
        myTargetFaceCount = (size_t)((double)numFace / (targetMeshCount - 0.5));
        }
    else if (targetFaceCount > 1 && targetMeshCount >= 2)
        {
        myTargetFaceCount = (size_t)((double)numFace / (targetMeshCount - 0.5));
        if (targetFaceCount < myTargetFaceCount)
            {
            myTargetFaceCount = targetFaceCount;
            }
        }
    else
        {
        /* Really don't know whats happening here. */
        myTargetFaceCount = numFace / 2;
        }

    if (myTargetFaceCount < 1)
        myTargetFaceCount = 0;

    size_t numThisBlock = 0;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        blockedReadIndexArray.push_back (visitor->GetReadIndex  ());
        numThisBlock++;
        if (numThisBlock >= myTargetFaceCount)
            {
            blockedReadIndexArray.push_back (-1);
            numThisBlock = 0;
            }
        }

    if (numThisBlock > 0)
        blockedReadIndexArray.push_back (-1);
    return true;
    }

// On input indices has (possibly signed) one-based, zero terminated indices to source.
// On output, the subset of source values that are actually referenced is copied (packed) to dest,
//   and the (still signed, one based) indice are to the packed data.
template <typename T>
void ReindexAndCopyOneBasedIndexedData
(
T const *source,
size_t   numSource,
bvector<T> &dest,
BlockedVectorInt &oneBasedIndices,
bvector<ptrdiff_t>oldToNew
)
    {
    if (oneBasedIndices.Active () && source != nullptr && numSource != 0)
        {
        oldToNew.clear ();
        oldToNew.reserve (numSource);
        for (size_t i = 0; i < numSource; i++)
            oldToNew.push_back (-1);       // source[i] is not yet needed in dest.

        dest.clear ();
        size_t numIndex = oneBasedIndices.size ();
        for (size_t i = 0; i < numIndex; i++)
            {
            int index1 = oneBasedIndices[i];
            if (index1 != 0)
                {
                size_t index0 = (size_t)(abs (index1) - 1);
                if (index0 < numSource)
                    {
                    if (oldToNew[index0] < 0) // This is the first use of source[index0]
                        {
                        oldToNew[index0] = dest.size ();
                        dest.push_back (source[index0]);
                        }
                    oneBasedIndices[i] = 1 + (int)oldToNew[index0];
                    if (index1 < 0) // restore sign
                        oneBasedIndices[i] = - oneBasedIndices[i];
                    }
                else
                    {
                    // bad index. change to zero to keep sizes right ....
                    oldToNew[i] = 0;
                    }
                }
            }
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
template <typename T>
static void CopyAndActivate (BlockedVector<T> & dest, T const *sourceData, size_t sourceCount)
    {
    dest.clear ();
    if (sourceData != NULL && sourceCount > 0)
        {
        dest.Append (sourceData, sourceCount);
        dest.SetActive (true);
        }
    }

// on input, dest has indices into source, but no data.
// Revise indices to copy minimal data.
// indices is work array.
static void ReindexAndAssembleOneBasedIndexedData (PolyfaceQueryCR source, PolyfaceHeaderR dest, bvector<ptrdiff_t> &oldToNew)
    {
    PolyfaceQueryP sourceP = const_cast <PolyfaceQueryP> (&source);
    size_t n = sourceP->GetPointCount ();
    // If copying from "normal per vertex" style, replicate the point index arrays for params/normals/colors
    if (sourceP->GetParamCount () == n
        && dest.ParamIndex ().size () == 0
        && sourceP->GetParamIndexCP (false) == nullptr
        && sourceP->GetParamIndexCP (true) != nullptr
        )
        CopyAndActivate (dest.ParamIndex (), dest.GetPointIndexCP (), dest.PointIndex ().size ());
    if (sourceP->GetNormalCount () == n
        && dest.NormalIndex ().size () == 0
        && sourceP->GetNormalIndexCP (false) == nullptr
        && sourceP->GetNormalIndexCP (true) != nullptr
        )
        CopyAndActivate (dest.NormalIndex (), dest.GetPointIndexCP (), dest.PointIndex ().size ());
    if (   sourceP->GetColorCount () == n
        && dest.ColorIndex ().size () == 0
        && sourceP->GetColorIndexCP (false) == nullptr
        && sourceP->GetColorIndexCP (true) != nullptr
        )
        CopyAndActivate (dest.ColorIndex (), dest.GetPointIndexCP (), dest.PointIndex ().size ());


    ReindexAndCopyOneBasedIndexedData (sourceP->GetPointCP (),  sourceP->GetPointCount (),  dest.Point (),  dest.PointIndex (),  oldToNew);
    ReindexAndCopyOneBasedIndexedData (sourceP->GetNormalCP (), sourceP->GetNormalCount (), dest.Normal (), dest.NormalIndex (), oldToNew);
    ReindexAndCopyOneBasedIndexedData (sourceP->GetParamCP (),  sourceP->GetParamCount (),  dest.Param (),  dest.ParamIndex (),  oldToNew);
    if (dest.ColorIndex ().Active ())
        {
        if (dest.IntColor ().Active ())
            ReindexAndCopyOneBasedIndexedData (sourceP->GetIntColorCP (), sourceP->GetColorCount (), dest.IntColor (), dest.ColorIndex (), oldToNew);
        }
    dest.Compress ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void AppendShiftedIndicesWithTerminator(BlockedVectorIntCR source, BlockedVectorIntR dest, int baseValue)
    {
    if (source.Active ())
        {
        for (size_t i = 0, n = source.size (); i < n; i++)
            dest.push_back (source[i] + baseValue);
        dest.push_back (0);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void AppendSignedOneBasedIndicesWithTerminator(BlockedVectorIntCR source, bvector<bool> &flag, BlockedVectorIntR dest)
    {
    if (source.Active ())
        {
        for (size_t i = 0, n = source.size (); i < n; i++)
            {
            dest.push_back (flag[i] ? source[i] + 1 : -(source[i] + 1));
            }
        dest.push_back (0);
        }    
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void AppendVisitorFace
(
PolyfaceQueryCR sourceMesh,
size_t faceIndex,
PolyfaceVisitor &   visitor,
PolyfaceHeader &destMesh
)
    {
    AppendSignedOneBasedIndicesWithTerminator (visitor.ClientPointIndex (), visitor.Visible (), destMesh.PointIndex());
    AppendShiftedIndicesWithTerminator (visitor.ClientNormalIndex (), destMesh.NormalIndex (), 1);
    AppendShiftedIndicesWithTerminator (visitor.ClientParamIndex (), destMesh.ParamIndex (), 1);
    AppendShiftedIndicesWithTerminator (visitor.ClientColorIndex (), destMesh.ColorIndex (), 1);
    AppendShiftedIndicesWithTerminator (visitor.ClientFaceIndex (),  destMesh.FaceIndex (), 1);
    }

static bool Find (bvector <ptrdiff_t> const &source, ptrdiff_t target)
    {
    for (size_t i = 0; i < source.size (); i++)
        {
        if (source[i] == target)
            return true;
        }
    return false;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::SelectBlockedIndices
(
bvector<ptrdiff_t> const &blockedReadIndex,
bvector<ptrdiff_t> const &selectedReadIndex,
bool keepIfSelected,
bvector<ptrdiff_t> &blockedReadIndexOut
)
    {
    blockedReadIndexOut.clear ();
    size_t i0 = 0;
    for (size_t i1 = 0; i1 < blockedReadIndex.size (); i1++)
        {
        if (blockedReadIndex[i1] < 0)
            {
            bool found = false;
            for (size_t i = i0; i < i1; i++)
                {
                if (Find (selectedReadIndex, blockedReadIndex[i]))
                    {
                    found = true;
                    break;
                    }
                }
            if (found == keepIfSelected)
                {
                for (size_t i = i0; i <= i1; i++)
                    blockedReadIndexOut.push_back (blockedReadIndex[i]);
                }                
            i0 = i1 + 1;
            }
        }
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::CopyPartitions 
(
bvector<ptrdiff_t> const& blockedReadIndex,
bvector<PolyfaceHeaderPtr> &submeshArray
) const
    {
    submeshArray.empty ();
    bvector<ptrdiff_t>readIndexToPartition;
    ptrdiff_t maxReadIndex = 0;
    for (size_t i = 0, n = blockedReadIndex.size (); i < n; i++)
        {
        if (blockedReadIndex[i] > maxReadIndex)
            maxReadIndex = blockedReadIndex[i];
        }

    for (size_t i = 0; i <= (size_t)maxReadIndex; i++)
        readIndexToPartition.push_back (-1);
    size_t numPartition = 0;
    bool needZero = true;
    for (size_t i = 0, n = blockedReadIndex.size (); i < n; i++)
        {
        ptrdiff_t readIndex = blockedReadIndex[i];
        if (blockedReadIndex[i] < 0)
            {
            numPartition++;
            needZero = false;
            }
        else
            {
            readIndexToPartition[(size_t)readIndex] = numPartition;
            needZero = true;
            }
        }

    if (needZero)
        numPartition++;

    PolyfaceHeader const * thisAsHeader = dynamic_cast <PolyfaceHeader const *> (this);
    for (size_t i = 0; i < numPartition; i++)
        {
        submeshArray.push_back (PolyfaceHeader::CreateVariableSizeIndexed ());
        if (thisAsHeader)
            submeshArray.back ()->CopyAllActiveFlagsFrom (*thisAsHeader);
        else
            submeshArray.back ()->CopyAllActiveFlagsFromQuery (*this);
        }

    PolyfaceVisitorPtr      visitor = PolyfaceVisitor::Attach (*this, true);
    visitor->SetNumWrap (0);
    size_t faceIndex = 0, readIndexToPartitionSize = readIndexToPartition.size();
    for (visitor->Reset (); visitor->AdvanceToNextFace (); faceIndex++)
        {
        size_t readIndex = visitor->GetReadIndex ();
        if (readIndex >= readIndexToPartitionSize) // Partition marked this as a face to ignore.
            continue;
        ptrdiff_t partitionIndex = readIndexToPartition[readIndex];
        if (partitionIndex < 0) // Partition marked this as a face to ignore.
            continue;
        assert ((size_t)partitionIndex < submeshArray.size ());
        AppendVisitorFace (*this, faceIndex, *visitor, *submeshArray[(size_t)partitionIndex]);
        }

    bvector<ptrdiff_t> indices;    // Work array, heavily used in copy step.
    for (size_t i = 0; i < submeshArray.size (); i++)
        ReindexAndAssembleOneBasedIndexedData (*this, *submeshArray[i], indices);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::CopyPartitions 
(
bvector<bvector<ptrdiff_t>> &blockedReadIndex,
bvector<PolyfaceHeaderPtr> &submeshArray
) const
    {
    PolyfaceVisitorPtr      visitor = PolyfaceVisitor::Attach(*this, true);
    visitor->SetNumWrap(0);

    submeshArray.empty ();
    size_t numPartition = blockedReadIndex.size();
    size_t faceIndex = 0;
    for (size_t partitionIndex = 0; partitionIndex < numPartition; partitionIndex++)
        {
        submeshArray.push_back (PolyfaceHeader::CreateVariableSizeIndexed ());
        submeshArray.back ()->CopyAllActiveFlagsFromQuery (*this);
        for (size_t readIndex : blockedReadIndex[partitionIndex])
            {
            faceIndex++;
            if (visitor->MoveToFacetByReadIndex (readIndex))
                AppendVisitorFace(*this, faceIndex, *visitor, *submeshArray[(size_t)partitionIndex]);
            }
        }

    bvector<ptrdiff_t> indices;    // Work array, heavily used in copy step.
    for (size_t partitionIndex = 0; partitionIndex < submeshArray.size (); partitionIndex++)
        ReindexAndAssembleOneBasedIndexedData (*this, *submeshArray[partitionIndex], indices);
    return true;
    }






bool PolyfaceQuery::PartitionByConnectivity (int connectivityType, bvector<ptrdiff_t> &blockedReadIndexArray) const
    {
    blockedReadIndexArray.clear ();

    PolyfaceVisitorPtr      visitor = PolyfaceVisitor::Attach (*this, false);
    visitor->SetNumWrap (1);
// Step 1: Build array of edge data.  Each edge is <vertexA, vertexB, readIndex, clusterId>
//     where
//          vertexA, vertexB are (absolute value of) vertex index from face loops.
//          readIndex is first read index for this face.
//          clusterId is initially assigned by face.
//   During step 1, also build cluster array (which is just sequential integers).  All the edges for a particular face
//          are assigned to the same cluster.
// Step 2: sort the edge array so edges with the same vertex pair are adjacent.
// Step 3: within the sorted array, adjacent edges with the same vertex pair represent faces sharing the edge.
//          Tell the cluster manager to merge those two clusters.
// Step 4: Walk the edge array asking the cluster manager for the "final" cluster id of each edge.
// Step 5: Resort by cluster id and read index.
// Step 6: Extract unique read indexes within each cluster.

// Remark:  For vertex connectity each "edge" record just names a vertex as both head and tail.
    bvector<EdgeComponentData> edgeSortArray;
    bvector<int> clusterArray;
    bvector<int> &indices = visitor->ClientPointIndex ();
    bvector<bool> &visible = visitor->Visible ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        int clusterIndex = jmdlVArrayInt_newClusterIndex (&clusterArray);
        size_t numThisface = visitor->NumEdgesThisFace ();
        size_t readIndex = visitor->GetReadIndex ();
        for (size_t i = 0; i < numThisface; i++)
            {
            int headVertex = indices[i];
            int baseVertex = indices[i+1];  // The array has wraparound -- no need for modulo.
            if (connectivityType != 0)
                {
                if (baseVertex != headVertex)   // ignore zero lenght edges at poles
                    edgeSortArray.push_back (
                        EdgeComponentData (baseVertex, headVertex, readIndex, clusterIndex, visible[i]));
                }
            else // vertex connectivity ONLY -- each individual vertex gets an entry.
                edgeSortArray.push_back (
                    EdgeComponentData (headVertex, headVertex, readIndex, clusterIndex, visible[i]));
            }
        }

    size_t numEdge = edgeSortArray.size ();
    std::sort (edgeSortArray.begin (), edgeSortArray.end (), EdgeComponentData::cb_lt_lowVertexHighVertex);

    if (connectivityType != 2)
        {
        // Merge unconditionally within each block of edges with same vertex indices.
        for (size_t i = 1; i < numEdge; i++)
            {
            if (!EdgeComponentData::cb_lt_lowVertexHighVertex (edgeSortArray[i-1], edgeSortArray[i]))
                jmdlVArrayInt_mergeClusters (&clusterArray, edgeSortArray[i-1].m_id, edgeSortArray[i].m_id);
            }
        }
    else
        {
        // find blocks.
        // merge across edges when there are all visible.
        for (size_t i1, i0 = 0; i0 < numEdge; i0 = i1)
            {
            for (i1 = i0 + 1; i1 < numEdge && !EdgeComponentData::cb_lt_lowVertexHighVertex (edgeSortArray[i0], edgeSortArray[i1]);)
                {
                i1++;
                }
            size_t numVisible = 0;
            for (size_t i = i0; i < i1; i++)
                {
                if (edgeSortArray[i].m_visible)
                    numVisible++;
                }
            if (numVisible == 0)
                {
                for (size_t i = i0 + 1; i < i1; i++)
                    {
                    jmdlVArrayInt_mergeClusters (&clusterArray, edgeSortArray[i0].m_id, edgeSortArray[i].m_id);
                    }
                }
            }
        
        }


    for (size_t i = 0; i < numEdge; i++)
        {
        edgeSortArray[i].m_id = jmdlVArrayInt_getMergedClusterIndex (&clusterArray, edgeSortArray[i].m_id);
        }

    std::sort (edgeSortArray.begin (), edgeSortArray.end (), EdgeComponentData::cb_lt_id_readIndex);

    // fill component array, split when componentId changes
    int previousComponentId = -1;
    size_t previousReadIndex   = -1;
    for (size_t i = 0; i < numEdge; i++)
        {
        if (i > 0 && edgeSortArray[i].m_id != previousComponentId)
            blockedReadIndexArray.push_back (-1);

        if (edgeSortArray[i].m_readIndex != previousReadIndex)
            blockedReadIndexArray.push_back ((ptrdiff_t)edgeSortArray[i].m_readIndex);

        previousComponentId = edgeSortArray[i].m_id;
        previousReadIndex   = edgeSortArray[i].m_readIndex;
        }
    blockedReadIndexArray.push_back (-1);
    return true;
    }



END_BENTLEY_GEOMETRY_NAMESPACE
