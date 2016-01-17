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
#else
#   define DEBUG_PRINTF(fmt, ...)
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

    // Now add everything that is in the secondary list but not the first.
    for (auto const& result : m_currQueryResults->m_closeElements)
        _OnLoadedElement(*result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t QueryModel::GetElementCount() const {return m_currQueryResults.IsValid() ? m_currQueryResults->GetCount() : 0;}

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
bool QueryModel::Processor::Query(StopWatch& watch)
    {
    DgnDb::VerifyQueryThread();

    watch.Start();
    GetModel().SetAbortQuery(false);

    DEBUG_PRINTF("Query started");
    Filter filter(GetModel(), m_params.m_plan.m_targetNumElements, m_params.m_highPriorityOnly ? nullptr : m_params.m_highPriority, m_params.m_neverDraw);
    filter.SetViewport(m_params.m_vp, m_params.m_plan.m_minPixelSize);
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
        m_results->m_reachedMaxElements = ((uint32_t)filter.m_occlusionScores.size() >= m_params.m_plan.m_targetNumElements);
        }

    if (m_dbStatus != BE_SQLITE_DONE)
        return false;

    DEBUG_PRINTF("loading elements");
    LoadElements(filter.m_secondaryFilter.m_occlusionScores, m_results->m_closeElements);
    LoadElements(filter.m_occlusionScores, m_results->m_elements);

    m_params.m_model.m_updatedResults = m_results;
    DEBUG_PRINTF("Query completed, %f seconds", watch.GetCurrentSeconds());
                
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
    CachedStatementPtr rangeStmt;
    GetModel().GetDgnDb().GetCachedStatement(rangeStmt, m_params.m_searchSql.c_str());

    static_cast<QueryViewControllerCP>(&m_params.m_vp.GetViewController())->BindModelAndCategory(*rangeStmt, filter);

    while(BE_SQLITE_ROW == (m_dbStatus=rangeStmt->Step()))
        {
        if (filter.CheckAbort())
            {
            DEBUG_PRINTF("Query aborted");
            return;
            }

        filter.AcceptElement(rangeStmt->GetValueId<DgnElementId>(0));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
QueryModel::Filter::Filter(QueryModelR model, uint32_t hitLimit, DgnElementIdSet const* alwaysDraw, DgnElementIdSet const* exclude)
    : m_model(model), m_useSecondary(false), m_alwaysDraw(nullptr), RTreeFilter(exclude)
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
            AcceptElement(id);
            }
        }

    //  We do this as the last step. Otherwise, the calls to RangeAccept in the previous step would not have any effect.
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
int QueryModel::Filter::_TestRTree(RTreeMatchFunction::QueryInfo const& info)
    {
    BeAssert(6 == info.m_nCoord);
    info.m_within = RTreeMatchFunction::Within::Outside;

    if (CheckAbort())
        return BE_SQLITE_ERROR;

    RTree3dValCP pt = (RTree3dValCP) info.m_coords;
    m_passedPrimaryTest   = (info.m_parentWithin == RTreeMatchFunction::Within::Inside) ? true : (m_boundingRange.Intersects(*pt) && SkewTest(pt));
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
            info.m_within = RTreeMatchFunction::Within::Partly;
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
            info.m_within = info.m_parentWithin == RTreeMatchFunction::Within::Inside ? RTreeMatchFunction::Within::Inside : m_boundingRange.Contains(*pt) ? RTreeMatchFunction::Within::Inside : RTreeMatchFunction::Within::Partly;
            }
        else
            {
            // For entries (ilevel==0), we return 0 so they are processed immediately (lowest score has highest priority).
            info.m_score = 0;
            info.m_within = RTreeMatchFunction::Within::Partly;
            }
        }

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

    if (m_passedPrimaryTest)
        {
        //  Don't add it if the constructor already added it.
        if (nullptr == m_alwaysDraw || m_alwaysDraw->find(elementId) == m_alwaysDraw->end())
            {
            if (m_occlusionMapCount >= m_hitLimit)
                {
                m_scorer.SetTestLOD(true); // now that we've found a minimum number of elements, start skipping small ones
                m_occlusionScores.erase(m_occlusionScores.begin());
                }
            else
                m_occlusionMapCount++;

            m_occlusionScores.Insert(m_lastScore, elementId.GetValueUnchecked());
            m_occlusionMapMinimum = m_occlusionScores.begin()->first;
            }
        }

    if (m_passedSecondaryTest)
        {
        if (m_secondaryFilter.m_occlusionMapCount >= m_secondaryFilter.m_hitLimit)
            m_secondaryFilter.m_occlusionScores.erase(m_secondaryFilter.m_occlusionScores.begin());
        else
            m_secondaryFilter.m_occlusionMapCount++;

        m_secondaryFilter.m_occlusionScores.Insert(m_secondaryFilter.m_lastScore, elementId.GetValueUnchecked());
        m_secondaryFilter.m_occlusionMapMinimum = m_secondaryFilter.m_occlusionScores.begin()->first;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
int QueryModel::AllElementsFilter::_TestRTree(RTreeMatchFunction::QueryInfo const& info)
    {
    BeAssert(6 == info.m_nCoord);
    info.m_within = RTreeMatchFunction::Within::Outside;

    RTree3dValCP pt = (RTree3dValCP) info.m_coords;
    if ((info.m_parentWithin != RTreeMatchFunction::Within::Inside) && !(m_boundingRange.Intersects(*pt) && SkewTest(pt)))
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

    if (info.m_level > 0) // only score nodes, not elements
        {
        bool   overlap, spansEyePlane;

        DPoint3d localCorners[8];
        toLocalCorners(localCorners, pt);

        double score = 0.0;
        if (m_doOcclusionScore && !m_scorer.ComputeOcclusionScore(&score, overlap, spansEyePlane, localCorners, true))
            return BE_SQLITE_OK;

        info.m_within = info.m_parentWithin == RTreeMatchFunction::Within::Inside ? RTreeMatchFunction::Within::Inside : m_boundingRange.Contains(*pt) ? RTreeMatchFunction::Within::Inside : RTreeMatchFunction::Within::Partly;
        info.m_score = info.m_maxLevel - info.m_level - score;
        }
    else
        {
        info.m_score = 0;
        info.m_within = RTreeMatchFunction::Within::Partly;
        }                                                                                                                                  
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveDisplay::Completion QueryModel::ProgressiveFilter::_Process(ViewContextR context, uint32_t batchSize)
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

    DbResult rc;
    DEBUG_PRINTF("begin progressive display");
    while (BE_SQLITE_ROW == (rc=m_rangeStmt->Step()))
        {
        if (context._CheckStop())
            {
            DEBUG_PRINTF("aborted progressive display");
            return Completion::Aborted;
            }

        if (AcceptElement(context, m_rangeStmt->GetValueId<DgnElementId>(0)))
            {
            if (!m_setTimeout)
                { // don't set the timeout until after we've drawn one element
                context.EnableStopAfterTimout(1000);
                m_setTimeout = true;
                }

            if (m_batchSize && ++m_thisBatch >= m_batchSize) // limit the number or elements added per batch (optionally)
                context.SetAborted();
            }
        }

    BeAssert(rc == BE_SQLITE_DONE);
    DEBUG_PRINTF("finished progressive display");
    return Completion::Finished;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryModel::AllElementsFilter::AcceptElement(ViewContextR context, DgnElementId elementId) 
    {
    if (m_queryModel.FindElementById(elementId))
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
