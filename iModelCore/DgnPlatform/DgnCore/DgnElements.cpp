/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnElements.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

typedef DgnElementCP* DgnElementH;

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    01/01
+===============+===============+===============+===============+===============+======*/
struct ElemIdRange
{
    uint64_t m_low, m_high;

    void Init(uint64_t low, uint64_t high) {m_low = low; m_high  = high;}
    void Extend(ElemIdRange const& range)
        {
        if (range.m_low < m_low)
            m_low = range.m_low;
        if (range.m_high > m_high)
            m_high = range.m_high;
        }

    void Extend(uint64_t newKey)
        {
        if (newKey < m_low)
            m_low = newKey;
        if (newKey > m_high)
            m_high = newKey;
        }

    bool Contains(uint64_t key) const {return (key >= m_low) &&(key <= m_high);}
    bool Contains(ElemIdRange const& range) const {return (range.m_high <= m_high) && (range.m_low >= m_low);}
};

enum class ElemPurge {Kept=0, Deleted=1};

struct ElemIdRangeNode;
struct ElemIdLeafNode;

typedef struct ElemIdRangeNode* ElemIdRangeNodeP;
typedef ElemIdRangeNode const * ElemIdRangeNodeCP;
typedef ElemIdRangeNodeP*       ElemIdRangeNodeH;
typedef ElemIdRangeNodeP const* ElemIdRangeNodeCH;
typedef bool(*T_NodeSortFunc)(ElemIdRangeNodeP, ElemIdRangeNodeP);

/*=================================================================================**//**
* a node in the tree that has children
* @bsiclass                                                     KeithBentley    01/01
+===============+===============+===============+===============+===============+======*/
struct ElemIdParent
{
    virtual ~ElemIdParent(){}
    virtual void _IncreaseRange(ElemIdRange const&) = 0;
    virtual void _AddChildNode(ElemIdRangeNodeP newNode) = 0;
    virtual ElemIdLeafNode const* _NextSibling(ElemIdRangeNodeCP me) const = 0;
};

/*=================================================================================**//**
* every node in the tree must have this.
* @bsiclass                                                     KeithBentley    12/97
+===============+===============+===============+===============+===============+======*/
struct ElemIdRangeNode
{
protected:
    ElemIdTree&    m_treeRoot;              // pointer to root of tree. Needed for FreeNode
    ElemIdParent*  m_parent;                // pointer to this node's immediate parent. Only necessary for iterators.
    mutable ElemIdRange m_range;            // highest and lowest DgnElementId held in this node
    uint64_t       m_lastUnreferenced;      // the "time" any entry from this node down last became garbage
    int            m_nEntries;              // the total number of entries held in this node
    bool           m_isLeaf;
    mutable bool   m_allReferenced;         // if true, we know all of the entries from this node down hold no garbage elements
    mutable bool   m_isSloppy;              // is m_range accurate? If not, we need to determine it.

public:
    ElemIdRangeNode(ElemIdTree& root, ElemIdParent* parent, bool leaf) : m_treeRoot(root),m_parent(parent),m_isLeaf(leaf) {m_nEntries=0; m_isSloppy=false; m_allReferenced=false; InitRange(); m_lastUnreferenced=0;}
    virtual void _CalculateNodeRange() const = 0;
    virtual void _Add(DgnElementCR entry, uint64_t counter) = 0;
    virtual struct ElemIdLeafNode const* _GetFirstNode() const = 0;
    virtual ElemPurge _Purge(int64_t memTarget) = 0;
    virtual ElemPurge _Drop(uint64_t key) = 0;
    virtual void _Empty() = 0;

    bool ContainsKey(uint64_t key) const {CheckSloppy(); return m_range.Contains(key);}
    void SetParent(ElemIdParent* newParent) {m_parent = newParent;}
    bool IsLeaf() const {return m_isLeaf;}
    ElemIdParent const* GetParent() const {return m_parent;}
    bool IsSloppy() const {return m_isSloppy;}
    void CheckSloppy() const {if (IsSloppy())_CalculateNodeRange();}
    int GetCount() const {return m_nEntries;}
    void InitRange(uint64_t min, uint64_t max) const {m_range.Init(min, max); m_isSloppy = false;}
    void InitRange() const {InitRange(ULLONG_MAX, 0);}
    void GetExactNodeRange(ElemIdRange& range) const {CheckSloppy(); range = m_range;}
    void SetNodeRange(ElemIdRange const& range) const { m_range = range; m_isSloppy = false; }
    void SetLastUnReferenced(uint64_t val) {m_lastUnreferenced=val; m_allReferenced=false;}
    uint64_t GetLastUnReferenced() const {return m_lastUnreferenced;}
    uint64_t GetLowestId() const {return m_range.m_low;}
    bool AreAllReferenced() const {return m_allReferenced;}
    DgnElementCP Find(uint64_t key, bool) const;
};

/*=================================================================================**//**
// Leaf nodes only hold elements
// @bsiclass                                                    Keith.Bentley   09/12
+===============+===============+===============+===============+===============+======*/
struct ElemIdLeafNode : ElemIdRangeNode
{
private:
    DgnElementCP m_elems[NUM_LEAFENTRIES];

    void CalculateLeafRange() const {if (0==m_nEntries) {InitRange(); return;} InitRange(m_elems[0]->GetElementId().GetValue(),(*LastEntry())->GetElementId().GetValue());}

    virtual ElemIdLeafNode const* _GetFirstNode() const {return this;}
    virtual void _CalculateNodeRange() const override {CalculateLeafRange();}
    virtual void _Add(DgnElementCR entry, uint64_t counter) override;
    virtual ElemPurge _Purge(int64_t) override;
    virtual ElemPurge _Drop(uint64_t key) override;
    virtual void _Empty() override;

public:
    DgnElementCP GetEntry(int index) const {return m_elems[index];}
    DgnElementCP const* FirstEntry() const {return m_elems;}
    DgnElementCP const* LastEntry() const {return &m_elems[m_nEntries-1];}
    ElemIdLeafNode(ElemIdTree& root, ElemIdParent* parent) : ElemIdRangeNode(root, parent, true) {}
    void AddEntry(DgnElementCR entry);
    void SplitLeafNode();
    DgnElementCP FindLeaf(uint64_t key, bool) const;
};

/*=================================================================================**//**
* an internal node either holds all Internal nodes, or all LeafNodes.
* @bsiclass                                                     KeithBentley    12/97
+===============+===============+===============+===============+===============+======*/
struct ElemIdInternalNode : public ElemIdRangeNode, ElemIdParent
{
protected:
    ElemIdRangeNodeP m_children[NUM_INTERNALENTRIES];

    virtual void _Add(DgnElementCR entry, uint64_t counter) override {SetLastUnReferenced(counter); ChooseBestNode(entry.GetElementId().GetValue())->_Add(entry, counter);}
    virtual void _IncreaseRange(ElemIdRange const&) override;
    virtual void _CalculateNodeRange() const override;
    virtual ElemIdLeafNode const* _GetFirstNode() const override {return (*FirstEntryC())->_GetFirstNode();}
    virtual ElemIdLeafNode const* _NextSibling(ElemIdRangeNodeCP from) const override;
    virtual ElemPurge _Purge(int64_t) override;
    virtual ElemPurge _Drop(uint64_t key) override;
    virtual void _Empty() override;
    void SortInto(ElemIdRangeNodeP* into, ElemIdRangeNodeP* from, T_NodeSortFunc sortFunc);

public:
    virtual void _AddChildNode(ElemIdRangeNodeP newNode) override;

    ElemIdInternalNode(ElemIdTree& root, ElemIdParent* parent) : ElemIdRangeNode(root, parent, false) {}

    ElemIdRangeNodeCH GetEntryC(int i) const {return m_children+i;}
    ElemIdRangeNodeCH FirstEntryC() const {return GetEntryC(0);}
    ElemIdRangeNodeCH LastEntryC() const {return GetEntryC(m_nEntries-1);}
    ElemIdRangeNodeH GetEntry(int i) {return m_children+i;}
    ElemIdRangeNodeH FirstEntry() {return GetEntry(0);}
    ElemIdRangeNodeH LastEntry() {return GetEntry(m_nEntries-1);}
    ElemIdRangeNodeP ChooseBestNode(uint64_t key);
    bool Contains(ElemIdRangeNodeP child);
    void SplitInternalNode();
    DgnElementCP FindInternal(uint64_t key, bool) const;
};

struct MyStats : DgnElements::Statistics
{
    void Reset() {m_newElements=m_unReferenced=m_reReferenced=m_purged = 0 ;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
inline DgnElementCP ElemIdRangeNode::Find(uint64_t key, bool setFreeEntryFlag) const
   {
   return m_isLeaf ?((ElemIdLeafNode const*) this)->FindLeaf(key,setFreeEntryFlag) : ((ElemIdInternalNode const*) this)->FindInternal(key,setFreeEntryFlag);
   }

//=======================================================================================
// A tree of elements, sorted by DgnElementId. Elements are held in the tree, even if their refCount goes to 0 so they can be reclaimed
// if they are needed again. Unreferenced elements are deleted when a "Purge" operation is performed, attempting to preferentially keep the most
// recently released elements.
// @bsiclass                                                    Keith.Bentley   09/12
//=======================================================================================
struct ElemIdTree : public ElemIdParent
{
    FixedSizePool1     m_leafPool;          // pool for allocating leaf nodes
    FixedSizePool1     m_internalPool;      // pool for allocating internal nodes
    ElemIdRangeNodeP   m_root;              // the root node. Starts as a leaf node for trees with less than 50 entries
    DgnDbR             m_dgndb;
    uint64_t           m_counter;           // always increasing, used to tell least recently accessed for garbage collection
    MyStats            m_stats;
    DgnElements::Totals m_totals;

    virtual void _IncreaseRange(ElemIdRange const&) override {}
    virtual void _AddChildNode(ElemIdRangeNodeP newNode) override;
    virtual ElemIdLeafNode const* _NextSibling(ElemIdRangeNodeCP curr) const override {return nullptr;}

    ElemIdInternalNode* NewInternalNode(ElemIdParent* parent) {return new((ElemIdInternalNode*) m_internalPool.malloc()) ElemIdInternalNode(*this, parent);}
    ElemIdLeafNode* NewLeafNode(ElemIdParent* parent)         {return new((ElemIdLeafNode*) m_leafPool.malloc()) ElemIdLeafNode(*this, parent);}
    void FreeNode(ElemIdRangeNodeP child, bool leaf);

public:
    ElemIdTree(DgnDbR project) : m_dgndb(project)
        {
        m_totals.m_entries        = 0;
        m_totals.m_unreferenced   = 0;
        m_totals.m_allocedBytes   = 0;
        m_totals.m_extant         = 0;
        m_counter           = 0;
        m_leafPool.SetSize(sizeof(ElemIdLeafNode), 4);
        m_internalPool.SetSize(sizeof(ElemIdInternalNode), 4);
        m_root  = nullptr;
        m_stats.Reset();
        }

    ~ElemIdTree() {Destroy();}

    bool ContainsKey(DgnElementId key) {return (nullptr != FindElement(key, false));}
    ElemIdRangeNodeP GetRoot() {return m_root;}
    DgnElementP FindElement(DgnElementId key, bool setAccessed);
    void AddElement(DgnElementCR);
    void DropElement(DgnElementCR);
    void KillElement(DgnElementCR el) {BeAssert(0 == el.GetRefCount()); RemoveElement(el); delete &el;}
    void RemoveElement(DgnElementCR element);
    void Purge(int64_t memTarget);
    void Destroy();
};
END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
ElemPurge ElemIdLeafNode::_Drop(uint64_t key)
    {
    for (int begin=0, end=m_nEntries; begin < end;)
        {
        int index = begin +(end - begin - 1)/2;
        uint64_t thisId = m_elems[index]->GetElementId().GetValue();
        if (key < thisId)
            end = index;
        else if (thisId < key)
            begin = ++index;
        else
            {
            //  this is the entry to delete
            DgnElementCP target = m_elems[index]; // save the target element
            m_isSloppy = true; // we can't tell whether we may have dropped the first or last entry.

            m_nEntries -= 1;
            memmove(m_elems + index, m_elems + index + 1,(m_nEntries-index) * sizeof(DgnElementP));

            // mark it as not in pool and adjust pool stats
            m_treeRoot.RemoveElement(*target);
            break;
            }
        }

    return (0 == m_nEntries) ? ElemPurge::Deleted : ElemPurge::Kept;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ElemPurge ElemIdLeafNode::_Purge(int64_t memTarget)
    {
    if (m_allReferenced || m_treeRoot.m_totals.m_allocedBytes < memTarget || 0 == m_treeRoot.m_totals.m_unreferenced)
        return ElemPurge::Kept;

    DgnElementH curr = m_elems;
    DgnElementH used = curr;
    DgnElementH end = m_elems + m_nEntries;

    unsigned killedIndex = 0;
    DgnElementCP killed[NUM_LEAFENTRIES];

    for (;curr < end; ++curr)
        {
        if (0 ==(*curr)->GetRefCount()) // is the element garbage?
            {
            //  Do not kill the element here.  If the element's app data holds a reference to another
            //  element -- possibly a symbol element -- killing the element here may cause the reference
            //  count of that element to go to zero. We don't want that to happen until the tree is in a
            //  consistent state.
            killed[killedIndex++] = *curr;
            BeAssert(killedIndex <= NUM_LEAFENTRIES);
            }
        else
            *(used++) = *curr;
        }

    m_isSloppy = true;           // we can't tell whether we may have dropped the first or last entry.
    m_allReferenced = true;      // since we know we've eliminated any garbage entries, mark this node as "all referenced"
    m_nEntries =(int)(used - m_elems);

    // this call deletes the element data, and all its AppData (e.g. XAttributes). It also keeps the total element/bytes count up to date.
    for (unsigned i = 0; i < killedIndex; i++)
        m_treeRoot.KillElement(*killed[i]);

    return (0 == m_nEntries) ? ElemPurge::Deleted : ElemPurge::Kept;
    }

/*---------------------------------------------------------------------------------**//**
* copy entries from one arry into another and sort via a function
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdInternalNode::SortInto(ElemIdRangeNodeP* into, ElemIdRangeNodeP* from, T_NodeSortFunc sortFunc)
    {
    memcpy(into, from, m_nEntries*sizeof(ElemIdRangeNodeP));
    std::sort(into, into+m_nEntries, sortFunc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool sortByAccessTime(ElemIdRangeNodeP n1, ElemIdRangeNodeP n2) {return n1->GetLastUnReferenced() < n2->GetLastUnReferenced();}
static bool sortById(ElemIdRangeNodeP n1, ElemIdRangeNodeP n2) {return n1->GetLowestId() < n2->GetLowestId();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
ElemPurge ElemIdInternalNode::_Drop(uint64_t key)
    {
    for (unsigned index = 0; index <(unsigned)m_nEntries; ++index)
        {
        ElemIdRangeNodeP node = m_children[index];
        ElemIdRange  currRange;
        node->GetExactNodeRange(currRange);

        if (key >= currRange.m_low && key <= currRange.m_high)
            {
            if (ElemPurge::Deleted != node->_Drop(key))
                {
                //  Should we compute m_allReferenced -- maybe call AreAllRefenced
                return ElemPurge::Kept;
                }

            m_nEntries -= 1;
            memmove(m_children + index, m_children + index + 1,(m_nEntries-index) * sizeof(ElemIdRangeNodeP));
            m_treeRoot.FreeNode(node, node->IsLeaf());    // child was deleted

            return (0 == m_nEntries) ? ElemPurge::Deleted : ElemPurge::Kept;
            }
        }

    BeAssert(false);
    return ElemPurge::Kept;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ElemPurge ElemIdInternalNode::_Purge(int64_t memTarget)
    {
    if (m_allReferenced || m_treeRoot.m_totals.m_allocedBytes < memTarget || 0 == m_treeRoot.m_totals.m_unreferenced)
        return ElemPurge::Kept;

    // We want to attempt to purge garbage, starting with the least-recently-accessed nodes.
    // Copy the list into a new array and sort by lastaccess time.
    ElemIdRangeNodeP purgeOrder[NUM_INTERNALENTRIES];
    SortInto(purgeOrder, m_children, sortByAccessTime);

    ElemIdRangeNodeH curr = purgeOrder;
    ElemIdRangeNodeH used = purgeOrder; // used to squeeze out deleted nodes.
    ElemIdRangeNodeH end = curr + m_nEntries;

    m_allReferenced = true;  // start out marking this node as completely filled. If we hit any children with garbage, we turn this off.
    unsigned nPurgeList = 0;
    ElemIdRangeNodeP purgeList[NUM_INTERNALENTRIES];
    for (;curr < end; ++curr)
        {
        ElemIdRangeNodeP node = *curr;
        if (ElemPurge::Deleted == node->_Purge(memTarget))
            //  Do not free the node here. Subsequent iterations may kill an element that holds a reference
            //  to another element and that may trigger logic that searches the tree.  The nodes can't be freed until
            //  we are done purging.
            purgeList[nPurgeList++] = node;
        else
            {
            if (!node->AreAllReferenced())
                m_allReferenced = false;  // if any node below is isn't completely full, we're not either

            *(used++) = node;
            }
        }

    m_isSloppy = true;

    // we've now potentially dropped entries in this node. See what happened
    int nLeft =(int)(used - purgeOrder);
    if (m_nEntries == nLeft)
        return ElemPurge::Kept;      // we didn't drop any, we're done

    // we dropped some but not all entries. Get the remaining ones from the "purgeOrder" array and put them back into child array,
    // sorted by their DgnElementId range
    m_nEntries = nLeft; // make sure this happens before sort!
    if (0 != nLeft)
        SortInto(m_children, purgeOrder, sortById);

    for (unsigned purgeIndex = 0; purgeIndex < nPurgeList; ++purgeIndex)
        m_treeRoot.FreeNode(purgeList[purgeIndex], purgeList[purgeIndex]->IsLeaf());    // child was deleted, we free and and don't copy into "used"

    return 0==nLeft ? ElemPurge::Deleted : ElemPurge::Kept;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdTree::DropElement(DgnElementCR element)
    {
    if (nullptr == m_root ||(ElemPurge::Deleted != m_root->_Drop(element.GetElementId().GetValue())))
        return;

    // tree is now empty
    FreeNode(m_root, m_root->IsLeaf());
    m_root = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* remove garbage (refCount==0) elements from the tree, until we have at most "memTarget" bytes used by this pool.
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdTree::Purge(int64_t memTarget)
    {
    if (memTarget < 0)
        memTarget = 0;

    if (nullptr == m_root ||(ElemPurge::Deleted != m_root->_Purge(memTarget)))
        return;

    // all of the elements were garbage!
    FreeNode(m_root, m_root->IsLeaf());
    m_root = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* add an element to this node, sorted by DgnElementId
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdLeafNode::AddEntry(DgnElementCR entry)
    {
    int index = m_nEntries;
    uint64_t key = entry.GetElementId().GetValue();

    if (key < m_range.m_high)
        {
        // binary search for position to put new entry
        for (int begin=0, end=m_nEntries; begin < end;)
            {
            index = begin +(end - begin - 1)/2;
            uint64_t thisId = m_elems[index]->GetElementId().GetValue();
            if (key < thisId)
                end = index;
            else if (thisId < key)
                begin = ++index;
            else
                {
                BeAssert(false); // duplicate keys!
                break;
                }
            }

        DgnElementCP const*  tEntry = m_elems + index;
        memmove((void*)(tEntry+1), tEntry,(m_nEntries-index) * sizeof(DgnElementP));
        }

    m_elems[index] = &entry;
    m_nEntries++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdLeafNode::_Add(DgnElementCR entry, uint64_t counter)
    {
    // since elements are always unreferenced when we add them to the tree, this correctly marks this node as not "allUnreferenced"
    SetLastUnReferenced(counter);

    if (m_isSloppy)
        CalculateLeafRange();

    AddEntry(entry);

    uint64_t key;
    if (!m_range.Contains(key = entry.GetElementId().GetValue()))
        {
        m_range.Extend(key);
        m_parent->_IncreaseRange(m_range);
        }

    if (NUM_LEAFENTRIES == GetCount())
        SplitLeafNode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP ElemIdLeafNode::FindLeaf(uint64_t key, bool setFreeEntryFlag) const
    {
    if (setFreeEntryFlag) // this means that the last reference to an element in this node was just released.
        m_allReferenced = false;

    for (int begin=0, end=m_nEntries; begin < end;)
        {
        int index = begin +(end - begin - 1)/2;
        uint64_t thisId = m_elems[index]->GetElementId().GetValue();
        if (key < thisId)
            end = index;
        else if (thisId < key)
            begin = index + 1;
        else
            return m_elems[index];
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdLeafNode::SplitLeafNode()
    {
    if (nullptr == m_parent)
        return;

    ElemIdLeafNode*   newNode = m_treeRoot.NewLeafNode(m_parent);
    newNode->m_lastUnreferenced = m_lastUnreferenced;

    int numEntries  = GetCount();
    int start       =(numEntries -(numEntries / 5)) + 1;

    for (int i = start; i < numEntries; i++)
        newNode->AddEntry(*GetEntry(i));

    m_nEntries = start;
    m_allReferenced = false;
    CalculateLeafRange();

    newNode->_CalculateNodeRange();
    m_parent->_AddChildNode(newNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ElemIdRangeNodeP ElemIdInternalNode::ChooseBestNode(uint64_t key)
    {
    if (m_isSloppy)
        _CalculateNodeRange();

    if (key >= m_range.m_high)
        return *LastEntry();

    if (key <= m_range.m_low)
        return *FirstEntry();

    ElemIdRangeNodeP lastNode = nullptr;
    uint64_t    distance, lastDist = ULLONG_MAX;

    for (ElemIdRangeNodeH curr = FirstEntry(), last = LastEntry(); curr <= last ; ++curr)
        {
        ElemIdRange  thisRange;
        (*curr)->GetExactNodeRange(thisRange);

        if (key >= thisRange.m_low)
            {
            if (key <= thisRange.m_high)
                return *curr;

            lastDist =(key - thisRange.m_high);
            lastNode = *curr;
            }
        else
            {
            distance =(thisRange.m_low - key);
            return (distance < lastDist) ? *curr : lastNode;
            }
        }

    return lastNode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP ElemIdInternalNode::FindInternal(uint64_t key, bool setFreeEntryFlag) const
    {
    if (setFreeEntryFlag) // this means that the last reference to an element in this node was just released.
        m_allReferenced = false;

    for (int begin=0, end=m_nEntries; begin < end;)
        {
        int index = begin +(end - begin - 1)/2;

        ElemIdRange  thisRange;
        m_children[index]->GetExactNodeRange(thisRange);

        if (key < thisRange.m_low)
            end = index;
        else if (thisRange.m_high < key)
            begin = index + 1;
        else
            return m_children[index]->Find(key, setFreeEntryFlag);
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElemIdInternalNode::Contains(ElemIdRangeNodeP child)
    {
    // this isn't sorted by child address, can't use binary search
    for (ElemIdRangeNodeH curr = FirstEntry(), last = LastEntry(); curr <= last ; ++curr)
        {
        if (*curr == child)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdInternalNode::_AddChildNode(ElemIdRangeNodeP newNode)
    {
    if (Contains(newNode))
        return;

    newNode->SetParent(this);

    ElemIdRange   range;
    newNode->GetExactNodeRange(range);
    m_range.Extend(range);

    int index = 0;
    for (; index <m_nEntries; index++)
        {
        ElemIdRange   thisRange;
        m_children[index]->GetExactNodeRange(thisRange);

        if (range.m_high <= thisRange.m_low)
            {
            memmove((void*) &m_children[index+1], &m_children[index],(m_nEntries-index) * sizeof(ElemIdRangeNodeP));
            break;
            }
        }

    m_children[index] = newNode;
    if (NUM_INTERNALENTRIES == ++m_nEntries)
        SplitInternalNode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdInternalNode::_IncreaseRange(ElemIdRange const& range)
    {
    if (m_isSloppy)
        _CalculateNodeRange();
    else
        {
        if (m_range.Contains(range))
            return;

        m_range.Extend(range);
        }

    m_parent->_IncreaseRange(m_range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdInternalNode::SplitInternalNode()
    {
    if (nullptr == m_parent)
        return;

    ElemIdInternalNode*  newNode = m_treeRoot.NewInternalNode(m_parent);
    int                 nEntries = GetCount();
    int                 start =(nEntries -(nEntries/4)) + 1;

    newNode->m_lastUnreferenced = m_lastUnreferenced;

    for (int i = start; i < nEntries; i++)
        newNode->_AddChildNode(m_children[i]);

    m_nEntries = start;
    _CalculateNodeRange();

    m_parent->_AddChildNode(newNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdInternalNode::_CalculateNodeRange() const
    {
    InitRange();

    ElemIdRange currRange;
    for (ElemIdRangeNodeCH curr = FirstEntryC(), last = LastEntryC(); curr <= last ; ++curr)
        {
        (*curr)->GetExactNodeRange(currRange);
        m_range.Extend(currRange);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ElemIdLeafNode const* ElemIdInternalNode::_NextSibling(ElemIdRangeNodeCP from) const
    {
    for (ElemIdRangeNodeCH curr = FirstEntryC(), last = LastEntryC(); curr <= last ; ++curr)
        {
        if (from == *curr)
            {
            if (++curr > last)
                {
                ElemIdRangeNodeCP mySibling = m_parent->_NextSibling(this);
                return (nullptr == mySibling) ? nullptr : mySibling->_GetFirstNode();
                }

            return (*curr)->_GetFirstNode();
            }
        }

    BeAssert(false);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdTree::AddElement(DgnElementCR element)
    {
    if (nullptr == m_root)
        m_root = NewLeafNode(this);

    ++m_totals.m_entries;
    if (0 == element.GetRefCount())
       ++m_totals.m_unreferenced;

    BeAssert(m_totals.m_entries >= m_totals.m_unreferenced);
    m_root->_Add(element, ++m_counter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdTree::RemoveElement(DgnElementCR element)
    {
    BeAssert(0 != m_totals.m_entries);
    --m_totals.m_entries;

    if (0 == element.GetRefCount())
        {
        BeAssert(0 != m_totals.m_unreferenced);
        --m_totals.m_unreferenced;
        }

    BeAssert(m_totals.m_entries >= m_totals.m_unreferenced);
    ++m_stats.m_purged;

    m_dgndb.Elements().ChangeMemoryUsed(0 - (int32_t) element._GetMemSize());
    element.SetPersistent(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementP ElemIdTree::FindElement(DgnElementId key, bool setFreeFlag)
    {
    if ((nullptr == m_root) || !m_root->ContainsKey(key.GetValueUnchecked()))
        return nullptr;

    return const_cast<DgnElementP>(m_root->Find(key.GetValueUnchecked(), setFreeFlag));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdTree::FreeNode(ElemIdRangeNodeP child, bool leaf)
    {
    if (leaf)
        m_leafPool.free(child);
    else
        m_internalPool.free(child);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdTree::_AddChildNode(ElemIdRangeNodeP newNode)
    {
    ElemIdRange          range;
    ElemIdRangeNodeP     currentRoot = m_root;
    ElemIdInternalNode*  newRoot;

    newRoot = new((ElemIdInternalNode*) m_internalPool.malloc()) ElemIdInternalNode(*this, this);
    newRoot->_AddChildNode(newNode);

    newNode->GetExactNodeRange(range);
    newRoot->SetNodeRange(range);

    if (nullptr != currentRoot)
        newRoot->_AddChildNode(currentRoot);

    m_root = newRoot;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdLeafNode::_Empty()
    {
    DgnElementH end = m_elems + m_nEntries;

    for (DgnElementH curr = m_elems; curr < end; ++curr)
        m_treeRoot.KillElement(**curr);

    m_nEntries = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdInternalNode::_Empty()
    {
    ElemIdRangeNodeH end = m_children + m_nEntries;

    for (ElemIdRangeNodeH curr = m_children; curr < end; ++curr)
        (*curr)->_Empty();

    m_nEntries = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdTree::Destroy()
    {
    if (m_root)
        m_root->_Empty(); // this frees all of the elements and their data

    BeAssert(0==m_totals.m_extant);       // make sure nobody has any DgnElements around from this DgnDb
    BeAssert(0==m_totals.m_allocedBytes); // we should have returned all the memory too.
    BeAssert(0==m_totals.m_entries);
    BeAssert(0==m_totals.m_unreferenced);

    m_root = nullptr;
    m_leafPool.purge_memory_and_reinitialize();
    m_internalPool.purge_memory_and_reinitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElements::~DgnElements() 
    {
    Destroy(); 
    DELETE_AND_CLEAR(m_tree);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::Destroy()
    {
    BeDbMutexHolder _v_v(m_mutex);
    m_tree->Destroy();
    m_heapZone.EmptyAll();
    m_stmts.Empty();
    m_handlerStmts.Empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::ChangeMemoryUsed(int32_t delta) const
    {
    if (0==delta) // nothing happened, don't bother to get mutex
        return;

    BeDbMutexHolder _v(m_mutex);
    m_tree->m_totals.m_allocedBytes += delta;
    BeAssert(m_tree->m_totals.m_allocedBytes >= 0);
    }

/*---------------------------------------------------------------------------------**//**
* an element that was previously garbage was just referenced
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::OnReclaimed(DgnElementCR el)
    {
    BeDbMutexHolder _v_v(m_mutex);
    BeAssert(0 != m_tree->m_totals.m_unreferenced);
    BeAssert(m_tree->m_totals.m_entries >= m_tree->m_totals.m_unreferenced);
    --m_tree->m_totals.m_unreferenced;
    ++m_tree->m_stats.m_reReferenced;
    }

/*---------------------------------------------------------------------------------**//**
* an element that was previously referenced just became garbage
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::OnUnreferenced(DgnElementCR el)
    {
    BeDbMutexHolder _v_v(m_mutex);
    m_tree->FindElement(el.GetElementId(), true);   // mark this entry as having purgable elements
    ++m_tree->m_totals.m_unreferenced;
    ++m_tree->m_stats.m_unReferenced;
    BeAssert(m_tree->m_totals.m_entries >= m_tree->m_totals.m_unreferenced);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::AddToPool(DgnElementCR element) const
    {
    BeDbMutexHolder _v_v(m_mutex);
    BeAssert(!element.IsPersistent());
    element.SetPersistent(true);

    ChangeMemoryUsed(element._GetMemSize());

    m_tree->AddElement(element);
    ++m_tree->m_stats.m_newElements;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::DropFromPool(DgnElementCR element) const
    {
    if (0 == element.GetRefCount())
        {
        BeAssert(false); // somebody else must own it or we cannot drop it from the pool
        return;
        }

    BeDbMutexHolder _v_v(m_mutex);
    m_tree->DropElement(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t DgnElements::_Purge(int64_t memTarget)
    {
    BeDbMutexHolder _v_v(m_mutex);
    m_tree->Purge(memTarget);
    return GetTotalAllocated();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP DgnElements::FindElement(DgnElementId id) const
    {
    BeDbMutexHolder _v_v(m_mutex);
    return m_tree->FindElement(id, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr DgnElements::GetStatement(Utf8CP sql) const
    {
    CachedStatementPtr stmt;
    m_stmts.GetPreparedStatement(stmt, *m_dgndb.GetDbFile(), sql);
    return stmt;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::DgnElement(CreateParams const& params) : m_refCount(0), m_elementId(params.m_id), m_dgndb(params.m_dgndb), m_modelId(params.m_modelId), m_classId(params.m_classId),
    m_code(params.m_code), m_parentId(params.m_parentId)
    {
    ++GetDgnDb().Elements().m_tree->m_totals.m_extant;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::~DgnElement()
    {
    BeAssert(!IsPersistent());
    ClearAllAppData();
    
    --GetDgnDb().Elements().m_tree->m_totals.m_extant;
    }

DgnElements::Totals DgnElements::GetTotals() const {return m_tree->m_totals;}
DgnElements::Statistics DgnElements::GetStatistics() const {return m_tree->m_stats;}
void DgnElements::ResetStatistics() {m_tree->m_stats.Reset();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElements::DgnElements(DgnDbR dgndb) : DgnDbTable(dgndb), m_heapZone(0, false), m_mutex(BeDbMutex::MutexType::Recursive), m_stmts(20)
    {
    m_tree = new ElemIdTree(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnReversedDelete(BeSQLite::Changes::Change const& change)
    {
    DgnElementId elementId = DgnElementId(change.GetValue(0, Changes::Change::Stage::New).GetValueUInt64());

    // We need to load this element, since filled models need to register it 
    DgnElementCPtr el = m_txnMgr.GetDgnDb().Elements().GetElement(elementId);
    BeAssert(el.IsValid());
    el->_OnReversedDelete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnReversedAdd(BeSQLite::Changes::Change const& change)
    {
    DgnElementId elementId = DgnElementId(change.GetValue(0, Changes::Change::Stage::Old).GetValueUInt64());

    // see if we have this element in memory, if so call its _OnDelete method.
    DgnElementPtr el = (DgnElementP) m_txnMgr.GetDgnDb().Elements().FindElement(elementId);
    if (el.IsValid()) 
        el->_OnReversedAdd(); // Note: this MUST be a DgnElementPtr, since we can't call _OnReversedAdd with an element with a zero ref count
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnReversedUpdate(BeSQLite::Changes::Change const& change) 
    {
    auto& elements = m_txnMgr.GetDgnDb().Elements();
    DgnElementId elementId = DgnElementId(change.GetValue(0, Changes::Change::Stage::Old).GetValueUInt64());
    DgnElementP el = (DgnElementP) elements.FindElement(elementId);
    if (el)
        {
        DgnElementCPtr postModified = elements.LoadElement(el->GetElementId(), false);
        BeAssert(postModified.IsValid());
        postModified->_OnReversedUpdate(*el);

        elements.FinishUpdate(*postModified.get(), *el);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::LoadElement(DgnElement::CreateParams const& params, DgnCategoryId categoryId, bool makePersistent) const
    {
    ElementHandlerP elHandler = dgn_ElementHandler::Element::FindHandler(m_dgndb, params.m_classId);
    if (nullptr == elHandler)
        {
        BeAssert(false);
        return nullptr;
        }

    DgnElementPtr el = elHandler->Create(params);
    if (!el.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    // We do this here to avoid having to do another (ECSql) SELECT statement solely to retrieve the CategoryId from the row we just selected...
    auto geomEl = categoryId.IsValid() ? el->ToGeometrySourceP() : nullptr;
    if (nullptr != geomEl)
        geomEl->PLEASE_DELETE_ME(categoryId);

    if (DgnDbStatus::Success != el->_LoadFromDb())
        return nullptr;

    if (makePersistent)
        {
        if (nullptr != geomEl && m_selectionSet.Contains(el->GetElementId()))
            geomEl->SetInSelectionSet(true);

        el->GetModel()->_OnLoadedElement(*el);
        AddToPool(*el);
        }

    return el;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::LoadElement(DgnElementId elementId, bool makePersistent) const
    {
    enum Column : int       {ClassId=0,ModelId=1,Code=2,ParentId=3,CodeAuthorityId=4,CodeNameSpace=5,CategoryId=6};
    CachedStatementPtr stmt = GetStatement("SELECT ECClassId,ModelId,Code,ParentId,CodeAuthorityId,CodeNameSpace,CategoryId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");
    stmt->BindId(1, elementId);

    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        return nullptr;

    DgnElement::Code code(stmt->GetValueId<DgnAuthorityId>(Column::CodeAuthorityId), stmt->GetValueText(Column::Code), stmt->GetValueText(Column::CodeNameSpace));

    return LoadElement(DgnElement::CreateParams(m_dgndb, stmt->GetValueId<DgnModelId>(Column::ModelId), 
                    stmt->GetValueId<DgnClassId>(Column::ClassId), 
                    code,
                    elementId, 
                    stmt->GetValueId<DgnElementId>(Column::ParentId)),
                    stmt->GetValueId<DgnCategoryId>(Column::CategoryId),
                    makePersistent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::GetElement(DgnElementId elementId) const
    {
    if (!elementId.IsValid())
        return nullptr;

    // since we can load elements on more than one thread, we need to check that the element doesn't already exist
    // *with the lock held* before we load it. This avoids a race condition where an element is loaded on more than one thread.
    BeDbMutexHolder _v(m_mutex);
    DgnElementCP element = FindElement(elementId);
    return (nullptr != element) ? element : LoadElement(elementId, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnElementKey DgnElements::QueryElementKey(DgnElementId elementId) const
    {
    CachedStatementPtr stmt =GetStatement("SELECT ECClassId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");
    stmt->BindInt64(1, elementId.GetValueUnchecked());
    return BE_SQLITE_ROW == stmt->Step() ? DgnElementKey(stmt->GetValueId<DgnClassId>(0), elementId) : DgnElementKey();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::InitNextId()
    {
    if (!m_nextAvailableId.IsValid())
        m_nextAvailableId = DgnElementId(m_dgndb, DGN_TABLE(DGN_CLASSNAME_Element), "Id");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::PerformInsert(DgnElementR element, DgnDbStatus& stat)
    {
    InitNextId();
    m_nextAvailableId.UseNext(m_dgndb);

    element.m_elementId = m_nextAvailableId; 

    if (DgnDbStatus::Success != (stat = element._OnInsert()))
        return nullptr;

    // ask parent whether its ok to add this child.
    DgnElementCPtr parent = GetElement(element.m_parentId);
    if (parent.IsValid() && DgnDbStatus::Success != (stat=parent->_OnChildInsert(element)))
        return nullptr;

    ECClassCP elementClass = element.GetElementClass();
    if (nullptr == elementClass)
        {
        BeAssert(false);
        stat = DgnDbStatus::BadElement;
        return nullptr;
        }

    if (DgnDbStatus::Success != (stat = element._InsertInDb()))
        return nullptr;

    DgnElementPtr newElement = element.CopyForEdit();
    AddToPool(*newElement);

    newElement->_OnInserted(&element);

    if (parent.IsValid())
        parent->_OnChildInserted(*newElement);

    return newElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::InsertElement(DgnElementR element, DgnDbStatus* outStat)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat,outStat);

    // don't allow elements that already have an id.
    if (element.m_elementId.IsValid()) 
        {
        stat = DgnDbStatus::WrongElement; // this element must already be persistent
        return nullptr;
        }

    if (&element.GetDgnDb() != &m_dgndb)
        {
        stat = DgnDbStatus::WrongDgnDb; // attempting to add an element from a different DgnDb
        return nullptr;
        }

    if (!element.GetModel().IsValid())      
        {
        stat = DgnDbStatus::BadModel; // they gave us an element with an invalid ModelId
        return nullptr;
        }

    if (typeid(element) != element.GetElementHandler()._ElementType())
        {
        stat = DgnDbStatus::WrongHandler; // they gave us an element with an invalid handler
        return nullptr;
        }

    DgnElementCPtr newEl = PerformInsert(element, stat);
    if (!newEl.IsValid())
        element.m_elementId = DgnElementId(); // Insert failed, make sure to invalidate the DgnElementId so they don't accidentally use it

    return newEl;
    }

/*---------------------------------------------------------------------------------**//**
* this method is called both after we've directly updated an element, and after we've reversed an update to an element.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::FinishUpdate(DgnElementCR replacement, DgnElementCR original)
    {
    uint32_t oldSize = original._GetMemSize(); // save current size
    (*const_cast<DgnElementP>(&original))._CopyFrom(replacement);    // copy new data into original element
    ChangeMemoryUsed(original._GetMemSize() - oldSize); // report size change
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::UpdateElement(DgnElementR replacement, DgnDbStatus* outStat)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat,outStat);

    if (replacement.IsPersistent())
        {
        stat =  DgnDbStatus::WrongElement;
        return nullptr;
        }

    // Get the original element so we can copy the new data into it.
    DgnElementCPtr orig = GetElement(replacement.GetElementId());
    if (!orig.IsValid())
        {
        stat = DgnDbStatus::InvalidId;
        return nullptr;
        }

    DgnElementR element = const_cast<DgnElementR>(*orig.get());
    if (&element.GetDgnDb() != &replacement.GetDgnDb())
        {
        stat = DgnDbStatus::WrongDgnDb;
        return nullptr;
        }

    if (DgnDbStatus::Success != (stat=replacement._OnUpdate(element)))
        return nullptr; // something rejected proposed change

    // ask parent whether its ok to update his child.
    auto parent = GetElement(element.m_parentId);
    if (parent.IsValid() && DgnDbStatus::Success != (stat = parent->_OnChildUpdate(element, replacement)))
        return nullptr;

    stat = replacement._UpdateInDb();   // perform the actual update in the database
    if (DgnDbStatus::Success != stat)
        return nullptr;

    replacement._OnUpdated(element);
    FinishUpdate(replacement, element);

    if (element.m_parentId != replacement.m_parentId) // did parent change?
        parent = GetElement(replacement.m_parentId);

    if (parent.IsValid())
        parent->_OnChildUpdated(element);

    return &element;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElements::PerformDelete(DgnElementCR element)
    {
    // delete children, if any.
    DgnElementIdSet children = element.QueryChildren();
    for (auto childId : children)
        {
        auto child = GetElement(childId);
        if (!child.IsValid())
            continue;

        auto stat = PerformDelete(*child);
        if (DgnDbStatus::Success != stat)
            return stat;
        }

    auto stat = element._DeleteInDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    element._OnDeleted();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElements::Delete(DgnElementCR element)
    {
    if (&element.GetDgnDb() != &m_dgndb || !element.IsPersistent())
        return DgnDbStatus::WrongElement;

    // Get the next available id now, before we perform any deletes, so if we delete and then undo the last element we don't reuse its id.
    // Otherwise, we may mistake a new element as being a modification to a deleted element in a changeset.
    InitNextId(); 

    DgnDbStatus stat = element._OnDelete();
    if (DgnDbStatus::Success != stat)
        return stat;

    // ask parent whether its ok to delete his child.
    auto parent = GetElement(element.m_parentId);
    if (parent.IsValid() && DgnDbStatus::Success != (stat=parent->_OnChildDelete(element)))
        return stat;

    stat = PerformDelete(element);
    if (DgnDbStatus::Success != stat)
        return stat;

    if (parent.IsValid())
        parent->_OnChildDeleted(element);

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2015
//---------------------------------------------------------------------------------------
DgnModelId DgnElements::QueryModelId(DgnElementId elementId) const
    {
    CachedStatementPtr stmt=GetStatement("SELECT ModelId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");
    stmt->BindId(1, elementId);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnModelId() : stmt->GetValueId<DgnModelId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    06/2015
//---------------------------------------------------------------------------------------
DgnElementId DgnElements::QueryElementIdByCode(DgnElement::Code const& code) const
    {
    if (!code.IsValid() || code.IsEmpty())
        return DgnElementId(); // An invalid code won't be found; an empty code won't be unique. So don't bother.
    else
        return QueryElementIdByCode(code.GetAuthority(), code.GetValue(), code.GetNameSpace());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnElements::QueryElementIdByCode(Utf8CP authority, Utf8StringCR value, Utf8StringCR nameSpace) const
    {
    return QueryElementIdByCode(GetDgnDb().Authorities().QueryAuthorityId(authority), value, nameSpace);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnElements::QueryElementIdByCode(DgnAuthorityId authority, Utf8StringCR value, Utf8StringCR nameSpace) const
    {
    CachedStatementPtr statement=GetStatement("SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Code=? AND CodeAuthorityId=? AND CodeNameSpace=? LIMIT 1"); // find first if code not unique
    statement->BindText(1, value, Statement::MakeCopy::No);
    statement->BindId(2, authority);
    statement->BindText(3, nameSpace, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW != statement->Step()) ? DgnElementId() : statement->GetValueId<DgnElementId>(0);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElements::HandlerStatementCache::Entry* DgnElements::HandlerStatementCache::FindEntry(ElementHandlerR handler) const
    {
    auto found = std::find_if(m_entries.begin(), m_entries.end(), [&handler](Entry& arg) { return &handler == arg.m_handler; });
    return m_entries.end() != found ? found : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::HandlerStatementCache::Empty()
    {
    m_entries.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElements::ElementSelectStatement DgnElements::GetPreparedSelectStatement(DgnElementR el) const
    {
    BeDbMutexHolder _v(m_mutex);
    auto& handler = el.GetElementHandler();
    return m_handlerStmts.GetPreparedSelectStatement(el, handler, handler.GetECSqlClassInfo());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr DgnElements::GetPreparedInsertStatement(DgnElementR el) const
    {
    // Not bothering to cache per handler...use our general-purpose ECSql statement cache
    ECSqlClassInfo const& info = el.GetElementHandler().GetECSqlClassInfo();
    return info.m_insert.empty() ? nullptr : GetDgnDb().GetPreparedECSqlStatement(info.m_insert.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr DgnElements::GetPreparedUpdateStatement(DgnElementR el) const
    {
    // Not bothering to cache per handler...use our general-purpose ECSql statement cache
    ECSqlClassInfo const& info = el.GetElementHandler().GetECSqlClassInfo();
    CachedECSqlStatementPtr stmt = info.m_update.empty() ? nullptr : GetDgnDb().GetPreparedECSqlStatement(info.m_update.c_str());
    if (stmt.IsValid())
        stmt->BindId(info.m_numUpdateParams+1, el.GetElementId());

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElements::ElementSelectStatement DgnElements::HandlerStatementCache::GetPreparedSelectStatement(DgnElementR el, ElementHandlerR handler, ECSqlClassInfo const& classInfo) const
    {
    CachedECSqlStatementPtr stmt;
    if (!classInfo.m_select.empty())
        {
        Entry* entry = FindEntry(handler);
        if (nullptr != entry)
            {
            if (entry->m_select.IsNull() || entry->m_select->GetRefCount() <= 1)
                {
                stmt = entry->m_select;
                }
            else
                {
                // The cached statement is already in use...create a new one for this caller
                stmt = new CachedECSqlStatement();
                if (ECSqlStatus::Success != stmt->Prepare(el.GetDgnDb(), classInfo.m_select.c_str()))
                    {
                    BeAssert(false);
                    stmt = nullptr;
                    }
                }
            }
        else
            {
            // First request for this handler...create an entry
            m_entries.push_back(Entry(&handler));
            entry = &m_entries.back();
            entry->m_select = new CachedECSqlStatement();
            if (ECSqlStatus::Success != entry->m_select->Prepare(el.GetDgnDb(), classInfo.m_select.c_str()))
                {
                BeAssert(false);
                entry->m_select = nullptr;
                }

            stmt = entry->m_select;
            }
        }

    if (stmt.IsValid())
        {
        BeAssert(el.GetElementId().IsValid());
        stmt->BindId(1, el.GetElementId());
        }

    return ElementSelectStatement(stmt.get(), classInfo.m_params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static uint16_t buildParamString(Utf8StringR str, ECSqlClassParams::Entries const& entries, ECSqlClassParams::StatementType type, T func)
    {
    uint16_t count = 0;
    for (auto const& entry : entries)
        {
        if (type != (entry.m_type & type))
            continue;

        if (0 < count)
            str.append(1, ',');

        func(entry.m_name, count);
        ++count;
        }

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlClassInfo const& dgn_ElementHandler::Element::GetECSqlClassInfo()
    {
    if (!m_classInfo.m_initialized)
        {
        _GetClassParams(m_classInfo.m_params);

        auto const& entries = m_classInfo.m_params.GetEntries();

        Utf8String fullClassName("[");
        fullClassName.append(GetDomain().GetDomainName()).append("].[").append(GetClassName()).append(1, ']');

        // Build SELECT statement
        m_classInfo.m_select = "SELECT ";
        uint16_t numSelectParams = buildParamString(m_classInfo.m_select, entries, ECSqlClassParams::StatementType::Select,
            [&](Utf8CP name, uint16_t count) { m_classInfo.m_select.append(1, '[').append(name).append(1, ']'); });

        if (0 < numSelectParams)
            {
            m_classInfo.m_select.append(" FROM ONLY ").append(fullClassName);
            m_classInfo.m_select.append(" WHERE ECInstanceId=?");
            }
        else
            {
            m_classInfo.m_select.clear();
            }

        // Build INSERT statement
        m_classInfo.m_insert.append("INSERT INTO ").append(fullClassName).append(1, '(');
        Utf8String insertValues;
        uint16_t numInsertParams = buildParamString(m_classInfo.m_insert, entries, ECSqlClassParams::StatementType::Insert,
            [&](Utf8CP name, uint16_t count)
                {
                m_classInfo.m_insert.append(1, '[').append(name).append(1, ']');
                if (0 < count)
                    insertValues.append(1, ',');

                insertValues.append(":[").append(name).append(1, ']');
                });

        if (0 < numInsertParams)
            m_classInfo.m_insert.append(")VALUES(").append(insertValues).append(1, ')');
        else
            m_classInfo.m_insert.clear();

        // Build UPDATE statement
        m_classInfo.m_update.append("UPDATE ONLY ").append(fullClassName).append(" SET ");
        m_classInfo.m_numUpdateParams = buildParamString(m_classInfo.m_update, entries, ECSqlClassParams::StatementType::Update,
            [&](Utf8CP name, uint16_t count)
                {
                m_classInfo.m_update.append(1, '[').append(name).append("]=:[").append(name).append(1, ']');
                });
        if (0 < m_classInfo.m_numUpdateParams)
            m_classInfo.m_update.append( "WHERE ECInstanceId=?");
        else
            m_classInfo.m_update.clear();

        // We no longer need any param names except those used in INSERT.
        m_classInfo.m_params.RemoveAllButSelect();

        m_classInfo.m_initialized = true;
        }

    return m_classInfo;
    }


