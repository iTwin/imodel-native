/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/QueryModel.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnModel.h"
#include "UpdatePlan.h"
#include <Bentley/BeTimeUtilities.h>
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
/**
A QueryModel is a virtual DgnModel that holds @ref DgnElementGroup loaded from the database according to a custom query criteria.
A QueryModel caches the results of the query.

A QueryModel is used in conjunction with a QueryViewController to display the results of the query. 
Applications do not directly deal with QueryModel's. Instead, the query that populates them is supplied by a QueryViewController.

*/
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct QueryModel : SpatialModel
{
    friend struct QueryViewController;
    typedef bmultimap<double, DgnElementId> OcclusionScores;
    
    //! Holds the results of a query.
    struct Results : RefCountedBase
    {
       friend struct Processor;
        bool            m_incomplete = false;
        uint32_t        m_count = 0;
        OcclusionScores m_scores;

    };
    typedef RefCountedPtr<Results> ResultsPtr;

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
        QueryModelR             m_model;
        ResultsPtr              m_results;

        bool CheckAbort();
        virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;
        void AcceptElement(DgnElementId elementId);

    public:
        Filter(QueryModelR model, Results& results, uint32_t hitLimit, DgnElementIdSet const* alwaysDraw, DgnElementIdSet const* exclude);
    };
    
    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   01/16
    //=======================================================================================
    struct AllElementsFilter : RTreeFilter
    {
        QueryModelR  m_model;
        AllElementsFilter(QueryModelR model, DgnElementIdSet const* exclude) : RTreeFilter(model.GetDgnDb(), exclude), m_model(model) {}
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
        ProgressiveFilter(QueryModelR model, DgnElementIdSet const* exclude) : AllElementsFilter(model, exclude) {}
        virtual Completion _DoProgressive(SceneContext& context, WantShow&) override;
    };


#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    struct Processor;
    //! Holds the results of a QueryModel's query.
    struct Results : RefCountedBase
    {
       friend struct Processor;

    private:
        Results() : m_needsProgressive(false), m_drawnBeforePurge(0) { }

    public:
        bvector<DgnElementCPtr> m_elements;
        bool     m_needsProgressive;
        uint32_t m_drawnBeforePurge;
        uint32_t GetCount() const {return (uint32_t) m_elements.size();}
    };
#endif


    //! Executes a query on a separate thread to load elements for a QueryModel
    struct Processor : RefCounted<NonCopyableClass>
    {
        //! Parameters specifying how the processor    should execute its query
        struct Params
        {
            QueryModelR         m_model;
            DgnViewportCR       m_vp;
            Utf8String          m_searchSql;
            UpdatePlan::Query   m_plan;
            uint64_t            m_maxMemory;
            DgnElementIdSet*    m_highPriority;
            DgnElementIdSet*    m_neverDraw;
            bool                m_highPriorityOnly;
            ClipVectorPtr       m_clipVector;

            Params(QueryModelR model, DgnViewportCR vp, Utf8StringCR sql, UpdatePlan::Query const& plan, uint64_t maxMem, DgnElementIdSet* highPriority,
                    DgnElementIdSet* neverDraw, bool highPriorityOnly, ClipVectorP clipVector)
                : m_model(model), m_vp(vp), m_searchSql(sql), m_plan(plan), m_maxMemory(maxMem), m_highPriority(highPriority),
                  m_neverDraw(neverDraw), m_highPriorityOnly(highPriorityOnly), m_clipVector(clipVector) {}
        };

    protected:
        Params      m_params;

        void ProcessRequest();
        void SearchIdSet(DgnElementIdSet& idList, Filter& filter);
        void SearchRangeTree(Filter& filter);
        bool LoadElements(OcclusionScores& scores, bvector<DgnElementCPtr>& elements); // return false if we halted before finishing iteration
        void DoQuery(StopWatch&);

    public:
        Processor(Params const& params) : m_params(params) {}
        void Query(StopWatch&);
        uint32_t GetDelayAfter() {return m_params.m_plan.GetDelayAfter();}
        QueryModelR GetModel() const {return m_params.m_model;}
        bool IsForModel(QueryModelCR model) const {return &m_params.m_model == &model;}
    };

    typedef RefCountedPtr<Processor> ProcessorPtr;

    //! Each DgnDb has an associated QueryModel::Queue upon which requests for processing QueryModels can be enqueued.
    //! The requests execute on a separate thread.
    struct Queue
    {
    private:
        enum class State { Active, TerminateRequested, Terminated };

        DgnDbR                 m_db;
        BeConditionVariable    m_cv;
        std::deque<ProcessorPtr>  m_pending;
        ProcessorPtr           m_active;
        State                  m_state;

        bool WaitForWork();
        void Process();
        THREAD_MAIN_DECL Main(void* arg);

    public:
        Queue(DgnDbR db);

        void Terminate();

        //! Enqueue a request to execute the query for a QueryModel
        DGNPLATFORM_EXPORT void Add(Processor::Params const& params);

        //! Cancel any pending requests to process the specified model.
        //! @param[in] model The model whose processing is to be canceled
        DGNPLATFORM_EXPORT void RemovePending(QueryModelR model);

        //! Suspends the calling thread until the specified model is in the idle state
        DGNPLATFORM_EXPORT void WaitForIdle();

        DGNPLATFORM_EXPORT bool IsIdle() const;
    };

private:
    bool        m_abortQuery;
    ResultsPtr  m_results;

    DGNPLATFORM_EXPORT explicit QueryModel(DgnDbR);
    virtual void _FillModel() override {} // QueryModels are never filled.

public:
    bool AbortRequested() const {return m_abortQuery;} //!< @private
    void SetAbortQuery(bool val) {m_abortQuery=val;} //!< @private
    void ClearQueryResults();

    //! Requests that any active or pending processing of the model be canceled, optionally not returning until the request is satisfied
    void RequestAbort(bool waitUntilFinished);
    void WaitUntilFinished(CheckStop* checkStop); //!< @private
};

END_BENTLEY_DGN_NAMESPACE
