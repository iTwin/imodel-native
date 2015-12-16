/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/QueryModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/QueryModel.h>
#include <DgnPlatform/QueryView.h>
#include <DgnPlatform/ViewContext.h>
#include <Bentley/BeThreadLocalStorage.h>
#include "UpdateLogging.h"

#if !defined (NDEBUG)
#define DEBUG_THREADS 1
#endif

#define TRACE_QUERY_LOGIC 1

BeThreadLocalStorage g_queryThreadChecker;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void verifyQueryThread(bool wantQueryThread)
    {
#ifdef DEBUG_THREADS
    BeAssert(wantQueryThread == TO_BOOL(g_queryThreadChecker.GetValueAsInteger()));
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2012
//--------------+------------------------------------------------------------------------
void QueryModel::ClearQueryResults()
    {
    m_currQueryResults = nullptr;

    EmptyModel();
    RequestAbort(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
void QueryModel::ResizeElementList(uint32_t newCount)
    {
    if (newCount >= m_currQueryResults->m_elements.size())
        return;

    m_currQueryResults->m_elements.resize(newCount);

    ResetResults();
    for (auto result :  m_currQueryResults->m_elements)
        _OnLoadedElement(*result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::RequestAbort(bool waitUntilFinished) { GetDgnDb().QueryModels().RequestAbort(*this, waitUntilFinished); }
void QueryModel::WaitUntilFinished(ICheckStop* checkStop) { GetDgnDb().QueryModels().WaitUntilFinished(*this, checkStop); }

/*---------------------------------------------------------------------------------**//**
* Move the QueryResults object from the Processor (where it is created on the other thread) to the QueryModel object
* where it can be accessed from the main thread. 
* NOTE: This method must run on the main thread.
* @bsimethod                                    John.Gooding                    06/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::SaveQueryResults()
    {
    verifyQueryThread(false);

    if (m_updatedResults.IsNull())
        {
        BeAssert(false); // this should only be called if we know there is a query available.
        return;
        }

    // move the object from query thread to this thread.
    m_currQueryResults = nullptr;
    ResetResults();
    std::swap(m_currQueryResults, m_updatedResults);

    // First add everything from the list used for drawing.
    for (auto const& result : m_currQueryResults->m_elements)
        _OnLoadedElement(*result);

    // Now add everything that is in the secondary list but not the first.
    for (auto const& result : m_currQueryResults->m_closeElements)
        _OnLoadedElement(*result);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
void QueryModel::SetState(State newState)
    {
    m_state = newState;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::SetUpdatedResults(Results* results)
    {
    m_updatedResults = results;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t QueryModel::GetElementCount() const {return m_currQueryResults.IsValid() ? m_currQueryResults->GetCount() : 0;}
double QueryModel::GetLastQueryElapsedSeconds() const {return m_currQueryResults.IsValid() ? m_currQueryResults->GetElapsedSeconds() : 0.0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::QueryModel(DgnDbR dgndb) : PhysicalModel(PhysicalModel::CreateParams(dgndb, DgnClassId(), CreateModelCode("Query"))), m_state(State::Idle)
    {
    //
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::Processor::Processor(Params const& params)
    : m_params(params), m_restartRangeQuery(false), m_inRangeSelectionStep(false), m_dbStatus(BE_SQLITE_ERROR)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* This only exists because of ICheckStop
* @bsistruct                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProcessorImpl : QueryModel::Processor, ICheckStop
{
    ProcessorImpl(Params const& params) : Processor(params) { }

    void ProcessRequest();
    void SearchIdSet(DgnElementIdSet& idList, DgnDbRTree3dViewFilter& filter);
    void SearchRangeTree(DgnDbRTree3dViewFilter& filter);

    virtual bool _Process() override;

    bool LoadElements(DgnDbRTree3dViewFilter::T_OcclusionScoreMap& map, bvector<DgnElementCP>& elements); // return false if we halted before finishing iteration

    virtual bool _CheckStop() override
        {
        if (WasAborted())
            return true;
        else if (QueryModel::State::AbortRequested != GetModel().GetState())
            {
            if (!m_inRangeSelectionStep || !GraphicsAndQuerySequencer::qt_isOperationRequiredForGraphicsPending())
                return false;

            m_restartRangeQuery = true;
            }

        SetAborted();
        return true;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
THREAD_MAIN_IMPL QueryModel::Queue::qt_Main(void* arg)
    {
#ifdef DEBUG_THREADS
    BeThreadUtilities::SetCurrentThreadName("QueryModel");
    g_queryThreadChecker.SetValueAsInteger(true);
#endif

    auto pThis = reinterpret_cast<Queue*>(arg);
    pThis->qt_WaitForWork();

    // After the owning DgnDb calls Terminate()
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryModel::Queue::_TestCondition(BeConditionVariable& cv)
    {
    return State::Terminated == m_state;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Queue::Terminate()
    {
    verifyQueryThread(false);

    BeMutexHolder lock(m_cv.GetMutex());
    if (State::Terminated != m_state)
        {
        m_state = State::TerminateRequested;
        while (State::TerminateRequested == m_state)
            {
            m_cv.notify_all();
            m_cv.ProtectedWaitOnCondition(lock, this, 100);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Queue::RequestProcessing(Processor::Params const& params)
    {
    verifyQueryThread(false);
    QueryModelR model = params.m_model;
    BeAssert(&model.GetDgnDb() == &m_db);

    RequestAbort(model, true);

    BeMutexHolder lock(m_cv.GetMutex());

    BeAssert(QueryModel::State::Idle == model.GetState());

    ProcessorPtr proc = new ProcessorImpl(params);
    model.SetState(QueryModel::State::Pending);
    m_pending.push_back(proc);

    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct WaitUntilIdlePredicate : IConditionVariablePredicate
{
private:
    QueryModelR m_model;

    virtual bool _TestCondition(BeConditionVariable& cv) override { return QueryModel::State::Idle == m_model.GetState(); }
public:
    WaitUntilIdlePredicate(QueryModelR model) : m_model(model) { }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Queue::RequestAbort(QueryModelR model, bool waitUntilFinished)
    {
    verifyQueryThread(false);
    BeAssert(&model.GetDgnDb() == &m_db);

    if (true) // hold lock while we test/set state
        {
        BeMutexHolder lock(m_cv.GetMutex());

        switch (model.GetState())
            {
            case QueryModel::State::Idle:
                return;
            case QueryModel::State::Pending:
                for (auto entry = m_pending.begin(); entry != m_pending.end(); /*...*/)
                    {
                    if (&((*entry)->GetModel()) == &model)
                        entry = m_pending.erase(entry);
                    else
                        ++entry;
                    }

                model.SetState(QueryModel::State::Idle);
                break;
            case QueryModel::State::Processing:
                model.SetState(QueryModel::State::AbortRequested);
                // fall-through...
            default:
                BeAssert(QueryModel::State::AbortRequested == model.GetState());
                break;
            }
        }

    if (waitUntilFinished)
        {
#if defined TRACE_QUERY_LOGIC
        StopWatch timer(nullptr, true);
#endif
        model.WaitUntilFinished(nullptr);
#if defined TRACE_QUERY_LOGIC
        timer.Stop();
        printf("QMQ: RequestAbort: %.8f elapsed\n", timer.GetElapsedSeconds());
#endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Queue::WaitUntilFinished(QueryModelR model, ICheckStop* checkStop)
    {
    WaitUntilIdlePredicate predicate(model);
    while (QueryModel::State::Idle != model.GetState())
        {
        if (nullptr != checkStop && checkStop->_CheckStop())
            {
            RequestAbort(model, true);
            break;
            }

        m_cv.WaitOnCondition(&predicate, 1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::Queue::Queue(DgnDbR db) : m_db(db), m_state(State::Active)
    {
    BeThreadUtilities::StartNewThread(50*1024, &qt_Main, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericPredicate : IConditionVariablePredicate
{
    std::function<bool()> m_predicate;

    GenericPredicate(std::function<bool()> predicate) : m_predicate(predicate) { }

    virtual bool _TestCondition(BeConditionVariable& cv) override { return m_predicate(); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Queue::qt_WaitForWork()
    {
    verifyQueryThread(true);

    GenericPredicate predicate([&]() { return State::TerminateRequested == this->m_state || !m_pending.empty(); });

    while (State::TerminateRequested != m_state)
        {
        m_cv.WaitOnCondition(&predicate, BeConditionVariable::Infinite);

        ProcessorPtr processing;

            {
            BeMutexHolder lock(m_cv.GetMutex());

            if (State::TerminateRequested == m_state)
                {
                break;
                }
            else if (!m_pending.empty())
                {
                processing = m_pending.front();
                m_pending.pop_front();
                if (processing->GetModel().GetState() == QueryModel::State::Pending)
                    processing->GetModel().SetState(QueryModel::State::Processing);
                }
            }

        if (processing.IsValid())
            {
            auto& model = processing->GetModel();
            StopWatch timer(nullptr, true);
            if (processing->Process())
                {
#if defined TRACE_QUERY_LOGIC
                printf("QMQ: Processing complete\n");
#endif
                auto updatedResults = processing->GetResults();
                BeAssert(nullptr != updatedResults);

                timer.Stop();
#if defined TRACE_QUERY_LOGIC
                printf("QMQ: Elapsed query time %.8f\n", timer.GetElapsedSeconds());
#endif
                updatedResults->m_elapsedSeconds = timer.GetElapsedSeconds();
                model.SetUpdatedResults(updatedResults);
                
                processing->OnCompleted();
                }
#if defined TRACE_QUERY_LOGIC
            else
                printf("QMQ: Processing aborted\n");
#endif

            processing = nullptr;

                {
                BeMutexHolder lock(m_cv.GetMutex());
                if (QueryModel::State::Pending != model.GetState())
                    model.SetState(QueryModel::State::Idle);
                }

            m_cv.notify_all();
            }
        }

    BeMutexHolder lock(m_cv.GetMutex());
    m_state = State::Terminated;
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessorImpl::LoadElements(DgnDbRTree3dViewFilter::T_OcclusionScoreMap& map, bvector<DgnElementCP>& elements)
    {
    DgnElements& pool = GetModel().GetDgnDb().Elements();

    for (auto curr = map.rbegin(); curr != map.rend(); ++curr)
        {
        if (_CheckStop())
            return false;   // did not finish

        bool hitLimit = pool.GetTotalAllocated() > (int64_t) (2 * m_params.m_maxMemory);
        DgnElementId elId(curr->second);
        DgnElementCPtr el = !hitLimit ? pool.GetElement(elId) : pool.FindElement(elId);

        if (el.IsValid())
            elements.push_back(el.get());
        }

    return true;    // finished
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessorImpl::_Process()
    {
    verifyQueryThread(true);

#if defined TRACE_QUERY_LOGIC
    printf("QMQ: Processing\n");
#endif
    //  Notify GraphicsAndQuerySequencer that this thread is running 
    //  a range tree operation and is therefore exempt from checks for high priority required.
    DgnDbR db = GetModel().GetDgnDb();
    qt_RangeTreeOperationBlock qt_RangeTreeOperationBlock(db);

    DgnDbRTree3dViewFilter filter(m_params.m_vp, this, db, m_params.m_maxElements, m_params.m_minPixels, m_params.m_highPriorityOnly ? nullptr : m_params.m_highPriority, m_params.m_neverDraw);
    if (m_params.m_clipVector.IsValid())
        filter.SetClipVector(*m_params.m_clipVector);

    if (0 != m_params.m_secondaryHitLimit)
        filter.InitializeSecondaryTest(m_params.m_secondaryVolume, m_params.m_secondaryHitLimit);

    m_results = QueryModel::Results::Create();

    if (m_params.m_highPriorityOnly)
        {
        SearchIdSet(*m_params.m_highPriority, filter);
        }
    else
        {
        SearchRangeTree(filter);
        m_results->m_reachedMaxElements = (filter.m_occlusionScoreMap.size() == m_params.m_maxElements);
        m_results->m_eliminatedByLOD = filter.m_eliminatedByLOD;

        if (m_results->m_reachedMaxElements)
            m_results->m_lowestOcclusionScore = filter.m_occlusionScoreMap.begin()->first;
        }

    if (WasAborted() || m_dbStatus != BE_SQLITE_ROW)
        return false;

    LoadElements(filter.m_secondaryFilter.m_occlusionScoreMap, m_results->m_closeElements);
    LoadElements(filter.m_occlusionScoreMap, m_results->m_elements);

    return !WasAborted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ProcessorImpl::SearchIdSet(DgnElementIdSet& idList, DgnDbRTree3dViewFilter& filter)
    {
    DgnElements& pool = GetModel().GetDgnDb().Elements();
    for (auto const& curr : idList)
        {
        if (_CheckStop())
            break;

        DgnElementCPtr el = pool.GetElement(curr);
        GeometrySourceCP geom = el.IsValid() ? el->ToGeometrySource() : nullptr;
        if (nullptr == geom || !geom->HasGeometry())
            continue;

        RTree3dVal rtreeRange;
        rtreeRange.FromRange(geom->CalculateRange3d());

        RTreeAcceptFunction::Tester::QueryInfo info;
        info.m_nCoord = 6;
        info.m_coords = &rtreeRange.m_minx;
        info.m_parentWithin = info.m_within = RTreeAcceptFunction::Tester::Within::Partly;
        info.m_parentScore  = info.m_score  = 1.0;
        info.m_rowid = curr.GetValue();
        info.m_level = 0;
        filter._TestRange(info);
        if (RTreeAcceptFunction::Tester::Within::Outside != info.m_within)
            filter.RangeAccept(curr.GetValueUnchecked());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Processor::BindModelAndCategory(CachedStatement& stmt)
    {
    static_cast<QueryViewControllerCP>(&m_params.m_vp.GetViewController())->BindModelAndCategory(stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Processor::OnCompleted() const
    {
    // NEEDS_WORK_CONTINUOUS_RENDER
    // This is not thread-safe. The worst that can happen is that the work thread reads false from it, or sets it to false, while the query thread sets it to true.
    // In that case we skip an update.
    // Alternative is to go through contortions to queue up a "heal viewport" task on the DgnClientFx work thread.
    const_cast<DgnViewportR>(m_params.m_vp).SetNeedsHeal();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ProcessorImpl::SearchRangeTree(DgnDbRTree3dViewFilter& filter)
    {
    do
        {
        ClearAborted();

        if (GraphicsAndQuerySequencer::qt_isOperationRequiredForGraphicsPending())
            {
            for (unsigned i = 0; i < 10 && !_CheckStop(); ++i)
                BeThreadUtilities::BeSleep(2); // Let it run for awhile. If there was one call to SQLite, there will probably be more.

            continue;
            }

        if (WasAborted())
            break;

        CachedStatementPtr rangeStmt;
        GetModel().GetDgnDb().GetCachedStatement(rangeStmt, m_params.m_searchSql.c_str());

        BindModelAndCategory(*rangeStmt);
        m_restartRangeQuery = false;

        m_inRangeSelectionStep = true;
        m_dbStatus = filter.StepRTree(*rangeStmt);

        m_inRangeSelectionStep = false;
        } while (m_restartRangeQuery);
    }
