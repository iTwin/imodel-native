/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Bentley/BeTimeUtilities.h>

#include <Geom/BinaryRangeHeap.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double square (double value) {return value * value;}

#ifndef NDEBUG
static bool IsPermutation (bvector<size_t> &data)
    {
    size_t n = data.size ();
    bvector<int> count;
    for (size_t i = 0; i < n; i++)
        count.push_back (0);
    for (size_t i = 0; i < n; i++)
        {
        size_t k = data[i];
        if (k >= n)
            return false;
        if (count[k] != 0)
            return false;
        count[k]++;
        }
    return true;
    }
#endif

static double GetDiagonal (DRange3dCR range, int numDimensions)
    {
    double sumSquares = 0.0;
    if (numDimensions > 0.0)
        {
        sumSquares += square (range.high.x - range.low.x);
        if (numDimensions > 1)
            {
            sumSquares += square (range.high.y - range.low.y);
            if (numDimensions > 2)
                sumSquares += square (range.high.z - range.low.z);
            }
        }
    return sqrt (sumSquares);
    }

/*__PUBLISH_SECTION_END__*/
struct PolyfaceIndexedHeapRangeTree;
typedef RefCountedPtr<PolyfaceIndexedHeapRangeTree>  PolyfaceIndexedHeapRangeTreePtr;

// Carry an array with <facetRange, readIndex, originalFacetIndex>
struct RangeIndexingContext : IndexedRangeHeap::IndexedRangeSource
{
bvector<DRange3dSizeSize> m_ranges;
RangeIndexingContext (){}
void AddRange (DRange3dCR range, size_t indexA, size_t indexB)
    {
    m_ranges.push_back (DRange3dSizeSize (range, indexA, indexB));
    }

bool GetRange (size_t i0, size_t i1, DRange3d &range) const override
    {
    range.Init ();
    int n = 0;
    size_t n1 = m_ranges.size ();
    for (size_t i = i0; i <= i1 && i < n1; i++)
        {
        range.Extend (m_ranges.at(i).Get ());
        n++;
        }
     return n > 0;
    }

struct InclusiveIndexBlock
    {
    size_t m_i0;
    size_t m_i1;
    size_t m_depth;
    InclusiveIndexBlock (size_t i0, size_t i1, size_t depth)
      : m_i0(i0), m_i1(i1), m_depth (depth)
      {
      }

    void PushChildrenIfSplittable (bvector<InclusiveIndexBlock> &stack)
        {
        if (m_i1 > m_i0 + 1)
            {
            size_t i2 = (m_i0 + m_i1) / 2;
            stack.push_back (InclusiveIndexBlock (m_i0, i2, m_depth+1));
            stack.push_back (InclusiveIndexBlock (i2 + 1, m_i1, m_depth+1));
            }
        }
    };

// Compute the diagonals of the range boxes at various levels.
// data[i].x = sum of diagonals at depth i.
// data[i].y = max diagonal at depth i.
// data[i].z = number of ranges represented.
void ComputeDiagonalProperties (bvector<DPoint3d> &data, int numDimensions)
    {
    data.clear ();
    if (m_ranges.size () == 0)
        return;
    bvector<InclusiveIndexBlock> stack;
    stack.push_back (InclusiveIndexBlock (0, m_ranges.size () - 1, 0));
    while (stack.size () > 0)
        {
        InclusiveIndexBlock window = stack.back ();
        stack.pop_back ();
        DRange3d range;
        if (GetRange (window.m_i0, window.m_i1, range))
            {
            while (data.size () <= window.m_depth)
                data.push_back (DPoint3d::From (0, 0, 0));
            DPoint3d &top = data[window.m_depth];
            double diagonal = GetDiagonal (range, numDimensions);
            top.x += diagonal;
            top.y = DoubleOps::Max (diagonal, top.y);
            top.z++;
            }
        window.PushChildrenIfSplittable (stack);
        }
    }
};
// Range-based search with indexed range heap.
// Implementation issue:  IndexedRangeHeap assumes sequential size_t.  Polyface readIndex has gaps.
// So this class has to have a mapping from heap position to readIndex.

//BENTLEY_GEOMETRY_INTERNAL_NAMESPACE_NAME::IndexedRangeHeap m_heap;
//bvector<size_t>  m_heapIndexToReadIndex;

PolyfaceIndexedHeapRangeTree::PolyfaceIndexedHeapRangeTree (){}
PolyfaceIndexedHeapRangeTree::~PolyfaceIndexedHeapRangeTree (){}

template<typename T0, typename T1>
bool ApplySortOrder (bvector<T0> &data0, bvector<T1> &data1, bvector<size_t> &index)
    {
    size_t dataCount = data0.size ();
    if (dataCount != index.size () || data1.size () != dataCount)
        return false;
    for (size_t seedPosition = 0; seedPosition < dataCount; seedPosition++)
        {
        if (index[seedPosition] >= dataCount)
            continue;
        T0 seedData0 = data0[seedPosition];
        T1 seedData1 = data1[seedPosition];
        size_t k = seedPosition;
        while (index[k] < dataCount && index[k] != seedPosition)
            {
            data0[k] = data0[index[k]];
            data1[k] = data1[index[k]];
            size_t k1 = k;
            k = index[k];
            index[k1] = SIZE_MAX;
            }
        if (index[k] == seedPosition)
            {
            data0[k] = seedData0;
            data1[k] = seedData1;
            index[k] = SIZE_MAX;
            }
        }
    return true;
    }

#ifndef NDEBUG
// At each level of the heap, compute (saved as a point)
//   x = sum of all diagonals.
//   y = max diagonal;
//    z = number of ranges.
static void ComputeHeapProperties (IndexedRangeHeap const &heap, bvector<DPoint3d> &sums, size_t index0, size_t depth)
    {
    if (heap.IsValidIndex (index0))
        {
        while (sums.size () <= depth)
            sums.push_back (DPoint3d::From (0, 0, 0));
        DRange3d range;
        heap.Get (index0, range);
        double diagonal = GetDiagonal (range, 2);
        DPoint3d &data = sums[depth];
        data.x += diagonal;
        data.y = DoubleOps::Max (diagonal, data.y);
        data.z++;
        for (size_t i = 0, childIndex;
              heap.GetChildIndex (index0, i, childIndex); i++)
            {
            ComputeHeapProperties (heap, sums, childIndex, depth+1);
            }
        }
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PolyfaceIndexedHeapRangeTree::LoadPolyface (PolyfaceQueryCR polyface, bool sortX, bool sortY, bool sortZ)
    {
    RangeIndexingContext rangeBuilderMap;

    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (polyface);
    size_t facetIndex = 0;
    bvector<DPoint3d> &visitorPoints = visitor->Point ();
    DRange3d facetRange;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        facetRange = DRange3d::From (visitorPoints);
        rangeBuilderMap.AddRange (facetRange, visitor->GetReadIndex (), facetIndex++);
        }



    size_t n = rangeBuilderMap.m_ranges.size ();
    m_heapIndexToReadIndex.clear ();
    for (size_t i = 0; i < n; i++)
        m_heapIndexToReadIndex.push_back (rangeBuilderMap.m_ranges[i].GetTagA ());

    bvector<size_t> sortOrder;
    if (sortX || sortY || sortZ)
        {
        RangeSortAlgorithms::PseudoBinarySplits (rangeBuilderMap.m_ranges, sortOrder, sortX, sortY, sortZ,
                  RangeSortAlgorithms::SplitType::TwoWayMoments);
#ifndef NDEBUG
        BeAssert (IsPermutation (sortOrder));
        bvector <DPoint3d> sum0, sum1;
        rangeBuilderMap.ComputeDiagonalProperties (sum0, 2);
#endif
        ApplySortOrder (rangeBuilderMap.m_ranges, m_heapIndexToReadIndex, sortOrder);
#ifndef NDEBUG
        rangeBuilderMap.ComputeDiagonalProperties (sum1, 2);
#endif
        }
    if (n > 0)
        m_heap.Build (1, &rangeBuilderMap, 0, n - 1);
#ifndef NDEBUG
    bvector<DPoint3d> sums;
    ComputeHeapProperties (m_heap, sums, 0, 0);
#endif
    return n;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PolyfaceIndexedHeapRangeTree::LoadPolygons (TaggedPolygonVectorCR polygons, bool sortX, bool sortY, bool sortZ)
    {
    // hmph .. The heapIndexToReadIndex is identity here.  The service provided is "just" the sorting search logic.
    RangeIndexingContext rangeBuilderMap;
    m_heapIndexToReadIndex.clear ();
    for (size_t i = 0; i < polygons.size (); i++)
        {
        rangeBuilderMap.AddRange (polygons[i].GetRange (), i, i);
        m_heapIndexToReadIndex.push_back (i);
        }

    bvector<size_t> sortOrder;
    if (sortX || sortY || sortZ)
        {
        RangeSortAlgorithms::PseudoBinarySplits (rangeBuilderMap.m_ranges, sortOrder, sortX, sortY, sortZ,
                  RangeSortAlgorithms::SplitType::TwoWayMoments);
#ifndef NDEBUG
        BeAssert (IsPermutation (sortOrder));
        bvector <DPoint3d> sum0, sum1;
        rangeBuilderMap.ComputeDiagonalProperties (sum0, 2);
#endif
        ApplySortOrder (rangeBuilderMap.m_ranges, m_heapIndexToReadIndex, sortOrder);
#ifndef NDEBUG
        rangeBuilderMap.ComputeDiagonalProperties (sum1, 2);
#endif
        }
    if (polygons.size () > 0)
        m_heap.Build (1, &rangeBuilderMap, 0, polygons.size () - 1);
#ifndef NDEBUG
    bvector<DPoint3d> sums;
    ComputeHeapProperties (m_heap, sums, 0, 0);
#endif
    return polygons.size ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceIndexedHeapRangeTreePtr PolyfaceIndexedHeapRangeTree::CreateForPolyface (PolyfaceQueryCR source)
    {
    PolyfaceIndexedHeapRangeTreePtr tree = new PolyfaceIndexedHeapRangeTree ();
    tree->LoadPolyface (source, false, false, false);
    return tree;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceIndexedHeapRangeTreePtr PolyfaceIndexedHeapRangeTree::CreateForPolyface (PolyfaceQueryCR source, bool sortX, bool sortY, bool sortZ)
    {
    PolyfaceIndexedHeapRangeTreePtr tree = new PolyfaceIndexedHeapRangeTree ();
    tree->LoadPolyface (source, sortX, sortY, sortZ);
    return tree;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceIndexedHeapRangeTreePtr PolyfaceIndexedHeapRangeTree::CreateForPolygons (TaggedPolygonVectorCR source, bool sortX, bool sortY, bool sortZ)
    {
    PolyfaceIndexedHeapRangeTreePtr tree = new PolyfaceIndexedHeapRangeTree ();
    tree->LoadPolygons (source, sortX, sortY, sortZ);
    return tree;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceIndexedHeapRangeTreePtr PolyfaceIndexedHeapRangeTree::CreateForPolyfaceXYSort (PolyfaceQueryCR source)
    {
    PolyfaceIndexedHeapRangeTreePtr tree = new PolyfaceIndexedHeapRangeTree ();
    tree->LoadPolyface (source, true, true, false);
    return tree;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool PolyfaceIndexedHeapRangeTree::TryGetReadIndex (size_t facetIndex, size_t &readIndex) const
    {
    if (facetIndex < m_heapIndexToReadIndex.size ())
        {
        readIndex = m_heapIndexToReadIndex[facetIndex];
        return true;
        }
    readIndex = 0;
    return false;
    }

struct RecursiveCollectReadIndicesByTreeDepth
{
BENTLEY_GEOMETRY_INTERNAL_NAMESPACE_NAME::IndexedRangeHeap const &m_heap;
bvector<bvector<size_t>> &m_readIndex;
int m_targetDepth;
public:
RecursiveCollectReadIndicesByTreeDepth
(
BENTLEY_GEOMETRY_INTERNAL_NAMESPACE_NAME::IndexedRangeHeap const &heap,
bvector<bvector<size_t>> &readIndex,
int targetDepth
)
  : m_heap(heap),
    m_readIndex(readIndex),
    m_targetDepth (targetDepth)
    {
    }

public: void Recurse ()
    {
    Recurse (0,0);
    }
private: void Recurse (size_t heapIndex, int treeDepth)
    {
    if (treeDepth >= m_targetDepth)
        {
        size_t i0, i1;
        DRange3d range;
        if (m_heap.Get(heapIndex, range, i0, i1))
            {
            m_readIndex.push_back (bvector<size_t> ());
            bvector<size_t> &back = m_readIndex.back ();
            for (size_t i = i0; i <= i1; i++)
                {
                back.push_back (i);
                }
            }
        }
    else
        {
        int newDepth = treeDepth+1;
        for (size_t childOffset = 0, childIndex;
            m_heap.GetChildIndex (heapIndex, childOffset, childIndex);
            childOffset++
            )
            {
            Recurse (childIndex, newDepth);
            }
        }
    }


};


size_t PolyfaceIndexedHeapRangeTree::GetNumRanges () const { return m_heapIndexToReadIndex.size ();}

bool PolyfaceIndexedHeapRangeTree::TryGetRange (size_t index, DRange3dR range) const
    {
    return m_heap.Get (index, range);
    }

// Collect read indices of each node at specified depth.
bool PolyfaceIndexedHeapRangeTree::CollectReadIndicesByTreeDepth
(
bvector<bvector<size_t>> &raggedReadIndices,
int depth
) const
    {
    raggedReadIndices.clear ();
    RecursiveCollectReadIndicesByTreeDepth searcher (m_heap, raggedReadIndices, depth);
    searcher.Recurse ();
    // convert from range heap index to facetReadIndex ...
    for (bvector<size_t> & block : raggedReadIndices)
        {
        size_t numAccept = 0;
        for (size_t i = 0; i < block.size (); i++)
            {
            size_t k = block[i];
            if (k < m_heapIndexToReadIndex.size ())
                block[numAccept++] = m_heapIndexToReadIndex[k];
            }
        block.resize (numAccept);
        }
    return raggedReadIndices.size () > 0;
    }


/*---------------------------------------------------------------------------------**//**
Callback context for a range search.
+---------------+---------------+---------------+---------------+---------------+------*/
struct PolyfaceIndexedHeapRangeTree_RangeSearcher : IndexedRangeHeap::SingleProcessor
{
private:
PolyfaceIndexedHeapRangeTree *m_tree;
DRange3d m_baseRange;

DRange3d m_currentRange;
bvector<size_t> &m_hits;
public:

// ctor .. Note that a REFERENCE TO the hits is captured.
PolyfaceIndexedHeapRangeTree_RangeSearcher (PolyfaceIndexedHeapRangeTree *tree, bvector<size_t> &hits)
    :
    m_tree(tree),
    m_hits(hits)
    {
    m_hits.clear ();
    }

~PolyfaceIndexedHeapRangeTree_RangeSearcher ()
    {
    }


public:
    size_t m_leafHit;
    size_t m_leafSkip;
    size_t m_subtreeHit;
    size_t m_subtreeSkip;

void ClearCounts ()
    {
    m_leafHit       = 0;
    m_leafSkip      = 0;
    m_subtreeHit    = 0;
    m_subtreeSkip   = 0;
    }


void SetBaseRange (DRange3dCR range)
    {
    m_baseRange = m_currentRange = range;
    }
void SetCurrentRangeFromExpandedBaseRange (double expansion)
    {    
    m_currentRange = m_baseRange;
    m_currentRange.low.x -= expansion;         m_currentRange.high.x += expansion;
    m_currentRange.low.y -= expansion;         m_currentRange.high.y += expansion;
    m_currentRange.low.z -= expansion;         m_currentRange.high.z += expansion;
    }

void ClearHits (){m_hits.clear ();}
void AddHit (size_t hit){m_hits.push_back (hit);}

bool NeedProcessing (
            DRange3dCR range, size_t iA0, size_t iA1
            ) override 
    {
    if (m_currentRange.IntersectsWith (range))
        {
        m_subtreeHit++;
        return true;
        }
    else
        {
        m_subtreeSkip++;
        return false;
        }
    }

bool IsLive () const override {return true;}

void Process (size_t facetIndex) override
    {
    size_t readIndex;
    if (m_tree->TryGetReadIndex (facetIndex, readIndex))
        {
        AddHit (readIndex); 
        m_leafHit++;
        }
    else
        {
        m_leafSkip++;
        }
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
void PolyfaceIndexedHeapRangeTree::CollectInRange (bvector<size_t> &hits, DRange3dCR range, double expansion)
    {
    PolyfaceIndexedHeapRangeTree_RangeSearcher searcher (this, hits);
    searcher.SetBaseRange (range);
    searcher.SetCurrentRangeFromExpandedBaseRange (expansion);
    IndexedRangeHeap::Search (m_heap, searcher);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
BENTLEY_GEOMETRY_INTERNAL_NAMESPACE_NAME::IndexedRangeHeap & PolyfaceIndexedHeapRangeTree::GetHeapR(){return m_heap;}




END_BENTLEY_GEOMETRY_NAMESPACE