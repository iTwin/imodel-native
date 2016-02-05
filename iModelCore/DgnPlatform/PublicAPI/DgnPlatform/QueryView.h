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

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct RTreeQuery
    {
        BeSQLite::CachedStatementPtr m_rangeStmt;
        virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) = 0;
        DgnElementId StepRtree();
        void Init(DgnDbR db);
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

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct RangeQuery : RTreeQuery
    {
        DEFINE_T_SUPER(RTreeQuery)
        bool        m_cameraOn = false;
        bool        m_testLOD = false;
        bool        m_doSkewtest = false;
        bool        m_doOcclusionScore = false;
        uint32_t    m_orthogonalProjectionIndex;
        uint32_t    m_count = 0;
        uint32_t    m_hitLimit = 0;
        uint64_t    m_lastId = 0;
        BeSQLite::RTree3dVal m_boundingRange;    // only return entries whose range intersects this cube.
        BeSQLite::RTree3dVal m_frontFaceRange;
        ClipVectorPtr   m_clips;
        DMatrix4d   m_localToNpc;
        DVec3d      m_viewVec;  // vector from front face to back face
        DPoint3d    m_cameraPosition;
        double      m_lodFilterNPCArea = 0.0;
        double      m_minScore = 1.0e20;
        double      m_lastScore = 0.0;

        virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;
        void SetTestLOD(bool val) {m_testLOD=val;}
        void InitForViewport(DgnViewportCR viewport, double minimumSizePixels);
        bool ComputeEyeSpanningRangeOcclusionScore(double* score, DPoint3dCP rangeCorners);
        bool ComputeNPC(DPoint3dR npcOut, DPoint3dCR localIn);
        bool ComputeOcclusionScore(double* score, bool& overlap, bool& spansEyePlane, DPoint3dCP localCorners);
        bool AllPointsClippedByOnePlane(ConvexClipPlaneSetCR cps, size_t nPoints, DPoint3dCP points) const;
        void SetClipVector(ClipVectorR clip) {m_clips = &clip;}
        void SetFrustum(FrustumCR);
        void SetViewport(DgnViewportCR, double minimumSizeScreenPixels, double frustumScale);
        bool SkewTest(BeSQLite::RTree3dValCP);
    
    public:
        void Init(DgnDbR db) {T_Super::Init(db); m_count=0;}
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/14
    //=======================================================================================
    struct ProgressiveElements : ProgressiveTask
    {
        enum {SHOW_PROGRESS_INTERVAL = 1000}; // once per second.
        bool     m_setTimeout = false;
        uint32_t m_total = 0;
        uint32_t m_thisBatch = 0;
        uint32_t m_batchSize = 0;
        uint64_t m_nextShow  = 0;
        RangeQuery m_rangeQuery;
        DgnElementIdSet m_inScene;
        DgnQueryViewR m_view;
        explicit ProgressiveElements(DgnQueryViewR view) : m_view(view) {m_rangeQuery.SetTestLOD(true);}
        virtual Completion _DoProgressive(SceneContext& context, WantShow&) override;
    };

protected:
    bool m_abortQuery;
    BeSQLite::Statement m_query;
    Frustum m_startQueryFrustum;
    Frustum m_saveQueryFrustum;
    DgnElementIdSet m_alwaysDrawn;
    DgnElementIdSet m_neverDrawn;
    QueryResultsPtr m_results;

    void SearchIdSet(DgnElementIdSet&, RangeQuery&);
    void QueryModelExtents(DRange3dR, DgnViewportR);
    bool FrustumChanged(DgnViewportCR vp) const;
    void QueueQuery(DgnViewportR, UpdatePlan const&);
    bool AddtoSceneQuick(SceneContextR context, ProgressiveElements& progressive, QueryResults& results);
    StatusInt VisitElement(ViewContextR context, DgnElementId);
    DGNPLATFORM_EXPORT virtual bool _IsInSet(int nVal, BeSQLite::DbValue const*) const override;
    DGNPLATFORM_EXPORT virtual void _InvalidateScene() override;
    virtual void _FillModels() override {} // query views do not load elements in advance
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

    DGNPLATFORM_EXPORT virtual bool _TestElement(DgnElementId);

public:
    QueryResultsPtr QueryByRange(DgnViewportCR vp, UpdatePlan::Query const& plan);

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
    void SetAbortQuery(bool val) {m_abortQuery=val;} //!< @private
    void ClearQueryResults();

    //! Requests that any active or pending processing of the model be canceled, optionally not returning until the request is satisfied
    void RequestAbort(bool waitUntilFinished);
    void WaitUntilFinished(CheckStop* checkStop); //!< @private
};

END_BENTLEY_DGN_NAMESPACE
