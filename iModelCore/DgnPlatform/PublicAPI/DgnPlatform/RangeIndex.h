/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGN_NAMESPACE

namespace RangeIndex
{

static const double ROUND_TOWARDS = (1.0 - 1.0/8388608.0);  // Round towards zero
static const double ROUND_AWAY =    (1.0 + 1.0/8388608.0);  // Round away from zero

//=======================================================================================
//! A single-precision 3d axis-aligned range for a RangeIndex. As double-precision DRange3d values are
//! converted to a Box, the low values are rounded down and the high values are rounded up. In this
//! manner a Box will always contain all of the space enclosed by the double-precision range, but
//! will not necessarily be as tight. This is merely to save memory.
// @bsiclass                                                    Keith.Bentley   11/16
//=======================================================================================
struct FBox
{
    //! convert a double to a float, ensuring that the float value is equal or lower than the double value
    static float RoundDown(double d) {float f = (float) d; return (f<d) ? f : (float) (d * (d<0 ? ROUND_AWAY : ROUND_TOWARDS));}

    //! convert a double to a float, ensuring that the float value is equal or higher than the double value
    static float RoundUp(double d) {float f = (float) d; return (f>d) ? f : (float) (d * (d<0 ? ROUND_TOWARDS : ROUND_AWAY));}

    FPoint3d m_low;
    FPoint3d m_high;

    FBox(DRange3dCR box)
        {
        m_low.x  = RoundDown(box.low.x);
        m_low.y  = RoundDown(box.low.y);
        m_low.z  = RoundDown(box.low.z);
        m_high.y = RoundUp(box.high.y);
        m_high.x = RoundUp(box.high.x);
        m_high.z = RoundUp(box.high.z);
        }

    void Invalidate() {m_low.x = m_low.y = m_low.z = std::numeric_limits<float>::max(); m_high.x = m_high.y = m_high.z = -std::numeric_limits<float>::max();}
    FBox(FPoint3d low, FPoint3d high) : m_low(low), m_high(high) {}
    FBox(float xlow, float ylow, float zlow, float xhigh, float yhigh, float zhigh) {m_low.x=xlow; m_low.y=ylow; m_low.z=zlow; m_high.x=xhigh; m_high.y=yhigh; m_high.z=zhigh;}
    FBox() {Invalidate();}
    bool IsNull() const {return m_low.x>m_high.x || m_low.y>m_high.y || m_low.z>m_high.z;}
    DRange3d ToRange3d() const {return DRange3d::From(m_low.x, m_low.y, m_low.z, m_high.x, m_high.y, m_high.z);}
    bool IntersectsWith(FBox const& rhs) const
        {
        return m_low.x <= rhs.m_high.x && m_low.y <= rhs.m_high.y && m_low.z <= rhs.m_high.z
            && rhs.m_low.x <= m_high.x && rhs.m_low.y <= m_high.y && rhs.m_low.z <= m_high.z;
        }
};

DEFINE_POINTER_SUFFIX_TYPEDEFS(FBox);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Entry);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Tree);

//=======================================================================================
//! An entry in a range index. 
// @bsiclass                                                    Keith.Bentley   11/16
//=======================================================================================
struct Entry
{
    DgnElementId m_id;
    DgnCategoryId m_category;
    FBox m_range;
    Entry(FBoxCR range, DgnElementId id, DgnCategoryId category) : m_range(range), m_id(id), m_category(category) {}
    Entry() {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct Traverser
{
    virtual ~Traverser() {}
    virtual bool _AbortOnWriteRequest() const {return true;}

    enum class Accept : bool {Yes=1, No=0,};
    virtual Accept _CheckRangeTreeNode(FBoxCR, bool is3d) const = 0;   // true == process node

    enum class Stop {No= 0, Yes= 1,};
    virtual Stop _VisitRangeTreeEntry(EntryCR) = 0;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/10
//=======================================================================================
struct Tree
{
    struct InternalNode;
    struct LeafNode;

    //=======================================================================================
    //! On construction block until no write active. Then hold read lock until destruction. 
    //! Note that there can be more than one simultaneous readers.
    // @bsiclass                                                    Keith.Bentley   09/16
    //=======================================================================================
    struct ReadLock
    {
        TreeCR m_tree;
        ReadLock(TreeCR tree) : m_tree(tree) {BeMutexHolder holder(tree.m_cv.GetMutex()); while (tree.m_writeActive) tree.m_cv.InfiniteWait(holder); ++tree.m_readers;}
        ~ReadLock() {{BeMutexHolder holder(m_tree.m_cv.GetMutex()); --m_tree.m_readers; BeAssert(m_tree.m_readers>=0);} m_tree.m_cv.notify_all();}
    };

    //=======================================================================================
    //! On construction block until no readers. Then hold write lock until destruction. 
    //! There can only be one writer.
    // @bsiclass                                                    Keith.Bentley   09/16
    //=======================================================================================
    struct WriteLock
    {
        TreeR m_tree;
        WriteLock(TreeR tree) : m_tree(tree) {BeMutexHolder holder(tree.m_cv.GetMutex()); while (tree.m_readers>0){tree.m_writeRequest=true; tree.m_cv.InfiniteWait(holder);} tree.m_writeRequest=false; BeAssert(!tree.m_writeActive); tree.m_writeActive=true;}
        ~WriteLock() {{BeMutexHolder holder(m_tree.m_cv.GetMutex()); BeAssert(m_tree.m_writeActive); m_tree.m_writeActive=false; } m_tree.m_cv.notify_all();}
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/15
    //=======================================================================================
    struct Node
    {
        enum class NodeType {Internal, Leaf};

    protected:
        FBox m_nodeRange;
        InternalNode* m_parent;
        NodeType m_type;
        bool m_is3d;

    public:
        Node(NodeType type, bool is3d) : m_type(type), m_is3d(is3d), m_parent(nullptr) {ClearRange();}
        void SetParent(InternalNode* parent) {m_parent = parent;}
        LeafNode* ToLeaf() const {return m_type != NodeType::Internal ? (LeafNode*)const_cast<Node*>(this) : nullptr;}
        bool IsLeaf() const {return nullptr != ToLeaf();}
        void ClearRange() {m_nodeRange.Invalidate();}
        FBoxCR GetRange() {return m_nodeRange;}
        FBoxCR GetRangeCR() {return m_nodeRange;}
        bool Overlaps(FBoxCR range) const;
        bool CompletelyContains(FBoxCR range) const;
        Traverser::Stop Traverse(Traverser&, TreeCR tree, bool is3d);
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/15
    //=======================================================================================
    struct LeafNode : Node
    {
        Entry* m_endChild;
        Entry m_firstChild[1];

        LeafNode(bool is3d) : Node(NodeType::Leaf, is3d) {m_endChild = m_firstChild;}
        void ClearChildren() {m_endChild = m_firstChild; ClearRange();}
        void SplitLeafNode(TreeR);
        void AddEntryToLeaf(Entry const&, TreeR);
        void ValidateLeafRange();
        bool DropElement(DgnElementId, TreeR);
        size_t GetEntryCount() const {return m_endChild - m_firstChild;}
        EntryCP FindElement(DgnElementId) const;
        Traverser::Stop Traverse(Traverser&, TreeCR tree, bool is3d);
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/15
    //=======================================================================================
    struct InternalNode : Node
    {
        Node** m_endChild;
        Node* m_firstChild[1];

        InternalNode(bool is3d) : Node(NodeType::Internal, is3d) {m_endChild = m_firstChild;}
        void AddEntry(Entry const&, TreeR);
        Node* ChooseBestNode(FBoxCP pRange, TreeR root);
        void AddInternalNode(Node* child, TreeR root);
        void SplitInternalNode(TreeR);
        void DropRange(FBoxCR range);
        void DropNode(Node* child, TreeR root);
        void ValidateInternalRange();
        size_t GetEntryCount() const {return m_endChild - m_firstChild;}
        void ClearChildren() {m_endChild = m_firstChild; ClearRange();}
        Traverser::Stop Traverse(Traverser&, TreeCR tree, bool is3d);
        DGNPLATFORM_EXPORT size_t GetElementCount();
    };

private:
    friend struct InternalNode;
    friend struct LeafNode;
    typedef bmap<DgnElementId,LeafNode*> LeafIdx;

    DgnMemoryPool<LeafNode,128> m_leafNodes;
    DgnMemoryPool<InternalNode,512> m_internalNodes;
    LeafIdx m_leafIdx;      // map to the leaf holding each entry
    Node* m_root = nullptr;
    bool m_is3d;
    bool m_writeActive = false;
    bool m_writeRequest = false;
    mutable int m_readers = 0;
    size_t m_internalNodeSize;
    size_t m_leafNodeSize;
    mutable BentleyApi::BeConditionVariable m_cv;

    InternalNode* AllocateInternalNode() {return new (m_internalNodes.AllocateNode()) InternalNode(m_is3d);}
    LeafNode* AllocateLeafNode() {return new (m_leafNodes.AllocateNode()) LeafNode(m_is3d);}
    void FreeInternalNode(InternalNode* node) {m_internalNodes.FreeNode(node);}
    void FreeLeafNode(LeafNode* node) {m_leafNodes.FreeNode(node);}

public:
    size_t DebugElementCount() const {return m_root ? ((InternalNode*) m_root)->GetElementCount() : 0;} //! @private
    size_t DebugAllocation() const {return m_leafNodes.GetMemoryAllocated() + m_internalNodes.GetMemoryAllocated();} //! @private

    FBox GetExtents() {return m_root ? m_root->GetRange() : FBox();}
    DGNPLATFORM_EXPORT Tree(bool is3d, size_t leafSize);
    Node* GetRoot(){return m_root;}
    size_t GetInternalNodeSize() {return m_internalNodeSize;}
    size_t GetLeafNodeSize() {return m_leafNodeSize;}
    void SetNodeSizes(size_t internalNodeSize, size_t leafNodeSize);
    bool Is3d() const {return m_is3d;}
    size_t GetCount() const {return m_leafIdx.size();}

    struct Iterator :  LeafIdx::const_iterator
    {
        Iterator(LeafIdx::const_iterator it) : LeafIdx::const_iterator(it) {}
        DgnElementId GetElementId(){return (*this)->first;}
        EntryCP GetEntry(){return (*this)->second->FindElement((*this)->first);}
    };

    typedef Iterator const_iterator;
    const_iterator begin() const {return m_leafIdx.begin();}
    const_iterator end() const {return m_leafIdx.end();}

    DGNPLATFORM_EXPORT Traverser::Stop Traverse(Traverser&);

    DGNPLATFORM_EXPORT void AddEntry(Entry const&);

    //! Find an element in the range index and return the Entry information.
    //! @param[in] id The id of the element to find
    //! @return the Entry for the specified element. Will be nullptr if the element is not in the index.
    DGNPLATFORM_EXPORT EntryCP FindElement(DgnElementId id) const;

    //! Add a new geometric element into the range index.
    //! @param[in] geom the element to add to the range index.
    DGNPLATFORM_EXPORT void AddElement(GeometrySourceCR geom) {if (geom.HasGeometry()) AddEntry(Entry(geom.CalculateRange3d(), geom.ToElement()->GetElementId(), geom.GetCategoryId()));}

    //! Remove an element from the range index.
    //! @param[in] id The id of the element to remove
    //! @return SUCCESS if the element was removed. ERROR if the the id was not in the range index.
    DGNPLATFORM_EXPORT StatusInt RemoveElement(DgnElementId id);
};

} // end RangeIndex namespace
END_BENTLEY_DGN_NAMESPACE
