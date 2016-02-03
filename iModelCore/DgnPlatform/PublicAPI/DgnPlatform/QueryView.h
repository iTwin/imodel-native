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
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct OcclusionScorer
{
    DMatrix4d   m_localToNpc;
    DPoint3d    m_cameraPosition;
    double      m_lodFilterNPCArea;
    uint32_t    m_orthogonalProjectionIndex;
    bool        m_cameraOn;
    bool        m_testLOD;
    void SetTestLOD(bool val) {m_testLOD=val;}
    void InitForViewport(DgnViewportCR viewport, double minimumSizePixels);
    bool ComputeEyeSpanningRangeOcclusionScore(double* score, DPoint3dCP rangeCorners);
    bool ComputeNPC(DPoint3dR npcOut, DPoint3dCR localIn);
    bool ComputeOcclusionScore(double* score, bool& overlap, bool& spansEyePlane, DPoint3dCP localCorners);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct RTreeTester
{
    BeSQLite::CachedStatementPtr m_rangeStmt;
    virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) = 0;
    Utf8String GetAcceptSql();
    DgnElementId StepRtree();
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/14
//=======================================================================================
struct RTreeFilter : RTreeTester
    {
    bool                    m_doSkewtest = false;
    bool                    m_doOcclusionScore = false;
    BeSQLite::RTree3dVal    m_boundingRange;    // only return entries whose range intersects this cube.
    BeSQLite::RTree3dVal    m_frontFaceRange;
    OcclusionScorer         m_scorer;
    DVec3d                  m_viewVec;  // vector from front face to back face
    ClipVectorPtr           m_clips;
    DgnElementIdSet const*  m_exclude;
    DgnDbR                  m_dgndb;

    bool AllPointsClippedByOnePlane(ConvexClipPlaneSetCR cps, size_t nPoints, DPoint3dCP points) const;
    void SetClipVector(ClipVectorR clip) {m_clips = &clip;}
    void SetFrustum(FrustumCR);
    void SetViewport(DgnViewportCR, double minimumSizeScreenPixels, double frustumScale);
    bool SkewTest(BeSQLite::RTree3dValCP);
    RTreeFilter(DgnDbR db, DgnElementIdSet const* exclude=nullptr);
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct RTreeFitFilter : RTreeTester
    {
    DRange3d m_fitRange;
    DRange3d m_lastRange;

    DGNPLATFORM_EXPORT virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;

public:
    RTreeFitFilter() {m_fitRange = DRange3d::NullRange();}
    DRange3dCR GetRange() const {return m_fitRange;}
    };

//=======================================================================================
//! Displays \ref DgnElementGroup from a SQL query. The query can combine 
//! spatial criteria with business and graphic criteria.
//!
//! @remarks QueryView is also used to produce graphics for picking and for purposes other than display.
// @bsiclass                                                    Keith.Bentley   07/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnQueryView : CameraViewController, BeSQLite::VirtualSet
{
    DEFINE_T_SUPER (CameraViewController)

    typedef bmultimap<double, DgnElementId> OcclusionScores;
    
    //! Holds the results of a query.
    struct QueryResults : RefCountedBase
    {
        bool            m_incomplete = false;
        uint32_t        m_count = 0;
        OcclusionScores m_scores;
    };
    typedef RefCountedPtr<QueryResults> QueryResultsPtr;

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   12/11
    //=======================================================================================
    struct Filter : RTreeFilter
    {
        uint32_t                m_hitLimit;
        uint64_t                m_lastId;
        double                  m_minScore;
        double                  m_lastScore;
        DgnElementIdSet const*  m_alwaysDraw;
        QueryResultsPtr         m_results;

        virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;
        void AcceptElement(DgnElementId elementId);

    public:
        Filter(DgnDbR db, QueryResults& results, uint32_t hitLimit, DgnElementIdSet const* alwaysDraw, DgnElementIdSet const* exclude);
    };
    
    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   01/16
    //=======================================================================================
    struct AllElementsFilter : RTreeFilter
    {
        AllElementsFilter(DgnDbR db, DgnElementIdSet const* exclude) : RTreeFilter(db, exclude) {}
        bool AcceptElement(ViewContextR context, DgnElementId elementId);
        virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/14
    //=======================================================================================
    struct ProgressiveFilter : ProgressiveTask, AllElementsFilter
    {
        enum {SHOW_PROGRESS_INTERVAL = 1000}; // once per second.
        bool     m_setTimeout = false;
        uint32_t m_total = 0;
        uint32_t m_thisBatch = 0;
        uint32_t m_batchSize = 0;
        uint64_t m_nextShow  = 0;
        DgnElementIdSet m_inScene;
        DgnQueryViewR m_view;
        ProgressiveFilter(DgnQueryViewR view, DgnElementIdSet const* exclude) : m_view(view), AllElementsFilter(view.GetDgnDb(), exclude) {}
        virtual Completion _DoProgressive(SceneContext& context, WantShow&) override;
    };

protected:
    bool        m_forceNewQuery;    //!< If true, before doing the next view update, repopulate the QueryModel with the result of the query 
    bool        m_noQuery;          //!< If true, *only* draw the "always drawn" list - do not query for other elements
    bool        m_needProgressiveDisplay;
    bool        m_abortQuery;
    Frustum     m_startQueryFrustum;
    Frustum     m_saveQueryFrustum;
    DgnElementIdSet m_alwaysDrawn;
    DgnElementIdSet m_neverDrawn;
    QueryResultsPtr  m_results;

    void SearchIdSet(DgnElementIdSet& idList, Filter& filter);
    void SearchRangeTree(Filter& filter, uint32_t timeout);
    void QueryModelExtents(DRange3dR, DgnViewportR);
    bool FrustumChanged(DgnViewportCR vp) const;
    void QueueQuery(DgnViewportR, UpdatePlan const&);

    DGNPLATFORM_EXPORT virtual bool _IsInSet(int nVal, BeSQLite::DbValue const*) const override;
    virtual void _FillModels() override {} // query views do not load elements in advance
    DGNPLATFORM_EXPORT virtual void _OnAttachedToViewport(DgnViewportR) override;

    //! @param[in] viewport    The viewport that will display the graphics
    //! @param[in] plan The update plan
    DGNPLATFORM_EXPORT virtual void _OnUpdate(DgnViewportR viewport, UpdatePlan const& plan) override;

    //! Called when the visibility of a category is changed.
    DGNPLATFORM_EXPORT virtual void _OnCategoryChange(bool singleEnabled) override;

    //! Called when the display of a model is changed on or off
    //! @param modelId  The model to turn on or off.
    //! @param onOff    If true, elements in the model are candidates for display; else elements in the model are not displayed.
    DGNPLATFORM_EXPORT virtual void _ChangeModelDisplay(DgnModelId modelId, bool onOff) override;

    //! @remarks It not normally necessary for apps to override this function.
    DGNPLATFORM_EXPORT virtual void _CreateScene(SceneContextR context) override;

    //! Draw the elements in the query model.
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
    //! The premise of a QueryView is that it displays only a small subset of the potential elements in a DgnDb, limited to a maximum number of 
    //! elements and bytes. Obviously, the criteria that determines which elements are loaded at a given time must combine business logic (e.g. elements that meet 
    //! a certain property test), display logic (e.g. which models and categories are turned on), plus spatial criteria (i.e. the position of the camera).
    //! Further, assuming more than the maximum number of elements meet all the search criteria, the candidate elements should be sorted such that the "best"
    //! set of elements are returned.
    //! <p> This method is used to obtain an SQL statement to achieve that goal. The "best set" of elements are determined 
    //! using a spatial scoring algorithm that traverses the persistent range tree (an RTree in SQLite), and scores elements based on an approximate number of
    //! pixels occluded by its axis aligned bounding box (AABB - aka "range box"). The SQL returned by the base-class implementation of this method contains 
    //! logic to affect that purpose, plus filters for category and models. To add additional, application-specific criteria to the query, override this method, call
    //! T_Super::_GetRTreeMatchSql, and append your filters as additional "AND" clauses on that string. Then, return the new combined SQL statement.
    DGNPLATFORM_EXPORT virtual Utf8String _GetQuery() const;

    void BindModelAndCategory(BeSQLite::StatementR stmt) const;

    //! Construct the view controller.                          
    //! @param dgndb  The DgnDb for the view
    //! @param viewId Id of view to be displayed
    DGNPLATFORM_EXPORT DgnQueryView(DgnDbR dgndb, DgnViewId viewId);
    DGNPLATFORM_EXPORT ~DgnQueryView();

    void SetForceNewQuery(bool newValue) {m_forceNewQuery = newValue;}

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
    void DoQuery(DgnViewportCR, UpdatePlan::Query const&, StopWatch&);
};

END_BENTLEY_DGN_NAMESPACE
