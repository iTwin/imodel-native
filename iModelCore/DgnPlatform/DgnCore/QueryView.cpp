/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/QueryView.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/QueryView.h>

#if !defined (BENTLEY_WIN32) && !defined (BENTLEY_WINRT)
#include <Bentley/BeSystemInfo.h>
#endif

#include "UpdateLogging.h"

#define TRACE_QUERY_LOGIC 1
#ifdef TRACE_QUERY_LOGIC
#   define DEBUG_PRINTF THREADLOG.debugv
#else
#   define DEBUG_PRINTF(fmt, ...)
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
QueryViewController::QueryViewController(DgnDbR dgndb, DgnViewId id) : CameraViewController(dgndb, id), m_queryModel(*new QueryModel(dgndb))
    {
    m_forceNewQuery = true; 
    m_lastUpdateType = DrawPurpose::NotSpecified;
    m_maxElementMemory = 0;
    m_noQuery = false;
    m_secondaryHitLimit = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
QueryViewController::~QueryViewController()
    {
    m_queryModel.RequestAbort(true);
    delete &m_queryModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::_OnDynamicUpdate(DgnViewportR vp, UpdatePlan const& plan)
    {
    PickUpResults();
    QueueQuery(vp, plan);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::_OnFullUpdate(DgnViewportR vp, UpdatePlan const& plan)
    {
    if (m_forceNewQuery || FrustumChanged(vp))
        QueueQuery(vp, plan);

    if (plan.GetQuery().WantWait())
        m_queryModel.GetDgnDb().QueryQueue().WaitForIdle();

    PickUpResults();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryViewController::FrustumChanged(DgnViewportCR vp) const
    {
    Frustum newFrustumPoints = vp.GetFrustum(DgnCoordSystem::World, true);
    return newFrustumPoints != m_saveQueryFrustum;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
void QueryViewController::QueueQuery(DgnViewportR viewport, UpdatePlan const& plan)
    {
    m_startQueryFrustum = viewport.GetFrustum(DgnCoordSystem::World, true);
    m_saveQueryFrustum.Invalidate();

    m_forceNewQuery = false;

    QueryModel::Processor::Params params(m_queryModel, viewport, _GetRTreeMatchSql(viewport), plan.GetQuery(), ComputeMaxElementMemory(viewport), 
            m_alwaysDrawn.empty() ? nullptr : &m_alwaysDrawn, m_neverDrawn.empty() ? nullptr : &m_neverDrawn, m_noQuery,
            GetClipVector().get(), m_secondaryHitLimit, m_secondaryVolume);

    m_queryModel.GetDgnDb().QueryQueue().Add(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::PickUpResults()
    {
    if (!m_queryModel.HasSelectResults())
        return;

    m_queryModel.SaveQueryResults();

    DgnElements& pool = m_queryModel.GetDgnDb().Elements();
    pool.ResetStatistics();
    pool.Purge(GetMaxElementMemory());

    m_forceNewQuery = false;
    m_saveQueryFrustum = m_startQueryFrustum;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
void QueryViewController::SaveSelectResults()
    {
    if (!m_queryModel.HasSelectResults())
        {
        if (m_queryModel.AbortRequested())
            {
            m_startQueryFrustum.Invalidate(); // Must be abort or error. Either way the startQueryFrustum is meaningless.
            m_saveQueryFrustum.Invalidate();
            }

        DEBUG_PRINTF("QVC: SaveSelectResults: results not ready");
        return;
        }

    DEBUG_PRINTF("QVC: SaveSelectResults: saving results");

    m_queryModel.SaveQueryResults();

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
    m_saveQueryFrustum = m_startQueryFrustum;

#ifdef ABORT_REQUEST_IN_PROCESS
    m_queryModel.RequestAbort(true);
#endif
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
    m_queryModel.RequestAbort(true);
    m_noQuery = exclusive;
    m_alwaysDrawn = newSet;
    m_forceNewQuery = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::ClearAlwaysDrawn()
    {
    m_queryModel.RequestAbort(true);
    m_noQuery = false;
    m_alwaysDrawn.clear();
    m_forceNewQuery = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::SetNeverDrawn(DgnElementIdSet const& newSet)
    {
    m_queryModel.RequestAbort(true);
    m_neverDrawn = newSet;
    m_forceNewQuery = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryViewController::ClearNeverDrawn()
    {
    m_queryModel.RequestAbort(true);
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
           DGN_VTABLE_RTree3d " AS r, " DGN_TABLE(DGN_CLASSNAME_Element) " AS e, " DGN_TABLE(DGN_CLASSNAME_SpatialElement) " AS g "
           "WHERE r.ElementId MATCH rTreeMatch(1) AND e.Id=r.ElementId AND g.Id=r.ElementId"
           " AND InVirtualSet(@vSet,e.ModelId,g.CategoryId)");
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

    DEBUG_PRINTF("clear progressive _DrawView");
    context.GetViewport()->ClearProgressiveDisplay();

    const uint64_t maxMem = GetMaxElementMemory();
    UNUSED_VARIABLE(maxMem);
    const int64_t purgeTrigger = static_cast <int64_t> (1.5 * static_cast <double> (maxMem));

    // this vector is sorted by occlusion score, so we use it to determine the order to draw the view
    uint32_t numDrawn = 0;
    for (auto& thisElement : results->m_elements)
        {
        BeAssert(thisElement->IsPersistent());
        GeometrySourceCP geom = thisElement->ToGeometrySource();

        if (nullptr != geom)
            context.VisitElement(*geom);

        ++numDrawn;

        if (context.WasAborted())
            break;

        DgnElements& pool = m_queryModel.GetDgnDb().Elements();
        if (numDrawn > results->m_drawnBeforePurge && pool.GetTotalAllocated() > purgeTrigger)
            {
            // Testing for HasSelectResults prevents this logic from purging elements that
            // are in the selected-elements list. Adding elements to that list does not increment the reference 
            // count. DgnElements use reference counting that is not thread safe so all reference counting is done
            // in the work thread.
            if (!m_queryModel.HasSelectResults())
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
        }

    UpdateLogging::RecordDoneUpdate(numDrawn, context.GetDrawPurpose());

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

    //  We count on progressive display to draw zero length strings and points that are excluded by LOD filtering in the occlusion step.
    if ((DrawPurpose::CreateScene == context.GetDrawPurpose()) && (results->m_reachedMaxElements) && !m_noQuery)
        {
        DgnDb::SQLRequest::Client highPriority;
        DgnViewportP vp = context.GetViewport();
        CachedStatementPtr rangeStmt;
        m_queryModel.GetDgnDb().GetCachedStatement(rangeStmt, _GetRTreeMatchSql(*context.GetViewport()).c_str());
        BindModelAndCategory(*rangeStmt);

        QueryModel::ProgressiveFilter* pvFilter = new QueryModel::ProgressiveFilter(*vp, m_queryModel, m_neverDrawn.empty() ? nullptr : &m_neverDrawn, maxMem, rangeStmt.get());
        if (GetClipVector().IsValid())
            pvFilter->SetClipVector(*GetClipVector());

        vp->ScheduleProgressiveDisplay(*pvFilter);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/2015
//---------------------------------------------------------------------------------------
void QueryViewController::_VisitAllElements(ViewContextR context)
    {
    // Visit the elements that were actually loaded
    context.VisitDgnModel(&m_queryModel);

    // And step through the rest of the elements that were not loaded (but would be displayed by progressive display).
    CachedStatementPtr rangeStmt;
    m_queryModel.GetDgnDb().GetCachedStatement(rangeStmt, _GetRTreeMatchSql(*context.GetViewport()).c_str());
    BindModelAndCategory(*rangeStmt);
    QueryModel::ProgressiveFilter pvFilter (*context.GetViewport(), m_queryModel, m_neverDrawn.empty() ? nullptr : &m_neverDrawn, GetMaxElementMemory(), rangeStmt.get());

    while (pvFilter._Process(context, 0) != ProgressiveDisplay::Completion::Finished)
        ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
uint64_t QueryViewController::GetMaxElementMemory()
    {
    BeAssert(m_maxElementMemory != 0);
    return m_maxElementMemory != 0 ? m_maxElementMemory : 20 * 1024 * 1024;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2013
//---------------------------------------------------------------------------------------
#if defined (_X64_)
uint64_t QueryViewController::ComputeMaxElementMemory(DgnViewportCR vp)
    {
    uint64_t oneGig = 1024 * 1024 * 1024;
    m_maxElementMemory = 8 * oneGig;
    return m_maxElementMemory;
    }
#else
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2013
//---------------------------------------------------------------------------------------
uint64_t QueryViewController::ComputeMaxElementMemory(DgnViewportCR vp)
    {
    uint64_t oneMeg = 1024 * 1024;
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
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

