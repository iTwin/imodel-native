/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/QueryModel.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include "UpdateLogging.h"

#define TRACE_QUERY_LOGIC 1

#ifdef TRACE_QUERY_LOGIC
#   define DEBUG_PRINTF THREADLOG.debugv
#   define DEBUG_ERRORLOG THREADLOG.errorv
#else
#   define DEBUG_PRINTF(fmt, ...)
#   define DEBUG_ERRORLOG(fmt, ...)
#endif

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
void QueryModel::RequestAbort(bool wait) 
    {
    DgnDb::VerifyClientThread();
    SetAbortQuery(true);

    auto& queue = GetDgnDb().QueryQueue();
    queue.RemovePending(*this);

    if (wait)
        queue.WaitForIdle();
    }

/*---------------------------------------------------------------------------------**//**
* Move the QueryResults object from the Processor (where it is created on the other thread) to the QueryModel object
* where it can be accessed from the main thread. 
* NOTE: This method must run on the main thread.
* @bsimethod                                    John.Gooding                    06/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::SaveQueryResults()
    {
    DgnDb::VerifyClientThread();

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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t QueryModel::GetElementCount() const 
    {
    return m_currQueryResults.IsValid() ? m_currQueryResults->GetCount() : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::QueryModel(DgnDbR dgndb) : SpatialModel(SpatialModel::CreateParams(dgndb, DgnClassId(), CreateModelCode("Query"))), m_abortQuery(false)
    {
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
THREAD_MAIN_IMPL QueryModel::Queue::Main(void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName("QueryModel");
    DgnDb::SetThreadId(DgnDb::ThreadId::Query);

    ((Queue*)arg)->Process();

    // After the owning DgnDb calls Terminate()
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Queue::Terminate()
    {
    DgnDb::VerifyClientThread();

    BeMutexHolder lock(m_cv.GetMutex());
    if (State::Active != m_state)
        return;

    m_state = State::TerminateRequested;
    while (State::TerminateRequested == m_state)
        {
        m_cv.notify_all();
        m_cv.RelativeWait(lock, 1000);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryModel::Filter::CheckAbort() 
    {
    if (m_model.AbortRequested())
        {
        DEBUG_PRINTF("Query aborted");
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Queue::RemovePending(QueryModelR model)
    {
    DgnDb::VerifyClientThread();

    // We may currently be processing a query for this model. If so, let it complete and queue up another one.
    // But remove any other previously-queued processing requests for this model.
    BeMutexHolder lock(m_cv.GetMutex());

    for (auto iter = m_pending.begin(); iter != m_pending.end(); /*...*/)
        {
        if ((*iter)->IsForModel(model))
            iter = m_pending.erase(iter);
        else
            ++iter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Queue::Add(Processor::Params const& params)
    {
    DgnDb::VerifyClientThread();

    QueryModelR model = params.m_model;
    if (&model.GetDgnDb() != &m_db)
        {
        BeAssert(false);
        return;
        }

    BeMutexHolder mux(m_cv.GetMutex());

    RemovePending(model);
    m_pending.push_back(new QueryModel::Processor(params));

    mux.unlock(); // release lock before notify so other thread will start immediately vs. "hurry up and wait" problem
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Queue::WaitForIdle()
    {
    DgnDb::VerifyClientThread();

    BeMutexHolder holder(m_cv.GetMutex());
    while (m_active.IsValid() || !m_pending.empty())
        m_cv.InfiniteWait(holder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::Queue::Queue(DgnDbR db) : m_db(db), m_state(State::Active)
    {
    BeThreadUtilities::StartNewThread(50*1024, Main, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryModel::Queue::IsIdle() const
    {
    BeMutexHolder holder(m_cv.GetMutex());
    return m_pending.empty() && !m_active.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryModel::Queue::WaitForWork()
    {
    BeMutexHolder holder(m_cv.GetMutex());
    while (m_pending.empty() && State::Active == m_state)
        m_cv.InfiniteWait(holder);

    if (State::Active != m_state)
        return false;

    m_active = m_pending.front();
    m_pending.pop_front();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Queue::Process()
    {
    DgnDb::VerifyQueryThread();

    StopWatch timer(false);

    while (WaitForWork())
        {
        if (!m_active.IsValid())
            continue;

        m_active->Query(timer);
        uint32_t delay = m_active->GetDelayAfter();

        {
        BeMutexHolder holder(m_cv.GetMutex());
        m_active = nullptr;
        }

        m_cv.notify_all();
        if (delay) // optionally, wait before starting the next task
            BeThreadUtilities::BeSleep(delay);
        }

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
        if (GetModel().AbortRequested())
            {
            DEBUG_PRINTF("Load elements aborted");
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
void QueryModel::Processor::DoQuery(StopWatch& watch)
    {
    watch.Start();

    GetModel().SetAbortQuery(false); // gets turned on by client thread

    DEBUG_PRINTF("Query started");
    Filter filter(GetModel(), m_params.m_plan.m_targetNumElements, m_params.m_highPriorityOnly ? nullptr : m_params.m_highPriority, m_params.m_neverDraw);
    filter.SetViewport(m_params.m_vp, m_params.m_plan.m_minPixelSize, m_params.m_plan.m_frustumScale);
    if (m_params.m_clipVector.IsValid())
        filter.SetClipVector(*m_params.m_clipVector);

    m_results = new Results();
    if (m_params.m_highPriorityOnly)
        SearchIdSet(*m_params.m_highPriority, filter);
    else
        SearchRangeTree(filter);

    if (filter.CheckAbort())
        {
        DEBUG_PRINTF("query aborted");
        return;
        }

    m_results->m_needsProgressive = filter.m_needsProgressive;
    DEBUG_PRINTF("loading elements, progressive=%d", m_results->m_needsProgressive);
    LoadElements(filter.m_occlusionScores, m_results->m_elements);

    m_params.m_model.m_updatedResults = m_results;
    DEBUG_PRINTF("Query completed, %f seconds", watch.GetCurrentSeconds());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Processor::Query(StopWatch& watch)
    {
    DgnDb::VerifyQueryThread();
    DoQuery(watch);

    // This is not strictly thread-safe. The worst that can happen is that the work thread reads false from it, or sets it to false, while the query thread sets it to true.
    // In that case we skip an update.
    // Alternative is to go through contortions to queue up a "heal viewport" task on the DgnClientFx work thread.
    m_params.m_vp.SetNeedsHeal();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Processor::SearchIdSet(DgnElementIdSet& idList, Filter& filter)
    {
    DgnElements& pool = GetModel().GetDgnDb().Elements();
    for (auto const& curr : idList)
        {
        if (GetModel().AbortRequested())
            break;

        DgnElementCPtr el = pool.GetElement(curr);
        GeometrySourceCP geom = el.IsValid() ? el->ToGeometrySource() : nullptr;
        if (nullptr == geom || !geom->HasGeometry())
            continue;

        RTree3dVal rtreeRange;
        rtreeRange.FromRange(geom->CalculateRange3d());

        RTreeMatchFunction::QueryInfo info;
        info.m_nCoord = 6;
        info.m_coords = &rtreeRange.m_minx;
        info.m_parentWithin = info.m_within = RTreeMatchFunction::Within::Partly;
        info.m_parentScore  = info.m_score  = 1.0;
        info.m_rowid = curr.GetValue();
        info.m_level = 0;
        filter._TestRTree(info);
        if (RTreeMatchFunction::Within::Outside != info.m_within)
            filter.AcceptElement(curr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Processor::SearchRangeTree(Filter& filter)
    {
    CachedStatementPtr viewStmt;
    Utf8String acceptSql = m_params.m_searchSql.c_str() + filter.GetAcceptSql();
    GetModel().GetDgnDb().GetCachedStatement(viewStmt, acceptSql.c_str());

    QueryViewControllerCR qViewController = static_cast<QueryViewControllerCR>(m_params.m_vp.GetViewController());
    qViewController.BindModelAndCategory(*viewStmt);

    uint64_t endTime = BeTimeUtilities::QueryMillisecondsCounter() + m_params.m_plan.GetTimeout();

    int idCol = viewStmt->GetParameterIndex("@elId");
    BeAssert(0 != idCol);

    uint64_t thisId;
    while (0 != (thisId=filter.StepRtree()))
        {
        viewStmt->Reset();
        viewStmt->BindInt64(idCol, thisId);

        if (filter.CheckAbort())
            {
            DEBUG_ERRORLOG("Query aborted");
            return;
            }

        if (BE_SQLITE_ROW == viewStmt->Step())
            filter.AcceptElement(DgnElementId(thisId));

        if (BeTimeUtilities::QueryMillisecondsCounter() > endTime)
            {
            DEBUG_ERRORLOG("Query timeout");
            filter.m_needsProgressive = true;
            break;
            }
        };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::Filter::Filter(QueryModelR model, uint32_t hitLimit, DgnElementIdSet const* alwaysDraw, DgnElementIdSet const* exclude)
    : m_model(model), m_alwaysDraw(nullptr), RTreeFilter(model.GetDgnDb(), exclude)
    {
    m_hitLimit = hitLimit;
    m_occlusionMapMinimum = 1.0e20;
    m_occlusionMapCount = 0;

    if (nullptr != alwaysDraw)
        {
        m_lastScore = DBL_MAX;
        for (auto const& id : *alwaysDraw)
            AcceptElement(id);
        }

    //  We do this as the last step. Otherwise, the calls to AcceptElement in the previous step would not have any effect.
    m_alwaysDraw = alwaysDraw;
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
int QueryModel::Filter::_TestRTree(RTreeMatchFunction::QueryInfo const& info)
    {
    BeAssert(6 == info.m_nCoord);
    info.m_within = RTreeMatchFunction::Within::Outside;

    if (CheckAbort())
        return BE_SQLITE_ERROR;

    RTree3dValCP pt = (RTree3dValCP) info.m_coords;
    if (!m_boundingRange.Intersects(*pt) || !SkewTest(pt))
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

    bool overlap, spansEyePlane;

    if (!m_scorer.ComputeOcclusionScore(&m_lastScore, overlap, spansEyePlane, localCorners))
        return BE_SQLITE_OK;

    if (m_occlusionMapCount >= m_hitLimit && m_lastScore <= m_occlusionMapMinimum)
        return BE_SQLITE_OK; // this one is smaller than the smallest entry we already have, skip it (and children).

    m_lastId = info.m_rowid;  // for debugging - make sure we get entries immediately after we score them.
    if (info.m_level>0)
        {
        // For nodes, return 'level-score' (the "-" is because for occlusion score higher is better. But for rtree priority, lower means better).
        info.m_score = info.m_level - m_lastScore;
        }
    else
        {
        // For entries (ilevel==0), we return 0 so they are processed immediately (lowest score has highest priority).
        info.m_score = 0;
        }

    info.m_within = RTreeMatchFunction::Within::Partly;
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryModel::Filter::AcceptElement(DgnElementId elementId)
    {
    BeAssert(m_lastId == elementId.GetValueUnchecked());
    
    if (nullptr != m_exclude && m_exclude->find(elementId) != m_exclude->end())
        return;

    //  Don't add it if the constructor already added it.
    if (nullptr != m_alwaysDraw && m_alwaysDraw->find(elementId) != m_alwaysDraw->end())
        return;

    if (m_occlusionMapCount >= m_hitLimit)
        {
        m_scorer.SetTestLOD(true); // now that we've found a minimum number of elements, start skipping small ones
        m_occlusionScores.erase(m_occlusionScores.begin());
        m_needsProgressive = true;
        }
    else
        m_occlusionMapCount++;

    m_occlusionScores.Insert(m_lastScore, elementId.GetValueUnchecked());
    m_occlusionMapMinimum = m_occlusionScores.begin()->first;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
int QueryModel::AllElementsFilter::_TestRTree(RTreeMatchFunction::QueryInfo const& info)
    {
    BeAssert(6 == info.m_nCoord);
    info.m_within = RTreeMatchFunction::Within::Outside;

    RTree3dValCP pt = (RTree3dValCP) info.m_coords;
    if (!m_boundingRange.Intersects(*pt) || !SkewTest(pt))
        return BE_SQLITE_OK;

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

    DPoint3d localCorners[8];
    toLocalCorners(localCorners, pt);

    double score = 0.0;
    bool   overlap, spansEyePlane;

    // NOTE: m_doOcclusionScore is off if we're trying to visit all elements. ComputeOcclusionScore returns false if the
    // size is smaller than our minimum NPC area.
    if (m_doOcclusionScore && !m_scorer.ComputeOcclusionScore(&score, overlap, spansEyePlane, localCorners))
        return BE_SQLITE_OK;

    // For entries (m_level==0), we return 0 so they are processed immediately (lowest score has highest priority).
    info.m_score = (info.m_level>0) ? (info.m_maxLevel - info.m_level - score) : 0;
    info.m_within = RTreeMatchFunction::Within::Partly;
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveDisplay::Completion QueryModel::ProgressiveFilter::_Process(ViewContextR context, uint32_t batchSize, WantShow& wantShow)
    {
    DgnDb::VerifyClientThread();

    // Progressive display happens on the client thread. It uses SQLite, and therefore contends with 
    // the query thread. This test is the only necessary 
    // synchronization to ensure that they do not run at the same time. It tests (unsynchronized) whether the query 
    // queue is currently idle. If not, we simply return "aborted" and wait for the next chance to begin. If the 
    // query queue is empty and inactive, it can't be restarted during this call because only this thread can add entries to it.
    // NOTE: this test is purposely for whether the query thread has work for ANY QueryModel, not just the one we're 
    // attempting to display - that's necessary.  KAB
    if (!m_dgndb.QueryQueue().IsIdle())
        return Completion::Aborted;

    m_thisBatch = 0; // restart every pass
    m_batchSize = batchSize;
    m_setTimeout = false;

    m_scorer.SetTestLOD(true);  

    CachedStatementPtr viewStmt;              
    DgnViewportR vp = *context.GetViewport();
    QueryViewControllerCR queryView = static_cast<QueryViewControllerCR>(vp.GetViewController());

    Utf8String viewSql = queryView._GetQuery(vp) + GetAcceptSql();
    m_dgndb.GetCachedStatement(viewStmt, viewSql.c_str());
    queryView.BindModelAndCategory(*viewStmt);

    DEBUG_PRINTF("begin progressive display");

    uint64_t thisId;
    int idCol = viewStmt->GetParameterIndex("@elId");
    BeAssert(0 != idCol);

    while (0 != (thisId=StepRtree()))
        {
        viewStmt->Reset();
        viewStmt->BindInt64(idCol, thisId);

        if (BE_SQLITE_ROW == viewStmt->Step())
            {
            AcceptElement(context, DgnElementId(thisId));

            if (!m_setTimeout) // don't set the timeout until after we've drawn one element
                {
                context.EnableStopAfterTimout(SHOW_PROGRESS_INTERVAL);
                m_setTimeout = true;
                }
        }

        if (context._CheckStop())
            break;

        if (m_batchSize && ++m_thisBatch >= m_batchSize) // limit the number or elements added per batch (optionally)
            {
            context.SetAborted();
            break;
            }
        }

    if (context.WasAborted())
        {
        // We only want to show the progress of ProgressiveDisplay once per second. 
        // See if its been more than a second since the last time we showed something.
        uint64_t now = BeTimeUtilities::QueryMillisecondsCounter();                                                                                                         
        if (now > m_nextShow)
            {
            m_nextShow = now + SHOW_PROGRESS_INTERVAL;
            wantShow = WantShow::Yes;
            }

        DEBUG_PRINTF("aborted progressive display");
        return Completion::Aborted;
        }

    // alway show the last batch.
    wantShow = WantShow::Yes;
    DEBUG_PRINTF("finished progressive. Total=%d", m_total);
    return Completion::Finished;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryModel::AllElementsFilter::AcceptElement(ViewContextR context, DgnElementId elementId) 
    {
    if (m_model.FindElementById(elementId))
        return false;

    if (nullptr != m_exclude && m_exclude->find(elementId) != m_exclude->end())
        return false;

    DgnElements& pool = m_dgndb.Elements();
    DgnElementCPtr el = pool.GetElement(elementId);
    if (!el.IsValid())
        {
        BeAssert(false);
        return false;
        }

    GeometrySourceCP geomElem = el->ToGeometrySource();
    if (nullptr != geomElem)
        context.VisitElement(*geomElem);

    if (pool.GetTotalAllocated() < (int64_t) m_elementReleaseTrigger)
        return true;

    pool.DropFromPool(*el);

    // Purging the element does not purge the symbols so it may be necessary to do a full purge
    if (pool.GetTotalAllocated() < (int64_t) m_purgeTrigger)
        return true;

    pool.Purge(m_elementReleaseTrigger);   // Try to get back to the elementPurgeTrigger

    static const double s_purgeFactor = 1.3;

    // The purge may not have succeeded if there are elements in the QueryView's list of elements and those elements hold symbol references.
    // When that is true, we leave it to QueryViewController::_DrawView to try to clean up.  This logic just tries to recover from the
    // growth is caused.  It allows some growth between calls to purge to avoid spending too much time in purge.
    uint64_t newTotalAllocated = (uint64_t)pool.GetTotalAllocated();
    m_purgeTrigger = (uint64_t)(s_purgeFactor * std::max(newTotalAllocated, m_elementReleaseTrigger));
    return true;
    }
