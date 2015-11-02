/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/QueryModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/DgnCore/QueryModel.h>
#include <DgnPlatform/DgnCore/QueryView.h>
#include "UpdateLogging.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2012
//--------------+------------------------------------------------------------------------
void QueryModel::ClearQueryResults()
    {
    delete m_currQueryResults;
    m_currQueryResults = nullptr;

    EmptyModel();
    GetSelector().Reset();
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
* Move the QueryResults object from the Selector (where it is created on the other thread) to the QueryModel object
* where it can be accessed from the main thread. 
* NOTE: This method must run on the main thread.
* @bsimethod                                    John.Gooding                    06/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::SaveQueryResults()
    {
    if (m_selector.GetState() != Selector::State::Completed)
        {
        BeAssert(false); // this should only be called if we know there is a query available.
        return;
        }

    if (m_currQueryResults)
        delete m_currQueryResults;

    ResetResults();

    m_currQueryResults = m_selector.m_results; // move the object from query thread to this thread.
    m_selector.m_results = 0;

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
void QueryModel::Selector::SetState(State newState)
    {
    BeMutexHolder synchIt(m_conditionVariable.GetMutex());
    m_state = newState;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
bool QueryModel::Selector::IsActive() const
    {
    switch (m_state)
        {
        case State::Inactive:
        case State::HandlerError:
        case State::Aborted:
        case State::Completed:
            return false;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
void QueryModel::Selector::RequestAbort(bool waitUntilFinished) 
    {
    if (true) // hold critical section while we test/set state
        {
        BeMutexHolder synchIt(m_conditionVariable.GetMutex());
        if (!IsActive())
            return;

        m_state = State::AbortRequested;
        }

    // we released the CS before calling this
    if (waitUntilFinished)
        {
        //  BeConditionVariable::Infinite should be fine but I feel safer with 1000/16 and there is not 
        //  much overhead to using it since the abort should nearly always finish before it expires.
        WaitUntilFinished(nullptr, 1, false);
        }
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   08/12
//=======================================================================================
struct WaitUntilNotActivePredicate : IConditionVariablePredicate
    {
    QueryModel::Selector const& m_selector;

    WaitUntilNotActivePredicate(QueryModel::Selector const& selector) : m_selector(selector){}
    virtual bool _TestCondition(BeConditionVariable &cv) override {return !m_selector.IsActive();}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
QueryModel::Selector::State QueryModel::Selector::WaitUntilFinished(ICheckStop* checkStop, uint32_t interval, bool stopQueryOnAbort) 
    {
    WaitUntilNotActivePredicate predicate(*this);

    // We found this always has to stop. It seemed like a good idea to let the query continue
    // possibly loading elements in the background. However, leaving the 
    // query thread in the sqlite step operation could block the work thread if
    // the work thread got into sqlite and tried to grab an sqlite mutex.
    stopQueryOnAbort = true;
    while (IsActive())
        {
        if (nullptr != checkStop && checkStop->_CheckStop())
            {
            if (stopQueryOnAbort)
                //  RequestAbort does not return until the abort has finished,
                //  so it is okay to exit from the loop after RequestAbort returns.
                RequestAbort(true);

            break;
            }

        m_conditionVariable.WaitOnCondition(&predicate, interval);
        }

    return m_state;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                                   John.Gooding    05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Selector::Reset()
    {
    RequestAbort(true);

    m_viewport = nullptr;
    SetState(State::Inactive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding    05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Selector::StartProcessing(DgnViewportCR viewport, QueryViewControllerCR qvc, Utf8CP sql, uint32_t maxElements, uint64_t maxMemory, double minimumSizePixels, 
                   DgnElementIdSet* alwaysDraw, DgnElementIdSet* neverDraw, bool noQuery, ClipVectorP cpsIn,
                   uint32_t secondaryHitLimit, DRange3dCR secondaryRange)
    {
    // This waits until the handler is no longer active. It is essential to wait until it is
    // inactive prior to changing the control members.
    RequestAbort(true);

    //  Save the parameters
    m_viewport = &viewport;
    m_maxElements = maxElements;
    m_searchSql = sql;
    m_frustum = viewport.GetFrustum(DgnCoordSystem::World, true);
    m_controller = &qvc;

    m_minimumPixels = minimumSizePixels;
    m_restartRangeQuery = false;
    m_alwaysDraw = alwaysDraw;
    m_noQuery = noQuery;
    m_maxMemory = maxMemory;
    m_neverDraw = neverDraw;

    m_clipVector = nullptr;
    if (nullptr != cpsIn)
        m_clipVector = cpsIn;

    m_secondaryHitLimit = secondaryHitLimit;
    m_secondaryVolume = secondaryRange;

    BeMutexHolder synchIt(m_conditionVariable.GetMutex());
    SetState(State::ProcessingRequested);
    m_conditionVariable.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryModel::Selector::_CheckStop() 
    {
    if (WasAborted())
        return true;

    if (GetState() != State::AbortRequested)
        {
        if (!m_inRangeSelectionStep || !GraphicsAndQuerySequencer::qt_isOperationRequiredForGraphicsPending())
            return false;

        m_restartRangeQuery = true;
        }

    SetAborted();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
void QueryModel::Selector::qt_NotifyCompletion()
    {
    BeMutexHolder synchIt(m_conditionVariable.GetMutex());

    //  Set state to one of the completed states
    if (WasAborted())
        SetState(State::Aborted);
    else if (m_dbStatus != BE_SQLITE_ROW)
        SetState(State::HandlerError);
    else
        SetState(State::Completed);

    m_conditionVariable.notify_all();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
void QueryModel::Selector::qt_SearchIdSet(DgnElementIdSet& idSet, DgnDbRTree3dViewFilter& filter)
    {
    DgnElements& pool = m_dgndb.Elements();

    for (auto const& curr : idSet)
        {
        if (_CheckStop())
            {
            SetState(State::Aborted);
            break;
            }

        DgnElementCPtr elRef = pool.GetElement(curr);
        RTree3dVal rtreeRange;
        if (!elRef.IsValid())
            continue; // id is in the list but not in the file

        GeometricElementCP geom=elRef->ToGeometricElement();
        if (nullptr==geom || !geom->HasGeometry())
            continue;

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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
void QueryModel::Selector::qt_SearchRangeTree(DgnDbRTree3dViewFilter& filter)
    {
#if defined (TRACE_QUERY_LOGIC)
    uint64_t start = BeTimeUtilities::QueryMillisecondsCounter();
    int restarts = 0;
#endif

    do
        {
        ClearAborted();

        if (GraphicsAndQuerySequencer::qt_isOperationRequiredForGraphicsPending())
            {
            for (unsigned i = 0; i < 10 && !_CheckStop(); ++i)
                BeThreadUtilities::BeSleep(2); // Let it run for awhile. If there was one call to XAttributeHandle::DoSelect, there will probably be more.

            continue;
            }

        if (WasAborted())
            break;

        CachedStatementPtr rangeStmt;
        m_dgndb.GetCachedStatement(rangeStmt, m_searchSql.c_str());

        static_cast<QueryViewControllerCP>(&m_viewport->GetViewController())->BindModelAndCategory(*rangeStmt);
        m_restartRangeQuery = false;

        m_inRangeSelectionStep = true;
        m_dbStatus = filter.StepRTree(*rangeStmt);

        m_inRangeSelectionStep = false;

#if defined (TRACE_QUERY_LOGIC)
        if (m_restartRangeQuery)
            restarts++;
#endif
        } while (m_restartRangeQuery);

#if defined (TRACE_QUERY_LOGIC)
    if (restarts > 0)
        printf("qt_Process: got %d restarts\n", restarts);
#endif

#if defined (DEBUGGING_LOAD_COUNTS)
    BeDebugLog(Utf8PrintfString("qt_ProcessRequest finished: m_passedToTestRange = %d, m_acceptedByTestRange = %d, m_rejectedByOcclusionCalc = %d, m_passedToRangeAccept = %d",
                                 filter.m_passedToTestRange, filter.m_acceptedByTestRange, filter.m_rejectedByOcclusionCalc, filter.m_passedToRangeAccept));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding    05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Selector::qt_ProcessRequest() 
    {
    UpdateLogging::RecordStartQuery();
    //  Notify GraphicsAndQuerySequencer that this thread is running 
    //  a range tree operation and is therefore exempt from checks for high priority required.
    qt_RangeTreeOperationBlock qt_RangeTreeOperationBlock(m_dgndb);

    DgnDbRTree3dViewFilter filter(*m_viewport, this, m_dgndb, m_maxElements, m_minimumPixels, m_noQuery ? nullptr : m_alwaysDraw, m_neverDraw);
    if (m_clipVector.IsValid())
        filter.SetClipVector(*m_clipVector);

    if (0 != m_secondaryHitLimit)
        filter.InitializeSecondaryTest(m_secondaryVolume, m_secondaryHitLimit);

    DELETE_AND_CLEAR (m_results);
    m_results = new Results();

    if (m_noQuery)
        qt_SearchIdSet(*m_alwaysDraw, filter);
    else
        {
        qt_SearchRangeTree(filter);
#if defined (DEBUG_CALLS)
        printf("ncalls=%d, nscores=%d\n", filter.m_nCalls, filter.m_nScores);
#endif
        m_results->m_reachedMaxElements = filter.m_occlusionScoreMap.size()==m_maxElements;
        m_results->m_eliminatedByLOD = filter.m_eliminatedByLOD;

        if (m_results->m_reachedMaxElements)
            m_results->m_lowestOcclusionScore = filter.m_occlusionScoreMap.begin()->first;
        }

#if defined (WANT_QUERYVIEW_UPDATE_LOGGING)
    uint32_t nAcceptCalls, nScores;
    filter.GetStats(nAcceptCalls, nScores);
    UpdateLogging::RecordDoneQuery(nAcceptCalls, nScores, (uint32_t)filter.m_occlusionScoreMap.size());
#endif

    if (WasAborted() || m_dbStatus != BE_SQLITE_ROW)
        {
        BeMutexHolder synchIt(m_conditionVariable.GetMutex());

        //  Set state to one of the completed states
        if (WasAborted())
            SetState(State::Aborted);
        else if (m_dbStatus != BE_SQLITE_ROW)
            SetState(State::HandlerError);

        m_conditionVariable.notify_all();
        return;
        }

#if defined (TRACE_QUERY_LOGIC)
    uint32_t elapsed1 = (uint32_t)(BeTimeUtilities::QueryMillisecondsCounter() - start);
#endif

    DgnElements& pool = m_dgndb.Elements();

    for (auto curr = filter.m_secondaryFilter.m_occlusionScoreMap.rbegin(); curr != filter.m_secondaryFilter.m_occlusionScoreMap.rend(); ++curr)
        {
        if (_CheckStop())
            {
            SetState(State::Aborted);
            break;
            }

        bool hitLimit = pool.GetTotalAllocated() > (int64_t) (2 * m_maxMemory);
        DgnElementCPtr el;
        if (!hitLimit)
            el = pool.GetElement(DgnElementId(curr->second));
        else
            el = pool.FindElement(DgnElementId(curr->second));

        if (el.IsValid())
            m_results->m_closeElements.push_back(el.get());
        }

    for (auto curr = filter.m_occlusionScoreMap.rbegin(); curr != filter.m_occlusionScoreMap.rend(); ++curr)
        {
        if (_CheckStop())
            {
            SetState(State::Aborted);
            break;
            }

        bool hitLimit = pool.GetTotalAllocated() > (int64_t) (2 * m_maxMemory);
        DgnElementCPtr el;
        if (!hitLimit)
            el = pool.GetElement(DgnElementId(curr->second));
        else
            el = pool.FindElement(DgnElementId(curr->second));

        if (el.IsValid())
            m_results->m_elements.push_back(el.get());
        }

#if defined (TRACE_QUERY_LOGIC)
    uint32_t elapsed2 = (uint32_t)(BeTimeUtilities::QueryMillisecondsCounter() - start);

    printf("qt_ProcessRequest: hitLimit = %d, query time = %d, total time = %d\n", m_hitLimit, elapsed1, elapsed2);
#endif
    UpdateLogging::RecordDoneLoad();

    qt_NotifyCompletion();
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   08/12
//=======================================================================================
struct WaitUntilRequestedPredicate : IConditionVariablePredicate
    {
    QueryModel::Selector const& m_selector;

    WaitUntilRequestedPredicate(QueryModel::Selector const& selector) : m_selector(selector){}
    virtual bool _TestCondition(BeConditionVariable &cv) override 
        {
        switch (m_selector.GetState())
            {
            case QueryModel::Selector::State::AbortRequested:
            case QueryModel::Selector::State::ProcessingRequested:
            case QueryModel::Selector::State::TerminateRequested:
                return  true;
            }
        return  false;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
void QueryModel::Selector::qt_WaitForWork()
    {
    WaitUntilRequestedPredicate predicate(*this);

    while (State::TerminateRequested != m_state)
        {
        m_conditionVariable.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
        Selector::State lastState;
            {
            BeMutexHolder synchIt(m_conditionVariable.GetMutex());
            lastState = m_state;
            if (State::ProcessingRequested == m_state)
                {
                SetState(State::Processing);
                }
            }
        if (State::ProcessingRequested == lastState || State::AbortRequested == lastState)
            qt_ProcessRequest();
        }

    // Block until it is waiting. Otherwise the Wake will be lost.
    BeMutexHolder synchIt(m_conditionVariable.GetMutex());
    m_state = QueryModel::Selector::State::Terminated;
    m_conditionVariable.notify_all();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
THREAD_MAIN_DECL queryModelThreadMain(void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName("QueryModel"); // for debugging only
    ((QueryModel::Selector*) arg)->qt_WaitForWork();
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
QueryModel::Selector::Selector(QueryModel& model) : m_dgndb(model.GetDgnDb()), m_conditionVariable()
    {
    m_viewport = 0;
    m_dbStatus = BE_SQLITE_ERROR;
    m_results = 0;
    m_maxElements = 0;
    m_controller = nullptr;
    m_secondaryVolume.Init();
    m_secondaryHitLimit = 0;
    m_inRangeSelectionStep = false;
    m_state = State::Inactive;

    // for every QueryModel, we create a QueryView thread to do the query and loading of elements
    BeThreadUtilities::StartNewThread(50*1024, queryModelThreadMain, this); 
    }

//=======================================================================================
// @bsiclass                                                    John.Gooding    11/12
//=======================================================================================
struct WaitUntilTerminatedPredicate : IConditionVariablePredicate
    {
    QueryModel::Selector const& m_selector;

    WaitUntilTerminatedPredicate(QueryModel::Selector const& selector) : m_selector(selector){}
    virtual bool _TestCondition(BeConditionVariable &cv) override 
        {
        return m_selector.GetState() == QueryModel::Selector::State::Terminated;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::Selector::~Selector() 
    {
    WaitUntilTerminatedPredicate    predicate(*this);
    BeMutexHolder synchIt(m_conditionVariable.GetMutex());

    m_state = State::TerminateRequested;
    while (m_state == State::TerminateRequested)
        {
        m_conditionVariable.notify_all();
        //  If should be possible to specify Infinite here but we have seen one deadlock.
        m_conditionVariable.ProtectedWaitOnCondition(synchIt, &predicate, 100);
        }
    }

uint32_t QueryModel::GetElementCount() const {return m_currQueryResults ? m_currQueryResults->GetCount() : 0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::QueryModel(DgnDbR dgndb) : PhysicalModel(PhysicalModel::CreateParams(dgndb, DgnClassId(), CreateModelCode("Query"))), m_selector(*this)
    {
    m_currQueryResults = 0;
    } 
