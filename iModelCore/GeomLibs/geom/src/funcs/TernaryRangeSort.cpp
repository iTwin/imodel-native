/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/TernaryRangeSort.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <Geom/BinaryRangeHeap.h>
BEGIN_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE

ptrdiff_t ThreeWayMomentsFunction (size_t numLeft, size_t numIn, size_t numRight)
    {
    size_t p = 1;
    if (numLeft > 1)
        p *= numLeft;
    if (numIn > 1)
        p *= numIn;
    if (numRight > 1)
        p *= numRight;
    return p;
    }

ptrdiff_t TwoWayMomentsFunction (size_t numLeft, size_t numIn, size_t numRight)
    {
    size_t p = 1;
    if (numLeft > 1)
        p *= numLeft;
    if (numRight > 1)
        p *= numRight;
    return p;
    }

typedef ptrdiff_t (*SplitFunction) (size_t, size_t, size_t);


struct SplitCounters
{
size_t numLeft;
size_t numIn;
size_t numRight;
ptrdiff_t index;
ptrdiff_t weight;

SplitCounters (size_t myLeft, size_t myIn, size_t myRight, size_t myIndex = 0, ptrdiff_t myWeight = 0)
    {
    numLeft  = myLeft;
    numIn    = myIn;
    numRight = myRight;
    index    = myIndex;
    weight   = myWeight;
    }

SplitCounters ()
    {
    numLeft  = 0;
    numIn    = 0;
    numRight = 0;
    index    = 0;
    weight   = 0;
    }

void EnterRange ()
    {
    numIn++;
    numRight--;
    }

void LeaveRange ()
    {
    numIn--;
    numLeft++;
    }

void SetWeight (SplitFunction function)
    {
    weight = function (numLeft, numIn, numRight);
    }
};




struct RangeSortContext
{
// These arrays can be large -- so the RangeSortContext on works with references to the arrays
// owned by the caller . . .
bvector<DRange3dSizeSize> const &m_ranges;
bvector<size_t> &m_sortedIndices;

// internal work array . . .
bvector<DoubleSizeSize> m_sortArray;
bool m_activeAxis[3];
SplitFunction m_splitFunction;

struct IndexBlock
    {
    size_t m_i0;
    size_t m_i1;
    IndexBlock (size_t i0, size_t i1) : m_i0(i0), m_i1(i1) {}
    IndexBlock () :m_i0(0), m_i1(0) {}
    bool IsEmpty () const {return m_i1 <= m_i0;}
    size_t NumIndex () const { return m_i1 <= m_i0 ? 0 : m_i1 - m_i0;}
    size_t Index0 () const {return m_i0;}
    size_t ResolveIndex (size_t i) const {return m_i0 + i;}
    bool MoreIndicesThan (size_t n) { return m_i1 > m_i0 + n;}
    void BinarySplit (IndexBlock &left, IndexBlock &right) const
        {
        size_t middle = m_i0 + (m_i1 - m_i0) / 2;
        left = IndexBlock (m_i0, middle);
        right = IndexBlock (middle, m_i1);
        }
    };

RangeSortContext (bvector<DRange3dSizeSize> const &ranges, bvector<size_t> &sortedIndices)
    : m_ranges(ranges), m_sortedIndices (sortedIndices)
    {
    m_splitFunction = ThreeWayMomentsFunction;
    }

void SetSplitFunction (SplitFunction function) {m_splitFunction = function;}

IndexBlock InitializeFullArraySortIndices ()
    {
    size_t numRange = m_ranges.size ();
    m_sortedIndices.reserve (numRange);
    for (size_t i = 0; i < numRange; i++)
        m_sortedIndices.push_back (i);
    return IndexBlock (0, numRange);
    }




// build the sort array with <x, 0or1, k index in m_ranges> where k are selected from a contiguous block of m_sortedIndices.
bool BuildSortArray (IndexBlock const &indexBlock, int axisSelect)
    {
    m_sortArray.clear ();
    size_t numRange = indexBlock.NumIndex ();
    if (numRange == 0)
        return false;
    m_sortArray.reserve (numRange);
    if (axisSelect == 0)
        {
        for (size_t i = 0; i < numRange; i++)
            {
            size_t k = indexBlock.ResolveIndex(i);  // index to m_sortArray
            size_t q = m_sortedIndices[k];
            DRange3d const &range = m_ranges[q].GetCR ();
            m_sortArray.push_back (DoubleSizeSize (range.low.x, 0, q));
            m_sortArray.push_back (DoubleSizeSize (range.high.x, 1, q));
            }
        }
    else if (axisSelect == 1)
        {
        for (size_t i = 0; i < numRange; i++)
            {
            size_t k = indexBlock.ResolveIndex(i);  // index to m_sortArray
            size_t q = m_sortedIndices[k];
            DRange3d const &range = m_ranges[q].GetCR ();
            m_sortArray.push_back (DoubleSizeSize (range.low.y, 0, q));
            m_sortArray.push_back (DoubleSizeSize (range.high.y, 1, q));
            }
        }
    else
        {
        for (size_t i = 0; i < numRange; i++)
            {
            size_t k = indexBlock.ResolveIndex(i);  // index to m_sortArray
            size_t q = m_sortedIndices[k];
            DRange3d const &range = m_ranges[q].GetCR ();
            m_sortArray.push_back (DoubleSizeSize (range.low.z, 0, q));
            m_sortArray.push_back (DoubleSizeSize (range.high.z, 1, q));
            }
        }
    return true;
    }

// copy all sorce indices in m_sortArray back into m_sortedIndices starting at index0.
void CopySortedIndices (IndexBlock const &block)
    {
    //BeAssert (block.NumIndex (0 == m_sortArray.size ());
    size_t n = m_sortArray.size ();
    size_t indexWithinBlock = 0;
    for (size_t i = 0; i < n; i++)
        {
        if (m_sortArray[i].GetTagA () == 0)   // this is a low coordinate
            {
            m_sortedIndices[block.ResolveIndex (indexWithinBlock)] = m_sortArray[i].GetTagB ();
            indexWithinBlock++;
            }
        }
    BeAssert (indexWithinBlock == block.NumIndex ());
    }



bool ComputeSplitProperties
(
IndexBlock const &indexBlock,
int axisSelect,
SplitCounters &bestCounters
)
    {
    size_t numRange = indexBlock.NumIndex ();
    if (numRange == 0)
        return false;
    BuildSortArray (indexBlock, axisSelect);
    DoubleSizeSize::SortValueTagATagB (m_sortArray);

    SplitCounters counters (0,0, numRange);
    counters.SetWeight (m_splitFunction);
    bestCounters = counters;

    for (size_t n = m_sortArray.size (), i = 0; i < n; i++)
        {
        if (m_sortArray[i].GetTagA () == 0)
            {
            counters.EnterRange ();
            }
        else
            counters.LeaveRange ();

        counters.SetWeight (m_splitFunction);
        if (counters.weight > bestCounters.weight)
            {
            bestCounters = counters;
            bestCounters.index = i;
            }
        }
    return true;
    }

DRange3d RangeOf (IndexBlock const &indices)
    {
    DRange3d range;
    range.Init ();
    for (size_t i = 0; i < indices.NumIndex (); i++)
        {
        size_t k = indices.ResolveIndex (i);
        size_t q = m_sortedIndices[k];
        range.Extend (m_ranges[q].Get ());
        }
    return range;
    }

void CheckReduction (DRange3dCR range0, DRange3dCR rangeLeft, DRange3dCR rangeRight, int bestAxis,
      IndexBlock const &block0, IndexBlock const &blockLeft, IndexBlock const &blockRight
      )
    {
    }
bool ChooseAndApplySplit (bvector<IndexBlock> &stack)
    {
    if (stack.size () == 0)
        return false;
    IndexBlock indexBlock = stack.back ();
    stack.pop_back ();
    int bestAxis = -1;
    if (indexBlock.MoreIndicesThan (1))
        {
        SplitCounters currentCounters, bestCounters;
        int numApplied = 0;
        for (int axisSelect = 0; axisSelect < 3; axisSelect++)
            {
            if (!m_activeAxis[axisSelect])
                continue;
            ComputeSplitProperties (indexBlock, axisSelect, currentCounters);
            if (numApplied == 0|| currentCounters.weight > bestCounters.weight)
                {
                bestAxis = axisSelect;
                CopySortedIndices (indexBlock);
                bestCounters = currentCounters;
                numApplied++;
                }
            }

        if (numApplied > 0)
            {
            // simple left right split of the counters ...
            IndexBlock left, right;
            indexBlock.BinarySplit (left, right);
            DRange3d range0 = RangeOf (indexBlock);
            DRange3d rangeLeft = RangeOf (left);
            DRange3d rangeRight = RangeOf (right);
            CheckReduction (range0, rangeLeft, rangeRight, bestAxis,
                    indexBlock, left, right);
            if (left.MoreIndicesThan (1))
                stack.push_back (left);
            if (right.MoreIndicesThan (1))
                stack.push_back (right);
            }
        }
    return true;
    }


void SplitAll (bool sortX, bool sortY, bool sortZ)
    {
    bvector<IndexBlock> stack;
    InitializeFullArraySortIndices ();
    stack.push_back (IndexBlock (0, m_ranges.size ()));
    m_activeAxis[0] = sortX;
    m_activeAxis[1] = sortY;
    m_activeAxis[2] = sortZ;
    while (ChooseAndApplySplit (stack))
        {
        }
    }

};

void RangeSortAlgorithms::PseudoBinarySplits (bvector<DRange3dSizeSize> const &ranges, bvector<size_t> &indices,
      bool sortX, bool sortY, bool sortZ,
      RangeSortAlgorithms::SplitType splitType
      )
    {
    RangeSortContext sorter (ranges, indices);
    if (splitType == SplitType::ThreeWayMoments)
        sorter.SetSplitFunction (ThreeWayMomentsFunction);
    else
        sorter.SetSplitFunction (TwoWayMomentsFunction);
    sorter.SplitAll (sortX, sortY, sortZ);
    }

END_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE
