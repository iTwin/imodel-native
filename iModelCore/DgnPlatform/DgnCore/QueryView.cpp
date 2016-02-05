/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/QueryView.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/QueryView.h>

#if !defined(BENTLEYCONFIG_VIRTUAL_MEMORY)
    #include <Bentley/BeSystemInfo.h>
#endif

#define TRACE_QUERY_LOGIC 1
#ifdef TRACE_QUERY_LOGIC
#   define DEBUG_PRINTF THREADLOG.debugv
#   define DEBUG_ERRORLOG THREADLOG.errorv
#else
#   define DEBUG_PRINTF(fmt, ...)
#   define DEBUG_ERRORLOG(fmt, ...)
#endif
#define TRACE_QUERY_LOGIC 1

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnQueryView::DgnQueryView(DgnDbR db, DgnViewId id) : CameraViewController(db, id)
    {
    m_query.Prepare(db, 
                    "SELECT e.Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " AS e, " 
                    DGN_TABLE(DGN_CLASSNAME_SpatialElement) " AS g "
                    "WHERE g.ElementId=e.Id AND InVirtualSet(?1,e.ModelId,g.CategoryId) AND e.Id=?2");
    m_query.BindVirtualSet(1, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnQueryView::~DgnQueryView()
    {
    RequestAbort(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::RequestAbort(bool wait) 
    {
    DgnDb::VerifyClientThread();

    auto& queue = m_dgndb.GetQueryQueue();
    queue.RemovePending(*this);

    SetAbortQuery(true);
    if (wait)
        queue.WaitFor(*this);
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::_OnUpdate(DgnViewportR vp, UpdatePlan const& plan)
    {
    if (m_forceNewQuery || FrustumChanged(vp))
        QueueQuery(vp, plan);

    if (plan.GetQuery().WantWait())
        GetDgnDb().GetQueryQueue().WaitForIdle();

//    PickUpResults();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnQueryView::FrustumChanged(DgnViewportCR vp) const
    {
    Frustum newFrustumPoints = vp.GetFrustum(DgnCoordSystem::World, true);
    return newFrustumPoints != m_saveQueryFrustum;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
void DgnQueryView::QueueQuery(DgnViewportR viewport, UpdatePlan const& plan)
    {
    m_startQueryFrustum = viewport.GetFrustum(DgnCoordSystem::World, true);
    m_saveQueryFrustum.Invalidate();

    m_dgndb.GetQueryQueue().Add(*new DgnQueryQueue::Task(*this, viewport, plan.GetQuery()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::SetAlwaysDrawn(DgnElementIdSet const& newSet, bool exclusive)
    {
    RequestAbort(true);
    m_alwaysDrawn = newSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::ClearAlwaysDrawn()
    {
    RequestAbort(true);
    m_alwaysDrawn.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::SetNeverDrawn(DgnElementIdSet const& newSet)
    {
    RequestAbort(true);
    m_neverDrawn = newSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::ClearNeverDrawn()
    {
    RequestAbort(true);
    m_neverDrawn.clear();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::_ChangeModelDisplay(DgnModelId modelId, bool onOff) 
    {
    if (onOff == m_viewedModels.Contains(modelId))
        return;

    if (onOff)
        {
        m_viewedModels.insert(modelId);
        //  Ensure the model is in the m_loadedModels list.  QueryModel 
        //  must not do this in the query thread.
        m_dgndb.Models().GetModel(modelId);
        }
    else
        {
        m_viewedModels.erase(modelId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::_OnCategoryChange(bool singleEnabled) 
    {
    T_Super::_OnCategoryChange(singleEnabled); 
    }

BEGIN_UNNAMED_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct RTreeFitFilter : DgnQueryView::RTreeQuery
    {
    DRange3d m_fitRange;
    DRange3d m_lastRange;

    virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;

public:
    RTreeFitFilter() {m_fitRange = DRange3d::NullRange();}
    DRange3dCR GetRange() const {return m_fitRange;}
    };

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
int RTreeFitFilter::_TestRTree(RTreeMatchFunction::QueryInfo const& info)
    {
    RTree3dValCP testRange = (RTree3dValCP) info.m_coords;

    if (m_fitRange.IsContained(testRange->m_minx, testRange->m_miny, testRange->m_minz) &&
        m_fitRange.IsContained(testRange->m_maxx, testRange->m_maxy, testRange->m_maxz))
        {
        info.m_within = RTreeMatchFunction::Within::Outside; // If this range is entirely contained there is no reason to continue (it cannot contribute to the fit)
        }
    else
        {
        info.m_within = RTreeMatchFunction::Within::Partly; 
        info.m_score  = info.m_level; // to get depth-first traversal
        if (info.m_level == 0)
            m_lastRange = DRange3d::From(testRange->m_minx, testRange->m_miny, testRange->m_minz, testRange->m_maxx, testRange->m_maxy, testRange->m_maxz);
        }

    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnQueryView::_TestElement(DgnElementId elId) 
    {
    if (m_neverDrawn.Contains(elId))
        return false;

    m_query.BindId(2, elId);
    bool stat = (BE_SQLITE_ROW == m_query.Step());
    m_query.Reset();
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::QueryModelExtents(DRange3dR range, DgnViewportR vp)
    {
    RTreeFitFilter filter;
    filter.Init(m_dgndb);

    DgnElementId thisId;
    while ((thisId = filter.StepRtree()).IsValid())
        {
        if (_TestElement(thisId))
            range.Extend(filter.m_lastRange);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController::FitComplete DgnQueryView::_ComputeFitRange(DRange3dR range, DgnViewportR vp, FitViewParamsR params) 
    {
    range = GetViewedExtents();
    Transform  transform;
    transform.InitFrom((nullptr == params.m_rMatrix) ? vp.GetRotMatrix() : *params.m_rMatrix);
    transform.Multiply(range, range);

    return FitComplete::Yes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnQueryView::_IsInSet(int nVals, DbValue const* vals) const 
    {
    BeAssert(nVals == 2);   // we need ModelId and Category

    // check that both the model is on and the category is on.
    return m_viewedModels.Contains(DgnModelId(vals[0].GetValueUInt64())) && m_viewedCategories.Contains(DgnCategoryId(vals[1].GetValueUInt64()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::_OnAttachedToViewport(DgnViewportR) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::_DrawView(ViewContextR context) 
    {
    _VisitAllElements(context);

    // Allow models to participate in picking
    for (DgnModelId modelId : GetViewedModels())
        {
        if (context.CheckStop())
            return;

        DgnModelPtr model = GetDgnDb().Models().GetModel(modelId);
        auto geomModel = model.IsValid() ? model->ToGeometricModelP() : nullptr;

        if (nullptr != geomModel)
            geomModel->DrawModel(context);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::_InvalidateScene() 
    {
    RequestAbort(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnQueryView::AddtoSceneQuick(SceneContextR context, ProgressiveElements& progressive, QueryResults& results)
    {
    DgnElements& pool = m_dgndb.Elements();

    context.SetNoStroking(true);
    bool allReady = true;

    // first, run through the query results seeing if all of the elements are loaded and have their graphis ready
    for (auto& thisScore : results.m_scores)
        {
        DgnElementCPtr thisElement = pool.FindElement(thisScore.second);
        if (!thisElement.IsValid())
            {
            allReady = false;
            continue;
            }
        GeometrySourceCP geom = thisElement->ToGeometrySource();
        if (nullptr != geom)
            {
            context.VisitElement(*geom);
            progressive.m_inScene.insert(thisScore.second);
            }

        if (context.WasAborted())
            break;
        }
    return context.SetNoStroking(false) && allReady;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::_CreateScene(SceneContextR context) 
    {
    QueryResultsPtr results;
    std::swap(results, m_results);

    DgnViewportR vp = *context.GetViewport();
    auto& plan = context.GetUpdatePlan().GetQuery();

    if (!results.IsValid())
        {
        if (!plan.WantWait())
            return;

        RequestAbort(true);
        results = QueryByRange(vp, plan);
        }

    RefCountedPtr<ProgressiveElements> progressive = new ProgressiveElements(*this);

    if (!AddtoSceneQuick(context, *progressive, *results))
        {
        DgnElements& pool = m_dgndb.Elements();
        for (auto& thisScore : results->m_scores)
            {
            if (progressive->m_inScene.Contains(thisScore.second))
                continue; // was added during "quick" pass

            DgnElementCPtr thisElement = pool.GetElement(thisScore.second);
            if (!thisElement.IsValid())
                continue;

            GeometrySourceCP geom = thisElement->ToGeometrySource();
            if (nullptr != geom)
                {
                context.VisitElement(*geom);
                progressive->m_inScene.insert(thisScore.second);
                }

            if (context.WasAborted())
                break;
            }
        }

    // Next, allow external data models to draw or schedule external data.
    for (DgnModelId modelId : GetViewedModels())
        {
        DgnModelPtr model = m_dgndb.Models().GetModel(modelId);
        auto geomModel = model.IsValid() ? model->ToGeometricModelP() : nullptr;
        if (nullptr != geomModel)
            geomModel->AddGraphicsToScene(context);
        }

    BeAssert(progressive->m_inScene.size() <= results->GetCount());
    if (results->m_incomplete || progressive->m_inScene.size() != results->GetCount())
        {
        progressive->m_rangeQuery.Init(m_dgndb); // the progressive elements are found via this query
        progressive->m_rangeQuery.SetViewport(vp, 6.0, 1.0);

        if (GetClipVector().IsValid())
            progressive->m_rangeQuery.SetClipVector(*GetClipVector());

        vp.ScheduleProgressiveTask(*progressive);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Visit all of the elements in a DgnQueryView. This is used for picking, etc.
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::_VisitAllElements(ViewContextR context)
    {
    RangeQuery rangeQuery;
    rangeQuery.Init(m_dgndb);
    rangeQuery.SetFrustum(context.GetFrustum());

    // the range tree will return all elements in the volume. Filter them by the view criteria
    DgnElementId thisId;
    int count = 0;
    while ((thisId = rangeQuery.StepRtree()).IsValid())
        {           
        if (_TestElement(thisId))
            {
            ++count;
            VisitElement(context, thisId);
            }

        if (context.CheckStop())
            {
            DEBUG_PRINTF("pick aborted %d", count);
            return;
            }
        }
    DEBUG_PRINTF("pick finished %d", count);
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
uint64_t DgnQueryView::GetMaxElementMemory()
    {
    BeAssert(m_maxElementMemory != 0);
    return m_maxElementMemory != 0 ? m_maxElementMemory : 20 * 1024 * 1024;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2013
//---------------------------------------------------------------------------------------
#if defined (_X64_)
uint64_t DgnQueryView::ComputeMaxElementMemory(DgnViewportCR vp)
    {
    uint64_t oneGig = 1024 * 1024 * 1024;
    m_maxElementMemory = 8 * oneGig;
    return m_maxElementMemory;
    }
#else
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2013
//---------------------------------------------------------------------------------------
uint64_t DgnQueryView::ComputeMaxElementMemory(DgnViewportCR vp)
    {
    uint64_t oneMeg = 1024 * 1024;
#if defined (BENTLEYCONFIG_VIRTUAL_MEMORY)
    uint64_t baseValue = 2000;
#else
    uint64_t baseValue = BeSystemInfo::GetAmountOfPhysicalMemory() > (600 * oneMeg) ? 50 : 30;
#endif
    baseValue *= oneMeg;

    int32_t inputFactor = 0; // NEEDS_WORK_CONTINUOUS_RENDER  _GetMaxElementFactor(vp);
    bool decrease = false;
    if (inputFactor < 0)
        {
        decrease = true;
        inputFactor = -inputFactor;
        }

    if (inputFactor > 100)
        inputFactor = 100;

    double maxMemoryFactor = inputFactor/100.0;

    if (decrease)
        {
        uint64_t decrementRange = baseValue - 7 * oneMeg;
        baseValue -= static_cast <uint64_t> (static_cast <double> (decrementRange) * maxMemoryFactor);
        }
    else
        {
        uint64_t incrementRange = 70 * oneMeg;
        baseValue += static_cast <uint64_t> (static_cast <double> (incrementRange) * maxMemoryFactor);
        }

    m_maxElementMemory = baseValue;
    return baseValue;
    }
#endif
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
THREAD_MAIN_IMPL DgnQueryQueue::Main(void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName("QueryModel");
    DgnDb::SetThreadId(DgnDb::ThreadId::Query);

    ((DgnQueryQueue*)arg)->Process();

    // After the owning DgnDb calls Terminate()
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryQueue::Terminate()
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
void DgnQueryQueue::RemovePending(DgnQueryViewR view)
    {
    DgnDb::VerifyClientThread();

    // We may currently be processing a query for this model. If so, let it complete and queue up another one.
    // But remove any other previously-queued processing requests for this model.
    BeMutexHolder lock(m_cv.GetMutex());

    for (auto iter = m_pending.begin(); iter != m_pending.end(); /*...*/)
        {
        if ((*iter)->IsForView(view))
            iter = m_pending.erase(iter);
        else
            ++iter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryQueue::Add(Task& task)
    {
    DgnDb::VerifyClientThread();

    if (&task.m_view.GetDgnDb() != &m_db)
        {
        BeAssert(false);
        return;
        }

    BeMutexHolder mux(m_cv.GetMutex());

    RemovePending(task.m_view);
    m_pending.push_back(&task);

    mux.unlock(); // release lock before notify so other thread will start immediately vs. "hurry up and wait" problem
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryQueue::WaitFor(DgnQueryViewR view)
    {
    DgnDb::VerifyClientThread();

    BeMutexHolder holder(m_cv.GetMutex());
    while (m_active.IsValid() && m_active->IsForView(view))
        m_cv.InfiniteWait(holder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnQueryQueue::DgnQueryQueue(DgnDbR db) : m_db(db), m_state(State::Active)
    {
    BeThreadUtilities::StartNewThread(50*1024, Main, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnQueryQueue::IsIdle() const
    {
    BeMutexHolder holder(m_cv.GetMutex());
    return m_pending.empty() && !m_active.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnQueryQueue::WaitForWork()
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
void DgnQueryQueue::Process()
    {
    DgnDb::VerifyQueryThread();

    while (WaitForWork())
        {
        if (!m_active.IsValid())
            continue;

        m_active->Process();
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
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryQueue::Task::Process()
    {
    DgnDb::VerifyQueryThread();
    m_view.QueryByRange(m_vp, m_plan);

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    // This is not strictly thread-safe. The worst that can happen is that the work thread reads false from it, or sets it to false, while the query thread sets it to true.
    // In that case we skip an update.
    // Alternative is to go through contortions to queue up a "heal viewport" task on the DgnClientFx work thread.
    m_params.m_vp.SetNeedsHeal();
#endif
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::SearchIdSet(DgnElementIdSet& idList, Filter& filter)
    {
    DgnElements& pool = m_dgndb.Elements();
    for (auto const& curr : idList)
        {
        if (AbortRequested())
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnQueryView::QueryResultsPtr DgnQueryView::QueryByRange(DgnViewportCR vp, UpdatePlan::Query const& plan)
    {
    StopWatch watch(true);
    SetAbortQuery(false); // gets turned on by client thread

    DEBUG_PRINTF("Query started");

    RangeQuery rangeQuery;
    rangeQuery.Init(m_dgndb);
    rangeQuery.SetViewport(vp, plan.m_minPixelSize, plan.m_frustumScale);

    if (m_clipVector.IsValid())
        rangeQuery.SetClipVector(*m_clipVector);

    QueryResultsPtr results = new QueryResults();
    uint64_t endTime = plan.GetTimeout() ? (BeTimeUtilities::QueryMillisecondsCounter() + plan.GetTimeout()) : 0;

    rangeQuery.m_hitLimit = plan.GetTargetNumElements();
    DgnElementId thisId;
    while ((thisId=rangeQuery.StepRtree()).IsValid())
        {
        if (m_abortQuery)
            {
            DEBUG_ERRORLOG("Query aborted");
            return results;
            }

        if (_TestElement(thisId))
            {
            if (++rangeQuery.m_count >= rangeQuery.m_hitLimit)
                {
                rangeQuery.SetTestLOD(true); // now that we've found a minimum number of elements, start skipping small ones
                results->m_scores.erase(m_results->m_scores.begin());
                results->m_incomplete = true;
                rangeQuery.m_count = rangeQuery.m_hitLimit;
                }

            results->m_scores.Insert(rangeQuery.m_lastScore, thisId);
            rangeQuery.m_minScore = results->m_scores.begin()->first;
            }

        if (endTime && (BeTimeUtilities::QueryMillisecondsCounter() > endTime))
            {
            DEBUG_ERRORLOG("Query timeout");
            results->m_incomplete = true;
            break;
            }
        };

    double total = watch.GetCurrentSeconds();
    DEBUG_PRINTF("Query completed, total=%d, progressive=%d, time=%f", results->GetCount(), results->m_incomplete, total);
    return results;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnQueryView::Filter::Filter(DgnDbR db, QueryResults& results, uint32_t hitLimit, DgnElementIdSet const* alwaysDraw, DgnElementIdSet const* exclude)
    :  m_results(&results), m_hitLimit(hitLimit), m_alwaysDraw(nullptr), RTreeFilter(db, exclude)
    {
    m_minScore = 1.0e20;

    if (nullptr != alwaysDraw)
        {
        m_lastScore = DBL_MAX;
        for (auto const& id : *alwaysDraw)
            AcceptElement(id);
        }

    // We do this as the last step. Otherwise, the calls to AcceptElement in the previous step would not have any effect.
    m_alwaysDraw = alwaysDraw;
    }
#endif

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
int DgnQueryView::RangeQuery::_TestRTree(RTreeMatchFunction::QueryInfo const& info)
    {
    BeAssert(6 == info.m_nCoord);
    info.m_within = RTreeMatchFunction::Within::Outside;

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

    if (!ComputeOcclusionScore(&m_lastScore, overlap, spansEyePlane, localCorners))
        return BE_SQLITE_OK;

    if (m_hitLimit && (m_count >= m_hitLimit && m_lastScore <= m_minScore))
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

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
int DgnQueryView::AllElementsFilter::_TestRTree(RTreeMatchFunction::QueryInfo const& info)
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion DgnQueryView::ProgressiveElements::_DoProgressive(SceneContext& context, WantShow& wantShow)
    {
    m_thisBatch = 0; // restart every pass
    m_batchSize = context.GetUpdatePlan().GetQuery().GetTargetNumElements();
    m_setTimeout = false;

    DEBUG_PRINTF("begin progressive display");

    DgnElementId thisId;
    while ((thisId=m_rangeQuery.StepRtree()).IsValid())
        {
        if (!m_inScene.Contains(thisId) && m_view._TestElement(thisId))
            {
            m_view.VisitElement(context, thisId); // no, draw it now

            if (!m_setTimeout) // don't set the timeout until after we've drawn one element
                {
                context.EnableStopAfterTimout(SHOW_PROGRESS_INTERVAL);
                m_setTimeout = true;
                }
            }

        if (m_batchSize && ++m_thisBatch >= m_batchSize) // limit the number or elements added per batch
            context.SetAborted();

        if (context.CheckStop())
            break;
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
StatusInt DgnQueryView::VisitElement(ViewContextR context, DgnElementId elementId) 
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (m_model.FindElementById(elementId))
        return false;

    if (nullptr != m_exclude && m_exclude->find(elementId) != m_exclude->end())
        return false;
#endif

    DgnElements& pool = m_dgndb.Elements();
    DgnElementCPtr el = pool.GetElement(elementId);
    if (!el.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    GeometrySourceCP geomElem = el->ToGeometrySource();
    if (nullptr != geomElem)
        return context.VisitElement(*geomElem);

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (pool.GetTotalAllocated() < (int64_t) m_elementReleaseTrigger)
        return true;

    pool.DropFromPool(*el);

    // Purging the element does not purge the symbols so it may be necessary to do a full purge
    if (pool.GetTotalAllocated() < (int64_t) m_purgeTrigger)
        return true;

    pool.Purge(m_elementReleaseTrigger);   // Try to get back to the elementPurgeTrigger

    static const double s_purgeFactor = 1.3;

    // The purge may not have succeeded if there are elements in the QueryView's list of elements and those elements hold symbol references.
    // When that is true, we leave it to QueryView::_DrawView to try to clean up.  This logic just tries to recover from the
    // growth is caused.  It allows some growth between calls to purge to avoid spending too much time in purge.
    uint64_t newTotalAllocated = (uint64_t)pool.GetTotalAllocated();
    m_purgeTrigger = (uint64_t)(s_purgeFactor * std::max(newTotalAllocated, m_elementReleaseTrigger));
#endif
    return true;
    }
