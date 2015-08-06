/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnRangeTree.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

DGNPLATFORM_TYPEDEFS (DgnRangeTree)

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/10
//=======================================================================================
struct DgnRangeTree
{
    struct InternalNode;
    struct LeafNode;

    enum class Match
    {
        Ok           = 0,
        Aborted      = 1,
        TooManyHits  = 2,
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/15
    //=======================================================================================
    struct Traverser
    {
        virtual ~Traverser() {}
        virtual bool  _CheckRangeTreeNode(DRange3dCR, bool is3d) const = 0;   // true == process node
        virtual Match _VisitRangeTreeElem(GeometricElementCP, DRange3dCR) = 0;    // true == keep going, false == stop traversal
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/15
    //=======================================================================================
    struct ProgressMonitor
    {
        virtual bool _MonitorProgress(double fractionComplete) = 0;
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/15
    //=======================================================================================
    struct Entry
    {
        DRange3d            m_range;
        GeometricElementCP  m_elm;
        Entry(DRange3dCR range, GeometricElementCR elm) : m_range(range), m_elm(&elm) {}
        Entry() : m_elm(nullptr) {}
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/15
    //=======================================================================================
    struct Node
    {
        enum class NodeType {Internal, Leaf};

    protected:
        DRange3d      m_nodeRange;
        InternalNode* m_parent;
        NodeType      m_type;
        bool          m_is3d;
        bool          m_sloppy;

    public:
        Node(NodeType type, bool is3d) : m_type(type), m_is3d(is3d), m_parent(nullptr) {ClearRange();}
        void SetParent(InternalNode* parent) { m_parent = parent; }
        LeafNode* ToLeaf() const { return m_type != NodeType::Internal ? (LeafNode*) this : nullptr; }
        bool IsLeaf() const {return nullptr != ToLeaf();}
        bool IsSloppy() const {return m_sloppy;}
        void ClearRange() {m_sloppy=false; m_nodeRange.Init();}
        void ValidateRange();
        DRange3dCR GetRange() {ValidateRange(); return m_nodeRange;}
        size_t GetEntryCount();
        DRange3dCR GetRangeCR() {return m_nodeRange;}
        bool Overlaps(DRange3dCR range) const;
        bool CompletelyContains(DRange3dCR range) const;
        Match Traverse(Traverser&, bool is3d);
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/15
    //=======================================================================================
    struct LeafNode : Node
    {
        Entry*  m_endChild;
        Entry   m_firstChild[1];

        LeafNode(bool is3d) : Node(NodeType::Leaf, is3d) {m_endChild = m_firstChild;}
        void ClearChildren() { m_endChild = m_firstChild; ClearRange();}
        void SplitLeafNode(DgnRangeTreeR);
        void AddElementToLeaf(Entry const&, DgnRangeTreeR);
        void ValidateLeafRange();
        bool DropElementFromLeaf(Entry const&, DgnRangeTreeR);
        size_t GetEntryCount() const {return m_endChild - m_firstChild;}
        Match Traverse(Traverser&, bool is3d);
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/15
    //=======================================================================================
    struct InternalNode : Node
    {
        Node**  m_endChild;
        Node*   m_firstChild[1];

        InternalNode(bool is3d) : Node(NodeType::Internal, is3d) {m_endChild = m_firstChild;}
        void AddElement(Entry const&, DgnRangeTreeR);
        Node* ChooseBestNode(DRange3dCP pRange, DgnRangeTreeR root);
        void AddInternalNode(Node* child, DgnRangeTreeR root);
        void SplitInternalNode(DgnRangeTreeR);
        bool DropElement(Entry const&, DgnRangeTreeR);
        void DropRange(DRange3dCR range);
        void DropNode(Node* child, DgnRangeTreeR root);
        void ValidateInternalRange();
        size_t GetEntryCount() const {return m_endChild - m_firstChild;}
        void ClearChildren() { m_endChild = m_firstChild; ClearRange();}
        Match Traverse(Traverser&, bool is3d);
        size_t GetLeafCount();
        size_t GetNodeCount();
        size_t GetElementCount();
        size_t GetMaxChildDepth();
    };

private:
    friend struct InternalNode;
    friend struct LeafNode;

    DgnMemoryPool<LeafNode,128>     m_leafNodes;
    DgnMemoryPool<InternalNode,512> m_internalNodes;

    double      m_elementsPerSecond;
    Node*       m_root;
    bool        m_is3d;
    size_t      m_internalNodeSize;
    size_t      m_leafNodeSize;

    InternalNode* AllocateInternalNode() {return new (m_internalNodes.AllocateNode()) InternalNode(m_is3d);}
    LeafNode* AllocateLeafNode() {return new (m_leafNodes.AllocateNode()) LeafNode(m_is3d);}
    void FreeInternalNode(InternalNode* node) {m_internalNodes.FreeNode(node);}
    void FreeLeafNode(LeafNode* node) {m_leafNodes.FreeNode(node);}

public:
    void LoadTree(DgnModelCR);
    DRange3dCP GetFullRange() {return  m_root ? &m_root->GetRange() : nullptr;}
    DgnRangeTree(bool is3d, size_t leafSize);
    Node* GetRoot(){return m_root;}
    size_t GetInternalNodeSize() {return m_internalNodeSize;}
    size_t GetLeafNodeSize() {return m_leafNodeSize;}
    void SetNodeSizes(size_t internalNodeSize, size_t leafNodeSize);
    bool Is3d() const {return m_is3d;}
    Match FindMatches(Traverser&);
    void AddElement(Entry const&);
    void AddGeomElement(GeometricElementCR geom){AddElement(Entry(geom.CalculateRange3d(), geom));}
    StatusInt RemoveElement(Entry const&);

    DGNPLATFORM_EXPORT void ProcessOcclusionSorted(ViewContextR, DgnModelP, ProgressMonitor* monitor, bool doFrustumCull, uint32_t* timeOut);
};

END_BENTLEY_DGN_NAMESPACE
