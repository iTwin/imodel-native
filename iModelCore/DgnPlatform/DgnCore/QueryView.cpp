/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/QueryView.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/QueryView.h>
#include <Bentley/BeSystemInfo.h>

#define TRACE_QUERY_LOGIC 1
#ifdef TRACE_QUERY_LOGIC
#   define DEBUG_PRINTF THREADLOG.debugv
#   define DEBUG_ERRORLOG THREADLOG.errorv
#else
#   define DEBUG_PRINTF(fmt, ...)
#   define DEBUG_ERRORLOG(fmt, ...)
#endif

BEGIN_UNNAMED_NAMESPACE
static const double   s_cameraLimit = 1.0E-5;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct RTreeFitFilter : DgnQueryView::SpatialQuery
    {
    DRange3d m_fitRange;
    DRange3d m_lastRange;

    virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;

public:
    RTreeFitFilter(DgnQueryView::SpecialElements const* special) : DgnQueryView::SpatialQuery(special) {m_fitRange = DRange3d::NullRange();}
    DRange3dCR GetRange() const {return m_fitRange;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct CornerBox : Frustum
{
    CornerBox(RTree3dValCR from)
        {
        m_pts[0].x = m_pts[3].x = m_pts[4].x = m_pts[7].x = from.m_minx;   //       7+------+6
        m_pts[1].x = m_pts[2].x = m_pts[5].x = m_pts[6].x = from.m_maxx;   //       /|     /|
                                                                           //      / |    / |
        m_pts[0].y = m_pts[1].y = m_pts[4].y = m_pts[5].y = from.m_miny;   //     / 4+---/--+5
        m_pts[2].y = m_pts[3].y = m_pts[6].y = m_pts[7].y = from.m_maxy;   //   3+------+2 /    y   z
                                                                           //    | /    | /     |  /
        m_pts[0].z = m_pts[1].z = m_pts[2].z = m_pts[3].z = from.m_minz;   //    |/     |/      |/
        m_pts[4].z = m_pts[5].z = m_pts[6].z = m_pts[7].z = from.m_maxz;   //   0+------+1      *---x
        }
    
    CornerBox(DRange3dCR range)
        {
        m_pts[0].x = m_pts[3].x = m_pts[4].x = m_pts[7].x = range.low.x; 
        m_pts[1].x = m_pts[2].x = m_pts[5].x = m_pts[6].x = range.high.x;
        m_pts[0].y = m_pts[1].y = m_pts[4].y = m_pts[5].y = range.low.y; 
        m_pts[2].y = m_pts[3].y = m_pts[6].y = m_pts[7].y = range.high.y;
        m_pts[0].z = m_pts[1].z = m_pts[2].z = m_pts[3].z = range.low.y; 
        m_pts[4].z = m_pts[5].z = m_pts[6].z = m_pts[7].z = range.high.z;
        }
};
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::FrustumClips::AddFrustum(FrustumCR frustum)
    {
    resize(6);
    ClipUtil::RangePlanesFromFrustum(&front(), frustum, true, true, 1.0E-6);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::RTreeMatchFunction::Within DgnQueryView::FrustumClips::TestBox(FrustumCR frustum)
    {
    auto containment = ClassifyPointContainment(frustum.m_pts, 8);
    if (containment == ClipPlaneContainment_StronglyInside) 
        return RTreeMatchFunction::Within::Inside;
    if (containment == ClipPlaneContainment_StronglyOutside) 
        return RTreeMatchFunction::Within::Outside;
    return RTreeMatchFunction::Within::Partly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnQueryView::DgnQueryView(DgnDbR db, DgnViewId id) : CameraViewController(db, id)
    {
    m_viewSQL = "SELECT e.Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " AS e, " DGN_TABLE(DGN_CLASSNAME_GeometricElement3d) " AS g "
                "WHERE g.ElementId=e.Id AND InVirtualSet(@vset,e.ModelId,g.CategoryId) AND e.Id=@elId";
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::_OnUpdate(DgnViewportR vp, UpdatePlan const& plan)
    {
    BeAssert(plan.GetQuery().GetTargetNumElements() > 0);
    BeAssert(plan.GetQuery().GetTargetNumElements() <= plan.GetQuery().GetMaxElements());

    QueueQuery(vp, plan.GetQuery());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
void DgnQueryView::QueueQuery(DgnViewportR viewport, UpdatePlan::Query const& plan)
    {
    Frustum frust = viewport.GetFrustum(DgnCoordSystem::World, true);
    if (plan.m_frustumScale != 1.0)
        frust.ScaleAboutCenter(plan.m_frustumScale);

    RangeQuery* query = new RangeQuery(*this, frust, viewport, plan);
    query->SetSizeFilter(viewport, 6.0);

    m_dgndb.GetQueryQueue().Add(*query);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::SetAlwaysDrawn(DgnElementIdSet const& newSet, bool exclusive)
    {
    RequestAbort(true);
    m_noQuery = exclusive;
    m_special.m_always = newSet; // NB: copies values
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::ClearAlwaysDrawn()
    {
    RequestAbort(true);
    m_special.m_always.clear();
    m_forceNewQuery = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::SetNeverDrawn(DgnElementIdSet const& newSet)
    {
    RequestAbort(true);
    m_special.m_never = newSet; // NB: copies values
    m_forceNewQuery = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::ClearNeverDrawn()
    {
    RequestAbort(true);
    m_special.m_never.clear();
    m_forceNewQuery = true;
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
        m_forceNewQuery = true;
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
    m_forceNewQuery = true;
    }

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
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::QueryModelExtents(DRange3dR range, DgnViewportR vp)
    {
    RTreeFitFilter filter(&m_special);
    filter.Start(*this);

    DgnElementId thisId;
    while ((thisId = filter.StepRtree()).IsValid())
        {
        if (filter.TestElement(thisId))
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
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnQueryView::SpatialQuery::TestElement(DgnElementId elId) 
    {
    if (IsNever(elId))
        return false;

    m_viewStmt->BindId(m_idCol, elId);
    bool stat = (BE_SQLITE_ROW == m_viewStmt->Step());
    m_viewStmt->Reset();
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* virtual method bound to the "InVirtualSet(@vset,e.ModelId,g.CategoryId)" SQL function for view query.
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
    m_forceNewQuery = true;
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
DgnQueryView::NonScene::NonScene(DgnQueryViewR view, DgnViewportCR vp, SceneMembers& scene) : m_rangeQuery(view, vp.GetFrustum(DgnCoordSystem::World, true), vp, UpdatePlan::Query()), m_scene(&scene), m_view(view)
    {
    m_rangeQuery.SetTestLOD(true);
    m_rangeQuery.SetSizeFilter(vp, 6.0);
    m_rangeQuery.SetDepthFirst(); // uses less memory
    m_rangeQuery.Start(view);
    }

/*---------------------------------------------------------------------------------**//**
* Add graphics for all elements that are: a) already loaded b) have an appropriate previously-created graphic 
* We do this first so that if any elements need to be loaded or stroked (which can take time), the available ones 
* are in the scene if we abort
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::AddtoSceneQuick(SceneContextR context, SceneMembers& members, QueryResults& results)
    {
    context.SetNoStroking(true); // tell the context to not create any graphics - just return exiting ones

    // first, run through the query results seeing if all of the elements are loaded and have their graphics ready
    // NOTE: This is not CheckStop'ed! It must be fast.
    for (auto& thisScore : results.m_scores)
        {
        if (SUCCESS == VisitElement(context, thisScore.second, false))
            members.insert(thisScore.second);
        }

    context.SetNoStroking(false); // reset the context 

    DEBUG_PRINTF("QuickCreate count=%d/%d", (int) members.size(), results.GetCount());
    }

/*---------------------------------------------------------------------------------**//**
* Create the scene and potentially schedule a progressive display 
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::_CreateScene(SceneContextR context) 
    {
    DgnDb::VerifyClientThread();

    StopWatch watch(true);
    DEBUG_PRINTF("Begin create scene");

    QueryResultsPtr results;
    std::swap(results, m_results);

    DgnViewportR vp = *context.GetViewport();
    if (!results.IsValid())
        return;

    SceneMembersPtr members = new SceneMembers();
    AddtoSceneQuick(context, *members, *results);

    if (members->size() < results->GetCount()) // did we get them all?
        {
        DEBUG_PRINTF("Begin create scene with load");
        for (auto& thisScore : results->m_scores)
            {
            if (members->Contains(thisScore.second))
                continue; // was already added during "quick" pass

            if (SUCCESS == VisitElement(context, thisScore.second, true))
                members->insert(thisScore.second);

            if (context.WasAborted())
                break;
            }
        }

    // Next, allow external data models to draw or schedule external data. NB: Do this even if we're already aborted
    for (DgnModelId modelId : GetViewedModels())
        {
        DgnModelPtr model = m_dgndb.Models().GetModel(modelId);
        auto geomModel = model.IsValid() ? model->ToGeometricModelP() : nullptr;
        if (nullptr != geomModel)
            geomModel->AddGraphicsToScene(context);
        }

    BeAssert(members->size() <= results->GetCount());
    bool sceneComplete = !results->m_incomplete && (members->size() == results->GetCount());
    if (!sceneComplete)
        {
        DEBUG_PRINTF("schedule progressive");
        NonScene* progressive = new NonScene(*this, vp, *members);
        vp.ScheduleProgressiveTask(*progressive);
        }

    DEBUG_PRINTF("Done create scene=%ld entries, aborted=%ld, time=%lf", members->size(), context.WasAborted(), watch.GetCurrentSeconds());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnQueryView::_IsSceneReady() const 
    {
    return m_results.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* Visit all of the elements in a DgnQueryView. This is used for picking, etc.
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::_VisitAllElements(ViewContextR context)
    {
    RangeQuery rangeQuery(*this, context.GetFrustum(), *context.GetViewport(), UpdatePlan::Query()); // NOTE: the context may have a smaller frustum than the view
    rangeQuery.Start(*this);

    // the range tree will return all elements in the volume. Filter them by the view criteria
    DgnElementId thisId;
    int count = 0;
    while ((thisId = rangeQuery.StepRtree()).IsValid())
        {           
        if (rangeQuery.TestElement(thisId))
            {
            ++count;
            VisitElement(context, thisId, true);
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
void DgnQueryQueue::RemovePending(DgnQueryViewCR view)
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
void DgnQueryQueue::WaitFor(DgnQueryViewCR view)
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

        m_active->_Go();
        uint32_t delay = m_active->GetDelayAfter();
        BeAssert(delay<2000);

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
void DgnQueryView::RangeQuery::_Go()
    {
    DgnDb::VerifyQueryThread();
    m_view.m_results = DoQuery();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::RangeQuery::AddAlwaysDrawn(DgnQueryViewCR view)
    {
    if (!HasAlways())
        return;

    DgnElements& pool = view.GetDgnDb().Elements();
    for (auto const& curr : m_special->m_always)
        {
        DgnElementCPtr el = pool.GetElement(curr);
        GeometrySourceCP geom = el.IsValid() ? el->ToGeometrySource() : nullptr;
        if (nullptr == geom || !geom->HasGeometry())
            continue;

        CornerBox box(geom->CalculateRange3d());
        if (RTreeMatchFunction::Within::Outside == m_clips.TestBox(box))
            continue;

        if (TestElement(curr))
            m_results->m_scores.Insert(3.0, curr); // value has to be higher than max occlusion score which is really 2.0
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnQueryView::QueryResultsPtr DgnQueryView::RangeQuery::DoQuery()
    {
    if (m_noQuery)
        return m_results; //this is just the set of "always drawn" elements

    StopWatch watch(true);
    m_view.SetAbortQuery(false); // gets turned on by client thread

    DEBUG_PRINTF("Query started");
    Start(m_view);

    uint64_t endTime = m_plan.GetTimeout() ? (BeTimeUtilities::QueryMillisecondsCounter() + m_plan.GetTimeout()) : 0;

    m_minScore = 0.0;
    m_hitLimit = m_plan.GetTargetNumElements();
    BeAssert(m_hitLimit>=0);

    DgnElementId thisId;
    while ((thisId=StepRtree()).IsValid())
        {
        BeAssert(m_lastId==thisId.GetValueUnchecked());
        if (m_view.m_abortQuery)
            {
            DEBUG_ERRORLOG("Query aborted");
            return m_results;
            }

        if (TestElement(thisId) && !IsAlways(thisId))
            {
            if (++m_count > m_hitLimit)
                {
                SetTestLOD(true); // now that we've found a minimum number of elements, start skipping small ones
                m_results->m_scores.erase(m_results->m_scores.begin());
                m_results->m_incomplete = true;
                m_count = m_hitLimit;
                }

            m_results->m_scores.Insert(m_lastScore, thisId);
            m_minScore = m_results->m_scores.begin()->first;
            }

        if (endTime && (BeTimeUtilities::QueryMillisecondsCounter() > endTime))
            {
            DEBUG_ERRORLOG("Query timeout");
            m_results->m_incomplete = true;
            break;
            }
        };

    double total = watch.GetCurrentSeconds();
    DEBUG_PRINTF("Query completed, total=%d, progressive=%d, time=%f", m_results->GetCount(), m_results->m_incomplete, total);
    return m_results;
    }

/*---------------------------------------------------------------------------------**//**
* callback function for "MATCH DGN_rTree(?1)" against the spatial index
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
int DgnQueryView::RangeQuery::_TestRTree(RTreeMatchFunction::QueryInfo const& info)
    {
    BeAssert(6 == info.m_nCoord);
    info.m_within = RTreeMatchFunction::Within::Outside;

    RTree3dValCP coords = (RTree3dValCP) info.m_coords;
    CornerBox box(*coords);

    RTreeMatchFunction::Within rangeTest;
    if (info.m_parentWithin == RTreeMatchFunction::Within::Inside)
        rangeTest = RTreeMatchFunction::Within::Inside; // if parent is contained, we're contained and don't need to test
    else
        {
        if (!m_boundingRange.Intersects(*coords) || !SkewTest(coords)) // quick test for entirely outside axis-aligned bounding range
            return BE_SQLITE_OK;

        rangeTest = m_clips.TestBox(box);
        if (rangeTest == RTreeMatchFunction::Within::Outside)
            return BE_SQLITE_OK; // overlaps outer bounding range but not inside clip planes
        }

    if (!ComputeOcclusionScore(m_lastScore, box))
        return BE_SQLITE_OK; // eliminated by LOD filter

    BeAssert(m_lastScore>0.0);
    BeAssert(m_minScore>=0.0);
    if (m_hitLimit && (m_count >= m_hitLimit && m_lastScore <= m_minScore))
        return BE_SQLITE_OK; // this one is smaller than the smallest entry we already have, skip it (and children).

    m_lastId = info.m_rowid;  // for debugging - make sure we get entries immediately after we score them.

    // for leaf nodes (elements), we must return a score of 0 so they will be immediately returned. That's necessary since we're keeping the
    // score in this object and we otherwise would not be able to correlate elements with their score. 
    // For depth-first traversals, we return values such that we decend down to the bottom of one branch before we look at the next node. Otherwise 
    // traversals take too much memory (the only reason it works for non-depth-first traversals is that we start eliminating nodes on size criteria 
    // to choose the n-best values).
    info.m_score = info.m_level==0 ? 0.0 : m_depthFirst ? (info.m_maxLevel - info.m_level - m_lastScore) : (info.m_level - m_lastScore);
    info.m_within = rangeTest;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion DgnQueryView::NonScene::_DoProgressive(SceneContext& context, WantShow& wantShow)
    {
    m_thisBatch = 0; // restart every pass
    m_batchSize = context.GetUpdatePlan().GetQuery().GetTargetNumElements();
    m_setTimeout = false;

    DEBUG_PRINTF("begin progressive display");

    DgnElementId thisId;
    while ((thisId=m_rangeQuery.StepRtree()).IsValid())
        {
        if (!m_scene->Contains(thisId) && m_rangeQuery.TestElement(thisId))
            {
            ++m_total;
            m_view.VisitElement(context, thisId, true); // no, draw it now

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
StatusInt DgnQueryView::VisitElement(ViewContextR context, DgnElementId elementId, bool allowLoad) 
    {
    DgnElements& pool = m_dgndb.Elements();
    DgnElementCPtr el = allowLoad ? pool.GetElement(elementId) : pool.FindElement(elementId);
    if (!el.IsValid())
        {
        BeAssert(!allowLoad);
        return ERROR;
        }

    GeometrySourceCP geomElem = el->ToGeometrySource();
    if (nullptr == geomElem)
        return ERROR;

    return context.VisitGeometry(*geomElem);

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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool DgnQueryView::RangeQuery::ComputeNPC(DPoint3dR npcOut, DPoint3dCR localIn)
    {
    double w;
    if (m_cameraOn)
        {
        w = m_localToNpc.coff[3][0] * localIn.x + m_localToNpc.coff[3][1] * localIn.y + m_localToNpc.coff[3][2] * localIn.z + m_localToNpc.coff[3][3];
        if (w < s_cameraLimit)
            return false;
        }
    else
        {
        w = 1.0;
        }

    npcOut.x = (m_localToNpc.coff[0][0] * localIn.x + m_localToNpc.coff[0][1] * localIn.y + m_localToNpc.coff[0][2] * localIn.z + m_localToNpc.coff[0][3]) / w;
    npcOut.y = (m_localToNpc.coff[1][0] * localIn.x + m_localToNpc.coff[1][1] * localIn.y + m_localToNpc.coff[1][2] * localIn.z + m_localToNpc.coff[1][3]) / w;
    npcOut.z = (m_localToNpc.coff[2][0] * localIn.x + m_localToNpc.coff[2][1] * localIn.y + m_localToNpc.coff[2][2] * localIn.z + m_localToNpc.coff[2][3]) / w;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Set the size of a filter that eliminates elements smaller than a given size (in pixels). To enable this filter, you must also call SetTestLOD.
* For range queries, we only enable this after we've found our maximum number of hits.
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::RangeQuery::SetSizeFilter(DgnViewportCR vp, double size)
    {
    BSIRect screenRect = vp.GetViewRect();
    if (screenRect.Width() > 0 && screenRect.Height() > 0)
        {
        double width  = size/screenRect.Width();
        double height = size/screenRect.Height();

        m_lodFilterNPCArea = width * width + height * height;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnQueryView::RangeQuery::RangeQuery(DgnQueryViewCR view, FrustumCR frustum, DgnViewportCR vp, UpdatePlan::Query const& plan) : SpatialQuery(&view.m_special), DgnQueryQueue::Task(view, plan)
    {
    m_count=0;
    m_localToNpc = vp.GetWorldToNpcMap()->M0;
    m_cameraOn   = vp.IsCameraOn();

    if (m_cameraOn)
        {
        m_cameraPosition = vp.GetCamera().GetEyePoint();
        }
    else
        {
        DPoint3d viewDirection[2] = {{0,0,0}, {0.0, 0.0, 1.0}};
        DPoint3d viewDirRoot[2];
        vp.NpcToWorld(viewDirRoot, viewDirection, 2);
        viewDirRoot[1].Subtract (viewDirRoot[0]);

        m_orthogonalProjectionIndex = ((viewDirRoot[1].x < 0.0) ? 1  : 0) +
                                      ((viewDirRoot[1].x > 0.0) ? 2  : 0) +
                                      ((viewDirRoot[1].y < 0.0) ? 4  : 0) +
                                      ((viewDirRoot[1].y > 0.0) ? 8  : 0) +
                                      ((viewDirRoot[1].z < 0.0) ? 16 : 0) +
                                      ((viewDirRoot[1].z > 0.0) ? 32 : 0);
        }

    SetFrustum(frustum);
    m_results = new QueryResults();
    AddAlwaysDrawn(view);
    }

/*---------------------------------------------------------------------------------**//**
* compute the size in "NPC area squared" 
* Algorithm by: Dieter Schmalstieg and Erik Pojar - ACM Transactions on Graphics.
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnQueryView::RangeQuery::ComputeOcclusionScore(double& score, FrustumCR box)
    {
    // Note - This routine is VERY time critical - Most of the calls to the geomlib
    // functions have been replaced with inline code as VTune had showed them as bottlenecks.

    static const short s_indexList[43][9] =
        {
        { 0, 3, 7, 6, 5, 1,   6,}, // 0 inside    (arbitrarily default to front, top, right.
        { 0, 4, 7, 3,-1,-1,   4}, // 1 left
        { 1, 2, 6, 5,-1,-1,   4}, // 2 right
        {-1,-1,-1,-1,-1,-1,   0}, // 3 -
        { 0, 1, 5, 4,-1,-1,   4}, // 4 bottom
        { 0, 1, 5, 4, 7, 3,   6}, // 5 bottom, left
        { 0, 1, 2, 6, 5, 4,   6}, // 6 bottom, right
        {-1,-1,-1,-1,-1,-1,   0}, // 7 -
        { 2, 3, 7, 6,-1,-1,   4}, // 8 top
        { 0, 4, 7, 6, 2, 3,   6}, // 9 top, left
        { 1, 2, 3, 7, 6, 5,   6}, //10 top, right
        {-1,-1,-1,-1,-1,-1,   0}, //11 -
        {-1,-1,-1,-1,-1,-1,   0}, //12 -
        {-1,-1,-1,-1,-1,-1,   0}, //13 -
        {-1,-1,-1,-1,-1,-1,   0}, //14 -
        {-1,-1,-1,-1,-1,-1,   0}, //15 -
        { 0, 3, 2, 1,-1,-1,   4}, //16 front
        { 0, 4, 7, 3, 2, 1,   6}, //17 front, left
        { 0, 3, 2, 6, 5, 1,   6}, //18 front, right
        {-1,-1,-1,-1,-1,-1,   0}, //19 -
        { 0, 3, 2, 1, 5, 4,   6}, //20 front, bottom
        { 1, 5, 4, 7, 3, 2,   6}, //21 front, bottom, left
        { 0, 3, 2, 6, 5, 4,   6}, //22 front, bottom, right
        {-1,-1,-1,-1,-1,-1,   0}, //23 -
        { 0, 3, 7, 6, 2, 1,   6}, //24 front, top
        { 0, 4, 7, 6, 2, 1,   6}, //25 front, top, left
        { 0, 3, 7, 6, 5, 1,   6}, //26 front, top, right
        {-1,-1,-1,-1,-1,-1,   0}, //27 -
        {-1,-1,-1,-1,-1,-1,   0}, //28 -
        {-1,-1,-1,-1,-1,-1,   0}, //29 -
        {-1,-1,-1,-1,-1,-1,   0}, //30 -
        {-1,-1,-1,-1,-1,-1,   0}, //31 -
        { 4, 5, 6, 7,-1,-1,   4}, //32 back
        { 0, 4, 5, 6, 7, 3,   6}, //33 back, left
        { 1, 2, 6, 7, 4, 5,   6}, //34 back, right
        {-1,-1,-1,-1,-1,-1,   0}, //35 -
        { 0, 1, 5, 6, 7, 4,   6}, //36 back, bottom
        { 0, 1, 5, 6, 7, 3,   6}, //37 back, bottom, left
        { 0, 1, 2, 6, 7, 4,   6}, //38 back, bottom, right
        {-1,-1,-1,-1,-1,-1,   0}, //39 -
        { 2, 3, 7, 4, 5, 6,   6}, //40 back, top
        { 0, 4, 5, 6, 2, 3,   6}, //41 back, top, left
        { 1, 2, 3, 7, 4, 5,   6}, //42 back, top, right
        };

    enum
        {
        OUTSIDE_Left   = (0x00001 << 0),
        OUTSIDE_Right  = (0x00001 << 1),
        OUTSIDE_Top    = (0x00001 << 2),
        OUTSIDE_Bottom = (0x00001 << 3),
        OUTSIDE_Front  = (0x00001 << 4),
        OUTSIDE_Back   = (0x00001 << 5),
        };

    uint32_t npcComputedMask = 0, zComputedMask = 0;
    uint32_t projectionIndex;

    if (m_cameraOn)
        {
        projectionIndex = ((m_cameraPosition.x < box.m_pts[0].x) ? 1  : 0) +
                          ((m_cameraPosition.x > box.m_pts[1].x) ? 2  : 0) +
                          ((m_cameraPosition.y < box.m_pts[0].y) ? 4  : 0) +
                          ((m_cameraPosition.y > box.m_pts[2].y) ? 8  : 0) +
                          ((m_cameraPosition.z < box.m_pts[0].z) ? 16 : 0) +       // Zs reversed for right-handed system.
                          ((m_cameraPosition.z > box.m_pts[4].z) ? 32 : 0);

        if (0 == projectionIndex)
            {
            score = 1.0;
            return true;
            }
        }
    else
        {
        projectionIndex = m_orthogonalProjectionIndex;
        }

    uint32_t    nVertices;
    double      npcZ[8];
    DPoint3d    npcVertices[6];

    BeAssert(projectionIndex <= 42);
    if (projectionIndex > 42 || 0 == (nVertices = s_indexList[projectionIndex][6]))
        {
        // Inside the box... Needs work.
        return false;
        }

    double  zTotal = 0.0;

    for (uint32_t i=0, mask = 0x0001; i<nVertices; i++, mask <<= 1)
        {
        int  cornerIndex = s_indexList[projectionIndex][i];
        if (0 == (mask & npcComputedMask) && !ComputeNPC(npcVertices[i], box.m_pts[cornerIndex]))
            {
            score = 2.0;  // range spans eye plane
            return true;
            }

        zTotal += (npcZ[cornerIndex] = npcVertices[i].z);
        zComputedMask |= (1 << cornerIndex);
        }

    if (m_testLOD && 0.0 != m_lodFilterNPCArea)
        {
        // In the cases where this does exclude the element it would be faster to do the other filtering first. However, even when we exclude something due
        // to LOD filtering we want to know if it should be drawn by the progressive display. We want progressive display to draw zero-length line strings
        // and points.
        int        diagonalVertex = nVertices/2;
        DPoint3dR  diagonalNPC    = npcVertices[diagonalVertex];

        if (!ComputeNPC(npcVertices[0], box.m_pts[s_indexList[projectionIndex][0]]) ||
            !ComputeNPC(diagonalNPC,    box.m_pts[s_indexList[projectionIndex][diagonalVertex]]))
            {
            score = 2.0;  // range spans eye plane
            return true;
            }

        DPoint3dR   npcCorner = npcVertices[nVertices/2];
        DPoint2d    extent = { npcCorner.x - npcVertices[0].x, npcCorner.y - npcVertices[0].y};

        if ((extent.x * extent.x + extent.y * extent.y) < m_lodFilterNPCArea)
            return false;

        npcComputedMask |= 1;
        npcComputedMask |= (1 << diagonalVertex);
        }

    score = (npcVertices[nVertices-1].x - npcVertices[0].x) * (npcVertices[nVertices-1].y + npcVertices[0].y);
    for (uint32_t i=0; i<nVertices-1; i++)
        score += (npcVertices[i].x - npcVertices[i+1].x) * (npcVertices[i].y + npcVertices[i+1].y);

    // an area of 0.0 means that we have a line. Recalculate using length/1000 (assuming width of 1000th of npc)
    if (score == 0.0)
        score = npcVertices[0].DistanceXY(npcVertices[2]) * .001;
    else if (score < 0.0)
        score = -score;

    // Multiply area by the Z total value (0 is the back of the view) so that the score is roughly
    // equivalent to the area swept by the range through the view which makes things closer to the eye more likely to display first.
    // Also by scoring based on swept volume we are proportional to occluded volume.
    score *= .5 * (zTotal / (double) nVertices);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::RangeQuery::SetFrustum(FrustumCR frustum)
    {
    DRange3d range = frustum.ToRange();
    m_boundingRange.FromRange(range);

    range.InitFrom(frustum.GetPts(), 4);
    m_backFace.FromRange(range);

    // get unit bvector from front plane to back plane
    m_viewVec = DVec3d::FromStartEndNormalize(*frustum.GetPts(), *(frustum.GetPts()+4));

    // check to see if it's worthwhile using skew scan (skew vector not along one of the three major axes)
    int alongAxes = (fabs(m_viewVec.x) < 1e-8) + (fabs(m_viewVec.y) < 1e-8) + (fabs(m_viewVec.z) < 1e-8);
    m_doSkewTest = alongAxes<2;

    m_clips.AddFrustum(frustum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BJB             12/89
+---------------+---------------+---------------+---------------+---------------+------*/
static inline void exchangeAndNegate(double& dbl1, double& dbl2)
    {
    double temp = -dbl1;
    dbl1 = -dbl2;
    dbl2 = temp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnQueryView::RangeQuery::SkewTest(RTree3dValCP testRange)
    {
    static bool s_doskew=true;
    if (!s_doskew)
        return true;

    if (!m_doSkewTest || testRange->Intersects(m_backFace))
        return true;

    DVec3d skVector = m_viewVec;
    DPoint3d dlo, dhi;

    dlo.x = testRange->m_minx - m_backFace.m_maxx;
    dlo.y = testRange->m_miny - m_backFace.m_maxy;
    dhi.x = testRange->m_maxx - m_backFace.m_minx;
    dhi.y = testRange->m_maxy - m_backFace.m_miny;

    if (skVector.x < 0.0)
        {
        skVector.x = - skVector.x;
        exchangeAndNegate(dlo.x, dhi.x);
        }

    if (skVector.y < 0.0)
        {
        skVector.y = - skVector.y;
        exchangeAndNegate(dlo.y, dhi.y);
        }

    // Check the projection of the element's xhigh to the plane where ylow of the element is equal to yhigh of the skewrange
    double va1 = dlo.x * skVector.y;
    double vb2 = dhi.y * skVector.x;
    if (va1 > vb2)
        return false;

    // Check the projection of the element's xlow to the plane where yhigh of the element is equal to ylow of the skewrange
    double vb1 = dlo.y * skVector.x;
    double va2 = dhi.x * skVector.y;
    if (va2 < vb1)
        return false;

    // now we need the Z stuff
    dlo.z = testRange->m_minz - m_backFace.m_maxz;
    dhi.z = testRange->m_maxz - m_backFace.m_minz;
    if (skVector.z < 0.0)
        {
        skVector.z = - skVector.z;
        exchangeAndNegate(dlo.z, dhi.z);
        }

    // project onto either the xz or yz plane
    if (va1 > vb1)
        {
        double va3 = dlo.x * skVector.z;
        double vc2 = dhi.z * skVector.x;
        if (va3 > vc2)
            return false;
        }
    else
        {
        double vb3 = dlo.y * skVector.z;
        double vc4 = dhi.z * skVector.y;
        if (vb3 > vc4)
            return false;
        }

    // project onto the other plane
    if (va2 < vb2)
        {
        double va4 = dhi.x * skVector.z;
        double vc1 = dlo.z * skVector.x;
        if (va4 < vc1)
            return false;
        }
    else
        {
        double vb4 = dhi.y * skVector.z;
        double vc3 = dlo.z * skVector.y;
        if (vb4 < vc3)
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* this method must be called from the client thread.
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnQueryView::SpatialQuery::Start(DgnQueryViewCR view)
    {
    DgnDb::VerifyClientThread();
    DgnDbR db = view.GetDgnDb();

    db.GetCachedStatement(m_rangeStmt, "SELECT ElementId FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId MATCH DGN_rTree(?1)");
    m_rangeStmt->BindInt64(1, (uint64_t) this);

    db.GetCachedStatement(m_viewStmt, view.m_viewSQL.c_str());
    int vSetCol = m_viewStmt->GetParameterIndex("@vset");
    BeAssert(0 != vSetCol);
    m_viewStmt->BindVirtualSet(vSetCol, view);

    m_idCol = m_viewStmt->GetParameterIndex("@elId");
    BeAssert(0 != m_idCol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnQueryView::SpatialQuery::StepRtree()
    {
    auto rc=m_rangeStmt->Step();
    return (rc != BE_SQLITE_ROW) ? DgnElementId() : m_rangeStmt->GetValueId<DgnElementId>(0);
    }
