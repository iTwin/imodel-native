/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/QueryModel.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/DgnCore/QueryModel.h>
#include <DgnPlatform/DgnCore/QueryView.h>
#include "UpdateLogging.h"

#include "UpdateLogging.h"

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/10
//=======================================================================================
struct QueryModelReader : DbElementReader
{
protected:
    QueryModel::Results& m_results;
    virtual PersistentElementRefP _FindExistingElementRef (ElementId id) override {return NULL;}

public:
    QueryModelReader (DgnProjectR project, QueryModel::Results& results) : DbElementReader(project), m_results(results) 
        {
        Utf8String sql (GetSelectElementSql());
        sql.append ("WHERE Id=?");
        project.GetCachedStatement(m_selectStmt, sql.c_str());
        }

    Statement* GetStatement() const {return m_selectStmt.get();}
    PersistentElementRefP GetElement (ElementId, bool loadIfNotFound);
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2012
//--------------+------------------------------------------------------------------------
PersistentElementRefP QueryModelReader::GetElement(ElementId id, bool loadIfNotFound)
    {
    bool wasLoaded = false;

    PersistentElementRefP elRef = DbElementReader::_FindExistingElementRef(id);
    if (NULL != elRef)
        return elRef;

    if (!loadIfNotFound)
        return NULL;

    m_selectStmt->BindId(1, id);
    DbResult result = GetOneElement(elRef, wasLoaded);
    m_selectStmt->Reset();
    if (BE_SQLITE_ROW !=  result)
        {
        BeAssert (false);
        return NULL;
        }
    
    BeAssert (NULL != elRef);
    return elRef;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2012
//--------------+------------------------------------------------------------------------
void QueryModel::ClearQueryResults ()
    {
    delete m_currQueryResults;
    m_currQueryResults = NULL;

    GetGraphicElementsP(true)->EmptyList();
    GetSelector().Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
void QueryModel::ResizeElementList(UInt32 newCount)
    {
    if (newCount >= m_currQueryResults->m_elements.size())
        return;

    m_currQueryResults->m_elements.resize(newCount);

    GetGraphicElementsP(true); // make sure we have a list
    m_graphicElems->EmptyList();

    for (auto result :  m_currQueryResults->m_elements)
        m_graphicElems->_OnElementAdded (*result);
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
        BeAssert (false);    // this should only be called if we know there is a query available.
        return;
        }

    if (m_currQueryResults)
        delete m_currQueryResults;

    GetGraphicElementsP(true); // make sure we have a list
    m_graphicElems->EmptyList();

    m_currQueryResults = m_selector.m_results; // move the object from query thread to this thread.
    m_selector.m_results = 0;

    //  First add everything from the list used for drawing.
    for (auto result : m_currQueryResults->m_elements)
        m_graphicElems->_OnElementAdded (*result);

    //  Now add everything that is in the secondary list but not the first.
    for (auto const& result : m_currQueryResults->m_closeElements)
        m_graphicElems->_OnElementAdded (*result);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
void QueryModel::Selector::SetState(State newState)
    {
    BeCriticalSectionHolder synchIt(m_conditionVariable.GetCriticalSection());
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
void QueryModel::Selector::RequestAbort (bool waitUntilFinished) 
    {
    if (true) // hold critical section while we test/set state
        {
        BeCriticalSectionHolder synchIt (m_conditionVariable.GetCriticalSection());
        if (!IsActive())
            return;

        m_state = State::AbortRequested;
        }

    // we released the CS before calling this
    if (waitUntilFinished)
        {
        //  BeConditionVariable::Infinite should be fine but I feel safer with 1000/16 and there is not 
        //  much overhead to using it since the abort should nearly always finish before it expires.
        WaitUntilFinished (NULL, 1, false);
        }
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   08/12
//=======================================================================================
struct WaitUntilNotActivePredicate : IConditionVariablePredicate
    {
    QueryModel::Selector const& m_selector;

    WaitUntilNotActivePredicate (QueryModel::Selector const& selector) : m_selector(selector){}
    virtual bool _TestCondition (BeConditionVariable &cv) override {return !m_selector.IsActive();}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
QueryModel::Selector::State QueryModel::Selector::WaitUntilFinished (ICheckStop* checkStop, UInt32 interval, bool stopQueryOnAbort) 
    {
    WaitUntilNotActivePredicate predicate(*this);

    // We found this always has to stop. It seemed like a good idea to let the query continue
    // possibly loading elements in the background. However, leaving the 
    // query thread in the sqlite step operation could block the work thread if
    // the work thread got into sqlite and tried to grab an sqlite mutex.
    stopQueryOnAbort = true;
    while (IsActive())
        {
        if (NULL != checkStop && checkStop->_CheckStop())
            {
            if (stopQueryOnAbort)
                //  RequestAbort does not return until the abort has finished,
                //  so it is okay to exit from the loop after RequestAbort returns.
                RequestAbort(true);

            break;
            }

        m_conditionVariable.WaitOnCondition (&predicate, interval);
        }

    return m_state;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                                   John.Gooding    05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Selector::Reset()
    {
    RequestAbort(true);

    m_viewport = NULL;
    SetState(State::Inactive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding    05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Selector::StartProcessing(ViewportCR viewport, QueryViewControllerCR qvc, Utf8CP sql, UInt32 maxElements, UInt64 maxMemory, double minimumSizePixels, 
                   ElementIdSet* alwaysDraw, ElementIdSet* neverDraw, bool noQuery, ClipVectorP cpsIn,
                   UInt32 secondaryHitLimit, DRange3dCR secondaryRange)
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

    m_clipVector = NULL;
    if (NULL != cpsIn)
        m_clipVector = cpsIn;

    m_secondaryHitLimit = secondaryHitLimit;
    m_secondaryVolume = secondaryRange;

    BeCriticalSectionHolder synchIt (m_conditionVariable.GetCriticalSection());
    SetState (State::ProcessingRequested);
    m_conditionVariable.Wake (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryModel::Selector::_CheckStop () 
    {
    if (WasAborted())
        return true;

    if (GetState() != State::AbortRequested)
        {
        if (!m_inRangeSelectionStep || !HighPriorityOperationSequencer::IsHighPriorityOperationActive())
            return false;

        m_restartRangeQuery = true;
        }

    SetAborted();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
void QueryModel::Selector::qt_NotifyCompletion ()
    {
    BeCriticalSectionHolder synchIt (m_conditionVariable.GetCriticalSection());

    //  Set state to one of the completed states
    if (WasAborted())
        SetState (State::Aborted);
    else if (m_dbStatus != BE_SQLITE_ROW)
        SetState (State::HandlerError);
    else
        SetState (State::Completed);

    m_conditionVariable.Wake(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
void QueryModel::Selector::qt_SearchIdSet(ElementIdSet& idSet, DgnDbRTree3dViewFilter& filter)
    {
    DgnElementPool& pool = m_project.Models().ElementPool();

    QueryModelReader reader (m_project, *m_results);

    for (auto const& curr : idSet)
        {
        if (_CheckStop())
            {
            SetState (State::Aborted);
            break;
            }

        PersistentElementRefP elRef = pool.FindElementById(curr);
        RTree3dVal rtreeRange;
        if (NULL != elRef)
            {
            rtreeRange.FromRange(elRef->GetRange());
            }
        else
            {
            Statement* stmt = reader.GetStatement();
            stmt->BindId(1, curr);
            if (BE_SQLITE_ROW != stmt->Step()) 
                continue;   //  ID is in the list but not in the file

            enum {FirstCol=5};
            rtreeRange.m_minx = static_cast <double> (stmt->GetValueInt64(FirstCol+0));
            rtreeRange.m_miny = static_cast <double> (stmt->GetValueInt64(FirstCol+1));
            rtreeRange.m_minz = static_cast <double> (stmt->GetValueInt64(FirstCol+2));
            rtreeRange.m_maxx = static_cast <double> (stmt->GetValueInt64(FirstCol+3));
            rtreeRange.m_maxy = static_cast <double> (stmt->GetValueInt64(FirstCol+4));
            rtreeRange.m_maxz = static_cast <double> (stmt->GetValueInt64(FirstCol+5));
            stmt->Reset();
            }

        RTreeMatch::QueryInfo info;
        info.m_nCoord = 6;
        info.m_coords = &rtreeRange.m_minx;
        info.m_parentWithin = info.m_within = RTreeMatch::Within::Partly;
        info.m_parentScore  = info.m_score  = 1.0;
        info.m_rowid = curr.GetValue();
        info.m_level = 0;
        filter._TestRange(info);
        if (RTreeMatch::Within::Outside != info.m_within)
            filter.RangeAccept(curr.GetValueUnchecked());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
void QueryModel::Selector::qt_SearchRangeTree (DgnDbRTree3dViewFilter& filter)
    {
#if defined (TRACE_QUERY_LOGIC)
    UInt64 start = BeTimeUtilities::QueryMillisecondsCounter();
    int restarts = 0;
#endif

    do
        {
        ClearAborted ();

        if (HighPriorityOperationSequencer::IsHighPriorityOperationActive())
            {
            for (unsigned i = 0; i < 10 && !_CheckStop(); ++i)
                BeThreadUtilities::BeSleep(2); // Let it run for awhile. If there was one call to XAttributeHandle::DoSelect, there will probably be more.

            continue;
            }

        if (WasAborted())
            break;

        CachedStatementPtr rangeStmt;
        m_project.GetCachedStatement(rangeStmt, m_searchSql.c_str());

        static_cast<QueryViewControllerCP>(&m_viewport->GetViewController())->BindModelAndLevel(*rangeStmt);
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
        printf ("qt_Process: got %d restarts\n", restarts);
#endif

#if defined (DEBUGGING_LOAD_COUNTS)
    BeDebugLog (Utf8PrintfString("qt_ProcessRequest finished: m_passedToTestRange = %d, m_acceptedByTestRange = %d, m_rejectedByOcclusionCalc = %d, m_passedToRangeAccept = %d",
                                 filter.m_passedToTestRange, filter.m_acceptedByTestRange, filter.m_rejectedByOcclusionCalc, filter.m_passedToRangeAccept));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding    05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Selector::qt_ProcessRequest() 
    {
    UpdateLogging::RecordStartQuery();
    //  Notify HighPriorityOperationSequencer that this thread is running 
    //  a range tree operation and is therefore exempt from checks for high priority required.
    RangeTreeOperationBlock rangeTreeOperationBlock;

    DgnDbRTree3dViewFilter filter (*m_viewport, this, m_project, m_maxElements, m_minimumPixels, m_noQuery ? NULL : m_alwaysDraw, m_neverDraw);
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
        printf ("ncalls=%d, nscores=%d\n", filter.m_nCalls, filter.m_nScores);
#endif
        m_results->m_reachedMaxElements = filter.m_occlusionScoreMap.size()==m_maxElements;
        m_results->m_eliminatedByLOD = filter.m_eliminatedByLOD;

        if (m_results->m_reachedMaxElements)
            m_results->m_lowestOcclusionScore = filter.m_occlusionScoreMap.begin()->first;
        }

#if defined (WANT_QUERYVIEW_UPDATE_LOGGING)
    UInt32 nAcceptCalls, nScores;
    filter.GetStats(nAcceptCalls, nScores);
    UpdateLogging::RecordDoneQuery(nAcceptCalls, nScores, (UInt32)filter.m_occlusionScoreMap.size());
#endif

    if (WasAborted() || m_dbStatus != BE_SQLITE_ROW)
        {
        BeCriticalSectionHolder synchIt (m_conditionVariable.GetCriticalSection());

        //  Set state to one of the completed states
        if (WasAborted ())
            SetState (State::Aborted);
        else if (m_dbStatus != BE_SQLITE_ROW)
            SetState (State::HandlerError);

        m_conditionVariable.Wake (true);
        return;
        }

#if defined (TRACE_QUERY_LOGIC)
    UInt32 elapsed1 = (UInt32)(BeTimeUtilities::QueryMillisecondsCounter() - start);
#endif

    DgnElementPool& pool = m_project.Models().ElementPool();

    QueryModelReader reader (m_project, *m_results);
    for (auto curr = filter.m_secondaryFilter.m_occlusionScoreMap.rbegin(); curr != filter.m_secondaryFilter.m_occlusionScoreMap.rend(); ++curr)
        {
        if (_CheckStop())
            {
            SetState (State::Aborted);
            break;
            }

        bool hitLimit = pool.GetTotalAllocated() > (Int64) (2 * m_maxMemory);
        PersistentElementRefP el = reader.GetElement(ElementId(curr->second), !hitLimit);
        if (NULL != el)
            m_results->m_closeElements.push_back(el);
        }

    for (auto curr = filter.m_occlusionScoreMap.rbegin(); curr != filter.m_occlusionScoreMap.rend(); ++curr)
        {
        if (_CheckStop())
            {
            SetState (State::Aborted);
            break;
            }

        bool hitLimit = pool.GetTotalAllocated() > (Int64) (2 * m_maxMemory);
        PersistentElementRefP el = reader.GetElement(ElementId(curr->second), !hitLimit);
        if (NULL != el)
            m_results->m_elements.push_back(el);
        }

#if defined (TRACE_QUERY_LOGIC)
    UInt32 elapsed2 = (UInt32)(BeTimeUtilities::QueryMillisecondsCounter() - start);

    printf ("qt_ProcessRequest: hitLimit = %d, query time = %d, total time = %d\n", m_hitLimit, elapsed1, elapsed2);
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

    WaitUntilRequestedPredicate (QueryModel::Selector const& selector) : m_selector(selector){}
    virtual bool _TestCondition (BeConditionVariable &cv) override 
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
        m_conditionVariable.WaitOnCondition (&predicate, BeConditionVariable::Infinite);
        Selector::State lastState;
            {
            BeCriticalSectionHolder synchIt (m_conditionVariable.GetCriticalSection());
            lastState = m_state;
            if (State::ProcessingRequested == m_state)
                {
                SetState (State::Processing);
                }
            }
        if (State::ProcessingRequested == lastState || State::AbortRequested == lastState)
            qt_ProcessRequest ();
        }

    // Block until it is waiting. Otherwise the Wake will be lost.
    BeCriticalSectionHolder synchIt (m_conditionVariable.GetCriticalSection());
    m_state = QueryModel::Selector::State::Terminated;
    m_conditionVariable.Wake (true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
THREAD_MAIN_DECL queryModelThreadMain (void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName("BentleyQueryModel"); // for debugging only
    ((QueryModel::Selector*) arg)->qt_WaitForWork ();
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
QueryModel::Selector::Selector (QueryModel& model) : m_project(model.GetDgnProject()), m_conditionVariable(NULL)
    {
    m_viewport = 0;
    m_dbStatus = BE_SQLITE_ERROR;
    m_results = 0;
    m_maxElements = 0;
    m_controller = NULL;
    m_secondaryVolume.Init();
    m_secondaryHitLimit = 0;
    m_inRangeSelectionStep = false;
    m_state = State::Inactive;

    // for every QueryModel, we create a QueryView thread to do the query and loading of elements
    BeThreadUtilities::StartNewThread (50*1024, queryModelThreadMain, this); 
    }

//=======================================================================================
// @bsiclass                                                    John.Gooding    11/12
//=======================================================================================
struct WaitUntilTerminatedPredicate : IConditionVariablePredicate
    {
    QueryModel::Selector const& m_selector;

    WaitUntilTerminatedPredicate (QueryModel::Selector const& selector) : m_selector(selector){}
    virtual bool _TestCondition (BeConditionVariable &cv) override 
        {
        return m_selector.GetState() == QueryModel::Selector::State::Terminated;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::Selector::~Selector () 
    {
    WaitUntilTerminatedPredicate    predicate(*this);
    BeCriticalSectionHolder synchIt (m_conditionVariable.GetCriticalSection());

    m_state = State::TerminateRequested;
    while(m_state == State::TerminateRequested)
        {
        m_conditionVariable.Wake (true);
        //  If should be possible to specify Infinite here but we have seen one deadlock.
        m_conditionVariable.ProtectedWaitOnCondition (&predicate, 100);
        }
    }

UInt32 QueryModel::GetElementCount() const {return m_currQueryResults ? m_currQueryResults->GetCount() : 0;}
#if defined (_MSC_VER)
    #pragma warning (disable:4355)
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::QueryModel (DgnProjectR project) : PhysicalModel (project, DgnModelId(), "Query"), m_selector(*this)
    {
    m_currQueryResults = 0;
    } 
