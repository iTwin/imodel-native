/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RangeIndex.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

using namespace RangeIndex;

BEGIN_UNNAMED_NAMESPACE

typedef Tree::Node&         DRTNodeR;
typedef Tree::Node*         DRTNodeP;
typedef DRTNodeP*           DRTNodeH;
typedef Tree::LeafNode*     DRTLeafNodeP;
typedef Tree::InternalNode* DRTInternalNodeP;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static bool rangeIsValid(FBoxCR range, bool is3d)
    {
    return (range.m_low.x <= range.m_high.x) && (range.m_low.y <= range.m_high.y) && (!is3d || (range.m_low.z <= range.m_high.z));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static double rangeExtentSquared(FBoxCR range)
    {
    double extentX = (double) range.m_high.x - range.m_low.x;
    double extentY = (double) range.m_high.y - range.m_low.y;
    double extentZ = (double) range.m_high.z - range.m_low.z;
    return extentX * extentX + extentY * extentY + extentZ * extentZ;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void extendRange(FBoxR thisRange, FBoxCR range)
    {
    if (range.m_low.x < thisRange.m_low.x)
        thisRange.m_low.x = range.m_low.x;

    if (range.m_low.y < thisRange.m_low.y)
        thisRange.m_low.y = range.m_low.y;

    if (range.m_low.z < thisRange.m_low.z)
        thisRange.m_low.z = range.m_low.z;

    if (range.m_high.x > thisRange.m_high.x)
        thisRange.m_high.x = range.m_high.x;

    if (range.m_high.y > thisRange.m_high.y)
        thisRange.m_high.y = range.m_high.y;

    if (range.m_high.z > thisRange.m_high.z)
        thisRange.m_high.z = range.m_high.z;
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DRTSplitEntry
{
    FBox m_range;
    DgnElementId m_id; 
    DgnCategoryId m_category; 
    void* m_vp;
    int m_groupNumber[3];
};

typedef DRTSplitEntry* DRTSplitEntryP;
typedef DRTSplitEntry const * DRTSplitEntryCP;
typedef DRTSplitEntry const& DRTSplitEntryCR;

static inline bool compareX(DRTSplitEntryCR entry1, DRTSplitEntryCR entry2) {return entry1.m_range.m_low.x < entry2.m_range.m_low.x;}
static inline bool compareY(DRTSplitEntryCR entry1, DRTSplitEntryCR entry2) {return entry1.m_range.m_low.y < entry2.m_range.m_low.y;}
static inline bool compareZ(DRTSplitEntryCR entry1, DRTSplitEntryCR entry2) {return entry1.m_range.m_low.z < entry2.m_range.m_low.z;}
typedef bool (*PF_CompareFunc)(DRTSplitEntryCR, DRTSplitEntryCR);

enum SplitAxis {X_AXIS = 0 ,Y_AXIS = 1, Z_AXIS = 2};

/*---------------------------------------------------------------------------------**//**
* @bsimethod    RangeNode                                       KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
static double checkSeparation(DRTSplitEntryP entries, size_t count, SplitAxis axis)
    {
    double maxSeparation = 0, separation;
    double maxMinDist = 1.0e200, minDist;

    static PF_CompareFunc  s_compareFuncs[3] = { compareX, compareY, compareZ };
    std::sort(entries, entries+count, s_compareFuncs[axis]);

    size_t minSize = count/3;
    DRTSplitEntryCP lastEntry    = entries + count;
    DRTSplitEntryCP entryEnd     = entries + (count-minSize);
    DRTSplitEntryCP startEntries = entries + minSize;
    DRTSplitEntryCP sepEntry=nullptr, minDistEntry=nullptr;

    switch (axis)
        {
        case X_AXIS:
            for (DRTSplitEntryCP currEntry = startEntries; currEntry < entryEnd-1; ++currEntry)
                {
                DRTSplitEntryCP nextEntry = currEntry + 1;

                if (currEntry->m_range.m_high.x < nextEntry->m_range.m_low.x)
                    {
                    if ((separation = (nextEntry->m_range.m_low.x - currEntry->m_range.m_high.x)) > maxSeparation)
                        {
                        maxSeparation = separation;
                        sepEntry = nextEntry;
                        }
                    }
                else
                    {
                    if ((minDist = (nextEntry->m_range.m_low.x - currEntry->m_range.m_low.x)) > maxMinDist)
                        {
                        maxMinDist    = minDist;
                        minDistEntry = nextEntry;
                        }
                    }
                }
            break;

        case Y_AXIS:
            for (DRTSplitEntryCP currEntry = startEntries; currEntry < entryEnd-1; ++currEntry)
                {
                DRTSplitEntryCP nextEntry = currEntry + 1;

                if (currEntry->m_range.m_high.y < nextEntry->m_range.m_low.y)
                    {
                    if ((separation = (nextEntry->m_range.m_low.y - currEntry->m_range.m_high.y)) > maxSeparation)
                        {
                        maxSeparation = separation;
                        sepEntry = nextEntry;
                        }
                    }
                else
                    {
                    if ((minDist = (nextEntry->m_range.m_low.y - currEntry->m_range.m_low.y)) > maxMinDist)
                        {
                        maxMinDist = minDist;
                        minDistEntry = nextEntry;
                        }
                    }
                }
            break;

        case Z_AXIS:
            for (DRTSplitEntryCP currEntry = startEntries; currEntry < entryEnd-1; ++currEntry)
                {
                DRTSplitEntryCP nextEntry = currEntry + 1;

                if (currEntry->m_range.m_high.z < nextEntry->m_range.m_low.z)
                    {
                    if ((separation = (nextEntry->m_range.m_low.z - currEntry->m_range.m_high.z)) > maxSeparation)
                        {
                        maxSeparation = separation;
                        sepEntry = nextEntry;
                        }
                    }

                else
                    {
                    if ((minDist = (nextEntry->m_range.m_low.z - currEntry->m_range.m_low.z)) > maxMinDist)
                        {
                        maxMinDist = minDist;
                        minDistEntry = nextEntry;
                        }
                    }
                }
            break;
        }

    if (nullptr == sepEntry)
        {
        if (nullptr != minDistEntry)
            sepEntry = minDistEntry;
        else
            sepEntry = entries + count/2;
        }

    for (DRTSplitEntryP currEntry = entries; currEntry < lastEntry; ++currEntry)
        {
        if (currEntry < sepEntry)
            currEntry->m_groupNumber[axis] = 0;
        else
            currEntry->m_groupNumber[axis] = 1;
        }

    return  maxSeparation;
    }
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::Node::ValidateRange()
    {
    if (!m_sloppy)
        return;

    DRTLeafNodeP leaf = ToLeaf();
    if (leaf)
        leaf->ValidateLeafRange();
    else
        ((DRTInternalNodeP) this)->ValidateInternalRange();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
Traverser::Stop Tree::Node::Traverse(Traverser& traverser, bool is3d)
    {
    DRTLeafNodeP leaf = ToLeaf();
    return leaf ? leaf->Traverse(traverser, is3d) : ((DRTInternalNodeP) this)->Traverse(traverser, is3d);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
inline void Tree::InternalNode::ValidateInternalRange()
    {
    if (!m_sloppy)
        return;

    m_sloppy = false;
    ClearRange();
    for (DRTNodeH curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        extendRange(m_nodeRange, (*curr)->GetRange());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
inline void Tree::LeafNode::ValidateLeafRange()
    {
    if (!m_sloppy)
        return;

    m_sloppy = false;
    ClearRange();

    for (Entry* curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        extendRange(m_nodeRange, curr->m_range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tree::Node::Overlaps(FBoxCR range) const
    {
    if (m_nodeRange.m_low.x > range.m_high.x || m_nodeRange.m_high.x < range.m_low.x ||
        m_nodeRange.m_low.y > range.m_high.y || m_nodeRange.m_high.y < range.m_low.y)
        return  false;

    return !m_is3d ? true : (m_nodeRange.m_low.z <= range.m_high.z && m_nodeRange.m_high.z >= range.m_low.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tree::Node::CompletelyContains(FBoxCR range) const
    {
    if (m_nodeRange.m_low.x >= range.m_low.x || m_nodeRange.m_high.x <= range.m_high.x ||
        m_nodeRange.m_low.y >= range.m_low.y || m_nodeRange.m_high.y <= range.m_high.y)
        return  false;

    return !m_is3d ? true : (m_nodeRange.m_low.z < range.m_low.z && m_nodeRange.m_high.z > range.m_high.z);
    }

/*---------------------------------------------------------------------------------**//**
* An InternalNode has become full, split into two nodes (a new one and this one) and determine an optimal division of the current
* enteries between this node and the new one.
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::InternalNode::SplitInternalNode(TreeR root)
    {
    size_t  count = GetEntryCount();
    DRTSplitEntryP  splitEntries = (DRTSplitEntryP) _alloca(count * sizeof(DRTSplitEntry));

    DRTSplitEntryP currEntry = splitEntries;
    DRTSplitEntryP endEntry = splitEntries + count;

    for (DRTNodeH curr = &m_firstChild[0]; curr < m_endChild; ++curr, ++currEntry)
        {
        currEntry->m_vp = *curr;
        currEntry->m_range = (*curr)->GetRangeCR();
        }

    double xSep = checkSeparation(splitEntries, count, X_AXIS);
    double ySep = checkSeparation(splitEntries, count, Y_AXIS);
    double zSep = m_is3d ? checkSeparation(splitEntries, count, Z_AXIS) : 0;

    SplitAxis  optimalSplit = X_AXIS;
    if (ySep > xSep)
        optimalSplit = Y_AXIS;
    if (m_is3d && (zSep > ySep) && (zSep > xSep))
        optimalSplit = Z_AXIS;

    // allocate a new InternalNode to hold (approx) half or our entries. If parent is nullptr, this is the root of the tree and it has become full.
    // We need to add a new level to the tree. Move all of the current entries into a new node that will become a child of this node.
    DRTInternalNodeP newNode1 = root.AllocateInternalNode();
    DRTInternalNodeP newNode2 = (nullptr == m_parent) ? root.AllocateInternalNode() : this;

    ClearChildren();

    for (currEntry = splitEntries; currEntry < endEntry; ++currEntry)
        {
        if (0 == currEntry->m_groupNumber[optimalSplit])
            newNode1->AddInternalNode((DRTNodeP) currEntry->m_vp, root);
        else
            newNode2->AddInternalNode((DRTNodeP) currEntry->m_vp, root);
        }

    // now add the newly created node into the parent of this node. If parent is nullptr, we're the root of the tree and we've just added a new level
    // to the tree (so both nodes are new and become children of this node).
    if (nullptr == m_parent)
        {
        AddInternalNode(newNode1, root);
        AddInternalNode(newNode2, root);
        }
    else
        {
        m_parent->AddInternalNode(newNode1, root);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DRTNodeP Tree::InternalNode::ChooseBestNode(FBoxCP pRange, TreeR root)
    {
    DRTNodeP best = nullptr;
    double   bestFit = 0.0;
    bool     isValid = false;

    for (DRTNodeH curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        {
        FBoxCR thisRange = (*curr)->GetRange();
        FBox newRange;

        newRange = thisRange;
        extendRange(newRange, *pRange);
        double newExtent = rangeExtentSquared(newRange);
        if (isValid && (bestFit < newExtent))
            continue;

        // "thisFit" is a somewhat arbitrary measure of how well the range fits into this
        // node, taking into account the total size ("new extent") of this node plus this range,
        // plus a penalty for increasing it from its existing size.
        double thisFit = newExtent + ((newExtent - rangeExtentSquared(thisRange)) * 10.0);

        if (!isValid || (thisFit < bestFit))
            {
            best    = *curr;
            bestFit = thisFit;
            isValid = true;
            }
        }

    BeAssert(nullptr != best);
    return  best;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::InternalNode::AddEntry(Entry const& entry, TreeR root)
    {
    ValidateRange();
    extendRange(m_nodeRange, entry.m_range);
    DRTNodeP node = ChooseBestNode(&entry.m_range, root);

    DRTLeafNodeP leaf = node->ToLeaf();
    if (leaf)
        leaf->AddEntryToLeaf(entry, root);
    else
        {
        ((DRTInternalNodeP) node)->AddEntry(entry, root);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::InternalNode::DropRange(FBoxCR range)
    {
    if (CompletelyContains(range))
        return;

    m_sloppy = true;
    if (m_parent)
        m_parent->DropRange(range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::InternalNode::DropNode(DRTNodeP entry, TreeR root)
    {
    for (DRTNodeH curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        {
        if (*curr != entry)
            continue;

        if (curr+1 < m_endChild)
            memmove(curr, curr+1, (m_endChild - curr) * sizeof(DRTNodeP));

        --m_endChild;

        if (m_firstChild == m_endChild)  // last entry in this leaf?
            {
            if (nullptr == m_parent) // last node in the tree? If so, create an empty LeafNode and delete this InternalNode.
                root.m_root = root.AllocateLeafNode();
            else
                m_parent->DropNode(this, root);

            root.FreeInternalNode(this);
            return;
            }

        DropRange(entry->GetRangeCR());
        return;
        }

    BeAssert(false); // we were asked to drop a child we didn't hold
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::InternalNode::AddInternalNode(DRTNodeP child, TreeR root)
    {
    child->SetParent(this);
    ValidateInternalRange();
    extendRange(m_nodeRange, child->GetRange());

    *m_endChild++ = child;
    if (GetEntryCount() > (root.m_internalNodeSize))
        SplitInternalNode(root);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Tree::InternalNode::GetElementCount()
    {
    DRTLeafNodeP leaf = ToLeaf();
    if (nullptr != leaf)
        return leaf->GetEntryCount();

    size_t count = 0;
    for (DRTNodeH curr = &m_firstChild[0]; curr != m_endChild; ++curr)
        count += ((DRTInternalNodeP)*curr)->GetElementCount();

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
Traverser::Stop Tree::InternalNode::Traverse(Traverser& traverser, bool is3d)
    {
    if (traverser._CheckRangeTreeNode(GetRange(), is3d))
        {
        for (DRTNodeH curr = &m_firstChild[0]; curr < m_endChild; ++curr)
            {
            if (Traverser::Stop::Yes == (*curr)->Traverse(traverser, is3d))
                return Traverser::Stop::Yes;
            }
        }

    return Traverser::Stop::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::LeafNode::AddEntryToLeaf(Entry const& entry, TreeR root)
    {
    ValidateRange();
    extendRange(m_nodeRange, entry.m_range);

    *m_endChild = entry;
    ++m_endChild;

    auto stat = root.m_leafIdx.Insert(entry.m_id, this);
    if (!stat.second)
        stat.first->second = this;   // already was in the map, change its value to this leaf

    if (GetEntryCount() > (root.m_leafNodeSize))
        SplitLeafNode(root);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
EntryCP Tree::LeafNode::FindElement(DgnElementId id) const
    {
    for (EntryCP curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        {
        if (curr->m_id == id)
            return curr;
        }

    BeAssert(false);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tree::LeafNode::DropElement(DgnElementId id, TreeR root)
    {
    for (Entry* curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        {
        if (curr->m_id != id)
            continue;

        FBox range = curr->m_range;
        if (curr+1 < m_endChild)
            memmove(curr, curr+1, (m_endChild - curr) * sizeof(Entry));

        --m_endChild;

        if (m_firstChild == m_endChild)  // last entry in this leaf?
            {
            if (nullptr == m_parent) // last node in the tree?
                {
                ClearChildren();
                return true;
                }

            m_parent->DropNode(this, root);
            root.FreeLeafNode(this);
            return true;
            }

        if (!CompletelyContains(range))
            {
            m_sloppy = true;
            if (m_parent)
                m_parent->DropRange(range);
            }
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::LeafNode::SplitLeafNode(TreeR root)
    {
    size_t  count = GetEntryCount();
    DRTSplitEntryP  splitEntries = (DRTSplitEntryP) _alloca(count * sizeof(DRTSplitEntry));

    DRTSplitEntryP currEntry = splitEntries;
    DRTSplitEntryP endEntry = splitEntries + count;

    for (Entry* curr = &m_firstChild[0]; curr < m_endChild; ++curr, ++currEntry)
        {
        currEntry->m_id = curr->m_id;
        currEntry->m_category = curr->m_category;
        currEntry->m_range = curr->m_range;
        }

    double xSep = checkSeparation(splitEntries, count, X_AXIS);
    double ySep = checkSeparation(splitEntries, count, Y_AXIS);
    double zSep = m_is3d ? checkSeparation(splitEntries, count, Z_AXIS) : 0;

    SplitAxis  optimalSplit = X_AXIS;
    if (ySep > xSep)
        optimalSplit = Y_AXIS;
    if (m_is3d && (zSep > ySep) && (zSep > xSep))
        optimalSplit = Z_AXIS;

    DRTLeafNodeP newNode1 = root.AllocateLeafNode();
    DRTLeafNodeP newNode2 = this;
    newNode2->m_type = newNode1->m_type;

    ClearChildren();    // clear range and child entries - about half of them will come back below.

    for (currEntry = splitEntries; currEntry < endEntry; ++currEntry)
        {
        if (0 == currEntry->m_groupNumber[optimalSplit])
            newNode1->AddEntryToLeaf(Entry(currEntry->m_range, currEntry->m_id, currEntry->m_category), root);
        else
            newNode2->AddEntryToLeaf(Entry(currEntry->m_range, currEntry->m_id, currEntry->m_category), root);
        }

    // if parent is nullptr, this node is currently the root of the tree (the only node in the tree). We need to allocate an InternalNode to
    // become the new root, and add both this node and the new node into it.
    if (nullptr == m_parent)
        {
        DRTInternalNodeP newRoot = root.AllocateInternalNode();
        newRoot->AddInternalNode(newNode1, root);
        newRoot->AddInternalNode(newNode2, root);
        root.m_root = newRoot;
        }
    else
        {
        m_parent->AddInternalNode(newNode1, root);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
Traverser::Stop Tree::LeafNode::Traverse(Traverser& traverser, bool is3d)
    {
    if (traverser._CheckRangeTreeNode(GetRange(), is3d))
        {
        for (Entry* curr = &m_firstChild[0]; curr < m_endChild; ++curr)
            {
            if (Traverser::Stop::Yes == traverser._VisitRangeTreeEntry(*curr))
                return Traverser::Stop::Yes;
            }
        }

    return Traverser::Stop::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
Tree::Tree(bool is3d, size_t leafSize) : m_is3d(is3d)
    {
    m_internalNodeSize = m_leafNodeSize = 0;

    if (0 >= leafSize || leafSize>20)
        leafSize = 20;

    SetNodeSizes(8, leafSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::AddEntry(Entry const& entry)
    {
    if (!rangeIsValid(entry.m_range, m_is3d))
        return;

    if (nullptr == m_root)
        m_root = AllocateLeafNode();
    
    BeAssert(m_leafIdx.find(entry.m_id) == m_leafIdx.end());

    DRTLeafNodeP leaf = m_root->ToLeaf();
    if (leaf)
        leaf->AddEntryToLeaf(entry, *this);
    else
        ((DRTInternalNodeP)m_root)->AddEntry(entry, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Tree::RemoveElement(DgnElementId id)
    {
    if (nullptr == m_root)
        return ERROR;

    auto it = m_leafIdx.find(id);
    if (it == m_leafIdx.end())
        return ERROR;

    bool dropped = it->second->DropElement(id, *this);
    BeAssert(dropped);
    UNUSED_VARIABLE(dropped);
    m_leafIdx.erase(it);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
EntryCP Tree::FindElement(DgnElementId id) const
    {
    auto it = m_leafIdx.find(id);
    return it == m_leafIdx.end() ? nullptr : it->second->FindElement(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::SetNodeSizes(size_t internalNodeSize, size_t leafNodeSize)
    {
    m_internalNodeSize = internalNodeSize;
    m_leafNodeSize = leafNodeSize;

    m_leafNodes.SetEntrySize(sizeof(Tree::LeafNode) + ((int) leafNodeSize*sizeof(Entry)), 1);
    m_internalNodes.SetEntrySize(sizeof(Tree::InternalNode) + ((int) internalNodeSize*sizeof(DRTNodeP)), 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
Traverser::Stop Tree::Traverse(Traverser& traverser)
    {
    return (nullptr == m_root) ? Traverser::Stop::No : m_root->Traverse(traverser, Is3d());
    }
