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
DGNPLATFORM_TYPEDEFS (DRTNode)
DGNPLATFORM_TYPEDEFS (DRTInternalNode)
DGNPLATFORM_TYPEDEFS (DRTLeafNode)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

enum class RangeMatch
{
    Ok           = 0,
    Aborted      = 1,
    TooManyHits  = 2,
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct RangeTreeTraverser
{
    virtual bool _CheckRangeIndexNode(DRange3dCR, bool is3d, bool isElement) const = 0;   // true == process node
    virtual RangeMatch _VisitRangeTreeElem(GeometricElementCP) = 0;    // true == keep going, false == stop traversal
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct RangeTreeProgressMonitor  
{ 
    virtual bool _MonitorProgress(double fractionComplete) = 0; 
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DRTEntry
{
    DRange3d            m_range;
    GeometricElementCP  m_elm;
    DRTEntry(DRange3dCR range, GeometricElementCR elm) : m_range(range), m_elm(&elm) {}
    DRTEntry() : m_elm(nullptr) {}
};


//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DRTNode
{
    enum class NodeType {Internal, Leaf};

protected:
    DRange3d         m_nodeRange;
    DRTInternalNodeP m_parent;
    NodeType         m_type;
    bool             m_is3d;
    bool             m_sloppy;

public:
    DRTNode(NodeType type, bool is3d) { m_type = type; m_is3d=is3d; m_parent = NULL; ClearRange();}
    void SetParent(DRTInternalNodeP parent) { m_parent = parent; }
    DRTLeafNodeP ToLeaf() const { return m_type != NodeType::Internal ? (DRTLeafNodeP) this : NULL; }
    bool IsLeaf() const {return NULL != ToLeaf();}
    bool IsSloppy() const {return m_sloppy;}
    void ClearRange();// {m_sloppy=false; m_nodeRange.init();}
    DRange3dCR GetRange() {ValidateRange(); return m_nodeRange;}
    DGNPLATFORM_EXPORT void ValidateRange();
    size_t GetEntryCount();
    DRange3dCR GetRangeCR() {return m_nodeRange;}
    bool Overlaps(DRange3dCR range) const;
    bool CompletelyContains(DRange3dCR range) const;
    RangeMatch Traverse(RangeTreeTraverser&, bool);
};


//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DRTLeafNode : DRTNode
{
    DRTEntry*  m_endChild;
    DRTEntry   m_firstChild[1];

    DRTLeafNode(bool is3d, NodeType nodeType = NodeType::Leaf) : DRTNode(nodeType, is3d) {m_endChild = m_firstChild;}

    void ClearChildren() { m_endChild = m_firstChild; ClearRange();}
    void SplitLeafNode(DgnRangeTreeR);
    void AddElementToLeaf(DRTEntry const&, DgnRangeTreeR);
    void ValidateLeafRange();
    bool DropElementFromLeaf(DRTEntry const&, DgnRangeTreeR);
    size_t GetEntryCount() const {return m_endChild - m_firstChild;}
    RangeMatch Traverse(RangeTreeTraverser&, bool);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DRTInternalNode : DRTNode
{
    DRTNodeP*  m_endChild;
    DRTNodeP   m_firstChild[1];

    DRTInternalNode(bool is3d) : DRTNode(NodeType::Internal, is3d) {m_endChild = m_firstChild;}

    void AddElement(DRTEntry const&, DgnRangeTreeR);
    DRTNodeP ChooseBestNode(DRange3dCP pRange, DgnRangeTreeR root);
    void AddInternalNode(DRTNodeP child, DgnRangeTreeR root);
    void SplitInternalNode(DgnRangeTreeR);
    bool DropElement(DRTEntry const&, DgnRangeTreeR);
    void DropRange(DRange3dCR range);
    void DropNode(DRTNodeP child, DgnRangeTreeR root);
    void ValidateInternalRange();
    size_t GetEntryCount() const {return m_endChild - m_firstChild;}
    void ClearChildren() { m_endChild = m_firstChild; ClearRange();}
    RangeMatch Traverse(RangeTreeTraverser&, bool);
    size_t GetLeafCount();
    size_t GetNodeCount();
    size_t GetElementCount();
    size_t GetMaxChildDepth();
    ;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/10
//=======================================================================================
struct DgnRangeTree
{
private:
    friend struct DRTInternalNode;
    friend struct DRTLeafNode;
    friend struct ElemRangeIndex;

    DgnMemoryPool<DRTLeafNode,128>     m_leafNodes;
    DgnMemoryPool<DRTInternalNode,512> m_internalNodes;

    double      m_elementsPerSecond;
    DRTNodeP    m_root;
    bool        m_is3d;
    size_t      m_internalNodeSize;
    size_t      m_leafNodeSize;

    DRTInternalNodeP AllocateInternalNode() {return new (m_internalNodes.AllocateNode()) DRTInternalNode(m_is3d);}
    DRTLeafNodeP AllocateLeafNode(DRTNode::NodeType nodeType = DRTNode::NodeType::Leaf) {return new (m_leafNodes.AllocateNode()) DRTLeafNode(m_is3d, nodeType);}
    void FreeInternalNode(DRTInternalNodeP node) {m_internalNodes.FreeNode(node);}
    void FreeLeafNode(DRTLeafNodeP node) {m_leafNodes.FreeNode(node);}

    void AddElement(DRTEntry const&);

public:
    void LoadTree(DgnModelCR);
    DRange3dCP GetFullRange() {return  m_root ? &m_root->GetRange() : nullptr;}

    DgnRangeTree(bool is3d, size_t leafSize);
    DRTNodeP GetRoot(){return m_root;}
    size_t GetInternalNodeSize() {return m_internalNodeSize;}
    size_t GetLeafNodeSize() {return m_leafNodeSize;}
    void SetNodeSizes(size_t internalNodeSize, size_t leafNodeSize);
    bool Is3d() const {return m_is3d;}
    RangeMatch FindMatches(RangeTreeTraverser&);
    void AddGeomElement(GeometricElementCR geom){AddElement(DRTEntry(geom._GetRange3d(), geom));}
    StatusInt RemoveElement(DRTEntry const&);

    DGNPLATFORM_EXPORT void ProcessOcclusionSorted(ViewContextR, DgnModelP, RangeTreeProgressMonitor* monitor, bool doFrustumCull, uint32_t* timeOut);
};


END_BENTLEY_DGNPLATFORM_NAMESPACE
