/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnRangeTree.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include    <BeSQLite/RTreeMatch.h>

DGNPLATFORM_TYPEDEFS (DgnRangeTree)
DGNPLATFORM_TYPEDEFS (DRTNode)
DGNPLATFORM_TYPEDEFS (DRTInternalNode)
DGNPLATFORM_TYPEDEFS (DRTLeafNode)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2009
+===============+===============+===============+===============+===============+======*/
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
    DRTNode (NodeType type, bool is3d) { m_type = type; m_is3d=is3d; m_parent = NULL; ClearRange();}
    void SetParent (DRTInternalNodeP parent) { m_parent = parent; }
    DRTLeafNodeP ToLeaf () const { return m_type != NodeType::Internal ? (DRTLeafNodeP) this : NULL; }
    bool IsLeaf() const {return NULL != ToLeaf();}
    bool IsSloppy() const {return m_sloppy;}
    void ClearRange ();// {m_sloppy=false; m_nodeRange.init();}
    DRange3dCR GetRange() {ValidateRange(); return m_nodeRange;}
    DGNPLATFORM_EXPORT void ValidateRange();
    size_t GetEntryCount();
    DRange3dCR GetRangeCR() {return m_nodeRange;}
    bool Overlaps (DRange3dCR range) const;
    bool CompletelyContains (DRange3dCR range) const;
    RangeMatchStatus TraverseDgnElements (struct DRTDgnElementTraverser& traverser, bool is3d);
    RangeMatchStatus Traverse (ElemRangeIndex::Traverser&, bool);
};

typedef DRange3dCR (*PFGetRangeForElement) (DgnElementP elRef);

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2009
+===============+===============+===============+===============+===============+======*/
struct DRTLeafNode : DRTNode
{
    GeometricElementCP*  m_endChild;
    GeometricElementCP   m_firstChild[1];

    DRTLeafNode (bool is3d, NodeType nodeType = NodeType::Leaf) : DRTNode (nodeType, is3d) {m_endChild = m_firstChild;}

    void ClearChildren () { m_endChild = m_firstChild; ClearRange();}
    void SplitLeafNode (DgnRangeTreeR);
    void AddElementToLeaf (DRange3dCR range, GeometricElementCR element, DgnRangeTreeR);
    void ValidateLeafRange();
    bool DropElementFromLeaf (DRange3dCR range, GeometricElementCP elementRef, DgnRangeTreeR);
    size_t GetEntryCount() const {return m_endChild - m_firstChild;}
    RangeMatchStatus Traverse (ElemRangeIndex::Traverser&, bool);
    RangeMatchStatus TraverseDgnElements (struct DRTDgnElementTraverser& traverser, bool is3d);
    PFGetRangeForElement GetRangeFunction ();
};

//=================================================================================**//**
// @bsiclass                                                     RayBentley      10/2009
//===============+===============+===============+===============+===============+======*/
struct DRTInternalNode : DRTNode
{
    DRTNodeP*  m_endChild;
    DRTNodeP   m_firstChild[1];

    DRTInternalNode(bool is3d) : DRTNode (NodeType::Internal, is3d) {m_endChild = m_firstChild;}

    void AddElement (DRange3dCR range, GeometricElementCR element, DgnRangeTreeR);
    DRTNodeP ChooseBestNode (DRange3dCP pRange, DgnRangeTreeR root);
    void AddInternalNode (DRTNodeP child, DgnRangeTreeR root);
    void SplitInternalNode (DgnRangeTreeR);
    bool DropElement (DRange3dCR range, GeometricElementCP elementRef, DgnRangeTreeR);
    void DropRange (DRange3dCR range);
    void DropNode (DRTNodeP child, DgnRangeTreeR root);
    void ValidateInternalRange();
    size_t GetEntryCount() const {return m_endChild - m_firstChild;}
    void ClearChildren () { m_endChild = m_firstChild; ClearRange();}
    RangeMatchStatus Traverse (ElemRangeIndex::Traverser&, bool);
    RangeMatchStatus TraverseDgnElements (struct DRTDgnElementTraverser& traverser, bool is3d);
    size_t GetLeafCount ();
    size_t GetNodeCount ();
    size_t GetElementCount ();
    size_t GetMaxChildDepth ();
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

    DgnMemoryPool<DRTLeafNode,512>        m_leafNodes;
    DgnMemoryPool<DRTInternalNode,512>    m_internalNodes;

    double      m_elementsPerSecond;
    DRTNodeP    m_root;
    bool        m_is3d;
    size_t      m_internalNodeSize;
    size_t      m_leafNodeSize;

    void LoadTree (DgnModelCR);
    DRTInternalNodeP AllocateInternalNode () {return new (m_internalNodes.AllocateNode()) DRTInternalNode(m_is3d);}
    DRTLeafNodeP AllocateLeafNode (DRTNode::NodeType nodeType = DRTNode::NodeType::Leaf) {return new (m_leafNodes.AllocateNode()) DRTLeafNode(m_is3d, nodeType);}
    void FreeInternalNode (DRTInternalNodeP node) {m_internalNodes.FreeNode(node);}
    void FreeLeafNode (DRTLeafNodeP node) {m_leafNodes.FreeNode(node);}

    virtual void _AddElement(GeometricElementCR element, DRange3dCR, int& stamp);
    virtual StatusInt _RemoveElement(GeometricElementCR element, DRange3dCR, int& stamp);
    virtual DRange3dCP _GetDgnModelRange();

public:
    DgnRangeTree(DgnModelCR, size_t leafSize);
    virtual ~DgnRangeTree();
    DRTNodeP GetRoot(){return m_root;}
    size_t GetInternalNodeSize() {return m_internalNodeSize;}
    size_t GetLeafNodeSize() {return m_leafNodeSize;}
    void SetNodeSizes (size_t internalNodeSize, size_t leafNodeSize);

    DGNPLATFORM_EXPORT void ProcessOcclusionSorted (ViewContextR, DgnModelP, RangeTreeProgressMonitor* monitor, bool doFrustumCull, uint32_t* timeOut);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct OcclusionScorer
{
    DMatrix4d   m_localToNpc;
    DPoint3d    m_cameraPosition;
    double      m_lodFilterNPCArea;
    uint32_t    m_orthogonalProjectionIndex;
    bool        m_cameraOn;
    void InitForViewport(DgnViewportCR viewport, double minimumSizePixels);
    bool ComputeEyeSpanningRangeOcclusionScore (double* score, DPoint3dCP rangeCorners, bool doFrustumCull);
    bool ComputeNPC (DPoint3dR npcOut, DPoint3dCR localIn);
    bool ComputeOcclusionScore (double* score, bool& overlap, bool& spansEyePlane, bool& eliminatedByLOD, DPoint3dCP localCorners, bool doFrustumCull);
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    10/13
//=======================================================================================
struct OverlapScorer
{
    BeSQLite::RTree3dVal m_boundingRange;
    void Initialize(DRange3dCR boundingRange);
    bool ComputeScore (double* score, BeSQLite::RTree3dValCR range);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/14
//=======================================================================================
struct RtreeViewFilter : BeSQLite::RTreeMatch
    {
    bool                    m_doSkewtest;
    Frustum                 m_frustum;
    double                  m_minimumSizePixels;
    BeSQLite::RTree3dVal    m_boundingRange;    // only return entries whose range intersects this cube.
    BeSQLite::RTree3dVal    m_frontFaceRange;
    OcclusionScorer         m_scorer;
    uint32_t                m_nCalls;
    uint32_t                m_nScores;
    uint32_t                m_nSkipped;
    DVec3d                  m_viewVec;  // vector from front face to back face
    ClipVectorPtr           m_clips;
    DgnElementIdSet const*  m_exclude;

    bool AllPointsClippedByOnePlane(ConvexClipPlaneSetCR cps, size_t nPoints, DPoint3dCP points) const;
    void SetClipVector(ClipVectorR clip) {m_clips = &clip;}
    bool SkewTest(BeSQLite::RTree3dValCP);
    DGNPLATFORM_EXPORT RtreeViewFilter(DgnViewportCR, BeSQLiteDbR db, double minimumSizeScreenPixels, DgnElementIdSet const* exclude);
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnDbRTree3dViewFilter : RtreeViewFilter
    {
    typedef bmultimap<double, int64_t> T_OcclusionScoreMap;

    struct SecondaryFilter
        {
        OverlapScorer       m_scorer;
        uint32_t            m_hitLimit;
        uint32_t            m_occlusionMapCount;
        double              m_occlusionMapMinimum;
        T_OcclusionScoreMap m_occlusionScoreMap;
        double              m_lastScore;
        };

    bool                    m_passedPrimaryTest;
    bool                    m_passedSecondaryTest;
    bool                    m_useSecondary;
    bool                    m_eliminatedByLOD;
    uint32_t                m_hitLimit;
    uint32_t                m_occlusionMapCount;
    int64_t                 m_lastId;
    T_OcclusionScoreMap     m_occlusionScoreMap;
    double                  m_occlusionMapMinimum;
    double                  m_lastScore;
    SecondaryFilter         m_secondaryFilter;
    ICheckStopP             m_checkStop;
    DgnElementIdSet const*  m_alwaysDraw;

    virtual int _TestRange(BeSQLite::RTreeMatch::QueryInfo const&) override;
    virtual void _StepAggregate(BeSQLite::DbFunction::Context*, int nArgs, BeSQLite::DbValue* args) override {RangeAccept(args[0].GetValueInt64());}
    void RangeAccept(int64_t elementId) ;
    double MaxOcclusionScore();

public:
    DGNPLATFORM_EXPORT DgnDbRTree3dViewFilter(DgnViewportCR, ICheckStopP, BeSQLiteDbR db, uint32_t hitLimit, double minimumSizeScreenPixels, DgnElementIdSet const* alwaysDraw, DgnElementIdSet const* neverDraw);
    void SetChceckStop(ICheckStopP checkStop) {m_checkStop = checkStop;}
    void InitializeSecondaryTest(DRange3dCR volume, uint32_t hitLimit);
    void GetStats(uint32_t& nAcceptCalls, uint32_t&nScores) { nAcceptCalls = m_nCalls; nScores = m_nScores; }
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/14
//=======================================================================================
struct ProgressiveViewFilter : RefCounted<IProgressiveDisplay>, RtreeViewFilter
{
    friend struct QueryViewController;

    ViewContextP                 m_context;
    uint32_t                     m_nThisPass;
    uint32_t                     m_nLastPass;
    bool                         m_drewElementThisPass;
    bool                         m_setTimeout;
    DgnModelR                    m_existing;
    DgnDbR                       m_dgndb;
    uint64_t                     m_elementReleaseTrigger;
    uint64_t                     m_purgeTrigger;
    BeSQLite::CachedStatementPtr m_rangeStmt;
    static const double          s_purgeFactor ;
    ProgressiveViewFilter(DgnViewportCR vp, DgnDbR project, DgnModelR existing, DgnElementIdSet const* exclude, uint64_t maxMemory, BeSQLite::CachedStatement* stmt)
         : RtreeViewFilter (vp, project, 0.0, exclude), m_dgndb(project), m_existing(existing), m_elementReleaseTrigger(maxMemory), m_purgeTrigger(maxMemory), m_rangeStmt(stmt) 
        {
        m_nThisPass = m_nLastPass = 0;
        m_drewElementThisPass = m_setTimeout = false;
        m_context=NULL;
        }  
    ~ProgressiveViewFilter();

    virtual int _TestRange (BeSQLite::RTreeMatch::QueryInfo const&) override;
    virtual void _StepAggregate(BeSQLite::DbFunction::Context*, int nArgs, BeSQLite::DbValue* args) override;
    virtual bool _WantTimeoutSet(uint32_t& limit) override;
    virtual Completion _Process(ViewContextR context) override;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct DgnDbRTreeFitFilter : BeSQLite::RTreeMatch
    {
    DRange3d m_fitRange;
    DRange3d m_lastRange;

    DGNPLATFORM_EXPORT virtual int _TestRange(BeSQLite::RTreeMatch::QueryInfo const&) override;
    virtual void _StepAggregate(BeSQLite::DbFunction::Context*, int nArgs, BeSQLite::DbValue* args) override {m_fitRange.Extend (m_lastRange);}

public:
    DgnDbRTreeFitFilter(BeSQLiteDbR db) : RTreeMatch (db) {m_fitRange = DRange3d::NullRange();}
    DRange3dCR GetRange () const {return m_fitRange;}
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
