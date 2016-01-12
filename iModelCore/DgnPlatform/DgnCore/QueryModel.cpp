/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/QueryModel.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/QueryModel.h>
#include <DgnPlatform/QueryView.h>
#include <DgnPlatform/ViewContext.h>
#include <Bentley/BeSystemInfo.h>
#include "UpdateLogging.h"

//#define TRACE_QUERY_LOGIC 1
#define DEBUG_PRINTF(arg) 

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
void QueryModel::RequestAbort(bool waitUntilFinished) {GetDgnDb().QueryModels().RequestAbort(*this, waitUntilFinished);}
void QueryModel::WaitUntilFinished(CheckStop* checkStop) {GetDgnDb().QueryModels().WaitUntilFinished(*this, checkStop);}

/*---------------------------------------------------------------------------------**//**
* Move the QueryResults object from the Processor (where it is created on the other thread) to the QueryModel object
* where it can be accessed from the main thread. 
* NOTE: This method must run on the main thread.
* @bsimethod                                    John.Gooding                    06/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::SaveQueryResults()
    {
    DgnPlatformLib::VerifyClientThread();

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
QueryModel::QueryModel(DgnDbR dgndb) : SpatialModel(SpatialModel::CreateParams(dgndb, DgnClassId(), CreateModelCode("Query"))), m_state(State::Idle)
    {
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
THREAD_MAIN_IMPL QueryModel::Queue::Main(void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName("QueryModel");
    DgnPlatformLib::SetThreadId(DgnPlatformLib::ThreadId::Query);

    ((Queue*)arg)->Process();

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
    DgnPlatformLib::VerifyClientThread();

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
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryModel::Filter::CheckAbort() 
    {
    if (QueryModel::State::AbortRequested == m_model.GetState())
        {
        printf("Query Filter aborted\n");
        return true;
        }

    if (GraphicsAndQuerySequencer::qt_isOperationRequiredForGraphicsPending())
        {
        printf("Query Filter interrupted\n");
        m_restartRangeQuery = true;
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Queue::RequestProcessing(Processor::Params const& params)
    {
    DgnPlatformLib::VerifyClientThread();

    QueryModelR model = params.m_model;
    BeAssert(&model.GetDgnDb() == &m_db);

    // We may currently be processing a query for this model. If so, let it complete and queue up another one.
    // But remove any other previously-queued processing requests for this model.
    BeMutexHolder lock(m_cv.GetMutex());

#if defined TRACE_QUERY_LOGIC
    auto initialPending = static_cast<int32_t>(m_pending.size());
#endif

    for (auto iter = m_pending.begin(); iter != m_pending.end(); /*...*/)
        {
        if ((*iter)->IsForModel(model))
            iter = m_pending.erase(iter);
        else
            ++iter;
        }
#if defined TRACE_QUERY_LOGIC
    printf("QMQ: RequestProcessing: %d initially pending, %d currently pending\n", initialPending, static_cast<int32_t>(m_pending.size()));
#endif

    ProcessorPtr proc = new QueryModel::Processor(params);

    //  NEEDSWORK_CONTINUOUS_RENDERING -- the query thread might be in the processing state. If so, changing it to Pending is invalid.
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
    DgnPlatformLib::VerifyClientThread();
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
                    if ((*entry)->IsForModel(model))
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
void QueryModel::Queue::WaitUntilFinished(QueryModelR model, CheckStop* checkStop)
    {
    DgnPlatformLib::VerifyClientThread();

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
    BeThreadUtilities::StartNewThread(50*1024, Main, this);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2012
//--------------+------------------------------------------------------------------------
static uint32_t getQueryDelay()
    {
    static uint32_t s_dynamicLoadFrequency4Cpus = 100;
    static uint32_t s_dynamicLoadFrequency2Cpus = 750;
    static uint32_t s_numberOfCpus = 0;

    if (0 == s_numberOfCpus)
        s_numberOfCpus = BeSystemInfo::GetNumberOfCpus();

    return s_numberOfCpus < 4 ? s_dynamicLoadFrequency2Cpus : s_dynamicLoadFrequency4Cpus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryModel::Queue::WaitForWork()
    {
    BeMutexHolder holder(m_cv.GetMutex());
    while (m_pending.empty())
        {
        m_cv.InfiniteWait(holder);

        if (State::TerminateRequested == m_state)
            return false;
        }

    m_active = m_pending.front();
    m_pending.pop_front();
    if (m_active->GetModel().GetState() == QueryModel::State::Pending)
        m_active->GetModel().SetState(QueryModel::State::Processing);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Queue::Process()
    {
    DgnPlatformLib::VerifyQueryThread();

    StopWatch timer(false);

    while (WaitForWork())
        {
        if (!m_active.IsValid())
            continue;

        m_active->Query(timer);

        if (true)
            {
            BeMutexHolder lock(m_cv.GetMutex());
            if (QueryModel::State::Pending != m_active->GetModel().GetState())
                m_active->GetModel().SetState(QueryModel::State::Idle);

            m_active = nullptr;
            }

        m_cv.notify_all();
        BeThreadUtilities::BeSleep(getQueryDelay());
        }

    BeMutexHolder lock(m_cv.GetMutex());
    m_state = State::Terminated;
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryModel::Processor::LoadElements(OcclusionScores& scores, bvector<DgnElementCPtr>& elements)
    {
    DgnElements& pool = GetModel().GetDgnDb().Elements();

    for (auto curr = scores.rbegin(); curr != scores.rend(); ++curr)
        {
        if (QueryModel::State::AbortRequested == GetModel().GetState())
            {
            printf("Load elements aborted\n");
            return false;   // did not finish
            }

        bool hitLimit = pool.GetTotalAllocated() > (int64_t) (2 * m_params.m_maxMemory);
        DgnElementId elId(curr->second);
        DgnElementCPtr el = !hitLimit ? pool.GetElement(elId) : pool.FindElement(elId);

        if (el.IsValid())
            elements.push_back(el);
        }

    return true;    // finished
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryModel::Processor::Query(StopWatch& watch)
    {
    DgnPlatformLib::VerifyQueryThread();

    watch.Start();

    //  Notify GraphicsAndQuerySequencer that this thread is running 
    //  a range tree operation and is therefore exempt from checks for high priority required.
    DgnDbR db = GetModel().GetDgnDb();
    qt_RangeTreeOperationBlock qt_RangeTreeOperationBlock(db);

    Filter filter(m_params.m_vp, GetModel(), m_params.m_plan.m_targetNumElements, m_params.m_plan.m_minPixelSize, m_params.m_highPriorityOnly ? nullptr : m_params.m_highPriority, m_params.m_neverDraw);
    if (m_params.m_clipVector.IsValid())
        filter.SetClipVector(*m_params.m_clipVector);

    if (0 != m_params.m_secondaryHitLimit)
        filter.InitializeSecondaryTest(m_params.m_secondaryVolume, m_params.m_secondaryHitLimit);

    m_results = new Results();

    if (m_params.m_highPriorityOnly)
        {
        SearchIdSet(*m_params.m_highPriority, filter);
        }
    else
        {
        SearchRangeTree(filter);
        m_results->m_reachedMaxElements = (filter.m_occlusionScores.size() >= m_params.m_plan.m_targetNumElements);
        }

    if (m_dbStatus != BE_SQLITE_ROW)
        return false;

    LoadElements(filter.m_secondaryFilter.m_occlusionScores, m_results->m_closeElements);
    LoadElements(filter.m_occlusionScores, m_results->m_elements);

    m_results->m_elapsedSeconds = watch.GetCurrentSeconds();
    m_params.m_model.SetUpdatedResults(m_results.get());
                
    // This is not strictly thread-safe. The worst that can happen is that the work thread reads false from it, or sets it to false, while the query thread sets it to true.
    // In that case we skip an update.
    // Alternative is to go through contortions to queue up a "heal viewport" task on the DgnClientFx work thread.
    m_params.m_vp.SetNeedsHeal();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Processor::SearchIdSet(DgnElementIdSet& idList, Filter& filter)
    {
    DgnElements& pool = GetModel().GetDgnDb().Elements();
    for (auto const& curr : idList)
        {
        if (QueryModel::State::AbortRequested == GetModel().GetState())
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
void QueryModel::Processor::SearchRangeTree(Filter& filter)
    {
    do
        {
        if (GraphicsAndQuerySequencer::qt_isOperationRequiredForGraphicsPending())
            {
            for (unsigned i = 0; i < 10 && !filter.CheckAbort(); ++i)
                BeThreadUtilities::BeSleep(2); // Let it run for awhile. If there was one call to SQLite, there will probably be more.

            continue;
            }

        if (filter.CheckAbort())
            break;

        CachedStatementPtr rangeStmt;
        GetModel().GetDgnDb().GetCachedStatement(rangeStmt, m_params.m_searchSql.c_str());

        BindModelAndCategory(*rangeStmt);
        filter.m_restartRangeQuery = false;

        m_dbStatus = filter.StepRTree(*rangeStmt);
        } while (filter.m_restartRangeQuery);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::Filter::Filter(DgnViewportCR viewport, QueryModelR model, uint32_t hitLimit, double minimumSizePixels,
                            DgnElementIdSet const* alwaysDraw, DgnElementIdSet const* exclude)
    : RTreeFilter(viewport, model.GetDgnDb(), minimumSizePixels, exclude), m_model(model), m_useSecondary(false), m_alwaysDraw(nullptr)
    {
    m_hitLimit = hitLimit;
    m_occlusionMapMinimum = 1.0e20;
    m_occlusionMapCount = 0;

    m_secondaryFilter.m_hitLimit = hitLimit;
    m_secondaryFilter.m_occlusionMapCount = 0;
    m_secondaryFilter.m_occlusionMapMinimum = DBL_MAX;

    if (nullptr != alwaysDraw)
        {
        m_lastScore = DBL_MAX;
        for (auto const& id : *alwaysDraw)
            {
            if (nullptr != m_exclude && m_exclude->find(id) != m_exclude->end())
                continue;

            m_passedPrimaryTest = true;
            m_passedSecondaryTest = false;
            RangeAccept(id.GetValueUnchecked());
            }
        }

    //  We do this as the last step. Otherwise, the calls to _RangeAccept in the previous step would not have any effect.
    m_alwaysDraw = alwaysDraw;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void QueryModel::Filter::InitializeSecondaryTest(DRange3dCR volume, uint32_t hitLimit)
    {
    m_useSecondary = true;
    m_secondaryFilter.m_hitLimit = hitLimit;
    m_secondaryFilter.m_occlusionMapCount = 0;
    m_secondaryFilter.m_occlusionMapMinimum = DBL_MAX;
    m_secondaryFilter.m_scorer.Initialize(volume);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
inline static void toLocalCorners(DPoint3dP localCorners, RTree3dValCP pt)
    {
    localCorners[0].x = localCorners[3].x = localCorners[4].x = localCorners[7].x = pt->m_minx;     //       7+------+6
    localCorners[1].x = localCorners[2].x = localCorners[5].x = localCorners[6].x = pt->m_maxx;     //       /|     /|
                                                                                                    //      / |    / |
    localCorners[0].y = localCorners[1].y = localCorners[4].y = localCorners[5].y = pt->m_miny;     //     / 4+---/--+5
    localCorners[2].y = localCorners[3].y = localCorners[6].y = localCorners[7].y = pt->m_maxy;     //   3+------+2 /    y   z
                                                                                                    //    | /    | /     |  /
    localCorners[0].z = localCorners[1].z = localCorners[2].z = localCorners[3].z = pt->m_minz;     //    |/     |/      |/
    localCorners[4].z = localCorners[5].z = localCorners[6].z = localCorners[7].z = pt->m_maxz;     //   0+------+1      *---x
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
int QueryModel::Filter::_TestRange(QueryInfo const& info)
    {
    BeAssert(6 == info.m_nCoord);
    info.m_within = Within::Outside;

    if (CheckAbort())
        return BE_SQLITE_ERROR;

    RTree3dValCP pt = (RTree3dValCP) info.m_coords;
    m_passedPrimaryTest   = (info.m_parentWithin == Within::Inside) ? true : (m_boundingRange.Intersects(*pt) && SkewTest(pt));
    m_passedSecondaryTest = m_useSecondary ? m_secondaryFilter.m_scorer.m_boundingRange.Intersects(*pt) : false;

    if (m_passedSecondaryTest)
        {
        if (!m_secondaryFilter.m_scorer.ComputeScore(&m_secondaryFilter.m_lastScore, *pt))
            {
            m_passedSecondaryTest = false;
            }
        else if (m_secondaryFilter.m_occlusionMapCount >= m_secondaryFilter.m_hitLimit && m_secondaryFilter.m_lastScore <= m_secondaryFilter.m_occlusionMapMinimum)
            {
            m_passedSecondaryTest = false;
            }

        if (m_passedSecondaryTest)
            info.m_within = Within::Partly;
        }

    if (!m_passedPrimaryTest)
        return BE_SQLITE_OK;

    DPoint3d localCorners[8];
    toLocalCorners(localCorners, pt);

#if defined (NEEDS_WORK_CLIPPING)
    if (m_clips.IsValid())
        {
        bool allClippedByOnePlane = false;
        for (ConvexClipPlaneSetCR cps : *m_clips)
            {
            if (allClippedByOnePlane = AllPointsClippedByOnePlane(cps, 8, localCorners))
                break;
            }

        if (allClippedByOnePlane)
            {
            m_passedPrimaryTest = false;
            return BE_SQLITE_OK;
            }
        }
#endif

    BeAssert(m_passedPrimaryTest);
    bool overlap, spansEyePlane;

    ++m_nScores;
    if (!m_scorer.ComputeOcclusionScore(&m_lastScore, overlap, spansEyePlane, localCorners, true))
        {
        m_passedPrimaryTest = false;
        }
    else if (m_occlusionMapCount >= m_hitLimit && m_lastScore <= m_occlusionMapMinimum)
        {
        // this box is smaller than the smallest entry we already have, skip it.
        m_passedPrimaryTest = false;
        }

    if (m_passedPrimaryTest)
        {
        m_lastId = info.m_rowid;  // for debugging - make sure we get entries immediately after we score them.

        if (info.m_level>0)
            {
            // For nodes, return 'level-score' (the "-" is because for occlusion score higher is better. But for rtree priority, lower means better).
            info.m_score = info.m_level - m_lastScore;
            info.m_within = info.m_parentWithin == Within::Inside ? Within::Inside : m_boundingRange.Contains(*pt) ? Within::Inside : Within::Partly;
            }
        else
            {
            // For entries (ilevel==0), we return 0 so they are processed immediately (lowest score has highest priority).
            info.m_score = 0;
            info.m_within = Within::Partly;
            }
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Filter::RangeAccept(uint64_t elementId)
    {
    BeAssert(m_lastId == elementId);
    
    if (nullptr != m_exclude && m_exclude->find(DgnElementId(elementId)) != m_exclude->end())
        return;

    if (m_passedPrimaryTest)
        {
        //  Don't add it if the constructor already added it.
        if (nullptr == m_alwaysDraw || m_alwaysDraw->find(DgnElementId(elementId)) == m_alwaysDraw->end())
            {
            ++m_nCalls;

            if (m_occlusionMapCount >= m_hitLimit)
                {
                m_scorer.SetTestLOD(true); // now that we've found a minimum number of elements, start skipping small ones
                m_occlusionScores.erase(m_occlusionScores.begin());
                }
            else
                m_occlusionMapCount++;

            m_occlusionScores.Insert(m_lastScore, elementId);
            m_occlusionMapMinimum = m_occlusionScores.begin()->first;
            }
        }

    if (m_passedSecondaryTest)
        {
        if (m_secondaryFilter.m_occlusionMapCount >= m_secondaryFilter.m_hitLimit)
            m_secondaryFilter.m_occlusionScores.erase(m_secondaryFilter.m_occlusionScores.begin());
        else
            m_secondaryFilter.m_occlusionMapCount++;

        m_secondaryFilter.m_occlusionScores.Insert(m_secondaryFilter.m_lastScore, elementId);
        m_secondaryFilter.m_occlusionMapMinimum = m_secondaryFilter.m_occlusionScores.begin()->first;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
int QueryModel::ProgressiveFilter::_TestRange(QueryInfo const& info)
    {
    BeAssert(6 == info.m_nCoord);
    info.m_within = Within::Outside;

    if (m_context->_CheckStop())
        return BE_SQLITE_ERROR;

    RTree3dValCP pt = (RTree3dValCP) info.m_coords;
    if ((info.m_parentWithin != Within::Inside) && !(m_boundingRange.Intersects(*pt) && SkewTest(pt)))
        return BE_SQLITE_OK;

    DPoint3d localCorners[8];
    toLocalCorners(localCorners, pt);

#if defined (NEEDS_WORK_CLIPPING)
    if (m_clips.IsValid())
        {
        bool allClippedByOnePlane = false;
        for (ConvexClipPlaneSetCR cps : *m_clips)
            {
            if (allClippedByOnePlane = AllPointsClippedByOnePlane(cps, 8, localCorners))
                break;
            }
        if (allClippedByOnePlane)
            return BE_SQLITE_OK;
        }
#endif

    if (info.m_level > 0) // only score nodes, not elements
        {
        bool   overlap, spansEyePlane;
        double score;

        if (!m_scorer.ComputeOcclusionScore(&score, overlap, spansEyePlane, localCorners, true))
            return BE_SQLITE_OK;

        info.m_within = info.m_parentWithin == Within::Inside ? Within::Inside : m_boundingRange.Contains(*pt) ? Within::Inside : Within::Partly;
        info.m_score = info.m_maxLevel - info.m_level - score;
        }
    else
        {
        info.m_score = 0;
        info.m_within = Within::Partly;
        }                                                                                                                                  
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveDisplay::Completion QueryModel::ProgressiveFilter::_Process(ViewContextR context, uint32_t batchSize)
    {
    DgnPlatformLib::VerifyClientThread();

    // Progressive display happens on the client thread. It uses SQLite, and therefore cannot run at the same time 
    // as the query thread (that causes deadlocks, race conditions, crashes, etc.). This test is the only necessary 
    // synchronization to ensure that they do not run at the same time. It tests (unsynchronized) whether the query 
    // queue is currently idle. If not, we simply return "aborted" and wait for the next chance to begin. If the 
    // query queue is empty and inactive, it can't be restarted during this call because only this thread can add entries to it.
    // NOTE: this test is purposely for whether the query thread has work for ANY QueryModel, not just the one we're 
    // attempting to display - that's necessary.  KAB
    if (!m_dgndb.QueryModels().IsIdle())
        {
        return Completion::Aborted;
        }

    m_context = &context;
    m_nThisPass = m_thisBatch = 0; // restart every pass
    m_batchSize = batchSize;
    m_setTimeout = false;

    DEBUG_PRINTF("start progressive display\n");
    if (BE_SQLITE_ROW != StepRTree(*m_rangeStmt))
        {
        m_rangeStmt->Reset();
        DEBUG_PRINTF("aborted progressive display\n");
        return Completion::Aborted;
        }
    DEBUG_PRINTF("finished progressive display\n");

    return Completion::Finished;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::ProgressiveFilter::~ProgressiveFilter()
    {
    m_rangeStmt = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::ProgressiveFilter::_StepRange(DbFunction::Context&, int nArgs, DbValue* args) 
    {
    if (m_context->WasAborted())
        return;

    // for restarts, skip calls up to the point where we finished last pass
    if (++m_nThisPass < m_nLastPass)
        return;

    ++m_nLastPass;

    DgnElementId elementId(args->GetValueUInt64());
    if (nullptr != m_exclude && m_exclude->find(elementId) != m_exclude->end())
        return;

    if (m_queryModel.FindElementById(elementId))
        return;

    DgnElements& pool = m_dgndb.Elements();
    DgnElementCPtr el = pool.GetElement(elementId);
    if (el.IsValid())
        {
        GeometrySourceCP geomElem = el->ToGeometrySource();
        if (nullptr != geomElem)
            {
            m_context->VisitElement(*geomElem);

            if (!m_setTimeout)
                { // don't set the timeout until after we've drawn one element
                m_context->EnableStopAfterTimout(1000);
                m_setTimeout = true;
                }

            if (m_batchSize && ++m_thisBatch >= m_batchSize) // limit the number or elements added per batch (optionally)
                m_context->SetAborted();
            }
        }

    if (pool.GetTotalAllocated() < (int64_t) m_elementReleaseTrigger)
        return;

    pool.DropFromPool(*el);

    // Purging the element does not purge the symbols so it may be necessary to do a full purge
    if (pool.GetTotalAllocated() < (int64_t) m_purgeTrigger)
        return;

    pool.Purge(m_elementReleaseTrigger);   // Try to get back to the elementPurgeTrigger

    static const double s_purgeFactor = 1.3;

    // The purge may not have succeeded if there are elements in the QueryView's list of elements and those elements hold symbol references.
    // When that is true, we leave it to QueryViewController::_DrawView to try to clean up.  This logic just tries to recover from the
    // growth is caused.  It allows some growth between calls to purge to avoid spending too much time in purge.
    uint64_t newTotalAllocated = (uint64_t)pool.GetTotalAllocated();
    m_purgeTrigger = (uint64_t)(s_purgeFactor * std::max(newTotalAllocated, m_elementReleaseTrigger));
    }

