/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnRangeTree.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGN_NAMESPACE

namespace RangeIndex
{
static double ROUND_TOWARDS = (1.0 - 1.0/8388608.0);  // Round towards zero
static double ROUND_AWAY =    (1.0 + 1.0/8388608.0);  // Round away from zero

static float roundDown(double d)
    {
    float f = (float)d;
    if (f>d)
        {
        f = (float) (d * (d<0 ? ROUND_AWAY : ROUND_TOWARDS));
        BeAssert(f<d);
        }
    return f;
    }

static float roundUp(double d)
    {
    float f = (float)d;
    if (f<d)
        {
        f = (float) (d * (d<0 ? ROUND_TOWARDS : ROUND_AWAY));
        BeAssert(f>d);
        }
    return f;
    }

//=======================================================================================
//! A single-precision 3d axis-aligned range for a RangeIndex. As double-precision DRange3d values are
//! converted to a Box, the low values are rounded down and the high values are rounded up. In this
//! way a Box will always contain all of the space enclosed by the double-precsision range, but
//! will not necessarily be as tight. This is merely to save memory.
// @bsiclass                                                    Keith.Bentley   11/16
//=======================================================================================
struct Box
{
    FPoint3d low;
    FPoint3d high;
    Box(DRange3dCR box)
        {
        low.x  = roundDown(box.low.x);
        low.y  = roundDown(box.low.y);
        low.z  = roundDown(box.low.z);
        high.y = roundUp(box.high.y);
        high.x = roundUp(box.high.x);
        high.z = roundUp(box.high.z);
        }

    void Init() {low.x = 1.0; high.x = -1.0; low.y = 1.0; high.y = -1.0; low.z = 1.0; high.z = -1.0;}
    Box() {Init();}
    bool IsNull() const {return low.x>high.x || low.y>high.y || low.z>high.z;}
    DRange3d ToRange3d() const {return DRange3d::From(low.x, low.y, low.z, high.x, high.y, high.z);}
};

DEFINE_POINTER_SUFFIX_TYPEDEFS(Box);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Entry);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Tree);

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct Entry
{
    Box m_range;
    DgnElementId m_id;
    DgnCategoryId m_category;
    Entry(BoxCR range, DgnElementId id, DgnCategoryId category) : m_range(range), m_id(id), m_category(category) {}
    Entry() {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct Traverser
{
    virtual ~Traverser() {}
    virtual bool  _CheckRangeTreeNode(BoxCR, bool is3d) const = 0;   // true == process node

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
    // @bsiclass                                                    Keith.Bentley   04/15
    //=======================================================================================
    struct Node
    {
        enum class NodeType {Internal, Leaf};

    protected:
        Box m_nodeRange;
        InternalNode* m_parent;
        NodeType m_type;
        bool m_is3d;
        bool m_sloppy;

    public:
        Node(NodeType type, bool is3d) : m_type(type), m_is3d(is3d), m_parent(nullptr) {ClearRange();}
        void SetParent(InternalNode* parent) {m_parent = parent;}
        LeafNode* ToLeaf() const {return m_type != NodeType::Internal ? (LeafNode*) this : nullptr;}
        bool IsLeaf() const {return nullptr != ToLeaf();}
        bool IsSloppy() const {return m_sloppy;}
        void ClearRange() {m_sloppy=false; m_nodeRange.Init();}
        void ValidateRange();
        BoxCR GetRange() {ValidateRange(); return m_nodeRange;}
        BoxCR GetRangeCR() {return m_nodeRange;}
        bool Overlaps(BoxCR range) const;
        bool CompletelyContains(BoxCR range) const;
        Traverser::Stop Traverse(Traverser&, bool is3d);
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
        Traverser::Stop Traverse(Traverser&, bool is3d);
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
        Node* ChooseBestNode(BoxCP pRange, TreeR root);
        void AddInternalNode(Node* child, TreeR root);
        void SplitInternalNode(TreeR);
        void DropRange(BoxCR range);
        void DropNode(Node* child, TreeR root);
        void ValidateInternalRange();
        size_t GetEntryCount() const {return m_endChild - m_firstChild;}
        void ClearChildren() {m_endChild = m_firstChild; ClearRange();}
        Traverser::Stop Traverse(Traverser&, bool is3d);
        size_t GetElementCount();
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
    size_t m_internalNodeSize;
    size_t m_leafNodeSize;

    InternalNode* AllocateInternalNode() {return new (m_internalNodes.AllocateNode()) InternalNode(m_is3d);}
    LeafNode* AllocateLeafNode() {return new (m_leafNodes.AllocateNode()) LeafNode(m_is3d);}
    void FreeInternalNode(InternalNode* node) {m_internalNodes.FreeNode(node);}
    void FreeLeafNode(LeafNode* node) {m_leafNodes.FreeNode(node);}

public:
    void LoadTree(DgnModelCR);
    BoxCP GetExtents() {return  m_root ? &m_root->GetRange() : nullptr;}
    Tree(bool is3d, size_t leafSize);
    Node* GetRoot(){return m_root;}
    size_t GetInternalNodeSize() {return m_internalNodeSize;}
    size_t GetLeafNodeSize() {return m_leafNodeSize;}
    void SetNodeSizes(size_t internalNodeSize, size_t leafNodeSize);
    bool Is3d() const {return m_is3d;}
    Traverser::Stop Traverse(Traverser&);
    void AddEntry(Entry const&);
    void AddGeomElement(GeometrySourceCR geom){AddEntry(Entry(geom.CalculateRange3d(), geom.ToElement()->GetElementId(), geom.GetCategoryId()));}
    StatusInt RemoveElement(DgnElementId);
};

} // end RangeIndex namespace
END_BENTLEY_DGN_NAMESPACE
