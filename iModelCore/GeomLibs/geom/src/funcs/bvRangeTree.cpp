/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/XYZRangeTree.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


static int s_noisy = 0;
static void Indent (int noise)
    {
    for (int i = 0; i < noise; i+= 1)
        BeConsole::Printf (" ");
    }

static void Announce (int noise, const char *name, size_t value)
    {
    if (s_noisy >= noise)
        {
        Indent (noise);
        BeConsole::Printf ("%s %d\n", name, value);
        }
    }

static void Announce (int noise, const char *name, size_t valueA, size_t valueB)
    {
    if (s_noisy >= noise)
        {
        Indent (noise);
        BeConsole::Printf ("%s %d %d\n", name, valueA, valueB);
        }
    }


static void Announce (int noise, const char *name, size_t value, double a)
    {
    if (s_noisy >= noise)
        {
        Indent (noise);
        BeConsole::Printf ("%s %d %g\n", name, value, a);
        }
    }


static void Announce (int noise, const char *name, DRange3dCR range, size_t value)
    {
    if (s_noisy >= noise)
        {
        Indent (noise);
        BeConsole::Printf ("%s %d (%g,%g,%g) (%g,%g,%g)\n", name, value,
              range.low.x, range.low.y, range.low.z,
              range.high.x, range.high.y, range.high.z
              );
        }
    }


#define MAX_TREE_CHILD 50
// pairing of a double and an index, with methods to update the index along with conditional updates of the double.
struct DoubleAndIndex
{
double m_value;
size_t m_index;
DoubleAndIndex (double value, size_t index)
    : m_value(value), m_index(index)
    {
    }
double Value () const {return m_value;}
size_t Index () const {return m_index;}

//! @description If input value is (strictly) greater than the stored value, update both the stored value and index
bool UpdateIfGT (double value, size_t index)
    {
    if (value > m_value)
        {
        m_value = value;
        m_index = index;
        return true;
        }
    return false;        
    }

//! @description If input value is (strictly) less than the stored value, update both the stored value and index
bool UpdateIfLT (double value, size_t index)
    {
    if (value < m_value)
        {
        m_value = value;
        m_index = index;
        return true;
        }
    return false;
    }

};

// expand rangeA to include rangeB.
// return true if any direction changed.
static bool Extend (DRange3dR rangeA, DRange3dCR rangeB)
    {
    bool bExtended = false;

    if (rangeB.low.x < rangeA.low.x)
        {
        rangeA.low.x = rangeB.low.x;
        bExtended = true;
        }

    if (rangeB.high.x > rangeA.high.x)
        {
        rangeA.high.x = rangeB.high.x;
        bExtended = true;
        }

    if (rangeB.low.y < rangeA.low.y)
        {
        rangeA.low.y = rangeB.low.y;
        bExtended = true;
        }

    if (rangeB.high.y > rangeA.high.y)
        {
        rangeA.high.y = rangeB.high.y;
        bExtended = true;
        }

    if (rangeB.low.z < rangeA.low.z)
        {
        rangeA.low.z = rangeB.low.z;
        bExtended = true;
        }

    if (rangeB.high.z > rangeA.high.z)
        {
        rangeA.high.z = rangeB.high.z;
        bExtended = true;
        }

    return bExtended;
    }


struct BVRangeNode;
struct BVRangeTree;

typedef struct BVRange const & BVRangeCR;
typedef struct BVRange & BVRangeR;
typedef struct BVRangeNode const &BVRangeNodeCR;
typedef struct BVRangeNode &BVRangeNodeR;

struct BVRange : public DRange3d
{
friend BVRangeNode;
friend BVRangeTree;

private:
        size_t m_index;
public:
BVRange (DRange3dCR range, size_t index) : DRange3d (range), m_index (index)
    {
    }

size_t GetIndex () const {return m_index;}
size_t SetIndex () const {return m_index;}
};


// A range node has bvectors for both leaves (user data) and child nodes (in the tree)
// However, the tree management only ever uses one of the two.  A node "just above" the leaves has contents only in m_leaves, and
//  any higher level node in the tree has non-empty m_children.
struct BVRangeNode
{
friend BVRangeTree;
private:
size_t m_parentIndex;
DRange3d m_compositeRange;
double   m_extentSquared;
bvector <BVRange> m_leaves;       // user range and index
bvector <size_t> m_children;       // child in BVRangeTree
public:
BVRangeNode (size_t parentIndex)
    : m_parentIndex (parentIndex)
    {
    m_compositeRange.Init ();
    }

static const int s_maxLeafInNode = 50;
static const int s_maxChildInNode = 50;
    
size_t GetParentIndex () const { return m_parentIndex;}
void   SetParentIndex (size_t parentIndex) { m_parentIndex = parentIndex;}    
DRange3d GetDRange3d () const {return m_compositeRange;}

// Clear leaf array.
void ClearLeaves (bool clearRange)
    {
    m_leaves.clear ();
    if (clearRange)
        m_compositeRange.Init ();
    }

// Clear child array -- leave range composite data untouched
void ClearChildren (bool clearRange)
    {
    m_children.clear ();
    if (clearRange)
        m_compositeRange.Init ();
    }

bool RangeChangesWhenExtended (DRange3dCR range)
    {
    if (Extend (m_compositeRange, range))
        {
        m_extentSquared = m_compositeRange.ExtentSquared ();
        return true;
        }
    return false;
    }

// Records the range and index as leaf data.
// Return true if the comosite range expanded.
void UnconditionalAddLeaf (BVRangeCR range)
    {
    m_leaves.push_back (range);
    RangeChangesWhenExtended (range);
    }

// Records the child index in m_children and extends the composite range.
// Return true if the comosite range expanded.
void UnconditionalAddChild (DRange3dCR range, size_t index)
    {
    m_children.push_back (index);
    RangeChangesWhenExtended (range);
    }


bool IsLeafArrayFull () const {return m_leaves.size () > s_maxLeafInNode;}
bool IsChildArrayFull () const {return m_children.size () >= s_maxChildInNode;}
bool HasLeaves () const {return m_leaves.size () > 0;}
bool HasChildren () const {return m_children.size () > 0;}

// return numerical value that penalizes expanded ranges.
double FitFunc (DRange3dCR insertedRange)
    {
    DRange3d newCompositeRange = m_compositeRange;
    Extend (newCompositeRange, insertedRange);
    double delta = newCompositeRange.ExtentSquared () - m_extentSquared;
    return m_extentSquared + delta;
    }
};



struct BVRangeSplitCandidate
{
BVRange         range;
int             groupNumber[3];
double          min;   // active axis min
double          max;   // active axis max

BVRangeSplitCandidate (BVRange const &inRange)
    : range (inRange)
    {
    groupNumber[0] = groupNumber[1] = groupNumber[2] = 0;
    min = max = 0.0;
    }    
static bool cb_compareForSeparator
(
const BVRangeSplitCandidate  &candidateA,
const BVRangeSplitCandidate  &candidateB
)
    {
    if (candidateA.min < candidateB.min)
        return  true;
    if (candidateA.min > candidateB.min)
        return   false;
    if (candidateA.max < candidateB.max)
        return  true;
    if (candidateA.max > candidateB.max)
        return   false;
    return  false;
    }

void SetMinMaxFromX ()
    {
    min = range.low.x;
    max = range.high.x;
    }
    
void SetMinMaxFromY ()
    {
    min = range.low.y;
    max = range.high.y;
    }

void SetMinMaxFromZ ()
    {
    min = range.low.z;
    max = range.high.z;
    }
};



// ===================================================================================
struct BVSplitCandidateArray
    {
    bvector<BVRangeSplitCandidate> m_candidates;
    
void Clear ()
    {
    m_candidates.clear ();
    }
void Add (BVRange const &range)
    {
    m_candidates.push_back (BVRangeSplitCandidate (range));
    }

void ActivateAxis (size_t axis)
    {
    size_t numCandidate = m_candidates.size ();
    if (axis == 0)
        {
        for (size_t i = 0; i < numCandidate; i++)
            m_candidates[i].SetMinMaxFromX ();
        }
    else if (axis == 1)
        {
        for (size_t i = 0; i < numCandidate; i++)
            m_candidates[i].SetMinMaxFromX ();
        }
    else
        {
        for (size_t i = 0; i < numCandidate; i++)
            m_candidates[i].SetMinMaxFromY ();
        }
    }

    
// Sort along specified axis.
// look for good split point.
// return numeric indicator of how good the split is.
double CheckSeparation (size_t axis)
    {
    size_t numCandidates = m_candidates.size ();
    size_t minSize = numCandidates / 3;
    ActivateAxis (axis);
    std::sort (m_candidates.begin (), m_candidates.end (), BVRangeSplitCandidate::cb_compareForSeparator);

    // Look in the middle third for the biggest step forward ...
    size_t i0 = minSize;                       // First m_candidates in "middle third"
    size_t i1 = numCandidates - minSize - 1;    // Last m_candidates in "middle third"

    ptrdiff_t iAfterGap        = -1;    // index of upper entry with largest gap (m_candidates[i].max to [i+1].min)
    ptrdiff_t iAfterAdvance    = -1;    // index of upper entry of non-gap with largest step in simple sort of min's
    double maxGap = 0.0;
    double maxAdvance = -DBL_MAX;
    for (size_t i = i0; i < i1; i++)
        {
        size_t j = i + 1;
        double gap = m_candidates[j].min - m_candidates[i].max;
        if (gap > 0.0)   // true gap ..
            {
            if (gap > maxGap)
                {
                maxGap = gap;
                iAfterGap = j;
                }
            }
        else        // non gap
            {
            double advance = m_candidates[j].min - m_candidates[i].min;
            if (advance > maxAdvance)
                {
                maxAdvance       = advance;
                iAfterAdvance = j;
                }
            }
        }


    size_t iSeparator;
    if (iAfterGap >= 0)
        iSeparator = iAfterGap;
    else if (iAfterAdvance > 0)
        iSeparator = iAfterAdvance;
    else
        iSeparator = numCandidates / 2;

    for (size_t i = 0; i < numCandidates; i++)
       m_candidates[i].groupNumber[axis] = i > iSeparator ? 1 : 0;
    Announce (4, "Axis Separation", axis, maxGap);

    return  maxGap;
    }

// Choose the axis index that gives the best separation.
size_t ChooseBestSeparationAxis ()
    {
    DoubleAndIndex bestAxisData (CheckSeparation (0), 0);
    bestAxisData.UpdateIfLT (CheckSeparation (1), 1);
    bestAxisData.UpdateIfLT (CheckSeparation (2), 2);
    return bestAxisData.Index ();
    }

// ASSUME .. each candidate has a destination index in groupNumber[selector]
// NOTE This can't dereference to the children, so it cannot reset their parent index!!!
void DistributeAsChildren
(
BVRangeNode &nodeA,
BVRangeNode &nodeB,
size_t      selector
)
    {
    nodeA.ClearChildren (true);
    nodeB.ClearChildren (true);
    for (BVRangeSplitCandidate const & candidate : m_candidates)
        {
        if (candidate.groupNumber[selector] == 0)
            nodeA.UnconditionalAddChild (candidate.range, candidate.range.GetIndex ());
        else
            nodeB.UnconditionalAddChild (candidate.range, candidate.range.GetIndex ());
        }
    }

// ASSUME .. each candidate has a destination index in groupNumber[selector]
// CLEAR nodeA and nodeB.
// Insert each candidate <range, index> in nodeA or nodeB according to its groupNumber[selector]
void DistributeAsLeaves
(
BVRangeNode &nodeA,
BVRangeNode &nodeB,
size_t   selector
)
    {
    nodeA.ClearLeaves (true);
    nodeB.ClearLeaves (true);

    for (BVRangeSplitCandidate const & candidate : m_candidates)
        {
        if (candidate.groupNumber[selector] == 0)
            nodeA.UnconditionalAddLeaf (candidate.range);
        else
            nodeB.UnconditionalAddLeaf (candidate.range);
        }
    }
};


struct BVRangeTree : IRangeTree3d
{
private:
bvector <BVRangeNode> m_nodes;
BVSplitCandidateArray m_splitCandidates;
size_t  m_rootIndex;

    
size_t AddEmptyNode ()
    {
    size_t index = m_nodes.size ();
    m_nodes.push_back (BVRangeNode (SIZE_MAX));
    return index;
    }

size_t CreateNewRoot ()
    {
    m_rootIndex = AddEmptyNode ();
    return m_rootIndex;
    }
    
size_t GetRootIndex (){return m_rootIndex;}
    
static const size_t s_invalidIndex = SIZE_MAX;

bool IsRoot (size_t index){ return index == m_rootIndex;}
bool IsValidTreeIndex (size_t index) { return index < m_nodes.size ();}

void Traverse (size_t nodeIndex, DRange3dRecursionHandler &handler)
    {
    if (!handler.IsActive ())
        return;
    if (!IsValidTreeIndex (nodeIndex))
        return;
    if (handler.ShouldRecurseIntoSubtree (m_nodes[nodeIndex].m_compositeRange))
        {
        BVRangeNode &node = m_nodes.at (nodeIndex);
        for (size_t i = 0, n = node.m_leaves.size (); handler.IsActive () && i < n; i++)
            handler.AnnounceLeaf (node.m_leaves[i], node.m_leaves[i].GetIndex ());
        for (size_t i = 0, n = node.m_children.size (); handler.IsActive () && i < n; i++)
            {
            Traverse (node.m_children[i], handler);
            }
        }
    }


// return numerical measure of expansion (bad) of this node's range.
double ImmediateFitFunction (DRange3dCR range, size_t index)
    {
    if (IsValidTreeIndex (index))
        return m_nodes[index].FitFunc (range);
    return DBL_MAX;
    }
    
size_t FindChildWithBestImmediateFitFunc (DRange3dCR range, size_t parentIndex)
    {
    if (!IsValidTreeIndex (parentIndex))
        return s_invalidIndex;
    size_t bestIndex = s_invalidIndex;
    double aMin = DBL_MAX;
    auto &node = m_nodes.at (parentIndex);
    for (size_t childIndex : node.m_children)
        {
        double a = ImmediateFitFunction (range, childIndex);
        if (a < aMin)
            {
            bestIndex = childIndex;
            aMin = a;
            }
        }
    return bestIndex;
    }    

// Search downwards for a fringe node that is best fit for new range.
size_t RecurseToBestFit (size_t startIndex, DRange3dCR range, size_t depth)
    {
    static size_t s_triggerDepth (10);
    if (!IsValidTreeIndex (startIndex))
        return s_invalidIndex;      // This should never happen?
    if (depth > s_triggerDepth)
        Announce (0, "Deep Recursion Problem?", startIndex);
    // If this is a fringe accept ...
    if (m_nodes[startIndex].m_children.size () == 0)
        return startIndex;

    size_t childIndex = FindChildWithBestImmediateFitFunc (range, startIndex);
    if (IsValidTreeIndex (childIndex))
        return RecurseToBestFit (childIndex, range, depth + 1);
    return startIndex;
    }

void FixupParentRanges (size_t index)
    {
    Announce (5, "FixupParentRanges", index);
    if (IsValidTreeIndex (index))
        {
        for (size_t currentIndex = index;;)
            {
            Announce (7, "    FixupParentRanges", index);
            DRange3d currentRange = m_nodes[currentIndex].m_compositeRange;
            size_t nextIndex = m_nodes[currentIndex].GetParentIndex ();
            if (nextIndex != currentIndex && IsValidTreeIndex(nextIndex)
                && m_nodes[nextIndex].RangeChangesWhenExtended (currentRange)
                )
                {
                currentIndex = nextIndex;
                // continue upwards
                }
            else
                break;
            }
        }
    }

void SetParentInAllChildren (size_t parentIndex)
    {
    if (!IsValidTreeIndex (parentIndex))
        return;
    for (size_t childIndex : m_nodes[parentIndex].m_children)
        m_nodes[childIndex].SetParentIndex (parentIndex);
    }
    
void RedistributeChildrenIfFull (size_t indexA)
    {
    if (!IsValidTreeIndex (indexA))
        return;
    if (!m_nodes[indexA].IsChildArrayFull ())
        return;
        
    m_splitCandidates.Clear ();
    for (size_t childIndex : m_nodes[indexA].m_children)
        {
        assert (IsValidTreeIndex (childIndex));
        m_splitCandidates.Add (BVRange (m_nodes[childIndex].m_compositeRange, childIndex));
        }
        
    // REMARK: The leaf data is now completely in the splitCandidates array -- indexA can be cleared in the Distribute step.
    size_t bestSplitAxis = m_splitCandidates.ChooseBestSeparationAxis ();
    size_t indexB = AddEmptyNode ();
    m_splitCandidates.DistributeAsChildren (m_nodes[indexA], m_nodes[indexB], bestSplitAxis);
    SetParentInAllChildren (indexA);
    SetParentInAllChildren (indexB);
    if (IsRoot (indexA))
        {
        size_t newRoot = CreateNewRoot ();
        // new root has no range data -- this is updated by the two insertions.
        m_nodes[newRoot].UnconditionalAddChild (m_nodes[indexA].GetDRange3d (), indexA);
        m_nodes[indexA].SetParentIndex (newRoot);
        m_nodes[newRoot].UnconditionalAddChild (m_nodes[indexB].GetDRange3d (), indexB);
        m_nodes[indexB].SetParentIndex (newRoot);
        }
    else
        {
        size_t parentIndex = m_nodes[indexA].GetParentIndex ();
        m_nodes[parentIndex].UnconditionalAddChild (m_nodes[indexB].GetDRange3d (), indexB);
        m_nodes[indexB].SetParentIndex (parentIndex);
        // (INV: parentIndex has children but no leaves !?!?)
        RedistributeChildrenIfFull (parentIndex);
        }
    }

void InsertChildInParent (size_t parentIndex, size_t childIndex)
    {
    Announce (2, "InsertChildInParent", parentIndex, childIndex);
    m_nodes[parentIndex].UnconditionalAddChild (m_nodes[childIndex].GetDRange3d (), childIndex);
    m_nodes[childIndex].SetParentIndex (parentIndex);
    }

void RedistributeLeavesIfFull (size_t indexA)
    {
    if (!IsValidTreeIndex (indexA))
        return;
    if (!m_nodes[indexA].IsLeafArrayFull ())
        return;
    Announce (2, "SplitLeaf", indexA);
    m_splitCandidates.Clear ();
    for (BVRangeCR leafData : m_nodes[indexA].m_leaves)
        m_splitCandidates.Add (leafData);
    // REMARK: The leaf data is now completely in the splitCandidates array -- indexA can be cleared in the Distribute step.
    size_t bestAxisIndex = m_splitCandidates.ChooseBestSeparationAxis ();

    size_t indexB = AddEmptyNode ();
    
    m_splitCandidates.DistributeAsLeaves (m_nodes[indexA], m_nodes[indexB], bestAxisIndex);
    
    if (IsRoot (indexA))
        {
        size_t newRoot = CreateNewRoot ();
        Announce (2, "NewRoot", newRoot);
        // new root has no range data -- this is updated by the two insertions.
        InsertChildInParent (newRoot, indexA);
        InsertChildInParent (newRoot, indexB);
        }
    else
        {
        size_t parentIndex = m_nodes[indexA].GetParentIndex ();
        // indexA is already a child of parentIndex
        InsertChildInParent (parentIndex, indexB);
        // (INV: parentIndex has children but no leaves !?!?)
        RedistributeChildrenIfFull (parentIndex);
        }
    }

/// <param name="bFixupRanges">If true, recursively update ranges of ancestors.</param>
bool InsertLeafInExistingNode (
size_t parentIndex,
BVRangeCR range,
bool   fixupParentRanges,
bool   splitIfFull
)
    {
    if (!IsValidTreeIndex (parentIndex))
        parentIndex = 0;        // And the _ctor always placed that node there ....
    Announce (6, "AddLeaf in node", range, parentIndex);
    m_nodes[parentIndex].UnconditionalAddLeaf (range);
    if (fixupParentRanges)
        FixupParentRanges (parentIndex);
    if (splitIfFull)
        RedistributeLeavesIfFull (parentIndex);
    return true;
    }

public:

BVRangeTree ()
    {
    CreateNewRoot (); // An empty tree has a root with no children, and the distinctive parent index.
    }

void _Traverse (DRange3dRecursionHandler &handler) override
    {
    Traverse (m_rootIndex, handler);
    }

bool _Add (DRange3dCR userRange, size_t userIndex) override
    {
    size_t fringeIndex = RecurseToBestFit(m_rootIndex, userRange, 0);
    Announce (4, "UserRange", userRange, userIndex);
    Announce (4, "     InsertToFringe", fringeIndex);
    return InsertLeafInExistingNode (fringeIndex, BVRange (userRange, userIndex), true, true);
    }

struct PairSearchContext
    {
    PairSearchContext (BVRangeTree &treeA, BVRangeTree &treeB, DRange3dPairRecursionHandler &handler)
        : m_treeA (treeA), m_treeB (treeB), m_handler (handler)
        {
        
        }
    BVRangeTree &m_treeA;
    BVRangeTree &m_treeB;
    DRange3dPairRecursionHandler &m_handler;
    
    void Recurse_LeafNode (BVRangeCR rangeA, size_t indexB)
        {
        if (!m_treeB.IsValidTreeIndex (indexB))
            return;
        if (!m_handler.TestLeafInteriorPair (rangeA, rangeA.GetIndex (), m_treeB.m_nodes[indexB].m_compositeRange))
            return;
        for (BVRangeCR rangeB : m_treeB.m_nodes[indexB].m_leaves)
            {
            if (!m_handler.StillSearching ())
                return;
            m_handler.TestLeafLeafPair (rangeA, rangeA.GetIndex (), rangeB, rangeB.GetIndex ());
            }
        for (size_t childIndexB : m_treeB.m_nodes[indexB].m_children)
            {
            if (!m_handler.StillSearching ())
                return;
            Recurse_LeafNode (rangeA, childIndexB);
            }            
        }
    void Recurse_NodeNode (size_t indexA, size_t indexB)
        {
        if (!m_treeA.IsValidTreeIndex (indexA) || !m_treeB.IsValidTreeIndex (indexB))
            return;
        if (!m_handler.TestInteriorInteriorPair (m_treeA.m_nodes[indexA].m_compositeRange, m_treeB.m_nodes[indexB].m_compositeRange))
            return;
        // treeB varies fastest.
        // process immediate leaves before children
        for (BVRangeCR rangeA : m_treeA.m_nodes[indexA].m_leaves)
            {
            if (!m_handler.StillSearching ())
                return;
            Recurse_LeafNode (rangeA, indexB);
            }
        for (size_t childIndexA : m_treeA.m_nodes[indexA].m_children)
            {
            if (!m_handler.StillSearching ())
                return;
            Recurse_NodeNode (childIndexA, indexB);
            }            
        }
    void Recurse_FromRoots ()
        {
        Recurse_NodeNode (m_treeA.GetRootIndex (), m_treeB.GetRootIndex ());
        }        
    };
static void Traverse (BVRangeTree &treeA, BVRangeTree &treeB, DRange3dPairRecursionHandler &handler)
    {
    PairSearchContext searcher (treeA, treeB, handler);
    searcher.Recurse_FromRoots ();
    }
};


//*************************************************************************
// published side of VPP implementation
//*************************************************************************
IRangeTree3d::IRangeTree3d (){};

bool IRangeTree3d::Add (DRange3dCR userRange, size_t userIndex){ return _Add (userRange, userIndex);}
void IRangeTree3d::Traverse (DRange3dRecursionHandler &handler){ return _Traverse (handler);}

IRangeTree3dPtr IRangeTree3d::Create ()
    {
    return new BVRangeTree ();
    }







struct Polyface01RangeSearcher : DRange3dRecursionHandler
{
private:
DRange3d m_baseRange;

DRange3d m_currentRange;
bvector<size_t> m_hits;
public:

Polyface01RangeSearcher ()
    {
    }

~Polyface01RangeSearcher ()
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
void CopyHits (bvector<size_t> &hits){hits = m_hits;}

bool ShouldRecurseIntoSubtree (DRange3dCR range) override 
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

bool IsActive ()  override {return true;}

void AnnounceLeaf (DRange3dCR range, size_t index) override 
    {
    if (m_currentRange.IntersectsWith (range))
        {
        AddHit (index); 
        m_leafHit++;
        }
    else
        {
        m_leafSkip++;
        }
    }

};

//================================================================================

PolyfaceRangeTree01::PolyfaceRangeTree01 ()
    : m_rangeTree (IRangeTree3d::Create ())
    {
    }

PolyfaceRangeTree01::~PolyfaceRangeTree01 ()  {}

size_t PolyfaceRangeTree01::LoadPolyface (PolyfaceQueryCR source)
    {
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (source, false);
    size_t numFacet = 0;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        size_t readIndex = visitor->GetReadIndex ();
        DRange3d range = DRange3d::From (visitor->Point());
        m_rangeTree->Add (range, readIndex);
        numFacet++;
        }
    return numFacet;
    }    

PolyfaceRangeTree01Ptr PolyfaceRangeTree01::CreateForPolyface (PolyfaceQueryCR source)
    {
    PolyfaceRangeTree01 *rangeTree = new PolyfaceRangeTree01 ();
    rangeTree->LoadPolyface (source);
    return rangeTree;
    }

void PolyfaceRangeTree01::CollectInRange (bvector<size_t> &hits, DRange3dCR range, double expansion)
    {
    Polyface01RangeSearcher searcher;
    searcher.SetBaseRange (range);
    searcher.SetCurrentRangeFromExpandedBaseRange (expansion);
    m_rangeTree->Traverse (searcher);
    searcher.CopyHits (hits);
    }



// Collect indices of intersecting facets.
//
// 
struct FacetClashCollector01 : DRange3dPairRecursionHandler
{
PolyfaceQueryR  m_polyfaceA;
PolyfaceQueryR m_polyfaceB;
double m_rangeExpansion;
bvector<std::pair<size_t, size_t>> &m_hits;
size_t m_maxHits;

BoolCounter m_II;
BoolCounter m_IL;
BoolCounter m_LI;
BoolCounter m_LL;
BoolCounter m_LLClash;
PolyfaceVisitorPtr m_visitorA;
PolyfaceVisitorPtr m_visitorB;

FacetClashCollector01
(
PolyfaceQueryR polyfaceA,
PolyfaceQueryR polyfaceB,
double rangeExpansion,
bvector<std::pair<size_t, size_t>> &hits,
size_t maxHits
)
: m_rangeExpansion (rangeExpansion),
  m_polyfaceA (polyfaceA),
  m_polyfaceB (polyfaceB),
  m_hits (hits),
  m_maxHits (maxHits)
    {
    BeConsole::Printf (" maxHits %d,%d\n", maxHits, m_maxHits);
    m_visitorA = PolyfaceVisitor::Attach (polyfaceA);
    m_visitorB = PolyfaceVisitor::Attach (polyfaceB);
    m_visitorA->Reset ();
    m_visitorB->Reset ();
    }
    
bool TestOverlap (DRange3dCR rangeA, DRange3dCR rangeB)
    {
    return rangeA.IntersectsWith (rangeB, m_rangeExpansion, 3);
    }

bool TestInteriorInteriorPair (DRange3dCR rangeA, DRange3dCR rangeB) override 
  {
  return m_II.Count (TestOverlap (rangeA, rangeB));
  }

bool TestInteriorLeafPair (DRange3dCR rangeA, DRange3dCR rangeB, size_t indexB) override
  {
  return m_IL.Count (TestOverlap (rangeA, rangeB));
  }

bool TestLeafInteriorPair (DRange3dCR rangeA, size_t indexA, DRange3dCR rangeB) override
  {
  return m_IL.Count (TestOverlap (rangeA, rangeB));
  }

void TestLeafLeafPair (DRange3dCR rangeA, size_t indexA, DRange3dCR rangeB, size_t indexB) override
    {
    if (m_LL.Count (rangeA.IntersectsWith (rangeB, m_rangeExpansion, 3)))
        {
        m_visitorA->MoveToFacetByReadIndex (indexA);
        m_visitorB->MoveToFacetByReadIndex (indexB);
        if (m_LLClash.Count (bsiDPoint3dArray_polygonClashXYZ (
            m_visitorA->GetPointCP (), (int) m_visitorA->Point ().size (),
            m_visitorB->GetPointCP (), (int) m_visitorB->Point ().size ()
            )))
            {
            m_hits.push_back (std::pair <size_t, size_t> (indexA, indexB));
            }
        }
    }

bool StillSearching () override
    {
    return m_hits.size () < m_maxHits;
    }
};    
    
// Search for clashing pairs.
//
GEOMDLLIMPEXP void PolyfaceRangeTree01::CollectClashPairs (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceRangeTree01 &treeA,           //!< range tree for polyfaceA
PolyfaceQueryR polyfaceB,           //!< second polyface
PolyfaceRangeTree01 &treeB,           //!< range tree for polyfaceB
bvector<std::pair<size_t, size_t>> &hits,   //!< read indices of clashing pairs
size_t maxHits                      //! maximum number of hits to collect.
)
    {
    hits.clear ();
    static double s_rangeExpansion = 1.0e-12;
    hits.clear ();
    FacetClashCollector01 collector (polyfaceA, polyfaceB, s_rangeExpansion, hits, maxHits);
    BVRangeTree *bvTreeA = dynamic_cast <BVRangeTree *> (treeA.m_rangeTree.get ());
    BVRangeTree *bvTreeB = dynamic_cast <BVRangeTree *> (treeB.m_rangeTree.get ());
    if (nullptr != bvTreeA && nullptr != bvTreeB)
        BVRangeTree::Traverse (*bvTreeA, *bvTreeB, collector);
    //searcher.RunSearch (treeA.GetXYZRangeTree (), treeB.GetXYZRangeTree (), collector);
    }
    
END_BENTLEY_GEOMETRY_NAMESPACE
            