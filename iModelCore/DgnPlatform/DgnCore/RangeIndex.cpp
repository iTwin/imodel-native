/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

using namespace RangeIndex;

BEGIN_UNNAMED_NAMESPACE

typedef Tree::LeafNode*     LeafNodeP;
typedef Tree::InternalNode* InternalNodeP;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct SplitEntry
{
    FBox m_range;
    DgnElementId m_id; 
    void* m_vp;
    int m_groupNumber[3];
};

typedef SplitEntry* SplitEntryP;
typedef SplitEntry const * SplitEntryCP;
typedef SplitEntry const& SplitEntryCR;

static inline bool compareX(SplitEntryCR entry1, SplitEntryCR entry2) {return entry1.m_range.Low().x < entry2.m_range.Low().x;}
static inline bool compareY(SplitEntryCR entry1, SplitEntryCR entry2) {return entry1.m_range.Low().y < entry2.m_range.Low().y;}
static inline bool compareZ(SplitEntryCR entry1, SplitEntryCR entry2) {return entry1.m_range.Low().z < entry2.m_range.Low().z;}
typedef bool (*PF_CompareFunc)(SplitEntryCR, SplitEntryCR);

enum SplitAxis {X_AXIS=0, Y_AXIS=1, Z_AXIS=2};

/*---------------------------------------------------------------------------------**//**
* @bsimethod    RangeNode                                       KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
static double checkSeparation(SplitEntryP entries, size_t count, SplitAxis axis)
    {
    double maxSeparation = 0, separation;
    double maxMinDist = 1.0e200, minDist;

    static PF_CompareFunc  s_compareFuncs[3] = {compareX, compareY, compareZ};
    std::sort(entries, entries+count, s_compareFuncs[axis]);

    size_t minSize = count/3;
    SplitEntryCP lastEntry    = entries + count;
    SplitEntryCP entryEnd     = entries + (count-minSize);
    SplitEntryCP startEntries = entries + minSize;
    SplitEntryCP sepEntry=nullptr, minDistEntry=nullptr;

    switch (axis)
        {
        case X_AXIS:
            for (SplitEntryCP currEntry = startEntries; currEntry < entryEnd-1; ++currEntry)
                {
                SplitEntryCP nextEntry = currEntry + 1;

                if (currEntry->m_range.High().x < nextEntry->m_range.Low().x)
                    {
                    if ((separation = (nextEntry->m_range.Low().x - currEntry->m_range.High().x)) > maxSeparation)
                        {
                        maxSeparation = separation;
                        sepEntry = nextEntry;
                        }
                    }
                else
                    {
                    if ((minDist = (nextEntry->m_range.Low().x - currEntry->m_range.Low().x)) > maxMinDist)
                        {
                        maxMinDist    = minDist;
                        minDistEntry = nextEntry;
                        }
                    }
                }
            break;

        case Y_AXIS:
            for (SplitEntryCP currEntry = startEntries; currEntry < entryEnd-1; ++currEntry)
                {
                SplitEntryCP nextEntry = currEntry + 1;

                if (currEntry->m_range.High().y < nextEntry->m_range.Low().y)
                    {
                    if ((separation = (nextEntry->m_range.Low().y - currEntry->m_range.High().y)) > maxSeparation)
                        {
                        maxSeparation = separation;
                        sepEntry = nextEntry;
                        }
                    }
                else
                    {
                    if ((minDist = (nextEntry->m_range.Low().y - currEntry->m_range.Low().y)) > maxMinDist)
                        {
                        maxMinDist = minDist;
                        minDistEntry = nextEntry;
                        }
                    }
                }
            break;

        case Z_AXIS:
            for (SplitEntryCP currEntry = startEntries; currEntry < entryEnd-1; ++currEntry)
                {
                SplitEntryCP nextEntry = currEntry + 1;

                if (currEntry->m_range.High().z < nextEntry->m_range.Low().z)
                    {
                    if ((separation = (nextEntry->m_range.Low().z - currEntry->m_range.High().z)) > maxSeparation)
                        {
                        maxSeparation = separation;
                        sepEntry = nextEntry;
                        }
                    }

                else
                    {
                    if ((minDist = (nextEntry->m_range.Low().z - currEntry->m_range.Low().z)) > maxMinDist)
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

    for (SplitEntryP currEntry = entries; currEntry < lastEntry; ++currEntry)
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
Traverser::Stop Tree::Node::Traverse(Traverser& traverser, TreeCR tree, bool is3d)
    {
    LeafNodeP leaf = ToLeaf();
    return leaf ? leaf->Traverse(traverser, tree, is3d) : ((InternalNodeP) this)->Traverse(traverser, tree, is3d);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
inline void Tree::InternalNode::ValidateInternalRange()
    {
    ClearRange();
    for (auto curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        m_nodeRange.Extend((*curr)->GetRange());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
inline void Tree::LeafNode::ValidateLeafRange()
    {
    ClearRange();
    for (Entry* curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        m_nodeRange.Extend(curr->m_range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tree::Node::Overlaps(FBoxCR range) const
    {
    if (m_nodeRange.Low().x > range.High().x || m_nodeRange.High().x < range.Low().x ||
        m_nodeRange.Low().y > range.High().y || m_nodeRange.High().y < range.Low().y)
        return  false;

    return !m_is3d ? true : (m_nodeRange.Low().z <= range.High().z && m_nodeRange.High().z >= range.Low().z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tree::Node::CompletelyContains(FBoxCR range) const
    {
    if (m_nodeRange.Low().x >= range.Low().x || m_nodeRange.High().x <= range.High().x ||
        m_nodeRange.Low().y >= range.Low().y || m_nodeRange.High().y <= range.High().y)
        return  false;

    return !m_is3d ? true : (m_nodeRange.Low().z < range.Low().z && m_nodeRange.High().z > range.High().z);
    }

/*---------------------------------------------------------------------------------**//**
* An InternalNode has become full, split into two nodes (a new one and this one) and determine an optimal division of the current
* enteries between this node and the new one.
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::InternalNode::SplitInternalNode(TreeR root)
    {
    size_t  count = GetEntryCount();
    SplitEntryP  splitEntries = (SplitEntryP) _alloca(count * sizeof(SplitEntry));

    SplitEntryP currEntry = splitEntries;
    SplitEntryP endEntry = splitEntries + count;

    for (auto curr = &m_firstChild[0]; curr < m_endChild; ++curr, ++currEntry)
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
    InternalNode* newNode1 = root.AllocateInternalNode();
    InternalNode* newNode2 = (nullptr == m_parent) ? root.AllocateInternalNode() : this;

    ClearChildren();

    for (currEntry = splitEntries; currEntry < endEntry; ++currEntry)
        {
        if (0 == currEntry->m_groupNumber[optimalSplit])
            newNode1->AddInternalNode((Node*) currEntry->m_vp, root);
        else
            newNode2->AddInternalNode((Node*) currEntry->m_vp, root);
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
Tree::Node* Tree::InternalNode::ChooseBestNode(FBoxCP pRange, TreeR root)
    {
    Tree::Node* best = nullptr;
    double   bestFit = 0.0;
    bool     isValid = false;

    for (auto curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        {
        FBoxCR thisRange = (*curr)->GetRange();
        FBox newRange;

        newRange = thisRange;
        newRange.Extend(*pRange);
        double newExtent = newRange.ExtentSquared();
        if (isValid && (bestFit < newExtent))
            continue;

        // "thisFit" is a somewhat arbitrary measure of how well the range fits into this
        // node, taking into account the total size ("new extent") of this node plus this range,
        // plus a penalty for increasing it from its existing size.
        double thisFit = newExtent + ((newExtent - thisRange.ExtentSquared()) * 10.0);

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
    m_nodeRange.Extend(entry.m_range);
    auto* node = ChooseBestNode(&entry.m_range, root);

    LeafNodeP leaf = node->ToLeaf();
    if (leaf)
        leaf->AddEntryToLeaf(entry, root);
    else
        {
        ((InternalNodeP) node)->AddEntry(entry, root);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::InternalNode::DropRange(FBoxCR range)
    {
    if (CompletelyContains(range))
        return;

    ValidateInternalRange();

    if (m_parent)
        m_parent->DropRange(range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::InternalNode::DropNode(Node* entry, TreeR root)
    {
    for (auto curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        {
        if (*curr != entry)
            continue;

        if (curr+1 < m_endChild)
            memmove(curr, curr+1, (m_endChild - curr) * sizeof(NodeP));

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
void Tree::InternalNode::AddInternalNode(Node* child, TreeR root)
    {
    child->SetParent(this);
    ValidateInternalRange();
    m_nodeRange.Extend(child->GetRange());

    *m_endChild++ = child;
    if (GetEntryCount() > (root.m_internalNodeSize))
        SplitInternalNode(root);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Tree::InternalNode::GetElementCount()
    {
    LeafNodeP leaf = ToLeaf();
    if (nullptr != leaf)
        return leaf->GetEntryCount();

    size_t count = 0;
    for (auto curr = &m_firstChild[0]; curr != m_endChild; ++curr)
        count += ((InternalNodeP)*curr)->GetElementCount();

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
Traverser::Stop Tree::InternalNode::Traverse(Traverser& traverser, TreeCR tree, bool is3d)
    {
    if (Traverser::Accept::Yes == traverser._CheckRangeTreeNode(GetRange(), is3d))
        {
        for (auto curr = &m_firstChild[0]; curr < m_endChild; ++curr)
            {
            if (Traverser::Stop::Yes == (*curr)->Traverse(traverser, tree, is3d))
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
    m_nodeRange.Extend(entry.m_range);

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
            ValidateLeafRange();
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
    SplitEntryP  splitEntries = (SplitEntryP) _alloca(count * sizeof(SplitEntry));

    SplitEntryP currEntry = splitEntries;
    SplitEntryP endEntry = splitEntries + count;

    for (Entry* curr = &m_firstChild[0]; curr < m_endChild; ++curr, ++currEntry)
        {
        currEntry->m_id = curr->m_id;
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

    LeafNodeP newNode1 = root.AllocateLeafNode();
    LeafNodeP newNode2 = this;
    newNode2->m_type = newNode1->m_type;

    ClearChildren();    // clear range and child entries - about half of them will come back below.

    for (currEntry = splitEntries; currEntry < endEntry; ++currEntry)
        {
        if (0 == currEntry->m_groupNumber[optimalSplit])
            newNode1->AddEntryToLeaf(Entry(currEntry->m_range, currEntry->m_id), root);
        else
            newNode2->AddEntryToLeaf(Entry(currEntry->m_range, currEntry->m_id), root);
        }

    // if parent is nullptr, this node is currently the root of the tree (the only node in the tree). We need to allocate an InternalNode to
    // become the new root, and add both this node and the new node into it.
    if (nullptr == m_parent)
        {
        InternalNodeP newRoot = root.AllocateInternalNode();
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
Traverser::Stop Tree::LeafNode::Traverse(Traverser& traverser, TreeCR tree, bool is3d)
    {
    if (Traverser::Accept::Yes == traverser._CheckRangeTreeNode(GetRange(), is3d))
        {
        for (Entry* curr = &m_firstChild[0]; curr < m_endChild; ++curr)
            {
            if (Traverser::Stop::Yes == traverser._VisitRangeTreeEntry(*curr))
                return Traverser::Stop::Yes;

            if (tree.m_writeRequest && traverser._AbortOnWriteRequest())
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
    if (!entry.m_range.IsValid())
        return;

    WriteLock lock(*this);
    if (nullptr == m_root)
        m_root = AllocateLeafNode();

    BeAssert(m_leafIdx.find(entry.m_id) == m_leafIdx.end());

    LeafNodeP leaf = m_root->ToLeaf();
    if (leaf)
        leaf->AddEntryToLeaf(entry, *this);
    else
        ((InternalNodeP)m_root)->AddEntry(entry, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Tree::RemoveElement(DgnElementId id)
    {
    if (nullptr == m_root)
        return ERROR;

    WriteLock lock(*this);

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
    ReadLock lock(*this);
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
    m_internalNodes.SetEntrySize(sizeof(Tree::InternalNode) + ((int) internalNodeSize*sizeof(NodeP)), 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
Traverser::Stop Tree::Traverse(Traverser& traverser)
    {
    ReadLock lock(*this);
    return (nullptr == m_root) ? Traverser::Stop::No : m_root->Traverse(traverser, *this, Is3d());
    }
