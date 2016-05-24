/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/QueryView.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/ViewController.h>
#include "UpdatePlan.h"
#include <Bentley/BeThread.h>
#include <BeSQLite/RTreeMatch.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Displays %DgnElements from a SQL query. The query can combine
//! spatial criteria with business and graphic criteria.
//!
//! @remarks QueryView is also used to produce graphics for picking and for purposes other than display.
// @bsiclass                                                    Keith.Bentley   07/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnQueryView : CameraViewController, BeSQLite::VirtualSet
{
    DEFINE_T_SUPER(CameraViewController)

    friend struct DgnQueryQueue::Task;

    //=======================================================================================
    // The Ids of elements that are somehow treated specially for a DgnQueryView
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct SpecialElements
    {
        DgnElementIdSet m_always;
        DgnElementIdSet m_never;
        bool IsEmpty() const {return m_always.empty() && m_never.empty();}
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   05/16
    //=======================================================================================
    struct ElementsQuery
    {
        BeSQLite::CachedStatementPtr m_viewStmt;
        SpecialElements const* m_special;
        ClipPrimitiveCPtr m_activeVolume;
        int m_idCol = 0;
        bool TestElement(DgnElementId);
        bool IsNever(DgnElementId id) const {return m_special && m_special->m_never.Contains(id);}
        bool IsAlways(DgnElementId id) const {return m_special && m_special->m_always.Contains(id);}
        bool HasAlwaysList() const {return m_special && !m_special->m_always.empty();}
        DGNPLATFORM_EXPORT void Start(DgnQueryViewCR); //!< when this method is called the SQL string for the "ViewStmt" is obtained from the DgnQueryView supplied.
        ElementsQuery(SpecialElements const* special, ClipPrimitiveCP activeVolume) {m_special = (special && !special->IsEmpty()) ? special : nullptr; m_activeVolume=activeVolume;}
    };

    //=======================================================================================
    // A query that uses both the BeSQLite spatial index and a DgnElementId-based filter for a QueryView.
    // This object holds two statements - one for the spatial query and one that filters element, by id,
    // on the "other" criteria for a QueryView.
    // The Statements are retrieved from the statement cache and prepared/bound in the Start method.
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct SpatialQuery : ElementsQuery
    {
        bool m_doSkewTest = false;
        BeSQLite::CachedStatementPtr m_rangeStmt;
        BeSQLite::RTree3dVal m_boundingRange;    // only return entries whose range intersects this cube.
        BeSQLite::RTree3dVal m_backFace;
        Render::FrustumPlanes m_planes;
        Frustum m_frustum;
        DMatrix4d m_localToNpc;
        DVec3d m_viewVec;  // vector from front face to back face, for SkewScan
        DPoint3d m_cameraPosition;

        virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) = 0;
        DgnElementId StepRtree();
        bool SkewTest(BeSQLite::RTree3dValCP testRange);
        BeSQLite::RTreeMatchFunction::Within TestVolume(FrustumCR box, BeSQLite::RTree3dValCP);
        void Start(DgnQueryViewCR); //!< when this method is called the SQL string for the "ViewStmt" is obtained from the DgnQueryView supplied.
        void SetFrustum(FrustumCR);
        SpatialQuery(SpecialElements const* special, ClipPrimitiveCP activeVolume) : ElementsQuery(special, activeVolume) {}
    };

    //! Holds the results of a query.
    struct QueryResults : RefCounted<NonCopyableClass>
    {
        typedef bmultimap<double, DgnElementId> OcclusionScores;
        bool m_incomplete = false;
        OcclusionScores m_scores;
        uint32_t GetCount() const {return (uint32_t) m_scores.size();}
    };
    typedef RefCountedPtr<QueryResults> QueryResultsPtr;

    //=======================================================================================
    // This object is created on the Client thread and queued to the Query thread. It populates its
    // QueryResults with the set of n-best elements that satisfy both range and view criteria.
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct RangeQuery : SpatialQuery, DgnQueryQueue::Task
    {
        DEFINE_T_SUPER(SpatialQuery)
        bool m_depthFirst = false;
        bool m_cameraOn = false;
        bool m_testLOD = false;
        uint32_t m_orthogonalProjectionIndex;
        uint32_t m_count = 0;
        uint32_t m_hitLimit = 0;     // find this many "best" elements sorted by occlusion score
        uint64_t m_lastId = 0;
        double m_lodFilterNPCArea = 0.0;
        double m_minScore = 0.0;
        double m_lastScore = 0.0;
        DgnQueryView::QueryResultsPtr m_results;

        virtual void _Go() override;
        virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;
        void AddAlwaysDrawn(DgnQueryViewCR);
        void SetDepthFirst() {m_depthFirst=true;}
        void SetTestLOD(bool onOff) {m_testLOD=onOff;}
        void SetSizeFilter(DgnViewportCR, double size);
        bool ComputeNPC(DPoint3dR npcOut, DPoint3dCR localIn);
        bool ComputeOcclusionScore(double& score, FrustumCR);

    public:
        RangeQuery(DgnQueryViewCR, FrustumCR, DgnViewportCR, UpdatePlan::Query const& plan);
        DgnQueryView::QueryResultsPtr DoQuery();
        DgnQueryView::QueryResultsPtr GetResults() {return m_results;}
    };

    //=======================================================================================
    // The set of DgnElements that are contained in a scene. This is used when performing a progressive
    // update or heal of a view to determine which elements are already visible.
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct SceneMembers : RefCounted<DgnElementMap>, NonCopyableClass
    {
        bool m_complete = false;
        uint32_t m_progressiveTotal = 0;
    };
    typedef RefCountedPtr<SceneMembers> SceneMembersPtr;

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   03/16
    //=======================================================================================
    struct NonSceneQuery : RangeQuery
    {
        NonSceneQuery(DgnQueryViewR view, FrustumCR frustum, DgnViewportCR vp);
    };

    //=======================================================================================
    // A ProgressiveTask for a DgnQueryView that draws all of the elements that satisfy the query and range
    // criteria, but were too small to be in the scene.
    // @bsiclass                                                    Keith.Bentley   04/14
    //=======================================================================================
    struct ProgressiveTask : Dgn::ProgressiveTask
    {
        enum {SHOW_PROGRESS_INTERVAL = 1000}; // once per second.
        bool m_setTimeout = false;
        uint32_t m_thisBatch = 0;
        uint32_t m_batchSize = 0;
        uint64_t m_nextShow  = 0;
        NonSceneQuery m_rangeQuery;
        DgnQueryViewR m_view;
        explicit ProgressiveTask(DgnQueryViewR view, DgnViewportCR);
        virtual Completion _DoProgressive(ProgressiveContext& context, WantShow&) override;
    };

protected:
    bool m_noQuery = false;
    mutable bool m_abortQuery = false;
    Utf8String m_viewSQL;
    double m_sceneLODSize = 6.0; 
    double m_nonSceneLODSize = 7.0; 
    SceneMembersPtr m_scene;
    SpecialElements m_special;
    bset<Utf8String> m_copyrightMsgs;  // from reality models. Only keep unique ones
    ClipPrimitivePtr m_activeVolume;     //!< the active volume. If present, elements inside this volume may be treated specially
    mutable QueryResultsPtr m_results;

    void QueryModelExtents(FitContextR);
    void QueueQuery(DgnViewportR, UpdatePlan::Query const&);
    void AddtoSceneQuick(SceneContextR context, QueryResults& results);
    bool AbortRequested() const {return m_abortQuery;} //!< @private
    void SetAbortQuery(bool val) const {m_abortQuery=val;} //!< @private
    DgnQueryViewCP _ToQueryView() const override {return this;}
    DGNPLATFORM_EXPORT void _DoHeal(HealContext&) override;
    DGNPLATFORM_EXPORT bool _IsInSet(int nVal, BeSQLite::DbValue const*) const override;
    DGNPLATFORM_EXPORT void _InvalidateScene() override;
    DGNPLATFORM_EXPORT bool _IsSceneReady() const override;
    void _FillModels() override {} // query views do not load elements in advance
    DGNPLATFORM_EXPORT void _OnUpdate(DgnViewportR vp, UpdatePlan const& plan) override;
    DGNPLATFORM_EXPORT void _OnAttachedToViewport(DgnViewportR) override;
    DGNPLATFORM_EXPORT void _CreateScene(SceneContextR) override;
    DGNPLATFORM_EXPORT void _CreateTerrain(TerrainContextR context) override;
    DGNPLATFORM_EXPORT void _VisitAllElements(ViewContextR) override;
    DGNPLATFORM_EXPORT void _DrawView(ViewContextR context) override;
    DGNPLATFORM_EXPORT void _OnCategoryChange(bool singleEnabled) override;
    DGNPLATFORM_EXPORT void _ChangeModelDisplay(DgnModelId modelId, bool onOff) override;
    DGNPLATFORM_EXPORT FitComplete _ComputeFitRange(struct FitContext&) override;
    DGNPLATFORM_EXPORT AxisAlignedBox3d _GetViewedExtents() const override final; // Always DgnDb::Units().GetProjectExtents() for QueryViews, don't allow override.
    DGNPLATFORM_EXPORT void _DrawDecorations(DecorateContextR) override;

public:
    //! @param dgndb  The DgnDb for the view
    //! @param viewId Id of view to be displayed in this DgnQueryView
    DGNPLATFORM_EXPORT DgnQueryView(DgnDbR dgndb, DgnViewId viewId);
    DGNPLATFORM_EXPORT ~DgnQueryView();

    //! Get the Level-of-Detail filtering size for scene creation for this DgnQueryView. This is the size, in pixels, of one side of a square. 
    //! Elements whose aabb projects onto the view an area less than this box are skippped during scene creation.
    double GetSceneLODSize() const {return m_sceneLODSize;}
    void SetSceneLODSize(double val) {m_sceneLODSize=val;} //!< see GetSceneLODSize

    //! Get the Level-of-Detail filtering size for non-scene (background) elements this DgnQueryView. This is the size, in pixels, of one side of a square. 
    //! Elements whose aabb projects onto the view an area less than this box are skippped during background-element display.
    double GetNonSceneLODSize() const {return m_nonSceneLODSize;}
    void SetNonSceneLODSize(double val) {m_nonSceneLODSize=val;} //!< see GetNonSceneLODSize

    //! Get the list of elements that are always drawn
    DgnElementIdSet const& GetAlwaysDrawn() {return m_special.m_always;}

    //! Establish a set of elements that are always drawn in the view.
    //! @param[in] exclusive If true, only these elements are drawn
    DGNPLATFORM_EXPORT void SetAlwaysDrawn(DgnElementIdSet const&, bool exclusive);

    //! Empty the set of elements that are always drawn
    DGNPLATFORM_EXPORT void ClearAlwaysDrawn();

    //! Establish a set of elements that are never drawn in the view.
    DGNPLATFORM_EXPORT void SetNeverDrawn(DgnElementIdSet const&);

    //! Get the list of elements that are never drawn.
    //! @remarks An element in the never-draw list is excluded regardless of whether or not it is
    //! in the always-draw list. That is, the never-draw list gets priority over the always-draw list.
    DgnElementIdSet const& GetNeverDrawn() {return m_special.m_never;}

    //! Empty the set of elements that are never drawn
    DGNPLATFORM_EXPORT void ClearNeverDrawn();

    //! Requests that any active or pending queries for this view be canceled, optionally not returning until the request is satisfied
    DGNPLATFORM_EXPORT void RequestAbort(bool waitUntilFinished);

    DGNPLATFORM_EXPORT void AssignActiveVolume(ClipPrimitiveR volume);
    DGNPLATFORM_EXPORT void ClearActiveVolume();
    ClipPrimitivePtr GetActiveVolume() const {return m_activeVolume;}
};

END_BENTLEY_DGN_NAMESPACE
