/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
#include <Geom/BinaryRangeHeap.h>
BEGIN_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE


IndexedRangeHeap::RangeEntry::RangeEntry ()
    {
    m_range = DRange3d::NullRange ();
    m_i0 = m_i1 = m_child = SIZE_MAX;
    }

bool IndexedRangeHeap::IsValidIndex (size_t index) const {return index < m_ranges.size ();}

bool IndexedRangeHeap::Get (size_t index, DRange3dR range) const
    {
    if (index < m_ranges.size ())
        {
        range = m_ranges[index].m_range;
        return true;
        }
    range.Init ();
    return false;    
    }

bool IndexedRangeHeap::Get (size_t index, DRange3dR range, size_t &i0, size_t &i1) const
    {
    if (index < m_ranges.size ())
        {
        range = m_ranges[index].m_range;
        i0 = m_ranges[index].m_i0;
        i1 = m_ranges[index].m_i1;
        return true;
        }
    i0 = i1 = SIZE_MAX;
    range.Init ();
    return false;    
    }

bool IndexedRangeHeap::IsLeafIndex (size_t index) const
    {
    return index < m_ranges.size ()
        && m_ranges[index].m_child == SIZE_MAX;
    }

bool IndexedRangeHeap::GetChildIndex (size_t index, size_t childOffset, size_t &childIndex) const
    {
    if (index < m_ranges.size () && childOffset < 2)
        {
        childIndex = m_ranges[index].m_child + childOffset;
        return IsValidIndex (childIndex);
        }
    childIndex = SIZE_MAX;
    return false;
    }

IndexedRangeHeap::RangeEntry::RangeEntry (DRange3dCR range, size_t i0, size_t i1, size_t child)
    {
    m_range = range;
    m_i0 = i0;
    m_i1 = i1;
    m_child = SIZE_MAX;
    }

void IndexedRangeHeap::SetNewChildIndices (size_t index, size_t &child0, size_t &child1)
    {
    child0 = 2 * index + 1;
    child1 = child0 + 1;
    InitializeThroughIndex (child1);
    m_ranges[index].m_child = child0;
    }

size_t IndexedRangeHeap::GetRootIndex () const
    {
    if (m_ranges.size () > 0)
        return 0;
    return SIZE_MAX;
    }

void IndexedRangeHeap::RangeEntry::Set (DRange3dCR range)
    {
    m_range = range;
    }

DRange3d IndexedRangeHeap::RangeEntry::Get () const
    {
    return m_range;
    }


void IndexedRangeHeap::InitializeThroughIndex (size_t index)
    {
    if (index >= m_ranges.size ())
        m_ranges.resize (index + 1);
    }

void IndexedRangeHeap::SetUnevaluatedRange (size_t index, size_t i0, size_t i1)
    {
    InitializeThroughIndex (index);
    m_ranges[index] = RangeEntry (DRange3d::NullRange (), i0, i1, SIZE_MAX);
    }

void IndexedRangeHeap::CorrectRangeFromChildren (size_t parent)
    {
    size_t leftChild = m_ranges[parent].m_child;
    size_t rightChild = leftChild + 1;
    DRange3d range;
    range.UnionOf (m_ranges[leftChild].Get (), m_ranges[rightChild].Get ());
    m_ranges[parent].Set (range);
    }

void IndexedRangeHeap::SetEvaluatedRange (size_t index, size_t i0, size_t i1)
    {
    InitializeThroughIndex (index);
    DRange3d range;
    m_source->GetRange (i0, i1, range);
    m_ranges[index] = RangeEntry (range, i0, i1, SIZE_MAX);
    }

void IndexedRangeHeap::SetRecursiveRange (size_t index, size_t i0, size_t i1)
    {
    if (i1 < i0 + m_numPerEntry)
        {
        SetEvaluatedRange (index, i0, i1);
        }
    else
        {
        size_t iMid = (i0 + i1) / 2;
        SetUnevaluatedRange (index, i0, i1);
        size_t child0, child1;
        SetNewChildIndices (index, child0, child1);
        SetRecursiveRange (child0, i0, iMid);
        SetRecursiveRange (child1, iMid + 1, i1);
        CorrectRangeFromChildren (index);
        }
    }

// Build index for source ranges i0 through i1 inclusive.
// Each entry in the returned array has a (composite) range for source entry range (inclusively) TagA through TagB.
// 
void IndexedRangeHeap::Build
(
size_t numPerEntry,
IndexedRangeSource const *source,
size_t i0,
size_t i1
)
    {
    if (numPerEntry < 1)
        numPerEntry = 1;
    m_ranges.clear ();
    m_numPerEntry = 1;
    m_source = source;

    SetRecursiveRange (0, i0, i1);
    }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//                             TWO HEAP SEARCH
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------



struct IndexedRangeHeapPairSearcher
{
IndexedRangeHeapCR m_heapA;
IndexedRangeHeapCR m_heapB;
IndexedRangeHeap::PairProcessor &m_processor;
int m_sortMethod;
public:

BoolCounter m_numII;
BoolCounter m_numIL;
BoolCounter m_numLL;

IndexedRangeHeapPairSearcher
(
IndexedRangeHeapCR heapA,
IndexedRangeHeapCR heapB,
IndexedRangeHeap::PairProcessor &processor
) : m_heapA (heapA), m_heapB (heapB), m_processor (processor), m_sortMethod (0)
    {
    }

void SetSortMethod (int value){m_sortMethod = value;}

bool Test (size_t indexA, size_t indexB)
    {
    DRange3d rangeA, rangeB;
    size_t iA0, iA1, iB0, iB1;
    return m_heapA.Get (indexA, rangeA, iA0, iA1)
        && m_heapB.Get (indexB, rangeB, iB0, iB1)
        && m_numII.Count (m_processor.NeedProcessing (rangeA, iA0, iA1, rangeB, iB0, iB1));
    }

void ProcessLeafLeaf (size_t indexA, size_t indexB)
    {
    DRange3d rangeA, rangeB;
    size_t iA0, iA1, iB0, iB1;
    m_heapA.Get (indexA, rangeA, iA0, iA1);
    m_heapB.Get (indexB, rangeB, iB0, iB1);
    for (size_t iA = iA0; iA <= iA1; iA++)
        {
        for (size_t iB = iB0; iB <= iB1 && m_processor.IsLive (); iB++)
            {
            m_numLL.Count (true);
            m_processor.Process (iA, iB);
            }
        }
    }

void TestAndRecurse (size_t indexA, size_t indexB);

void ProcessLeafInterior (size_t indexA, size_t indexB)
    {
    size_t childB;
    for (size_t i = 0; m_heapB.GetChildIndex (indexB, i, childB); i++)
        TestAndRecurse (indexA, childB);
    }

void ProcessInteriorLeaf (size_t indexA, size_t indexB)
    {
    size_t childA;
    for (size_t i = 0; m_heapA.GetChildIndex (indexA, i, childA); i++)
        TestAndRecurse (childA, indexB);
    }

struct SizeSizeDouble
    {
    size_t m_indexA;
    size_t m_indexB;
    double m_a;
    void Init (size_t indexA, size_t indexB, double a)
        {
        m_indexA = indexA;
        m_indexB = indexB;
        m_a = a;
        }
    bool operator < (SizeSizeDouble const &other) const
        {
        return m_a < other.m_a;
        }
    };

void ProcessInteriorInterior (size_t indexA, size_t indexB)
    {
    if (m_sortMethod == 1)
        {
      // ASSUME only 2 children per interior.
      // Dig up the ranges of each pair.
      // predict that the smallest composite range is most interesting to the caller.
      SizeSizeDouble sortSelect[4];
      size_t numChildren = 0;
      size_t childA, childB;
      DRange3d rangeA, rangeB, rangeAB;
      size_t iA0, iA1, iB0, iB1;
      for (size_t i = 0; i < 2; i++)
          {
          if (m_heapA.GetChildIndex (indexA, i, childA))
              {
              m_heapA.Get (childA, rangeA, iA0, iA1);
              for (size_t j = 0;  j < 2; j++)
                  {
                  if (m_heapB.GetChildIndex (indexB, j, childB))
                      {
                      m_heapB.Get (childB, rangeB, iB0, iB1);
                      rangeAB.UnionOf (rangeA, rangeB);
                      double a = rangeAB.low .Distance (rangeAB.high);
                      sortSelect[numChildren++].Init (childA, childB, a);
                      }
                  }
              }
          }

      for (size_t i = 0; i + 1 < numChildren; i++)
          {
          for (size_t j = i + 1; j < numChildren; j++)
              {
              if (sortSelect[j] < sortSelect[i])
                  std::swap (sortSelect[i], sortSelect[j]);
              }
          }

      for (size_t i = 0; i < numChildren; i++)
          TestAndRecurse (sortSelect[i].m_indexA, sortSelect[i].m_indexB);
        }
    else
        {
        size_t childA, childB;
        for (size_t i = 0; m_heapA.GetChildIndex (indexA, i, childA); i++)
            {
            for (size_t j = 0; m_heapB.GetChildIndex (indexB, j, childB); j++)
                TestAndRecurse (childA, childB);
            }
        }
    }

};

void IndexedRangeHeapPairSearcher::TestAndRecurse (size_t indexA, size_t indexB)
    {
    if (m_processor.IsLive () && Test (indexA, indexB))
        {
        if (m_heapA.IsLeafIndex (indexA))
            {
            if (m_heapB.IsLeafIndex (indexB))
                ProcessLeafLeaf (indexA, indexB);
            else
                ProcessLeafInterior (indexA, indexB);
            }
        else if (m_heapB.IsLeafIndex (indexB))
            {
            ProcessInteriorLeaf (indexA, indexB);
            }
        else
            {
            ProcessInteriorInterior (indexA, indexB);
            }
        }
    }


void IndexedRangeHeap::Search (IndexedRangeHeapCR heapA, IndexedRangeHeapCR heapB,
    IndexedRangeHeap::PairProcessor &processor, int sortMethod)
    {
    IndexedRangeHeapPairSearcher searcher (heapA, heapB, processor);
    searcher.SetSortMethod (sortMethod);
    searcher.TestAndRecurse (heapA.GetRootIndex (), heapB.GetRootIndex ());
    }

void IndexedRangeHeap::Search (IndexedRangeHeapCR heapA, IndexedRangeHeapCR heapB,
    IndexedRangeHeap::PairProcessor &processor)
    {
    IndexedRangeHeapPairSearcher searcher (heapA, heapB, processor);
    searcher.TestAndRecurse (heapA.GetRootIndex (), heapB.GetRootIndex ());
    }

void IndexedRangeHeap::CollectInRange (DRange3dCR range, int numDimensions, bvector<size_t> &indices)
    {
    // STACK_ALLOCATION of 32 would be 4 Billion leaf nodes.
#define STACK_ALLOCATION 48
    // This stack contains indices of tree nodes that are yet to be explored.
    size_t stack[STACK_ALLOCATION];
    for (auto& index : stack) index = SIZE_MAX; // init with invalid indices
    indices.clear ();
    uint32_t stackDepth = 1;
    stack[stackDepth++] = GetRootIndex ();
    while (stackDepth > 0)
        {
        size_t k = stack[--stackDepth];
        if (IsValidIndex(k))
            {
            DRange3d nodeRange = m_ranges[k].m_range;
            if (range.IntersectsWith(nodeRange, numDimensions))
                {
                if (IsLeafIndex(k))
                    {
                    for (size_t i = m_ranges[k].m_i0;
                        i <= m_ranges[k].m_i1;
                        i++)
                        indices.push_back (i);
                    }
                else
                    {
                    if (stackDepth + 2 < STACK_ALLOCATION)
                        {
                        stack[stackDepth++] = m_ranges[k].m_child;
                        stack[stackDepth++] = m_ranges[k].m_child+1;
                        }
                    }
                }
            }
        }
    }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//                             SINGLE HEAP SEARCH
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

struct IndexedRangeHeapSingleSearcher
{
IndexedRangeHeapCR m_heap;
IndexedRangeHeap::SingleProcessor &m_processor;
public:

IndexedRangeHeapSingleSearcher
(
IndexedRangeHeapCR heap,
IndexedRangeHeap::SingleProcessor &processor
)
    : m_heap (heap), m_processor (processor)
    {}

void TestAndRecurse (size_t index);

bool Test (size_t index)
    {
    DRange3d range;
    size_t i0, i1;
    return m_heap.Get (index, range, i0, i1)
        && m_processor.NeedProcessing (range, i0, i1);
    }
void ProcessInterior (size_t index)
    {
    size_t childA;
    for (size_t i = 0; m_heap.GetChildIndex (index, i, childA); i++)
        {
        TestAndRecurse (childA);
        }
    }

void ProcessLeaf (size_t index)
    {
    DRange3d range;
    size_t i0, i1;
    if (m_heap.Get (index, range, i0, i1))
        for (size_t i = i0; i <= i1 && m_processor.IsLive (); i++)
            m_processor.Process (i);
    }

};
void IndexedRangeHeapSingleSearcher::TestAndRecurse (size_t index)
    {
    if (m_processor.IsLive () && Test (index))
        {
        if (m_heap.IsLeafIndex (index))
            ProcessLeaf (index);
        else ProcessInterior (index);
        }
    }

void IndexedRangeHeap::Search
(
IndexedRangeHeapCR heap,
IndexedRangeHeap::SingleProcessor &processor
)
    {
    IndexedRangeHeapSingleSearcher searcher (heap, processor);
    searcher.TestAndRecurse (heap.GetRootIndex ());
    }


END_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE