/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnElements.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

typedef DgnElementCP* DgnElementH;

BEGIN_BENTLEY_DGN_NAMESPACE
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
typedef std::function<void(DgnElementCR)> T_VisitElemFunc;

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
    virtual ElemPurge _Purge(uint64_t memTarget) = 0;
    virtual ElemPurge _Drop(uint64_t key) = 0;
    virtual void _Empty() = 0;
    virtual void _Visit(T_VisitElemFunc) const = 0;

    bool ContainsKey(uint64_t key) const {CheckSloppy(); return m_range.Contains(key);}
    void SetParent(ElemIdParent* newParent) {m_parent = newParent;}
    bool IsLeaf() const {return m_isLeaf;}
    ElemIdParent const* GetParent() const {return m_parent;}
    bool IsSloppy() const {return m_isSloppy;}
    void CheckSloppy() const {if (IsSloppy())_CalculateNodeRange();}
    int GetCount() const {return m_nEntries;}
    void InitRange(uint64_t min, uint64_t max) const {m_range.Init(min, max); m_isSloppy = false;}
    void InitRange() const {InitRange(ULLONG_MAX, 0);}
    ElemIdRange const& GetExactNodeRange() const {CheckSloppy(); return m_range;}
    void SetNodeRange(ElemIdRange const& range) const {m_range = range; m_isSloppy = false;}
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

    ElemIdLeafNode const* _GetFirstNode() const override {return this;}
    void _CalculateNodeRange() const override {CalculateLeafRange();}
    void _Add(DgnElementCR entry, uint64_t counter) override;
    ElemPurge _Purge(uint64_t) override;
    ElemPurge _Drop(uint64_t key) override;
    void _Empty() override;
    void _Visit(T_VisitElemFunc) const override;

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

    void _Add(DgnElementCR entry, uint64_t counter) override {SetLastUnReferenced(counter); ChooseBestNode(entry.GetElementId().GetValue())->_Add(entry, counter);}
    void _IncreaseRange(ElemIdRange const&) override;
    void _CalculateNodeRange() const override;
    ElemIdLeafNode const* _GetFirstNode() const override {return (*FirstEntryC())->_GetFirstNode();}
    ElemIdLeafNode const* _NextSibling(ElemIdRangeNodeCP from) const override;
    ElemPurge _Purge(uint64_t) override;
    ElemPurge _Drop(uint64_t key) override;
    void _Empty() override;
    void _Visit(T_VisitElemFunc) const override;
    void SortInto(ElemIdRangeNodeP* into, ElemIdRangeNodeP* from, T_NodeSortFunc sortFunc);

public:
    void _AddChildNode(ElemIdRangeNodeP newNode) override;

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
struct ElemIdTree : ElemIdParent
{
    FixedSizePool1     m_leafPool;          // pool for allocating leaf nodes
    FixedSizePool1     m_internalPool;      // pool for allocating internal nodes
    ElemIdRangeNodeP   m_root;              // the root node. Starts as a leaf node for trees with less than 50 entries
    DgnDbR             m_dgndb;
    uint64_t           m_counter;           // always increasing, used to tell least recently accessed for garbage collection
    MyStats            m_stats;
    DgnElements::Totals m_totals;

    void _IncreaseRange(ElemIdRange const&) override {}
    void _AddChildNode(ElemIdRangeNodeP newNode) override;
    ElemIdLeafNode const* _NextSibling(ElemIdRangeNodeCP curr) const override {return nullptr;}

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
    void VisitElements(T_VisitElemFunc func) const {if (nullptr != m_root) m_root->_Visit(func);}
};
END_BENTLEY_DGN_NAMESPACE

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
ElemPurge ElemIdLeafNode::_Purge(uint64_t memTarget)
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
        if (0 == (*curr)->GetRefCount()) // is the element garbage?
            {
            //  Do not kill the element here.  If the element's app data holds a reference to another
            //  element, killing the element here may cause the reference
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

    // this call deletes the element data, and all its AppData. It also keeps the total element/bytes count up to date.
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
        ElemIdRange const& currRange = node->GetExactNodeRange();

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
ElemPurge ElemIdInternalNode::_Purge(uint64_t memTarget)
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

    if (nullptr == m_root || (ElemPurge::Deleted != m_root->_Purge(memTarget)))
        return;

    // all of the elements were garbage. The tree is now empty.
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
        ElemIdRange const& thisRange = (*curr)->GetExactNodeRange();

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

        ElemIdRange const& thisRange = m_children[index]->GetExactNodeRange();

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

    ElemIdRange const& range = newNode->GetExactNodeRange();
    m_range.Extend(range);

    int index = 0;
    for (; index <m_nEntries; index++)
        {
        ElemIdRange const& thisRange = m_children[index]->GetExactNodeRange();

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

    for (ElemIdRangeNodeCH curr = FirstEntryC(), last = LastEntryC(); curr <= last ; ++curr)
        {
        ElemIdRange const& currRange = (*curr)->GetExactNodeRange();
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
    ElemIdRangeNodeP     currentRoot = m_root;
    ElemIdInternalNode*  newRoot;

    newRoot = new((ElemIdInternalNode*) m_internalPool.malloc()) ElemIdInternalNode(*this, this);
    newRoot->_AddChildNode(newNode);

    newNode->GetExactNodeRange();

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
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdLeafNode::_Visit(T_VisitElemFunc func) const
    {
    DgnElementCP const* end = m_elems + m_nEntries;
    for (DgnElementCP const* curr = m_elems; curr < end; ++curr)
        func(**curr);
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
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemIdInternalNode::_Visit(T_VisitElemFunc func) const
    {
    ElemIdRangeNodeP const* end = m_children + m_nEntries;
    for (ElemIdRangeNodeP const* curr = m_children; curr < end; ++curr)
        (*curr)->_Visit(func);
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::Destroy()
    {
    BeMutexHolder _v_v(m_mutex);
    m_tree->Destroy();
    ClearECCaches();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::ChangeMemoryUsed(int32_t delta) const
    {
    if (0==delta) // nothing happened, don't bother to get mutex
        return;

    BeMutexHolder _v(m_mutex);
    m_tree->m_totals.m_allocedBytes += delta;
    BeAssert(m_tree->m_totals.m_allocedBytes >= 0);
    }

/*---------------------------------------------------------------------------------**//**
* an element that was previously garbage was just referenced
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::OnReclaimed(DgnElementCR el)
    {
    BeMutexHolder _v_v(m_mutex);
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
    BeMutexHolder _v_v(m_mutex);
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
    BeMutexHolder _v_v(m_mutex);
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

    BeMutexHolder _v_v(m_mutex);
    m_tree->DropElement(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t DgnElements::_Purge(uint64_t memTarget)
    {
    BeMutexHolder _v_v(m_mutex);
    m_tree->Purge(memTarget);
    return GetTotalAllocated();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP DgnElements::FindLoadedElement(DgnElementId id) const
    {
    BeMutexHolder _v_v(m_mutex);
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
DgnElement::DgnElement(CreateParams const& params) : m_refCount(0), m_elementId(params.m_id), 
    m_dgndb(params.m_dgndb), m_modelId(params.m_modelId), m_classId(params.m_classId), 
    m_federationGuid(params.m_federationGuid), m_code(params.m_code), m_parentId(params.m_parentId), m_parentRelClassId(params.m_parentId.IsValid() ? params.m_parentRelClassId : DgnClassId()),
    m_userLabel(params.m_userLabel), m_ecPropertyData(nullptr), m_ecPropertyDataSize(0), m_structInstances(nullptr)
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

    if (nullptr != m_ecPropertyData)
        bentleyAllocator_free(m_ecPropertyData);

    --GetDgnDb().Elements().m_tree->m_totals.m_extant;
    }

DgnElements::Totals const& DgnElements::GetTotals() const {return m_tree->m_totals;}
DgnElements::Statistics DgnElements::GetStatistics() const {return m_tree->m_stats;}
void DgnElements::ResetStatistics() {m_tree->m_stats.Reset();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElements::DgnElements(DgnDbR dgndb) : DgnDbTable(dgndb), m_stmts(20), m_snappyFrom(m_snappyFromBuffer, _countof(m_snappyFromBuffer))
    {
    m_tree.reset(new ElemIdTree(dgndb));
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                Ramanujam.Raman                    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnApply()
    {
    if (!m_txnMgr.IsInAbandon())
        _OnValidate();
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                Ramanujam.Raman                    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnApplied()
    {
    if (!m_txnMgr.IsInAbandon())
        _OnValidated();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnAppliedAdd(BeSQLite::Changes::Change const& change)
    {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Insert);

    DgnElementId elementId = DgnElementId(change.GetValue(0, Changes::Change::Stage::New).GetValueUInt64());

    // We need to load this element, since filled models need to register it 
    DgnElementCPtr el = m_txnMgr.GetDgnDb().Elements().GetElement(elementId);
    BeAssert(el.IsValid());
    el->_OnAppliedAdd();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnAppliedDelete(BeSQLite::Changes::Change const& change)
    {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Delete);

    DgnElementId elementId = DgnElementId(change.GetValue(0, Changes::Change::Stage::Old).GetValueUInt64());

    // see if we have this element in memory, if so call its _OnDelete method.
    DgnElementPtr el = (DgnElementP) m_txnMgr.GetDgnDb().Elements().FindLoadedElement(elementId);
    if (el.IsValid()) 
        el->_OnAppliedDelete(); // Note: this MUST be a DgnElementPtr, since we can't call _OnAppliedDelete with an element with a zero ref count
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnAppliedUpdate(BeSQLite::Changes::Change const& change) 
    {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Update);

    auto& elements = m_txnMgr.GetDgnDb().Elements();
    DgnElementId elementId = DgnElementId(change.GetValue(0, Changes::Change::Stage::Old).GetValueUInt64());
    DgnElementCPtr el = elements.FindLoadedElement(elementId);
    if (el.IsValid())
        {
        DgnElementCPtr postModified = elements.LoadElement(el->GetElementId(), false);
        BeAssert(postModified.IsValid());
        postModified->_OnAppliedUpdate(*el);

        elements.FinishUpdate(*postModified.get(), *el);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::LoadElement(DgnElement::CreateParams const& params, Utf8CP jsonProps, bool makePersistent) const
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

    if (jsonProps)
        Json::Reader::Parse(jsonProps, el->m_jsonProperties);

    if (DgnDbStatus::Success != el->_LoadFromDb())
        return nullptr;

    el->_OnLoadedJsonProperties();

    if (makePersistent)
        {
        auto geomEl = el->ToGeometrySourceP();
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
DgnElementCPtr DgnElements::LoadElement(DgnElementId elementId,  bool makePersistent) const
    {
    enum Column : int {ClassId=0,ModelId=1,CodeSpec=2,CodeScope=3,CodeValue=4,UserLabel=5,ParentId=6,ParentRelClassId=7,FederationGuid=8,JsonProps=9};
    CachedStatementPtr stmt = GetStatement("SELECT ECClassId,ModelId,CodeSpecId,CodeScopeId,CodeValue,UserLabel,ParentId,ParentRelECClassId,FederationGuid,JsonProperties FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id=?");
    stmt->BindId(1, elementId);

    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        return nullptr;

    DgnCode code(stmt->GetValueId<CodeSpecId>(Column::CodeSpec), stmt->GetValueId<DgnElementId>(Column::CodeScope), stmt->GetValueText(Column::CodeValue));

    DgnElement::CreateParams createParams(m_dgndb, stmt->GetValueId<DgnModelId>(Column::ModelId), 
                    stmt->GetValueId<DgnClassId>(Column::ClassId), 
                    code,
                    stmt->GetValueText(Column::UserLabel), 
                    stmt->GetValueId<DgnElementId>(Column::ParentId),
                    stmt->GetValueId<DgnClassId>(Column::ParentRelClassId),
                    stmt->GetValueGuid(Column::FederationGuid));

    createParams.SetElementId(elementId);

    return LoadElement(createParams, stmt->GetValueText(Column::JsonProps), makePersistent);
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
    BeMutexHolder _v(m_mutex);
    DgnElementCP element = FindLoadedElement(elementId);
    return (nullptr != element) ? element : LoadElement(elementId, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElements::LoadGeometryStream(GeometryStreamR geom, void const* blob, int blobSize)
    {
    BeMutexHolder _v(m_mutex);
    return geom.ReadGeometryStream(GetSnappyFrom(), GetDgnDb(), blob, blobSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::QueryElementByFederationGuid(BeGuidCR federationGuid) const
    {
    if (!federationGuid.IsValid())
        return nullptr;

    CachedStatementPtr statement = GetStatement("SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE FederationGuid=?");
    statement->BindGuid(1, federationGuid);
    return (BE_SQLITE_ROW != statement->Step()) ? nullptr : GetElement(statement->GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementIterator DgnElements::MakeIterator(Utf8CP className, Utf8CP whereClause, Utf8CP orderByClause, PolymorphicQuery polymorphic) const
    {
    Utf8String sql("SELECT ECInstanceId,ECClassId,FederationGuid,CodeSpec.Id,CodeScope.Id,CodeValue,Model.Id,Parent.Id,UserLabel,LastMod FROM ");
    if (PolymorphicQuery::No == polymorphic)
        sql.append("ONLY ");

    sql.append(className);

    if (whereClause)
        {
        sql.append(" ");
        sql.append(whereClause);
        }

    if (orderByClause)
        {
        sql.append(" ");
        sql.append(orderByClause);
        }

    ElementIterator iterator;
    iterator.Prepare(m_dgndb, sql.c_str(), 0 /* Index of ECInstanceId */);
    return iterator;
    }

DgnElementId ElementIteratorEntry::GetElementId() const {return m_statement->GetValueId<DgnElementId>(0);}
DgnClassId ElementIteratorEntry::GetClassId() const {return m_statement->GetValueId<DgnClassId>(1);}
BeGuid ElementIteratorEntry::GetFederationGuid() const {return m_statement->GetValueGuid(2);}
DgnCode ElementIteratorEntry::GetCode() const {return DgnCode(m_statement->GetValueId<CodeSpecId>(3), m_statement->GetValueId<DgnElementId>(4), m_statement->GetValueText(5));}
Utf8CP ElementIteratorEntry::GetCodeValue() const {return m_statement->GetValueText(5);}
DgnModelId ElementIteratorEntry::GetModelId() const {return m_statement->GetValueId<DgnModelId>(6);}
DgnElementId ElementIteratorEntry::GetParentId() const {return m_statement->GetValueId<DgnElementId>(7);}
Utf8CP ElementIteratorEntry::GetUserLabel() const {return m_statement->GetValueText(8);}
DateTime ElementIteratorEntry::GetLastModifyTime() const {return m_statement->GetValueDateTime(9);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAspectIterator DgnElement::MakeAspectIterator() const
    {
    ElementAspectIterator iterator;
    iterator.Prepare(GetDgnDb(), 
        "SELECT ECInstanceId,ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementUniqueAspect) " WHERE Element.Id=:ElementIdParam1 UNION "
        "SELECT ECInstanceId,ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementMultiAspect)  " WHERE Element.Id=:ElementIdParam2",
        0 /* Index of ECInstanceId */);

    ECSqlStatement* statement = iterator.GetStatement();
    if (statement)
        {
        statement->BindId(statement->GetParameterIndex("ElementIdParam1"), GetElementId());
        statement->BindId(statement->GetParameterIndex("ElementIdParam2"), GetElementId());
        }

    return iterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAspectIterator DgnElements::MakeAspectIterator(Utf8CP className, Utf8CP whereClause, Utf8CP orderByClause) const
    {
    Utf8String sql("SELECT ECInstanceId,ECClassId,Element.Id FROM ");
    sql.append(className);

    if (whereClause)
        {
        sql.append(" ");
        sql.append(whereClause);
        }

    if (orderByClause)
        {
        sql.append(" ");
        sql.append(orderByClause);
        }

    ElementAspectIterator iterator;
    iterator.Prepare(m_dgndb, sql.c_str(), 0 /* Index of ECInstanceId */);
    return iterator;
    }

ECInstanceId ElementAspectIteratorEntry::GetECInstanceId() const {return m_statement->GetValueId<ECInstanceId>(0);}
DgnClassId ElementAspectIteratorEntry::GetClassId() const {return m_statement->GetValueId<DgnClassId>(1);}
DgnElementId ElementAspectIteratorEntry::GetElementId() const {return m_statement->GetValueId<DgnElementId>(2);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::PerformInsert(DgnElementR element, DgnDbStatus& stat)
    {
    if (!element.m_flags.m_preassignedId)
        {
        if (BE_SQLITE_OK != m_dgndb.GetElementIdSequence().GetNextValue(element.m_elementId))
            return nullptr;
        }

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
    DgnDb::VerifyClientThread();

    DgnDbStatus ALLOW_NULL_OUTPUT(stat,outStat);

    // don't allow elements that already have an id unless the "preassignedId" flag is set (PKPM requested a "back door" for sync workflows)
    if (element.m_elementId.IsValid() && !element.m_flags.m_preassignedId)
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

#ifdef __clang__
    if (0 != strcmp(typeid(element).name(), element.GetElementHandler()._ElementType().name()))
#else
    if (typeid(element) != element.GetElementHandler()._ElementType())
#endif
        {
        LOG.errorv("InsertElement element must have its own handler: element typeid=%s, handler typeid=%s", typeid(element).name(), element.GetElementHandler()._ElementType().name());
        BeAssert(false && "you can only insert an element that has ITS OWN handler");
        stat = DgnDbStatus::WrongHandler; // they gave us an element with an invalid handler
        return nullptr;
        }

    DgnElementCPtr newEl = PerformInsert(element, stat);
    if (!newEl.IsValid())
        element.m_elementId = DgnElementId(); // Insert failed, make sure to invalidate the DgnElementId so they don't accidentally use it

    element.m_flags.m_preassignedId = 0; // ensure flag is set to default value
    return newEl;
    }

/*---------------------------------------------------------------------------------**//**
* PKPM requested a "back door" for sync workflows that need to force an DgnElementId for Insert
* @bsimethod                                                    ShaunSewall     01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::ForceElementIdForInsert(DgnElementId elementId)
    {
    if (IsPersistent())
        {
        BeAssert(false);
        return;
        }

    m_flags.m_preassignedId = true;
    m_elementId = elementId;
    }

/*---------------------------------------------------------------------------------**//**
* this method is called both after we've directly updated an element, and after we've reversed an update to an element.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::FinishUpdate(DgnElementCR replacement, DgnElementCR original)
    {
    BeAssert(0 != original.GetRefCount());
    BeAssert(original.IsPersistent());

    // The pool can only hold immutable objects. We are about to change the original element which is now in the pool.
    // To keep the pool's memory statistics valid, we must drop the element from the pool, modify it, and then add it back
    // into the pool in its changed state. Attempting to track size changes here doesn't work since the element's class may 
    // attempt to adjust the pool itself. 
    DropFromPool(original);
    (*const_cast<DgnElementP>(&original))._CopyFrom(replacement);    // copy new data into original element
    AddToPool(original);

    original._OnUpdateFinished(); // this gives geometric elements a chance to clear their graphics
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::UpdateElement(DgnElementR replacement, DgnDbStatus* outStat)
    {
    DgnDb::VerifyClientThread();

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

    if (element.m_parentId != replacement.m_parentId) // did parent change?
        {
        // ask original parent if it is okay to drop the child
        DgnElementCPtr originalParent = GetElement(element.m_parentId);
        if (originalParent.IsValid() && DgnDbStatus::Success != (stat = originalParent->_OnChildDrop(element)))
            return nullptr;

        // ask new parent if it is okay to add the child
        DgnElementCPtr replacementParent = GetElement(replacement.m_parentId);
        if (replacementParent.IsValid() && DgnDbStatus::Success != (stat = replacementParent->_OnChildAdd(replacement)))
            return nullptr;
        }
    else
        {
        // ask parent whether it is ok to update its child.
        DgnElementCPtr parent = GetElement(element.m_parentId);
        if (parent.IsValid() && DgnDbStatus::Success != (stat = parent->_OnChildUpdate(element, replacement)))
            return nullptr;
        }

    stat = replacement._UpdateInDb();   // perform the actual update in the database
    if (DgnDbStatus::Success != stat)
        return nullptr;

    replacement._OnUpdated(element);
    FinishUpdate(replacement, element);

    if (element.m_parentId != replacement.m_parentId) // did parent change?
        {
        // notify original parent that child has been dropped
        DgnElementCPtr originalParent = GetElement(element.m_parentId);
        if (originalParent.IsValid())
            originalParent->_OnChildDropped(element);

        // notify new parent that child has been added
        DgnElementCPtr replacementParent = GetElement(replacement.m_parentId);
        if (replacementParent.IsValid())
            replacementParent->_OnChildAdded(replacement);
        }
    else
        {
        // notify parent that its child has been updated
        DgnElementCPtr parent = GetElement(replacement.m_parentId);
        if (parent.IsValid())
            parent->_OnChildUpdated(element);
        }

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
DgnDbStatus DgnElements::Delete(DgnElementCR elementIn)
    {
    DgnDb::VerifyClientThread();

    if (&elementIn.GetDgnDb() != &m_dgndb)
        return DgnDbStatus::WrongDgnDb;

    DgnElementCPtr el = elementIn.IsPersistent() ? &elementIn : GetElement(elementIn.GetElementId());
    if (!el.IsValid())
        return DgnDbStatus::BadElement;

    DgnElementCR element = *el;

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
    CachedStatementPtr stmt=GetStatement("SELECT ModelId FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id=?");
    stmt->BindId(1, elementId);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnModelId() : stmt->GetValueId<DgnModelId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    06/2015
//---------------------------------------------------------------------------------------
DgnElementId DgnElements::QueryElementIdByCode(DgnCodeCR code) const
    {
    if (!code.IsValid() || code.IsEmpty())
        return DgnElementId(); // An invalid code won't be found; an empty code won't be unique. So don't bother.

    return QueryElementIdByCode(code.GetCodeSpecId(), code.GetScopeElementId(GetDgnDb()), code.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnElements::QueryElementIdByCode(Utf8CP codeSpec, DgnElementId scopeElementId, Utf8StringCR value) const
    {
    return QueryElementIdByCode(GetDgnDb().CodeSpecs().QueryCodeSpecId(codeSpec), scopeElementId, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnElements::QueryElementIdByCode(CodeSpecId codeSpec, DgnElementId scopeElementId, Utf8StringCR value) const
    {
    CachedStatementPtr statement=GetStatement("SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE CodeSpecId=? AND CodeScopeId=? AND CodeValue=? LIMIT 1");
    statement->BindId(1, codeSpec);
    statement->BindId(2, scopeElementId);
    statement->BindText(3, value, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW != statement->Step()) ? DgnElementId() : statement->GetValueId<DgnElementId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericClassParamsProvider : IECSqlClassParamsProvider
    {
    DgnClassId m_classId;
    DgnElements const& m_elements;
    GenericClassParamsProvider(DgnClassId classId, DgnElements const& e) : m_classId(classId), m_elements(e) {}
    void _GetClassParams(ECSqlClassParamsR ecSqlParams) override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GenericClassParamsProvider::_GetClassParams(ECSqlClassParamsR ecSqlParams)
    {
    // *** WIP_AUTO_HANDLED_PROPERTIES: "ECInstanceId" is handled specially. It's in the table but not in the properties collection
    ecSqlParams.Add("ECInstanceId", ECSqlClassParams::StatementType::Insert);

    auto ecclass = m_elements.GetDgnDb().Schemas().GetClass(ECN::ECClassId(m_classId.GetValue()));
    AutoHandledPropertiesCollection props(*ecclass, m_elements.GetDgnDb(), ECSqlClassParams::StatementType::All, true);
    for (auto i = props.begin(); i != props.end(); ++i)
        {
        ecSqlParams.Add((*i)->GetName(), i.GetStatementType());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlClassParams const& DgnElements::GetECSqlClassParams(DgnClassId classId) const
    {
    BeMutexHolder _v(m_mutex);

    ECSqlClassParams& params = m_classParams[classId];
    if (!params.IsInitialized())
        {
        GenericClassParamsProvider provider(classId, *this);
        params.Initialize(provider);
        }
    return params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR DgnElements::GetSelectEcPropsECSql(ECSqlClassInfo& classInfo, ECN::ECClassCR ecclass) const
    {
    BeMutexHolder _v(m_mutex);  // guard lazy initialization of classInfo.m_selectEcProps

    if (!classInfo.m_selectEcProps.empty())
        return classInfo.m_selectEcProps;

    Utf8String props;
    Utf8CP comma = "";
    bvector<ECN::ECPropertyCP> autoHandledProperties;
    for (auto prop : AutoHandledPropertiesCollection(ecclass, GetDgnDb(), ECSqlClassParams::StatementType::Select, false))
        {
        Utf8StringCR propName = prop->GetName();
        props.append(comma).append("[").append(propName).append("]");
        comma = ",";
        }

    if (props.empty())
        return classInfo.m_selectEcProps = "";

    classInfo.m_selectEcProps = Utf8PrintfString("SELECT %s FROM %s WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter", 
                                                            props.c_str(), ecclass.GetECSqlName().c_str());
    return classInfo.m_selectEcProps;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR DgnElements::GetAutoHandledPropertiesSelectECSql(ECN::ECClassCR ecclass) const
    {
    ECSqlClassInfo& classInfo = FindClassInfo(DgnClassId(ecclass.GetId().GetValue())); // Note: This "Find" method will create a ClassInfo if necessary
    return GetSelectEcPropsECSql(classInfo, ecclass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlClassInfo& DgnElements::FindClassInfo(DgnClassId classId) const
    {
    BeMutexHolder _v(m_mutex);
    auto found = m_classInfos.find(classId);
    if (m_classInfos.end() != found)
        return found->second;

    ECSqlClassInfo& classInfo = m_classInfos[classId];
    ECSqlClassParams const& params = GetECSqlClassParams(classId);

    bool populated = params.BuildClassInfo(classInfo, GetDgnDb(), classId);
    BeAssert(populated);
    UNUSED_VARIABLE(populated);

    return classInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlClassInfo& DgnElements::FindClassInfo(DgnElementCR el) const
    {
    return FindClassInfo(el.GetElementClassId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElements::ElementSelectStatement DgnElements::GetPreparedSelectStatement(DgnElementR el) const
    {
    auto stmt = FindClassInfo(el).GetSelectStmt(GetDgnDb(), ECInstanceId(el.GetElementId().GetValue()));
    return ElementSelectStatement(stmt.get(), GetECSqlClassParams(el.GetElementClassId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr DgnElements::GetPreparedInsertStatement(DgnElementR el) const
    {
    // Not bothering to cache per class...use our general-purpose ECSql statement cache
    return FindClassInfo(el).GetInsertStmt(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr DgnElements::GetPreparedUpdateStatement(DgnElementR el) const
    {
    // Not bothering to cache per class...use our general-purpose ECSql statement cache
    return FindClassInfo(el).GetUpdateStmt(GetDgnDb(), ECInstanceId(el.GetElementId().GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::DropGraphicsForViewport(DgnViewportCR viewport)
    {
    m_tree->VisitElements([&viewport](DgnElementCR el)
        {
        auto geom = el.ToGeometrySource();
        if (nullptr != geom)
            {
            geom->Graphics().DropFor(viewport);
            return;
            }

        DgnGeometryPartCP part = el.ToGeometryPart();
        if (nullptr != part)
            part->Graphics().DropFor(viewport);
        });
    }

