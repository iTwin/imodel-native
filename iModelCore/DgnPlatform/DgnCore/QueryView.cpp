/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/QueryView.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/QueryView.h>
#include <Bentley/BeSystemInfo.h>

#include "UpdateLogging.h"

#if defined (BENTLEY_WIN32)
    #define MAX_TO_DRAW_IN_DYNAMIC_UPDATE  6000
#else
    #define MAX_TO_DRAW_IN_DYNAMIC_UPDATE  1700
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
QueryViewController::QueryViewController(DgnDbR dgndb, DgnViewId id) : CameraViewController(dgndb, id), m_queryModel(*new QueryModel(dgndb))
    {
    m_forceNewQuery = true; 
    m_lastUpdateType = DrawPurpose::NotSpecified;
    m_maxToDrawInDynamicUpdate = MAX_TO_DRAW_IN_DYNAMIC_UPDATE;
    m_intermediatePaintsThreshold = 0;
    m_maxDrawnInDynamicUpdate = 0;
    m_noQuery = false;
    m_secondaryHitLimit = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
QueryViewController::~QueryViewController()
    {
    delete &m_queryModel;
    }

// On iOS we draw less in a frame that occurs while the query is running.
// Holding back some leads to fewer flashing frames.
static double s_dynamicLoadFrequency = 0.75;
static uint32_t s_dynamicLoadTrigger = 800;
static double s_threshold = .2;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::ComputeFps()
    {
    uint64_t currentTime  = BeTimeUtilities::QueryMillisecondsCounter();
    uint64_t deltaTime    = currentTime - m_lastUpdateTime;
    m_fps = 1000.0 / (deltaTime == 0 ? 1 : deltaTime);
    m_lastUpdateTime = currentTime;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2012
//--------------+------------------------------------------------------------------------
bool QueryViewController::_WantElementLoadStart(DgnViewportR vp, double currentTime, double lastQueryTime, uint32_t maxElementsDrawnInDynamicUpdate, Frustum const& queryFrustum)
    {
    if (maxElementsDrawnInDynamicUpdate > s_dynamicLoadTrigger || maxElementsDrawnInDynamicUpdate >= m_queryModel.GetElementCount())
        {
#if defined (TRACE_QUERY_LOGIC)
        printf("_WantElementLoadStart : returning true based on element count MaxDrawn (%d) LoadTrigger(%d), ElementCount(%d)\n", 
                (int)maxElementsDrawnInDynamicUpdate, (int)s_dynamicLoadTrigger, (int)m_queryModel.GetElementCount());
#endif
        return true;
        }

    static uint32_t s_numberOfCpus;
    if (0 == s_numberOfCpus)
        s_numberOfCpus = BeSystemInfo::GetNumberOfCpus();

    if (s_numberOfCpus <= 4)
        {
        //  Wait a while if there is contention for CPU's. Generally, we want to wait on platforms
        //  where we sometimes fail the test for dynamic trigger so we don't often get to this code 
        //  if we don't want it to execute. We've seen that this is important on iOS and don't know if
        //  it is on other platforms.
        if ((currentTime - lastQueryTime) < s_dynamicLoadFrequency)
            {
#if defined (TRACE_QUERY_LOGIC)
            printf("_WantElementLoadStart : FAILED time test = %g\n", currentTime - lastQueryTime);
#endif
            return false;
            }
        }

#if defined (TRACE_QUERY_LOGIC)
    printf("_WantElementLoadStart : passed time test = %g\n", currentTime - lastQueryTime);
#endif

    // It shouldn't matter whether we look at m_startQueryFrustum or m_saveQueryFrustum. The previous steps in this method
    // return if a query is underway or if there are outstanding results. If the method gets to this step
    // m_startQueryFrustum and m_saveQueryFrustum should be identical.
    bool retval = ClipUtil::ComputeFrustumOverlap(vp, queryFrustum.GetPts()) < s_threshold;
#if defined (TRACE_QUERY_LOGIC)
    if (!retval)
        printf("_WantElementLoadStart rejected on frustum overlap\n");
#endif
    return retval;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::_OnDynamicUpdate(DgnViewportR vp, ViewContextR context, DynamicUpdateInfo& info)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    DrawPurpose newUpdateType = info.GetDoBackingStore() ? DrawPurpose::Update : DrawPurpose::UpdateDynamic;
    m_lastUpdateType = newUpdateType;
    if (m_forceNewQuery || info.GetDoBackingStore())
        {
        // Skip any other tests and force a search and load
        LoadElementsForUpdate(vp, newUpdateType, &context, true, true, false);
        return;
        }
#endif

    QueryModel::Selector& selector = m_queryModel.GetSelector();
    if (selector.IsActive())
        {
#if defined (TRACE_QUERY_LOGIC)
        printf("(%d) _OnDynamicUpdate: IsActive is true\n", ++s_count);
#endif
        return;
        }

    if (selector.HasSelectResults())
        {
#if defined (TRACE_QUERY_LOGIC)
        printf("(%d) _OnDynamicUpdate: calling SaveSelectResults\n", ++s_count);
#endif
        SaveSelectResults();
        return;
        }

    // The selector is idle. Decide if this is a good time to start another query.
    if (!_WantElementLoadStart(vp, BeTimeUtilities::QuerySecondsCounter(), m_lastQueryTime, m_maxDrawnInDynamicUpdate, m_startQueryFrustum))
        return;

    // Restarting select processing, don't wait for result
#if defined (TRACE_QUERY_LOGIC)
    printf("_OnDynamicUpdate: calling StartSelectProcessing\n");
#endif
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    StartSelectProcessing(vp, DrawPurpose::UpdateDynamic);
#endif
    ComputeFps();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::_OnHealUpdate(DgnViewportR vp, ViewContextR context, bool fullHeal)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    m_lastUpdateType = DrawPurpose::UpdateHealing;
    if (!m_forceNewQuery && !fullHeal)
        return;

    Frustum newFrustumPoints = vp.GetFrustum(DgnCoordSystem::World, true);

    if (!m_forceNewQuery)
        {
        if (newFrustumPoints == m_saveQueryFrustum)
            return;
        }

    bool needNewQuery = m_forceNewQuery || (newFrustumPoints != m_startQueryFrustum);

    LoadElementsForUpdate(vp, DrawPurpose::UpdateHealing, &context, needNewQuery, true, false);
    ComputeFps();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::_OnFullUpdate(DgnViewportR vp, ViewContextR context)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    m_lastUpdateType = DrawPurpose::Update;
#endif
    LoadElementsForUpdate(vp, DrawPurpose::CreateScene, &context, true, true, false);
    ComputeFps();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::LoadElementsForUpdate(DgnViewportR viewport, DrawPurpose updateType, ICheckStopP checkStop, 
                                                bool needNewQuery, bool waitForQueryToFinish, bool stopQueryOnAbort)
    {
    QueryModel::Selector& selector = m_queryModel.GetSelector();

    if (waitForQueryToFinish)
        {
        if (needNewQuery)
            {
            UpdateLogging::RecordStartCycle();
            StartSelectProcessing(viewport, updateType);
            }

        selector.WaitUntilFinished(checkStop, 1, stopQueryOnAbort);

        // It is safe to ignore the StartSelectProcessing return value because 
        // SaveSelectResults does nothing unless the selector has the results of a successful search.
        SaveSelectResults();
        return;
        }

    // IsActive means that it is searching or that there is an outstanding request to abort or to start processing.
    if (selector.IsActive())
        return;

    SaveSelectResults();
    StartSelectProcessing(viewport, updateType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
void QueryViewController::StartSelectProcessing(DgnViewportR viewport, DrawPurpose updateType)
    {
    uint32_t hitLimit = GetMaxElementsToLoad();
    double minimumPixels = _GetMinimumSizePixels(updateType);

    size_t lastSize = 0;
    QueryModel::Results* results = m_queryModel.GetCurrentResults();
    if (nullptr != results)
        lastSize = results->m_elements.size();

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    // When loading for view dynamics we don't want the overhead of loading too much. If the last update for view dynamics did not
    // draw all of the elements then load 50% more than what was drawn.
    if (DrawPurpose::UpdateDynamic == updateType && m_maxDrawnInDynamicUpdate > 0 && m_maxDrawnInDynamicUpdate < lastSize)
        {
        uint32_t computedLimit = (uint32_t)(1.5 * m_maxDrawnInDynamicUpdate);
        if (hitLimit > computedLimit)
            hitLimit = std::min(computedLimit,(uint32_t)MAX_TO_DRAW_IN_DYNAMIC_UPDATE);
        }
#endif

    QueryModel::Selector& selector = m_queryModel.GetSelector();
    selector.StartProcessing(viewport, *this, _GetRTreeMatchSql(viewport).c_str(), hitLimit, GetMaxElementMemory(), minimumPixels, 
                                m_alwaysDrawn.empty() ? nullptr : &m_alwaysDrawn, 
                                m_neverDrawn.empty() ?  nullptr : &m_neverDrawn, 
                                m_noQuery, GetClipVector().get(), m_secondaryHitLimit, m_secondaryVolume);

    m_startQueryFrustum = selector.GetFrustum();
    m_saveQueryFrustum.Invalidate();

    // Once we start select processing we don't want to draw any more than we have already drawn. 
    // Otherwise we may end up with the draw logic blocked on a SQLite mutex.
    m_maxToDrawInDynamicUpdate = m_maxDrawnInDynamicUpdate > 400 ? m_maxDrawnInDynamicUpdate : MAX_TO_DRAW_IN_DYNAMIC_UPDATE;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
void QueryViewController::SaveSelectResults()
    {
    QueryModel::Selector& selector = m_queryModel.GetSelector();
    if (!selector.HasSelectResults())
        {
        if (!selector.IsActive() || selector.GetState() == QueryModel::Selector::State::AbortRequested)
            {
            m_startQueryFrustum.Invalidate(); // Must be abort or error. Either way the startQueryFrustum is meaningless.
            m_saveQueryFrustum.Invalidate();
            }

        return;
        }

    m_queryModel.SaveQueryResults();
    m_lastQueryTime = BeTimeUtilities::QuerySecondsCounter();

    DgnElements& pool = m_queryModel.GetDgnDb().Elements();

#if defined (TRACE_ELEMENT_POOL_USE)
    DgnElements::Totals totals = pool.GetTotals();
    DgnElements::Statistics stats = pool.GetStatistics();
    uint64_t start = BeTimeUtilities::QueryMillisecondsCounter();
#endif

    pool.ResetStatistics();
    GetDgnDb().Memory().Purge(GetMaxElementMemory());

#if defined (TRACE_ELEMENT_POOL_USE)
    uint32_t elapsed = (uint32_t)(BeTimeUtilities::QueryMillisecondsCounter() - start);
    DgnElements::Statistics postStats = pool.GetStatistics();
    NotificationManager::OutputPrompt(WPrintfString(L"Elms=%u,Free=%u,Mem=%.3f",totals.m_entries,totals.m_unreferenced,(double)totals.m_allocedBytes/(1024.*1024.)));
    NotificationManager::OutputMessage(NotifyMessageDetails(OutputMessagePriority::Info, WPrintfString(L"purge time %d viewed=%u,new=%u,purged=%u",elapsed, m_queryModel.GetElementCount(),stats.m_newElements,postStats.m_purged)));
#endif

    m_forceNewQuery = false;
    m_saveQueryFrustum = selector.GetFrustum();
    selector.Reset();

    m_maxDrawnInDynamicUpdate = 0;
    m_maxToDrawInDynamicUpdate = MAX_TO_DRAW_IN_DYNAMIC_UPDATE;     //  No limit to number of elements drawn except during select; then we don't want
                                                                    //  to draw something unless it was previously drawn
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::EmptyQueryModel() 
    {
    m_forceNewQuery = true;
    m_queryModel.ClearQueryResults();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::SetAlwaysDrawn(DgnElementIdSet const& newSet, bool exclusive)
    {
    m_queryModel.GetSelector().RequestAbort(true);
    m_noQuery = exclusive;
    m_alwaysDrawn = newSet;
    m_forceNewQuery = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::ClearAlwaysDrawn()
    {
    m_queryModel.GetSelector().RequestAbort(true);
    m_noQuery = false;
    m_alwaysDrawn.clear();
    m_forceNewQuery = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::SetNeverDrawn(DgnElementIdSet const& newSet)
    {
    m_queryModel.GetSelector().RequestAbort(true);
    m_neverDrawn = newSet;
    m_forceNewQuery = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::ClearNeverDrawn()
    {
    m_queryModel.GetSelector().RequestAbort(true);
    m_neverDrawn.clear();
    m_forceNewQuery = true;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void QueryViewController::EnableSecondaryQueryRange(uint32_t hitLimit, DRange3dCR volume)
    {
    m_secondaryHitLimit = hitLimit;
    m_secondaryVolume = volume;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::_ChangeModelDisplay(DgnModelId modelId, bool onOff) 
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
        EmptyQueryModel();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::_OnCategoryChange(bool singleEnabled) 
    {
    T_Super::_OnCategoryChange(singleEnabled); 
    m_forceNewQuery = true;
    if (!singleEnabled)
        EmptyQueryModel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::QueryModelExtents(DRange3dR range, DgnViewportR vp)
    {
    // make sure this is local variable so it is removed before the call to LoadElementsForUpdate below.
    DgnDbRTreeFitFilter filter(m_dgndb);

    Statement getRange;
    DbResult rc = getRange.Prepare(m_dgndb, _GetRTreeMatchSql(vp).c_str());
    BindModelAndCategory(getRange);

    rc = filter.StepRTree(getRange);
    BeAssert(rc == BE_SQLITE_ROW);
    range = filter.GetRange();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController::FitComplete QueryViewController::_ComputeFitRange(DRange3dR range, DgnViewportR vp, FitViewParamsR params) 
    {
    range = GetViewedExtents();

    Transform  transform;
    transform.InitFrom((nullptr == params.m_rMatrix) ? vp.GetRotMatrix() : *params.m_rMatrix);
    transform.Multiply(range, range);

    return FitComplete::Yes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String QueryViewController::_GetRTreeMatchSql(DgnViewportR) 
    {
    //  The query produces a thread race condition if it calls QueryModelById and 
    //  the model is not already loaded.
    for (auto& id : GetViewedModels())
        m_dgndb.Models().GetModel(id);

    return Utf8String("SELECT rTreeAccept(r.ElementId) FROM "
           DGN_VTABLE_RTree3d " AS r, " DGN_TABLE(DGN_CLASSNAME_Element) " AS e "
           "WHERE r.ElementId MATCH rTreeMatch(1) AND e.Id=r.ElementId"
           " AND InVirtualSet(@vSet,e.ModelId,e.CategoryId)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryViewController::_IsInSet(int nVals, DbValue const* vals) const 
    {
    BeAssert(nVals == 2);   // we need ModelId and Category

    // check that both the model is on and the category is on.
    return m_viewedModels.Contains(DgnModelId(vals[0].GetValueUInt64())) && m_viewedCategories.Contains(DgnCategoryId(vals[1].GetValueUInt64()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/14
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::_OnAttachedToViewport(DgnViewportR) 
    {
    m_forceNewQuery = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::BindModelAndCategory(StatementR stmt) const
    {
    int vSetIdx = stmt.GetParameterIndex("@vSet");
    if (0 == vSetIdx)
        return;

    stmt.BindVirtualSet(vSetIdx, *this);
    }

//  #define DUMP_DYNAMIC_UPDATE_STATS 1

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2012
//--------------+------------------------------------------------------------------------
void QueryViewController::_DrawView(ViewContextR context) 
    {
    QueryModel::Results* results = m_queryModel.GetCurrentResults();
    if (nullptr == results)
        return;

    // use the in-memory range tree for picks (to limit to sub-range)
    if (DrawPurpose::Pick == context.GetDrawPurpose() )
        {
        if (!m_queryModel.IsEmpty())    // if we have no elements, nothing to do.
            context.VisitDgnModel(&m_queryModel);

        if (context.CheckStop())
            return;

        // Allow models to participate in picking
        for (DgnModelId modelId : GetViewedModels())
            {
            DgnModelPtr model = GetDgnDb().Models().GetModel(modelId);
            auto geomModel = model.IsValid() ? model->ToGeometricModelP() : nullptr;
            if (nullptr != geomModel)
                geomModel->AddGraphicsToScene(context);

            if (context.CheckStop())
                break;
            }

        return;
        }

    context.GetViewport()->ClearProgressiveDisplay();

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
#endif
    bool isDynamicUpdate = false;
    uint32_t maxToDraw = isDynamicUpdate ? m_maxToDrawInDynamicUpdate : UINT_MAX;

    const uint64_t maxMem = GetMaxElementMemory();
    UNUSED_VARIABLE(maxMem);
#if !defined (_X64_)
    const int64_t purgeTrigger = static_cast <int64_t> (1.5 * static_cast <double> (maxMem));
#endif
    const uint32_t intermediatePaintsThreshold = (uint32_t)(1.1 * m_intermediatePaintsThreshold);

    context.SetFilterLODFlag(FILTER_LOD_Off); // there's no point in doing lod filtering on the elements we've found by a QueryView

    // this vector is sorted by occlusion score, so we use it to determine the order to draw the view
    uint32_t numDrawn = 0;
    while (numDrawn < results->m_elements.size())
        {
        if (intermediatePaintsThreshold == numDrawn)
            {
            UpdateLogging::RecordAllowShowProgress();
            }

        GeometricElementCP geom = results->m_elements[numDrawn]->ToGeometricElement();

        if (nullptr != geom)
            context.VisitElement(*geom);

        if (context.WasAborted() || ++numDrawn >= maxToDraw)
            break;

#if !defined (_X64_)
        DgnElements& pool = m_queryModel.GetDgnDb().Elements();
        if (numDrawn > results->m_drawnBeforePurge && pool.GetTotalAllocated() > purgeTrigger)
            {
            QueryModel::Selector& selector = m_queryModel.GetSelector();

            // Testing for selector.IsActive prevents race conditions between the work thread and the 
            // query thread. Testing for HasSelectResults prevents this logic from purging elements that
            // are in the selected-elements list. Adding elements to that list does not increment the reference 
            // count. DgnElements use reference counting that is not thread safe so all reference counting is done
            // in the work thread.
            if (!selector.IsActive() && !selector.HasSelectResults())
                {
                results->m_drawnBeforePurge = numDrawn;
                pool.Purge(maxMem);  //  the pool may contain unused elements

                int64_t lastTotalAllocated;
                while ((lastTotalAllocated = pool.GetTotalAllocated()) > purgeTrigger && results->m_elements.size() > numDrawn)
                    {
                    if (context.CheckStop())
                        break;

                    uint32_t entriesToRemove = (uint32_t)results->m_elements.size() - numDrawn;
                    if (entriesToRemove > 25)
                        entriesToRemove /= 2;

                    if (entriesToRemove > results->m_elements.size()/4)
                        entriesToRemove = (uint32_t)results->m_elements.size()/4;

                    uint32_t newSize = (uint32_t)results->m_elements.size() - entriesToRemove;
                    BeAssert((int32_t)newSize > 0);
                    m_queryModel.ResizeElementList(newSize);
                    pool.Purge(maxMem);  //  the pool may contain unused elements
                    //  This happens if entriesToRemove is 0 (exceeded maximum memory with fewer than 4 elements).
                    //  It could also happen if there is a bug in ResizeElementList or Purge.
                    if (pool.GetTotalAllocated() == lastTotalAllocated)
                        break;
                    }

                if (pool.GetTotalAllocated() > purgeTrigger)
                    break;   //  Unable to get low enough
                }
            }
#endif
        }

    UpdateLogging::RecordDoneUpdate(numDrawn, context.GetDrawPurpose());
    m_intermediatePaintsThreshold = isDynamicUpdate ? numDrawn : 0;
    if (isDynamicUpdate && numDrawn > m_maxDrawnInDynamicUpdate)
        m_maxDrawnInDynamicUpdate = numDrawn;

    if (context.WasAborted())
        return;

    // Next, allow external data models to draw or schedule external data.
    for (DgnModelId modelId : GetViewedModels())
        {
        DgnModelPtr model = GetDgnDb().Models().GetModel(modelId);
        auto geomModel = model.IsValid() ? model->ToGeometricModelP() : nullptr;
        if (nullptr != geomModel)
            geomModel->AddGraphicsToScene(context);
        }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    //  We count on progressive display to draw zero length strings and points that are excluded by LOD filtering in the occlusion step.
    if ((DrawPurpose::UpdateHealing == context.GetDrawPurpose() || 
         DrawPurpose::Update == context.GetDrawPurpose()) && (results->m_reachedMaxElements || results->m_eliminatedByLOD) && !m_noQuery)
        {
        HighPriorityOperationBlock highPriority;  //  see comments in BeSQLite.h
        DgnViewportP vp = context.GetViewport();
        DgnDbR project = m_queryModel.GetDgnDb();
        CachedStatementPtr rangeStmt;
        project.GetCachedStatement(rangeStmt, _GetRTreeMatchSql(*context.GetViewport()).c_str());
        BindModelAndCategory(*rangeStmt);

        ProgressiveViewFilter* pvFilter = new ProgressiveViewFilter(*vp, project, m_queryModel,
                                                m_neverDrawn.empty() ? nullptr : &m_neverDrawn, maxMem, rangeStmt.get());
        if (GetClipVector().IsValid())
            pvFilter->SetClipVector(*GetClipVector());

        vp->ScheduleProgressiveDisplay(*pvFilter);
        }
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/2015
//---------------------------------------------------------------------------------------
void QueryViewController::_VisitElements(ViewContextR context)
    {
    // Visit the elements that were actually loaded
    context.VisitDgnModel(&m_queryModel);

    // And step through the rest of the elements that were not loaded (but would be displayed by progressive display).
    DgnDbR project = m_queryModel.GetDgnDb();
    CachedStatementPtr rangeStmt;
    project.GetCachedStatement(rangeStmt, _GetRTreeMatchSql(*context.GetViewport()).c_str());
    BindModelAndCategory(*rangeStmt);
    ProgressiveViewFilter pvFilter (*context.GetViewport(), project, m_queryModel, m_neverDrawn.empty() ? nullptr : &m_neverDrawn, GetMaxElementMemory(), rangeStmt.get());
    while (pvFilter._Process(context) != IProgressiveDisplay::Completion::Finished)
        ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2013
//---------------------------------------------------------------------------------------
#if defined (_X64_)
uint64_t QueryViewController::GetMaxElementMemory()
    {
    uint64_t oneGig = 1024 * 1024 * 1024;
    return 8 * oneGig;
    }
#else
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2013
//---------------------------------------------------------------------------------------
uint64_t QueryViewController::GetMaxElementMemory()
    {
    uint64_t oneMeg = 1024 * 1024;
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    uint64_t baseValue = 2000;
#else
    uint64_t baseValue = BeSystemInfo::GetAmountOfPhysicalMemory() > (600 * oneMeg) ? 50 : 30;
#endif
    baseValue *= oneMeg;

    int32_t inputFactor = _GetMaxElementFactor();
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

    return baseValue;
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2013
//---------------------------------------------------------------------------------------
uint32_t QueryViewController::GetMaxElementsToLoad()
    {
    uint32_t maxElementsToLoad = _GetMaxElementsToLoad();
    int32_t inputFactor = _GetMaxElementFactor();

    if (inputFactor < -100)
        inputFactor = -100;

    if (inputFactor > 100)
        inputFactor = 100;

    double maxElementsFactor = inputFactor/100.0;
    maxElementsToLoad += static_cast <int> (static_cast <double> (maxElementsToLoad) * maxElementsFactor * 0.90);

    return maxElementsToLoad;
    }
