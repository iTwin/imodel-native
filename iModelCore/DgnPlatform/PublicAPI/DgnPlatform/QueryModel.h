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

//=======================================================================================
// @bsiclass                                                    John.Gooding    10/13
//=======================================================================================
struct OverlapScorer
{
    BeSQLite::RTree3dVal m_boundingRange;
    void Initialize(DRange3dCR boundingRange);
    bool ComputeScore(double* score, BeSQLite::RTree3dValCR range);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct RTreeTester
{
    virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) = 0;
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

    bool AllPointsClippedByOnePlane(ConvexClipPlaneSetCR cps, size_t nPoints, DPoint3dCP points) const;
    void SetClipVector(ClipVectorR clip) {m_clips = &clip;}
    void SetFrustum(FrustumCR);
    void SetViewport(DgnViewportCR, double minimumSizeScreenPixels, double frustumScale);
    bool SkewTest(BeSQLite::RTree3dValCP);
    RTreeFilter(DgnElementIdSet const* exclude) {m_exclude=exclude;}
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

The method DgnClientFx::DgnClientApp::OpenDgnDb creates a default QueryModel for an application. 
Applications may use DgnClientFx::DgnClientApp::GetQueryModel to retrieve a reference to that QueryModel. 
QueryModels are associated with a QueryViewController by passing a QueryModel to the QueryViewController constructor.
*/
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct QueryModel : SpatialModel
{
    friend struct QueryViewController;
    typedef bmultimap<double, uint64_t> OcclusionScores;
    
    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   12/11
    //=======================================================================================
    struct Filter : RTreeFilter
    {
        bool                    m_needsProgressive = false;
        uint32_t                m_hitLimit;
        uint32_t                m_occlusionMapCount;
        uint64_t                m_lastId;
        OcclusionScores         m_occlusionScores;
        double                  m_occlusionMapMinimum;
        double                  m_lastScore;
        DgnElementIdSet const*  m_alwaysDraw;
        QueryModel&             m_model;

        bool CheckAbort();
        virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;
        void AcceptElement(DgnElementId elementId);

    public:
        Filter(QueryModelR model, uint32_t hitLimit, DgnElementIdSet const* alwaysDraw, DgnElementIdSet const* exclude);
        uint32_t GetCount() const {return m_occlusionMapCount;}
    };
    
    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   01/16
    //=======================================================================================
    struct AllElementsFilter : RTreeFilter
    {
        friend struct QueryViewController;

        uint64_t       m_elementReleaseTrigger;
        uint64_t       m_purgeTrigger;
        DgnDbR         m_dgndb;
        QueryModelR    m_queryModel;
        CheckStopP     m_checkStop;
        AllElementsFilter(QueryModelR queryModel, DgnElementIdSet const* exclude, uint64_t maxMemory)
             : RTreeFilter(exclude), m_dgndb(queryModel.GetDgnDb()), m_queryModel(queryModel), m_elementReleaseTrigger(maxMemory), m_purgeTrigger(maxMemory), m_checkStop(nullptr)
            {
            }
                                                                                                            
        bool AcceptElement(ViewContextR context, DgnElementId elementId);
        virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   04/14
    //=======================================================================================
    struct ProgressiveFilter : AllElementsFilter, ProgressiveDisplay
    {
        enum {SHOW_PROGRESS_INTERVAL = 1000}; // once per second.
        uint32_t       m_total = 0;
        uint32_t       m_thisBatch = 0;
        uint32_t       m_batchSize = 0;
        uint64_t       m_nextShow  = 0;
        bool           m_setTimeout = false;
        BeSQLite::CachedStatementPtr m_rangeStmt;
        ProgressiveFilter(DgnViewportCR vp, QueryModelR queryModel, DgnElementIdSet const* exclude, uint64_t maxMemory, BeSQLite::CachedStatement* stmt, double minPixelSize)
            : AllElementsFilter(queryModel, exclude, maxMemory), m_rangeStmt(stmt)
            {
            SetViewport(vp, minPixelSize, 1.0);
            }

        virtual Completion _Process(ViewContextR context, uint32_t batchSize, WantShow&) override;
    };

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

    typedef RefCountedPtr<Results> ResultsPtr;

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
        ResultsPtr  m_results;

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
    ResultsPtr  m_updatedResults;
    ResultsPtr  m_currQueryResults;

    void ResetResults() {ReleaseAllElements(); ClearRangeIndex(); m_filled=true;}
    DGNPLATFORM_EXPORT explicit QueryModel(DgnDbR);
    virtual void _FillModel() override {} // QueryModels are never filled.

public:
    bool AbortRequested() const { return m_abortQuery; } //!< @private
    void SetAbortQuery(bool val) {m_abortQuery=val;} //!< @private
    Results* GetCurrentResults() {return m_currQueryResults.get();} //!< @private
    void SaveQueryResults(); //!< @private
    void ResizeElementList(uint32_t newCount); //!< @private
    void ClearQueryResults(); //!< @private

    //! Returns a count of elements held by the QueryModel. This is the count of elements returned by the most recent query.
    uint32_t GetElementCount() const; //!< @private
    bool HasSelectResults() const { return m_updatedResults.IsValid(); } //!< @private

    //! Requests that any active or pending processing of the model be canceled, optionally not returning until the request is satisfied
    void RequestAbort(bool waitUntilFinished);
    void WaitUntilFinished(CheckStop* checkStop); //!< @private
};

END_BENTLEY_DGN_NAMESPACE
