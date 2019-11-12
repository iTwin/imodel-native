/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <Geom/BinaryRangeHeap.h>
BEGIN_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE


void BuildSortArray
(
bvector<DRange3d> const &sourceRange,
int axisSelect,
bvector<DoubleSizeSize> &sortArray
)
    {
    sortArray.clear ();
    size_t numRange = sourceRange.size ();
    if (axisSelect == 0)
        {
        for (size_t i = 0; i < numRange; i++)
            {
            sortArray.push_back (DoubleSizeSize (sourceRange[i].low.x, 0, i));
            sortArray.push_back (DoubleSizeSize (sourceRange[i].high.x, 1, i));
            }
        }
    else if (axisSelect == 1)
        {
        for (size_t i = 0; i < numRange; i++)
            {
            sortArray.push_back (DoubleSizeSize (sourceRange[i].low.y, 0, i));
            sortArray.push_back (DoubleSizeSize (sourceRange[i].high.y, 1, i));
            }
        }
    else
        {
        for (size_t i = 0; i < numRange; i++)
            {
            sortArray.push_back (DoubleSizeSize (sourceRange[i].low.z, 0, i));
            sortArray.push_back (DoubleSizeSize (sourceRange[i].high.z, 1, i));
            }
        }
    }


ptrdiff_t VolumeMoment (size_t numLeft, size_t numIn, size_t numRight)
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

typedef ptrdiff_t (*WeightFunction) (size_t, size_t, size_t);
struct SplitDetail
{
size_t numLeft;
size_t numIn;
size_t numRight;
ptrdiff_t index;
ptrdiff_t weight;

SplitDetail (size_t myLeft, size_t myIn, size_t myRight, size_t myIndex = 0, ptrdiff_t myWeight = 0)
    {
    numLeft  = myLeft;
    numIn    = myIn;
    numRight = myRight;
    index    = myIndex;
    weight   = myWeight;
    }

SplitDetail ()
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

void SetWeight (WeightFunction weightFunction)
    {
    weight = weightFunction (numLeft, numIn, numRight);
    }
};




void RangeSplitter::GetSelectedRangeStatistics
(
bvector <DRange3d> const &sourceRange,
bvector<int> const &sourceSelector,
int selector,
size_t &numSelected,
DRange3d &selectedUnion,
DRange3d &selectedIntersection
)
    {
    selectedUnion.Init ();
    selectedIntersection.Init ();
    numSelected = 0;
    size_t numRange = sourceRange.size ();
    for (size_t i = 0; i < numRange; i++)
        {
        if (sourceSelector[i] == selector)
            {
            numSelected++;
            if (numSelected == 1)
                selectedIntersection = sourceRange[i];
            else
                selectedIntersection.IntersectIndependentComponentsOf (selectedIntersection, sourceRange[i]);
            selectedUnion.Extend (sourceRange[i]);
            }
        }
    }

bool RangeSplitter::SplitRanges
(
bvector<DRange3d> const &sourceRange,
int axisSelect,
bvector<int> &leftCenterRight,
size_t &numLeft,
size_t &numCenter,
size_t &numRight
)
    {
    size_t numRange = sourceRange.size ();
    numLeft = 0;
    numRight = 0;
    numCenter = 0;
    if (numRange == 0)
        return false;
    BuildSortArray (sourceRange, axisSelect, sortArray);
    //size_t numSort = sortArray.size ();
    DoubleSizeSize::SortValueTagATagB (sortArray);

    SplitDetail counters (0,0, numRange);
    counters.SetWeight (VolumeMoment);
    SplitDetail maxWeight = counters;

    maxWeight.SetWeight (VolumeMoment);
    for (size_t n = sortArray.size (), i = 0; i < n; i++)
        {
        if (sortArray[i].GetTagA () == 0)
            {
            counters.EnterRange ();
            }
        else
            counters.LeaveRange ();

        counters.SetWeight (VolumeMoment);
        if (counters.weight > maxWeight.weight)
            {
            maxWeight = counters;
            maxWeight.index = i;
            }
        }

    //size_t j = 0;
    leftCenterRight.reserve (numRange);
    leftCenterRight.clear ();
    for (size_t i = 0; i < numRange; i++)
        leftCenterRight.push_back (1);
    size_t numUntouched = numRange;
    size_t numPassed  = 0;
    numLeft = maxWeight.numLeft;
    numRight = maxWeight.numRight;
    numCenter = maxWeight.numIn;
    for (size_t i = 0, n = sortArray.size (); i < n; i++)
        {
        if (sortArray[i].GetTagA () == 0)
            {
            numUntouched--;
            if (numUntouched < numRight)
                leftCenterRight[sortArray[i].GetTagB ()] = 2;
            }
        else
            {
            numPassed++;
            if (numPassed <= numLeft)
                leftCenterRight[sortArray[i].GetTagB ()] = 0;
            }
        }
    return true;
    }
#ifdef CompileAll
// transitionArray[i].Get () == coordinate of transition
// transitionArray[i].GetTagA () == 0 at left, 1 at right.
// transitionArray[i].GetTagB () == caller's id of range.
static SplitDetail AssignSplitsFromTransitions
(
bvector<DoubleSizeSize> &transitionArray
)
    {
    size_t numSort = transitionArray.size ();
    DoubleSizeSize::SortValueTagATagB (transitionArray);

    SplitDetail counters (0,0, numSort / 2);
    counters.SetWeight (VolumeMoment);
    SplitDetail maxWeight = counters;

    maxWeight.SetWeight (VolumeMoment);
    for (size_t n = transitionArray.size (), i = 0; i < n; i++)
        {
        if (transitionArray[i].GetTagA () == 0)
            {
            counters.EnterRange ();
            }
        else
            counters.LeaveRange ();

        counters.SetWeight (VolumeMoment);
        if (counters.weight > maxWeight.weight)
            {
            maxWeight = counters;
            maxWeight.index = i;
            }
        }

    return maxWeight;
    }
#endif



typedef ValueSizeSize <DRange3d> DRange3dSizeSize;
static const size_t s_terminatorIndex = SIZE_MAX;

// bvector<DRange3dSizeSize> using the TagB members as singly linked list chains.
struct VectorOfRangeLists : bvector <DRange3dSizeSize>
{

size_t GetSuccessor (size_t index) const
    {
    if (index < size ())
        return at(index).GetTagB ();
    return s_terminatorIndex;
    }

DRange3d GetDRange3dAt (size_t index) const
    {
    if (index < size ())
        return at(index).Get ();
    return DRange3d::NullRange ();
    }


bool IsValidIndex (size_t index)
    {
    return index < size ();
    }

ptrdiff_t WalkToTail (size_t head)
    {
    size_t curr = head, next;
    for (;IsValidIndex (next = GetSuccessor (curr));
        curr = next)
        {
        }
    return curr;
    }

size_t ListLengthAt (size_t head)
    {
    size_t n = 0;
    size_t vectorSize = size ();
    for (size_t curr = head; curr < vectorSize; curr = at(curr).GetTagB ())
        {
        n++;
        }
    return n;
    }

size_t WalkToTail (size_t head, DRange3dR listUnion, DRange3dR listIntersection)
    {
    if (!IsValidIndex (head))
        {
        listUnion.Init ();
        listIntersection.Init ();
        return s_terminatorIndex;
        }

    listUnion = listIntersection = at(head).Get ();
    size_t curr = head, next;
    for (;IsValidIndex (next = GetSuccessor (curr));
        curr = next)
        {
        DRange3d currRange = at(curr).Get ();
        listIntersection.IntersectIndependentComponentsOf (listIntersection, currRange);
        listUnion.Extend (currRange);
        }
    return curr;
    }
};

static const double s_universeSize = 1.0e40;

struct TernaryRangeTreeNode
{
size_t m_primaryListHead;     // Index of head of linked list of ranges in range array.

size_t m_leftChild;  // Index of child with left children.
size_t m_midChild;   // Index of child with mid children.
size_t m_rightChild; // Index of child with right children.

unsigned int m_mySplitSelect;      // describes how this split was done (0x,1y,2z)
unsigned int m_blockedAxisMask;
                        // mask of axes that may NOT be used for splits.
                        // (Once an axis is used as a split, it cannot be used in anything along the m_midChild branch)
                        // (For an xy-only range tree, the z mask is set at the root)


size_t m_parent;     // parent node

DRange3d m_union;           // union of all ranges in m_ranges and 3 three children.
DRange3d m_intersection;    // intersection of all ranges.

TernaryRangeTreeNode ()
    {
    m_primaryListHead = s_terminatorIndex;
    m_leftChild = m_rightChild = m_midChild = s_terminatorIndex;
    m_mySplitSelect = 0;
    m_blockedAxisMask = 0;
    m_parent = s_terminatorIndex;
    m_union.Init ();
    m_intersection.Init ();
    m_intersection.Extend (0,0,0);
    m_intersection.Extend (s_universeSize);   // product of zero sets is the universe !? This is important for 
    }

// set head as the head of the primary list. (NO CHECK FOR VALIDITY -- caller responsible for coordinating with previous content)
// update ranges (using previous content)
void UpdatePrimaryList (size_t head, DRange3dCR unionUpdate, DRange3dCR intersectionUpdate)
    {
    m_intersection.IntersectIndependentComponentsOf (m_intersection, intersectionUpdate);
    m_union.Extend (unionUpdate);
    m_primaryListHead = head;    
    }

bool IsAxisBlocked (int axisSelect)
    {
    if (axisSelect < 0 || axisSelect > 2)
        return true;
    return 0 != (m_blockedAxisMask & (0x01 << axisSelect));
    }
};

struct VectorOfTernaryRangeNodes : bvector<TernaryRangeTreeNode>
{
size_t GetPrimaryListAt (size_t index)
    {
    if (index < size ())
        return at(index).m_primaryListHead;
    return -1;
    }

bool IsValidIndex (size_t index) const {return index < size ();}

size_t GetLeftChildAt (size_t parentIndex) const
    {
    if (IsValidIndex (parentIndex))
        return at(parentIndex).m_leftChild;
    return s_terminatorIndex;
    }
size_t GetRightChildAt (size_t parentIndex) const
    {
    if (IsValidIndex (parentIndex))
        return at(parentIndex).m_rightChild;
    return s_terminatorIndex;
    }
size_t GetMidChildAt (size_t parentIndex) const
    {
    if (IsValidIndex (parentIndex))
        return at(parentIndex).m_midChild;
    return s_terminatorIndex;
    }
};


// Helper struct for recursive search.

struct RecursiveRangeCollector
{
    VectorOfRangeLists const &m_ranges;
    VectorOfTernaryRangeNodes const &m_nodes;
    size_t m_numRanges;
    size_t m_numNodes;
    bool m_recurseToChildren;
    bvector <DRange3dSizeSize> &m_data;

RecursiveRangeCollector
(
VectorOfTernaryRangeNodes const &nodes,
VectorOfRangeLists const &ranges,
bool recurseToChildren,
bvector <DRange3dSizeSize> &data
) : m_ranges(ranges),
    m_nodes (nodes),
    m_numRanges (ranges.size ()),
    m_numNodes  (nodes.size ()),
    m_recurseToChildren (recurseToChildren),
    m_data(data)
    {
    }

void AddRanges (size_t nodeIndex) const
    {
    if (nodeIndex < m_numNodes)
        {
        size_t rangeIndex = m_nodes[nodeIndex].m_primaryListHead;
        while (rangeIndex < m_numRanges)
            {
            m_data.push_back (m_ranges[rangeIndex]);
            //data.back ().SetTagB (s_terminatorIndex);
            rangeIndex = m_ranges[rangeIndex].GetTagB ();
            }
        if (m_recurseToChildren)
            {
            AddRanges (m_nodes[nodeIndex].m_leftChild);
            AddRanges (m_nodes[nodeIndex].m_midChild);
            AddRanges (m_nodes[nodeIndex].m_rightChild);
            }
        }
    }
};



struct RecursiveDistributor
{
    VectorOfRangeLists &m_ranges;
    VectorOfTernaryRangeNodes &m_nodes;
    size_t m_numRanges;
    size_t m_numNodes;
    size_t m_maxPerNode;

    bvector<DoubleSizeSize> m_transitions;    // To be shared among split steps.

RecursiveDistributor
(
VectorOfTernaryRangeNodes &nodes,
VectorOfRangeLists &ranges,
size_t maxPerNode
) : m_ranges(ranges),
    m_nodes (nodes),
    m_numRanges (ranges.size ()),
    m_numNodes  (nodes.size ()),
    m_maxPerNode (maxPerNode)
    {
    }

struct AxisData
{
int m_index;
unsigned int m_mask;
SplitDetail m_splitData;
AxisData (int index, int mask)
    {
    m_index = index;
    m_mask  = mask;
    }
};

// Distribute from given node index downward.
void Go (size_t nodeIndex)
    {
    if (!m_nodes.IsValidIndex (nodeIndex))
        return;

    //AxisData axisData[3] = {AxisData(0, s_maskX), AxisData(1,s_maskY), AxisData(2,s_maskZ)};
    //int numActiveAxis = 0;
    size_t numRange = m_ranges.size ();
    for (int axisSelect = 0; axisSelect < 3; axisSelect++)
        {
        if (!m_nodes[nodeIndex].IsAxisBlocked (axisSelect))
            {
            m_transitions.clear ();
            size_t rangeIndex = m_nodes[nodeIndex].m_primaryListHead;
            for (;rangeIndex < numRange; rangeIndex = m_ranges[rangeIndex].GetTagB())
                {
                DRange3d range = m_ranges[rangeIndex].Get ();
                m_transitions.push_back (DoubleSizeSize (range.low.GetComponent (axisSelect), 0, rangeIndex));
                m_transitions.push_back (DoubleSizeSize (range.high.GetComponent (axisSelect), 1, rangeIndex));
                }
            }
        }
    }
};

struct TernaryRangeTree : public ITernaryRangeTree
{
VectorOfRangeLists m_ranges;
VectorOfTernaryRangeNodes m_nodes;

TernaryRangeTree ()
    {
    m_nodes.push_back (TernaryRangeTreeNode ());
    }

bool IsValidRangeIndex (size_t value) {return value < m_ranges.size ();}
bool IsValidTreeIndex (size_t value) {return value < m_ranges.size ();}

// Insert a (list of ranges!) to the primary list of 
void InsertListPrimary (size_t parentNodeIndex, size_t newListHead)
    {
    size_t oldHead = m_nodes[parentNodeIndex].m_primaryListHead;
    DRange3d newListUnion, newListIntersection;
    size_t tailIndex = m_ranges.WalkToTail (newListHead, newListUnion, newListIntersection);
    m_ranges[tailIndex].SetTagB (oldHead);
    m_nodes[parentNodeIndex].UpdatePrimaryList (newListHead, newListUnion, newListIntersection);
    }

// Brute force range insertion during batch load.
// (Always insert directly to unsorted list)
void _Preload (DRange3dCR range, size_t tag) override
    {
    size_t newIndex = m_ranges.size ();
    m_ranges.push_back (DRange3dSizeSize (range, tag, s_terminatorIndex));
    InsertListPrimary (0, newIndex);
    }

void _DistributePreloads (size_t maxPerNode) override
    {
    RecursiveDistributor distributor (m_nodes, m_ranges, maxPerNode);
    distributor.Go (0);
    }


size_t _GetRoot () const override {return 0;}

size_t _GetLeftChild (size_t parentIndex) const override
    { return m_nodes.GetLeftChildAt (parentIndex);}
size_t _GetRightChild (size_t parentIndex) const override
    { return m_nodes.GetRightChildAt (parentIndex);}
size_t _GetMidChild (size_t parentIndex) const override
    { return m_nodes.GetMidChildAt (parentIndex);}

void _GetRangesInNode
(
size_t nodeIndex,
bool recurseToChildren,
bvector<DRange3dSizeSize> &data
) const override
    {
    data.clear ();
    RecursiveRangeCollector collector (m_nodes, m_ranges,
                recurseToChildren, data);
    collector.AddRanges (nodeIndex);
    }

size_t CountPrimaryRanges (size_t nodeIndex)
    {
    return m_ranges.ListLengthAt (m_nodes.GetPrimaryListAt (nodeIndex));
    }

size_t CountAllDescendentRanges (size_t nodeIndex)
    {
    if (true) // nodeIndex >= 0)        unsigned value is always >= 0
        {
        size_t index = nodeIndex;
        if (index < m_ranges.size ())
            return CountPrimaryRanges (index)
                  + CountAllDescendentRanges (m_nodes[index].m_leftChild)
                  + CountAllDescendentRanges (m_nodes[index].m_midChild)
                  + CountAllDescendentRanges (m_nodes[index].m_rightChild);
        }
    return 0;
    }

size_t Depth (size_t nodeIndex = 0)
    {
    //if (nodeIndex < 0)            unsigned value is never < 0
    //    return 0;
    size_t index = nodeIndex;
    if (index >= m_nodes.size ())
        return 0;
    size_t leftDepth = Depth (m_nodes[index].m_leftChild);
    size_t midDepth  = Depth (m_nodes[index].m_midChild);
    size_t rightDepth = Depth (m_nodes[index].m_rightChild);
    size_t maxDepth = leftDepth;
    if (midDepth > maxDepth)
        maxDepth = midDepth;
    if (rightDepth > maxDepth)
        rightDepth = maxDepth;
    return 1 + maxDepth;
    }


};

ITernaryRangeTreePtr ITernaryRangeTree::Create ()
    {
    return new TernaryRangeTree ();
    }
//! Add a range to an unsorted list that can be efficiently reorganized later.
void ITernaryRangeTree::Preload (DRange3dCR range, size_t tag)
    {return _Preload (range, tag);}
//! Distribute preloaded ranges into a search structure.
void ITernaryRangeTree::DistributePreloads (size_t maxPerNode) {_DistributePreloads (maxPerNode);}

//! Get the index of the root node.
size_t ITernaryRangeTree::GetRoot () const
    {return _GetRoot ();}

//! Get index of left  children of nodeIndex;
size_t ITernaryRangeTree::GetLeftChild (size_t nodeIndex) const
    {return _GetLeftChild (nodeIndex);}
size_t ITernaryRangeTree::GetRightChild (size_t nodeIndex) const
    {return _GetRightChild (nodeIndex);}
size_t ITernaryRangeTree::GetMidChild (size_t nodeIndex) const
    {return _GetMidChild  (nodeIndex);}

//! Copy all the ranges from a node.
void ITernaryRangeTree::GetRangesInNode
(
size_t nodeIndex,
bool recurseToChildren,
bvector<DRange3dSizeSize> &data
) const
    {return _GetRangesInNode (nodeIndex, recurseToChildren, data);}


END_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE