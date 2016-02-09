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
//! Displays \ref DgnElementGroup from a SQL query. The query can combine 
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
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct RTreeQuery
    {
        BeSQLite::CachedStatementPtr m_rangeStmt;
        BeSQLite::CachedStatementPtr m_viewStmt;
        DgnElementIdSet const* m_neverDrawn;
        int m_idCol = 0;
        virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) = 0;
        DgnElementId StepRtree();
        bool TestElement(DgnElementId);
        void Start(DgnQueryViewCR);
        RTreeQuery(DgnElementIdSet const* ignore) : m_neverDrawn(ignore) 
            {
            if (m_neverDrawn && m_neverDrawn->empty())
                m_neverDrawn=nullptr;
            }
    };

    typedef bmultimap<double, DgnElementId> OcclusionScores;
    
    //! Holds the results of a query.
    struct QueryResults : RefCounted<NonCopyableClass>
    {
        bool m_incomplete = false;
        OcclusionScores m_scores;
        uint32_t GetCount() const {return (uint32_t) m_scores.size();}
    };
    typedef RefCountedPtr<QueryResults> QueryResultsPtr;

    struct Clips : ConvexClipPlaneSet
    {
        void AddFrustum(FrustumCR frustum);
        BeSQLite::RTreeMatchFunction::Within TestBox(DPoint3d*);
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct RangeQuery : RTreeQuery, DgnQueryQueue::Task
    {
        DEFINE_T_SUPER(RTreeQuery)
        bool        m_depthFirst = false;
        bool        m_cameraOn = false;
        bool        m_testLOD = false;
        bool        m_doSkewTest = false;
        uint32_t    m_orthogonalProjectionIndex;
        uint32_t    m_count = 0;
        uint32_t    m_hitLimit = 0;
        uint64_t    m_lastId = 0;
        BeSQLite::RTree3dVal m_boundingRange;    // only return entries whose range intersects this cube.
        BeSQLite::RTree3dVal m_backFace;
        
        Clips       m_clips;
        Frustum     m_frustum;
        DMatrix4d   m_localToNpc;
        DVec3d      m_viewVec;  // vector from front face to back face
        DPoint3d    m_cameraPosition;
        double      m_lodFilterNPCArea = 0.0;
        double      m_minScore = 0.0;
        double      m_lastScore = 0.0;

        virtual void _Go() override;
        virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;
        bool SkewTest(BeSQLite::RTree3dValCP testRange);
        void SetDepthFirst() {m_depthFirst=true;}
        void SetTestLOD(bool onOff) {m_testLOD=onOff;}
        void SetSizeFilter(DgnViewportCR, double size);
        bool ComputeNPC(DPoint3dR npcOut, DPoint3dCR localIn);
        bool ComputeOcclusionScore(double* score, bool& overlap, bool& spansEyePlane, DPoint3dCP localCorners);
        void SetFrustum(FrustumCR);
    
    public:
        RangeQuery(DgnQueryViewCR, FrustumCR, DgnViewportCR, UpdatePlan::Query const& plan);
        DgnQueryView::QueryResultsPtr DoQuery();
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct SceneMembers : RefCounted<DgnElementIdSet>, NonCopyableClass
    {
    };
    typedef RefCountedPtr<SceneMembers> SceneMembersPtr;

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/14
    //=======================================================================================
    struct NonScene : ProgressiveTask
    {
        enum {SHOW_PROGRESS_INTERVAL = 1000}; // once per second.
        bool     m_setTimeout = false;
        uint32_t m_total = 0;
        uint32_t m_thisBatch = 0;
        uint32_t m_batchSize = 0;
        uint64_t m_nextShow  = 0;
        SceneMembersPtr m_scene;
        RangeQuery m_rangeQuery;
        DgnQueryViewR m_view;
        explicit NonScene(DgnQueryViewR view, DgnViewportCR, SceneMembers& scene);// : m_view(view), m_scene(&scene) {m_rangeQuery.SetTestLOD(true);}
        virtual Completion _DoProgressive(SceneContext& context, WantShow&) override;
    };

protected:
    bool m_forceNewQuery;
    mutable bool m_abortQuery;
    Utf8String m_viewSQL;

    DgnElementIdSet m_alwaysDrawn;
    DgnElementIdSet m_neverDrawn;
    mutable QueryResultsPtr m_results;

    void SearchIdSet(DgnElementIdSet&, RangeQuery&);
    void QueryModelExtents(DRange3dR, DgnViewportR);
    void QueueQuery(DgnViewportR, UpdatePlan::Query const&);
    void AddtoSceneQuick(SceneContextR context, SceneMembers&, QueryResults& results);
    StatusInt VisitElement(ViewContextR context, DgnElementId, bool allowLoad);
    DGNPLATFORM_EXPORT virtual bool _IsInSet(int nVal, BeSQLite::DbValue const*) const override;
    DGNPLATFORM_EXPORT virtual void _InvalidateScene() override;
    DGNPLATFORM_EXPORT virtual bool _IsSceneReady() const override;
    virtual void _FillModels() override {} // query views do not load elements in advance
    DGNPLATFORM_EXPORT void _OnUpdate(DgnViewportR vp, UpdatePlan const& plan) override;
    DGNPLATFORM_EXPORT virtual void _OnAttachedToViewport(DgnViewportR) override;

    //! Called when the visibility of a category is changed.
    DGNPLATFORM_EXPORT virtual void _OnCategoryChange(bool singleEnabled) override;

    //! Called when the display of a model is changed on or off
    //! @param modelId  The model to turn on or off.
    //! @param onOff    If true, elements in the model are candidates for display; else elements in the model are not displayed.
    DGNPLATFORM_EXPORT virtual void _ChangeModelDisplay(DgnModelId modelId, bool onOff) override;

    DGNPLATFORM_EXPORT virtual void _CreateScene(SceneContextR) override;

    //! Draw the elements in the query view.
    DGNPLATFORM_EXPORT virtual void _DrawView(ViewContextR context) override;

    //! Allow the supplied ViewContext to visit every element in the view, not just the best elements in the query model.
    DGNPLATFORM_EXPORT void _VisitAllElements(ViewContextR) override;

    //! Compute the range of the elements and graphics in the QueryView.
    //! @remarks This function may also load elements to determine the range.
    //! @param[out] range    the computed range 
    //! @param[in]  viewport the viewport that will display the graphics
    //! @param[in]  params   options for computing the range.
    //! @return \a true if the returned \a range is complete. Otherwise the caller will compute the tightest fit for all loaded elements.
    DGNPLATFORM_EXPORT virtual FitComplete _ComputeFitRange(DRange3dR range, DgnViewportR viewport, FitViewParamsR params) override;

public:
    //! Construct the view controller.                          
    //! @param dgndb  The DgnDb for the view
    //! @param viewId Id of view to be displayed
    DGNPLATFORM_EXPORT DgnQueryView(DgnDbR dgndb, DgnViewId viewId);
    DGNPLATFORM_EXPORT ~DgnQueryView();

    //! Get the list of elements that are always drawn
    DgnElementIdSet const& GetAlwaysDrawn() {return m_alwaysDrawn;}

    //! Establish a set of elements that are always drawn in the view.
    DGNPLATFORM_EXPORT void SetAlwaysDrawn(DgnElementIdSet const&, bool exclusive);

    DGNPLATFORM_EXPORT void ClearAlwaysDrawn();

    //! Get the list of elements that are never drawn.
    //! @remarks An element in the never-draw list is excluded regardless of whether or not it is 
    //! in the always-draw list. That is, the never-draw list gets priority over the always-draw list.
    DgnElementIdSet const& GetNeverDrawn() {return m_neverDrawn;}

    DGNPLATFORM_EXPORT void SetNeverDrawn(DgnElementIdSet const&);
    DGNPLATFORM_EXPORT void ClearNeverDrawn();

    bool AbortRequested() const {return m_abortQuery;} //!< @private
    void SetAbortQuery(bool val) const {m_abortQuery=val;} //!< @private
    void ClearQueryResults();

    //! Requests that any active or pending processing of the model be canceled, optionally not returning until the request is satisfied
    void RequestAbort(bool waitUntilFinished);
    void WaitUntilFinished(CheckStop* checkStop); //!< @private
};

END_BENTLEY_DGN_NAMESPACE
