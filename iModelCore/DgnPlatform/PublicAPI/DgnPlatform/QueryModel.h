/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/QueryModel.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnModel.h"
#include <Bentley/BeThread.h>
#include <deque>

BEGIN_BENTLEY_DGN_NAMESPACE

struct DgnDbRTree3dViewFilter;
struct ICheckStop;
                                                                                                                    
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
struct QueryModel : PhysicalModel
{
    friend struct QueryViewController;

    //! Holds the results of a QueryModel's query.
    struct Results : RefCountedBase
    {
    private:
        Results() : m_reachedMaxElements(false), m_eliminatedByLOD(false), m_drawnBeforePurge(0), m_lowestOcclusionScore(0.0) { }
    public:
        bvector<DgnElementCP> m_elements;
        bvector<DgnElementCP> m_closeElements;
        bool   m_reachedMaxElements;
        bool   m_eliminatedByLOD;
        uint32_t m_drawnBeforePurge;
        double m_lowestOcclusionScore;

        uint32_t GetCount() const {return (uint32_t) m_elements.size();}

        static RefCountedPtr<Results> Create() { return new Results(); }
    };

    typedef RefCountedPtr<Results> ResultsPtr;

    //! Executes a query on a separate thread to load elements for a QueryModel
    struct Processor : RefCounted<NonCopyableClass>
    {
        //! Parameters specifying how the processor should execute its query
        struct Params
        {
            QueryModelR         m_model;
            DgnViewportCR       m_vp;
            Utf8String          m_searchSql;
            uint32_t            m_maxElements;
            uint64_t            m_maxMemory;
            double              m_minPixels;
            DgnElementIdSet*    m_highPriority;
            DgnElementIdSet*    m_neverDraw;
            bool                m_highPriorityOnly;
            ClipVectorPtr       m_clipVector;
            uint32_t            m_secondaryHitLimit;
            DRange3d            m_secondaryVolume;

            Params(QueryModelR model, DgnViewportCR vp, Utf8StringCR sql, uint32_t maxElements, uint64_t maxMem, double minPixels, DgnElementIdSet* highPriority,
                    DgnElementIdSet* neverDraw, bool highPriorityOnly, ClipVectorP clipVector, uint32_t secondaryHitLimit, DRange3dCR secondaryRange)
                : m_model(model), m_vp(vp), m_searchSql(sql), m_maxElements(maxElements), m_maxMemory(maxMem), m_minPixels(minPixels), m_highPriority(highPriority),
                    m_neverDraw(neverDraw), m_highPriorityOnly(highPriorityOnly), m_clipVector(clipVector), m_secondaryHitLimit(secondaryHitLimit), m_secondaryVolume(secondaryRange) { }
        };

    protected:
        Params              m_params;
        ResultsPtr          m_results;
        bool                m_restartRangeQuery;
        bool                m_inRangeSelectionStep;
        BeSQLite::DbResult  m_dbStatus;

        Processor(Params const& params);

        virtual bool _Process() = 0;

        void BindModelAndCategory(BeSQLite::CachedStatement& rangeStmt);
    public:
        bool Process() { return _Process(); }
        
        QueryModelR GetModel() const { return m_params.m_model; }
        Results* GetResults() { return m_results.get(); }
    };

    typedef RefCountedPtr<Processor> ProcessorPtr;

    //! Each DgnDb has an associated QueryModel::Queue upon which requests for processing QueryModels can be enqueued.
    //! The requests execute on a separate thread.
    struct Queue : IConditionVariablePredicate
    {
    private:
        enum class State { Active, TerminateRequested, Terminated };

        DgnDbR                      m_db;
        BeConditionVariable         m_cv;
        std::deque<ProcessorPtr>    m_pending;
        State                       m_state;

        void qt_WaitForWork();

        THREAD_MAIN_DECL qt_Main(void* arg);

        virtual bool _TestCondition(BeConditionVariable&) override;
    public:
        Queue(DgnDbR db);

        void Terminate();

        //! Enqueue a request to execute the query for a QueryModel
        DGNPLATFORM_EXPORT void RequestProcessing(Processor::Params const& params);

        //! Cancel any active or pending requests to process the specified model.
        //! @param[in]      model             The model whose processing is to be canceled
        //! @param[in]      waitUntilFinished If true, this function does not return until the model is in the idle state
        DGNPLATFORM_EXPORT void RequestAbort(QueryModelR model, bool waitUntilFinished);

        //! Suspends the calling thread until the specified model is in the idle state
        DGNPLATFORM_EXPORT void WaitUntilFinished(QueryModelR model, ICheckStop* checkStop);
    };

    //! The possible states of a QueryModel
    enum class State
    {
        Idle,           //!< No processing
        Pending,        //!< Has been enqueued for processing
        Processing,     //!< Is currently processing
        AbortRequested, //!< A request to cancel processing has been made
    };

private:
    State       m_state;
    ResultsPtr  m_updatedResults;
    ResultsPtr  m_currQueryResults;

    void ResetResults(){ ReleaseAllElements(); ClearRangeIndex(); m_filled=true;}
    DGNPLATFORM_EXPORT explicit QueryModel (DgnDbR);
    virtual void _FillModel() override {} // QueryModels are never filled.

public:
    State GetState() const { return m_state; } //!< @private
    void SetState(State state); //!< @private
    void SetUpdatedResults(Results* results); //!< @private

    Results* GetCurrentResults() {return m_currQueryResults.get();} //!< @private

    void SaveQueryResults(); //!< @private
    void ResizeElementList(uint32_t newCount); //!< @private
    void ClearQueryResults(); //!< @private

    //! Returns a count of elements held by the QueryModel. This is the count of elements returned by the most recent query.
    uint32_t GetElementCount() const; //!< @private
    bool HasSelectResults() const { return m_updatedResults.IsValid(); } //!< @private
    bool IsIdle() const { return State::Idle == GetState(); } //!< @private
    bool IsActive() const { return !IsIdle(); } //!< @private

    //! Requests that any active or pending processing of the model be canceled, optionally not returning until the request is satisfied
    void RequestAbort(bool waitUntilFinished);

    void WaitUntilFinished(ICheckStop* checkStop); //!< @private
};

END_BENTLEY_DGN_NAMESPACE
