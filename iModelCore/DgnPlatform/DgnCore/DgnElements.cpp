/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnElements.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

typedef DgnElementP* DgnElementH;

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

    bool Contains(uint64_t key) const {return (key >= m_low) && (key <= m_high);}
    bool Contains(ElemIdRange const& range) const {return (range.m_high <= m_high) && (range.m_low >= m_low);}
};

enum class ElemPurge {Kept=0, Deleted=1};

struct ElemIdRangeNode;
struct ElemIdLeafNode;

typedef struct ElemIdRangeNode* ElemIdRangeNodeP;
typedef ElemIdRangeNode const * ElemIdRangeNodeCP;
typedef ElemIdRangeNodeP*       ElemIdRangeNodeH;
typedef ElemIdRangeNodeP const* ElemIdRangeNodeCH;
typedef bool (*T_NodeSortFunc)(ElemIdRangeNodeP, ElemIdRangeNodeP);

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
    virtual void _Add(DgnElementR entry, uint64_t counter) = 0;
    virtual struct ElemIdLeafNode const* _GetFirstNode() const = 0;
    virtual ElemPurge _Purge(int64_t memTarget) = 0;
    virtual ElemPurge _ReleaseAndCleanup(uint64_t key) = 0;
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
    DgnElementP Find(uint64_t key, bool) const;
};

/*=================================================================================**//**
// Leaf nodes only hold elements
// @bsiclass                                                    Keith.Bentley   09/12
+===============+===============+===============+===============+===============+======*/
struct ElemIdLeafNode : ElemIdRangeNode
{
private:
    DgnElementP m_elems[NUM_LEAFENTRIES];

    void CalculateLeafRange() const {InitRange(m_elems[0]->GetElementId().GetValue(),(*LastEntry())->GetElementId().GetValue());}
    void ReleaseAllPinned(DgnElementP ref);

    virtual ElemIdLeafNode const* _GetFirstNode() const {return this;}
    virtual void _CalculateNodeRange() const override {CalculateLeafRange();}
    virtual void _Add(DgnElementR entry, uint64_t counter) override;
    virtual ElemPurge _Purge(int64_t) override;
    virtual ElemPurge _ReleaseAndCleanup(uint64_t key) override;
    virtual void _Empty() override;

public:
    DgnElementP GetEntry(int index) const {return m_elems[index];}
    DgnElementP const* FirstEntry() const {return m_elems;}
    DgnElementP const* LastEntry() const {return &m_elems[m_nEntries-1];}
    ElemIdLeafNode(ElemIdTree& root, ElemIdParent* parent) : ElemIdRangeNode(root, parent, true) {}
    void AddEntry(DgnElementR entry);
    void SplitLeafNode();
    DgnElementP FindLeaf(uint64_t key, bool) const;
};

/*=================================================================================**//**
* an internal node either holds all Internal nodes, or all LeafNodes.
* @bsiclass                                                     KeithBentley    12/97
+===============+===============+===============+===============+===============+======*/
struct ElemIdInternalNode : public ElemIdRangeNode, ElemIdParent
{
protected:
    ElemIdRangeNodeP m_children[NUM_INTERNALENTRIES];

    virtual void _Add(DgnElementR entry, uint64_t counter) override {SetLastUnReferenced(counter); ChooseBestNode(entry.GetElementId().GetValue())->_Add(entry, counter);}
    virtual void _IncreaseRange(ElemIdRange const&) override;
    virtual void _CalculateNodeRange() const override;
    virtual ElemIdLeafNode const* _GetFirstNode() const override {return (*FirstEntryC())->_GetFirstNode();}
    virtual ElemIdLeafNode const* _NextSibling(ElemIdRangeNodeCP from) const override;
    virtual ElemPurge _Purge(int64_t) override;
    virtual ElemPurge _ReleaseAndCleanup(uint64_t key) override;
    virtual void _Empty() override;

    void AccessedNode(int i);
    void SortInto(ElemIdRangeNodeP* into, ElemIdRangeNodeP* from, T_NodeSortFunc sortFunc);

public:
    virtual void _AddChildNode(ElemIdRangeNodeP newNode) override;

    ElemIdInternalNode(ElemIdTree& root, ElemIdParent* parent) : ElemIdRangeNode(root, parent, false) {}

    ElemIdRangeNodeCH GetEntryC(int i) const     {return m_children+i;}
    ElemIdRangeNodeCH FirstEntryC()     const     {return GetEntryC(0);}
    ElemIdRangeNodeCH LastEntryC()      const     {return GetEntryC(m_nEntries-1);}
    ElemIdRangeNodeH  GetEntry(int i)            {return m_children+i;}
    ElemIdRangeNodeH  FirstEntry()                {return GetEntry(0);}
    ElemIdRangeNodeH  LastEntry()                 {return GetEntry(m_nEntries-1);}
    ElemIdRangeNodeP ChooseBestNode(uint64_t key);
    bool Contains(ElemIdRangeNodeP child);
    void SplitInternalNode();
    DgnElementP FindInternal(uint64_t key, bool) const;
};

struct MyStats : DgnElements::Statistics
{
void Reset() {m_newElements=m_unReferenced=m_reReferenced=m_purged = 0 ;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
inline DgnElementP ElemIdRangeNode::Find(uint64_t key, bool setFreeEntryFlag) const
   {
   return m_isLeaf ?((ElemIdLeafNode const*) this)->FindLeaf(key,setFreeEntryFlag) :
                    ((ElemIdInternalNode const*) this)->FindInternal(key,setFreeEntryFlag);
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

    ElemIdInternalNode* NewInternalNode(ElemIdParent* parent) {return new ((ElemIdInternalNode*) m_internalPool.malloc()) ElemIdInternalNode(*this, parent);}
    ElemIdLeafNode* NewLeafNode(ElemIdParent* parent)         {return new ((ElemIdLeafNode*) m_leafPool.malloc()) ElemIdLeafNode(*this, parent);}
    void FreeNode(ElemIdRangeNodeP child, bool leaf);

public:
    ElemIdTree(DgnDbR project) : m_dgndb(project)
        {
        m_totals.m_entries        = 0;
        m_totals.m_unreferenced   = 0;
        m_totals.m_allocedBytes   = 0;
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
    void AddElement(DgnElementR);
    void KillElement(DgnElementP, bool);
    void ReleaseAndCleanup(DgnElementCP element);
    void Purge(int64_t memTarget);
    void Destroy();
};
END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/
//---------------------------------------------------------------------------------------
ElemPurge ElemIdLeafNode::_ReleaseAndCleanup(uint64_t key)
    {
    // binary search for position to put new entry
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
            BeAssert(m_elems[index]->GetRefCount() == 0); // duplicate keys!
            if (m_elems[index]->GetRefCount() != 0)
                return ElemPurge::Kept;

            DgnElementP target = m_elems[index];
            m_isSloppy = true;           // we can't tell whether we may have dropped the first or last entry.
            //  Is it worth computing m_allReferenced?

            m_nEntries -= 1;
            memmove(m_elems + index, m_elems + index + 1,(m_nEntries-index) * sizeof(DgnElementP));

            // this call deletes the element data, and all its AppData (e.g. XAttributes). It also keeps the total element/bytes count up to date.
            m_treeRoot.KillElement(target, false);
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
    DgnElementP killed[NUM_LEAFENTRIES];

    for (;curr < end; ++curr)
        {
        if (0 == (*curr)->GetRefCount()) // is the element garbage?
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
    m_nEntries = (int)(used - m_elems);

    // this call deletes the element data, and all its AppData (e.g. XAttributes). It also keeps the total element/bytes count up to date.
    for (unsigned i = 0; i < killedIndex; i++)
        m_treeRoot.KillElement(killed[i], false);

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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2014
//---------------------------------------------------------------------------------------
ElemPurge ElemIdInternalNode::_ReleaseAndCleanup(uint64_t key)
    {
    for (unsigned index = 0; index <(unsigned)m_nEntries; ++index)
        {
        ElemIdRangeNodeP node = m_children[index];
        ElemIdRange  currRange;
        node->GetExactNodeRange(currRange);

        if (key >= currRange.m_low && key <= currRange.m_high)
            {
            if (ElemPurge::Deleted != node->_ReleaseAndCleanup(key))
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

    BeAssert(false && "did not find _ReleaseAndCleanup target");
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
    int nLeft = (int)(used - purgeOrder);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2014
//---------------------------------------------------------------------------------------
void ElemIdTree::ReleaseAndCleanup(DgnElementCP element)
    {
    if (element->GetRefCount() != 0)
        {
        BeAssert(false && "ref count is not 0");
        return;
        }

    uint64_t key = element->GetElementId().GetValue();
    if (nullptr == m_root ||(ElemPurge::Deleted != m_root->_ReleaseAndCleanup(key)))
        return;

    // all of the elements were garbage!
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
void ElemIdLeafNode::AddEntry(DgnElementR entry)
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

        DgnElementP const*  tEntry = m_elems + index;
        memmove((void*)(tEntry+1), tEntry,(m_nEntries-index) * sizeof(DgnElementP));
        }

    m_elems[index] = &entry;
    m_nEntries++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdLeafNode::_Add(DgnElementR entry, uint64_t counter)
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
DgnElementP ElemIdLeafNode::FindLeaf(uint64_t key, bool setFreeEntryFlag) const
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
    int start       = (numEntries -(numEntries / 5)) + 1;

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

            lastDist = (key - thisRange.m_high);
            lastNode = *curr;
            }
        else
            {
            distance = (thisRange.m_low - key);
            return (distance < lastDist) ? *curr : lastNode;
            }
        }

    return lastNode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementP ElemIdInternalNode::FindInternal(uint64_t key, bool setFreeEntryFlag) const
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
    int                 start = (nEntries -(nEntries/4)) + 1;

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
void ElemIdTree::AddElement(DgnElementR entry)
    {
    if (nullptr == m_root)
        m_root = NewLeafNode(this);

    ++m_totals.m_entries;
    ++m_totals.m_unreferenced;
    m_root->_Add(entry, ++m_counter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdTree::KillElement(DgnElementP elRef, bool killingWholeTree)
    {
    BeAssert(0 == elRef->GetRefCount());
    if (!killingWholeTree)
        {
        BeAssert(0 != m_totals.m_entries);
        BeAssert(0 != m_totals.m_unreferenced);
        --m_totals.m_entries;
        --m_totals.m_unreferenced;
        ++m_stats.m_purged;
        }

    elRef->DeallocateRef(killingWholeTree);
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
        m_treeRoot.KillElement(*curr, true);

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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::AllocatedMemory(int32_t size)
    {
    if (size<0)
        {
        BeAssert(false);
        return;
        }
    BeDbMutexHolder _v(m_mutex);
    m_tree->m_totals.m_allocedBytes += size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::ReturnedMemory(int32_t size)
    {
    if (size<0)
        {
        BeAssert(false);
        return;
        }
    BeDbMutexHolder _v(m_mutex);
    m_tree->m_totals.m_allocedBytes -= size;
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
    m_tree->FindElement(el.GetElementId(), true);   // mark this entry as having free content
    ++m_tree->m_totals.m_unreferenced;
    ++m_tree->m_stats.m_unReferenced;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::AddDgnElement(DgnElementR element)
    {
    BeDbMutexHolder _v_v(m_mutex);
    BeAssert(!element.IsInPool());
    element.SetInPool();

    AllocatedMemory(element._GetMemSize());

    m_tree->AddElement(element);
    ++m_tree->m_stats.m_newElements;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2014
//---------------------------------------------------------------------------------------
void DgnElements::ReleaseAndCleanup(DgnElementCPtr& elementRef)
    {
    if (elementRef->GetRefCount() != 1)
        return;

    elementRef = nullptr;
    BeDbMutexHolder _v_v(m_mutex);
    m_tree->ReleaseAndCleanup(elementRef.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::Purge(int64_t memTarget)
    {
    BeDbMutexHolder _v_v(m_mutex);
    m_tree->Purge(memTarget);
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
DbResult DgnElements::GetStatement(CachedStatementPtr& stmt, Utf8CP sql) const
    {
    return m_stmts.GetPreparedStatement(stmt, *m_dgndb.GetDbFile(), sql);
    }
    
DgnElements::Totals DgnElements::GetTotals() const {return m_tree->m_totals;}
DgnElements::Statistics DgnElements::GetStatistics() const {return m_tree->m_stats;}
void DgnElements::ResetStatistics() {m_tree->m_stats.Reset();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElements::DgnElements(DgnDbR dgndb) : DgnDbTable(dgndb), m_heapZone(0, false), m_mutex(BeDbMutex::MutexType::Recursive), m_stmts(20)
    {
    m_listeners = nullptr;
    m_tree = new ElemIdTree(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* A changeset was just applied to the database. That means that the element data in memory potentially does not match
* what is now in the database. We need to find any elements in memory that were affected by the changeset and fix them.
* Note that it is entirely possible that some elements in the changeset are not currently in memory. That's fine, we
* don't need to worry about them - they'll be reloaded with the correct data if/when they're needed.
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::OnChangesetApplied(TxnSummary const& summary)
    {
    // ADDS: elements that were added by the changeset are generally not of interest. However, when we
    // delete elements in a session we don't remove them from the pool or from loaded models.
    // Rather, they are marked as deleted. That is for two reasons: 1) otherwise we wouldn't know what model to
    // put it in when it is undeleted in the case where we have loaded models, and 2) because we can't force their
    // reference count to 0 and someone may be holding a reference to it. So, we need to check all of the adds in the
    // changeset and see whether they are in fact "undeletes" from undo/redo.
    for (auto& el : summary.m_addedElements)
        {
        DgnElementP elRef = const_cast<DgnElementP>(FindElement(el));
        if (nullptr != elRef) // if it is not found, there's nothing to do
            elRef->UnDeleteElement();
        }

    // DELETES: elements that were deleted by the changeset must be marked as deleted in the pool (see discussion above).
    for (auto& el : summary.m_deletedElements)
        {
        DgnElementP elRef = const_cast<DgnElementP>(FindElement(el));
        if (nullptr != elRef) // if not found, nothing to do
            {
            DgnModelR model = elRef->GetDgnModel();
            elRef->MarkAsDeleted();
            model._OnDeletedElement(*elRef, false);
            }
        }

    // MODIFIED: elements that are modified must be reloaded from the database
    for (auto& el: summary.m_updatedElements)
        {
        DgnModelStatus stat = el->ReloadFromDb();
        BeAssert(DGNMODEL_STATUS_Success == stat);
        UNUSED_VARIABLE(stat);
        }
    }

/*---------------------------------------------------------------------------------**//**
* A previously undone changeset was canceled (meaning we made a different change while the changeset was reversed).
* Or, we had uncommitted changes when we issued the undo command, that were reversed.
* At this point all of the elements that were deleted by reversing the changeset (i.e. elements that were created by the original change)
* are no longer useful. Tell their DgnModel to drop its reference (if it has one) to them so they become garbage and can be purged.
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::OnChangesetCanceled(TxnSummary const& summary)
    {
    // This is only necessary because we keep deleted elements in memory. For XAttributes, we free the memory when
    // we delete them so there is no corresponding work here for them.
    for (auto& el : summary.m_deletedElements)
        {
        DgnElementP elRef = const_cast<DgnElementP>(FindElement(el));
        if (nullptr == elRef)
            continue;

        // Note: we can't actually free the memory here. We must wait for the next purge. They are garbage and no one
        // should ever find them again. The element must have been deleted when the add was changeset was undone, but
        // we need to tell the model to drop its reference.
        DgnModelR model = elRef->GetDgnModel();

        // If the element was undone, then it is already deleted. However, if the transaction was never committed, and we're
        // undoing, then we need to delete the element to free its XAttributes, QV graphics, etc.
        if (!elRef->IsDeleted())
            elRef->MarkAsDeleted();

        model._OnDeletedElement(*elRef, true); // this causes the model to remove this element from its "owned" list.
        BeAssert(elRef->GetRefCount() == 0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::LoadElement(DgnElement::CreateParams const& params) const
    {
    ElementHandlerP elHandler = ElementHandler::FindHandler(m_dgndb, params.m_classId);
    if (nullptr == elHandler)
        {
        BeAssert(false);
        return nullptr;
        }

    DgnElementPtr elRef = elHandler->Create(params);
    if (!elRef.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    if (DGNMODEL_STATUS_Success != elRef->_LoadFromDb())
        return nullptr;

    params.m_model._OnLoadedElement(*elRef);
    return elRef;
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

    DgnElementCP elRef = FindElement(elementId);
    if (nullptr != elRef)
        return elRef;

    CachedStatementPtr stmt;
    enum Column : int       {ClassId=0,ModelId=1,CategoryId=2,Code=3,ParentId=4};
    GetStatement(stmt, "SELECT ECClassId,ModelId,CategoryId,Code,ParentId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");
    stmt->BindId(1, elementId);

    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        return nullptr;

    DgnModelP dgnModel = m_dgndb.Models().GetModelById(stmt->GetValueId<DgnModelId>(Column::ModelId));
    if (nullptr == dgnModel)
        {
        BeAssert(false);
        return nullptr;
        }

    return LoadElement(DgnElement::CreateParams(*dgnModel, 
                    stmt->GetValueId<DgnClassId>(Column::ClassId), 
                    stmt->GetValueId<DgnCategoryId>(Column::CategoryId), 
                    stmt->GetValueText(Column::Code), 
                    elementId, 
                    stmt->GetValueId<DgnElementId>(Column::ParentId)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElements::IsElementIdUsed(DgnElementId id) const
    {
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_dgndb.GetCachedStatement(stmt, "SELECT 1 FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?"))
        {
        BeAssert(0);
        return false;
        }

    stmt->BindInt64(1, id.GetValueUnchecked());
    return BE_SQLITE_ROW == stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnElements::GetHighestElementId()
    {
    if (!m_highestElementId.IsValid())
        {
        BeLuid nextRepo(m_dgndb.GetRepositoryId().GetNextRepositoryId().GetValue(),0);
        Statement stmt;
        
        stmt.Prepare(m_dgndb, "SELECT max(Id) FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id<?");
        stmt.BindInt64(1,nextRepo.GetValue());

        DbResult result = stmt.Step();
        UNUSED_VARIABLE(result);
        BeAssert(result == BE_SQLITE_ROW);

        int64_t currMax = stmt.GetValueInt64(0);

        BeRepositoryBasedId firstId(m_dgndb.GetRepositoryId(), 0);
        m_highestElementId.m_id = (currMax < firstId.m_id) ? firstId.m_id : currMax;
        }

    return m_highestElementId;
    }

/*---------------------------------------------------------------------------------**//**
 DgnElementIds are 64 bits, divided into two 32 bit parts {high:low}. The high 32 bits are reserved for the
 repositoryId of the creator and the low 32 bits hold the identifier of the element. This scheme is
 designed to allow multiple users on differnt computers to create new elements without
 generating conflicting ids, since the repositoryId is intended to be unqiue for every copy of the project.
 We are allowed to make DgnElementIds in the range of [{repositoryid:1},{repositoryid+1:0}). So, find the highest currently
 used id in that range and add 1. If none, use the first id. If the highest possible id is already in use, search for an
 available id randomly.
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnElements::MakeNewElementId()
    {
    GetHighestElementId();

    // see if the next id is the highest possible Id for our repositoryId. If not, use it. Otherwise try random ids until we find one that's
    // not currently in use.
    BeRepositoryBasedId lastId(m_dgndb.GetRepositoryId().GetNextRepositoryId(), 0);
    if (m_highestElementId.m_id < lastId.m_id-100) // reserve a few ids for special meaning
        {
        m_highestElementId.UseNext();
        return m_highestElementId;
        }

    // highest id already used, try looking for a random available id
    BeLuid val;
    do
        {
        val.CreateRandom();
        val.m_luid.i[1] = m_dgndb.GetRepositoryId().GetValue();
        } while(IsElementIdUsed(DgnElementId(val.m_luid.u)));

    return DgnElementId(val.m_luid.u);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElements::InsertElement(DgnElementR element)
    {
    if (element.GetElementId().IsValid())
        return DGNMODEL_STATUS_IdExists;

    DgnModelR model = element.GetDgnModel();
    if (&model.GetDgnDb() != &m_dgndb)
        return DGNMODEL_STATUS_WrongDgnDb;

    DgnModelStatus stat = element._OnAdd();
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    stat = model._OnAddElement(element);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    element.m_elementId = MakeNewElementId();
    stat = element._InsertInDb();
    if (DGNMODEL_STATUS_Success != stat)
        {
        element.m_elementId = DgnElementId();
        return stat;
        }

    element._ApplyScheduledChangesToInstances(element);
    element._ClearScheduledChangesToInstances();

    element._OnAdded();

    AddDgnElement(element);
    model._OnAddedElement(element);

    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElements::UpdateElement(DgnElementR element)
    {
    return DGNMODEL_STATUS_Success;
    }
