/*-------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudProgressiveDisplay.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

// These pragma are necessary because of the "DEBUG_PRINTF" macro. With clang, they cause a "-Wunused-value" error.
#ifdef __APPLE__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunused-value"
#endif

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
PointCloudProgressiveDisplay::PointCloudProgressiveDisplay (PointCloudModel const& model, PtViewport& ptViewport)
    :m_ptViewport(&ptViewport),
    m_model(model),
    m_waitTime(0),
    m_nextRetryTime(0),
    m_lastTentativeStopped(false),
    m_progressivePts(0),
    m_firstPassPts(0),
    m_totalProgressivePts(0),
    m_doLowDensity(false)
    {
    m_tentativeId = 0;
    m_model.GetRange(m_sceneRangeWorld, PointCloudModel::Unit::World);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
PointCloudProgressiveDisplay::~PointCloudProgressiveDisplay() 
    {
    INFO_PRINTF("END Display firstPass=%ld progressive=%ld TotalProgressive=%ld", m_firstPassPts, m_progressivePts, m_totalProgressivePts);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void PointCloudProgressiveDisplay::SetupPtViewport(Dgn::RenderContextR context)
    {
    PointCloudVortex::SetViewport(m_ptViewport->GetId());
    VisualizationManager::SetViewportInfo(context, m_model.GetSceneToWorld(), m_sceneRangeWorld);

    // Changing the global density has no effect once points have been loaded. Instead, alter the display query density 
    PointCloudVortex::GlobalDensity(1.0f);

    auto spatial = context.GetViewport()->GetViewController()._ToSpatialView();
    if (nullptr==spatial)
        {
        BeAssert(false);
        return;
        }
    auto const& settings = spatial->GetPointCloudSettings();

    PointCloudVortex::SetEnabledState(PtEnable::RGB_SHADER, settings.GetUseRgb());
    PointCloudVortex::SetEnabledState(PtEnable::FRONT_BIAS, settings.GetUseFrontBias());
    PointCloudVortex::SetEnabledState(PtEnable::ADAPTIVE_POINT_SIZE, false);
    PointCloudVortex::SetEnabledState(PtEnable::LIGHTING, settings.GetUseLightning());
    PointCloudVortex::SetEnabledState(PtEnable::INTENSITY_SHADER, settings.GetUseIntensity());
    PointCloudVortex::SetEnabledState(PtEnable::PLANE_SHADER, settings.GetUsePlane());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
struct MyPointCloudDraw : Render::PointCloudDraw    //NEEDS_WORK_CONTINUOUS_RENDER temporary
    {
    //&&MM QV OPTIMIZATION
    // qv_addPoints / qv_addPointsFloat
    //      - use memcpy to copy Rgb values instead of byte per byte.
    //      - use float and memcpy instead of float per float copy.
    //      - add range param if we can provide it?
    //      - Can we call qv_addPoints from multiple threads?
    // Ask Karin for a good chunk size. currently DRAW_QUERYCAPACITY=1,048,576
    // can we pre-allocate qv graphic and assigned directly in qv buffer.
    // Performance difference between qvi_displayPoints and qv_addPoints/qvi_displayElement ?

    uint32_t m_ptCount;
    DPoint3dCP m_ptCP;
    ColorDef const* m_pRgb;

    virtual bool _IsThreadBound() { return false; } // I think we can remove that or somehow rework that concept.
    virtual bool _GetRange(DPoint3dP range) { return false; }
    virtual bool _GetOrigin(DPoint3dP origin) { return false; }

    virtual ColorDef const* _GetRgbColors() { return m_pRgb; }

    virtual uint32_t _GetNumPoints() { return m_ptCount; }
    virtual DPoint3dCP _GetDPoints() { return m_ptCP; }
    virtual FPoint3dCP _GetFPoints() { return nullptr; }        //&&MM use qv_addPointsFloat
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
bool PointCloudProgressiveDisplay::DrawPointCloud(Render::GraphicBuilderPtr* pGraphicsPtr, int64_t& pointsToLoad, uint64_t& pointsDrawn, Dgn::RenderContextR context, PtQueryDensity densityType, float density, bool doCheckStop)
    {
    PointCloudQueryHandlePtr queryHandle(m_model.GetPointCloudSceneP()->GetFrustumQueryHandle());

    PointCloudVortex::StartDrawFrameMetrics();
    PointCloudVortex::ResetQuery(queryHandle->GetHandle());
    PointCloudVortex::SetQueryDensity(queryHandle->GetHandle(), densityType, density);
    
    uint32_t channelFlags = (uint32_t) PointCloudChannelId::Xyz;
    if (m_model.GetPointCloudSceneP()->_HasRGBChannel())
        channelFlags |= (uint32_t) PointCloudChannelId::Rgb;
    
    RefCountedPtr<PointCloudQueryBuffers> queryBuffers = PointCloudQueryBuffers::Create(DRAW_QUERYCAPACITY, channelFlags);

    bool queryCompleted = true;
    m_lastTentativeStopped = false;

    auto pGraphic = context.CreateGraphic(Render::Graphic::CreateParams(context.GetViewport(), m_model.GetSceneToWorld()));
    Render::GraphicParams graphicParams;
    graphicParams.SetLineColor(m_model.GetColor());
    graphicParams.SetFillColor(m_model.GetColor());
    graphicParams.SetWidth(context.GetViewport()->GetIndexedLineWidth(m_model.GetWeight()));
    pGraphic->ActivateGraphicParams(graphicParams);

    uint32_t buffersCount = 0;

    while (1)
        {
        if (doCheckStop && context.CheckStop())
            {
            DEBUG_PRINTF("CHECK_STOP reached, aborting");
            queryCompleted = false;
            m_lastTentativeStopped = true;
            break;
            }

        uint64_t numQueryPoints = queryBuffers->GetPoints(queryHandle->GetHandle());
        if (numQueryPoints == 0)
            break;

        // NEEDS_WORK_CONTINUOUS_RENDER temporary
        if(queryBuffers->HasXyz())
            {          
            MyPointCloudDraw pointCloudDraw;
            pointCloudDraw.m_ptCP = queryBuffers->GetXyzChannel()->GetChannelBuffer();
            pointCloudDraw.m_pRgb = queryBuffers->HasRgb() ? (ColorDef const*) queryBuffers->GetRgbChannel()->GetChannelBuffer() : nullptr;
            pointCloudDraw.m_ptCount = queryBuffers->GetNumPoints();

            pGraphic->AddPointCloud(&pointCloudDraw);
            pointsDrawn += pointCloudDraw.m_ptCount;
            ++buffersCount;
            }
        }

    pointsToLoad = 0;
    if (queryCompleted && PtQueryDensity::QUERY_DENSITY_VIEW == densityType)
        {
        // From observation, we need to recompute ptsToload otherwise the count is not [always?] updated.
        pointsToLoad = PointCloudVortex::PtsToLoadInViewport(m_model.GetPointCloudSceneP()->GetSceneHandle(), true/*recompute*/);
        }
        
    PointCloudVortex::EndDrawFrameMetrics();

    context.OutputGraphic(*pGraphic, nullptr);

    if (pGraphicsPtr != nullptr)
        *pGraphicsPtr = pGraphic;
    DEBUG_PRINTF("DrawPointCloud outputting %d, ptToload =%ld", buffersCount, pointsToLoad);

    return queryCompleted;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
void PointCloudProgressiveDisplay::DrawView (Dgn::RenderContextR context)
    {
    BeAssert(nullptr != context.GetViewport());
    BeAssert(nullptr != m_model.GetPointCloudSceneP());

    // **********************************
    // *** NB: This method must be fast. 
    // **********************************
    // Do not try to read from SQLite or allocate huge amounts of memory in here. 
    // Defer time-consuming tasks to progressive display    

    // Output previous low density if we have any.  A new one will be computed in the progressive display. 
    Dgn::Render::Graphic* pGraphic = m_model.GetLowDensityGraphicP(context.GetViewportR());
    if(nullptr != pGraphic)
        context.OutputGraphic(*pGraphic, nullptr);
       
    context.GetViewportR().ScheduleTerrainProgressiveTask(*this);
    m_waitTime = 100;
    m_nextRetryTime = 0;//BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;
    m_lastTentativeStopped = true;   // do not wait to display full res. do it right away
    m_doLowDensity = true;
    }

//----------------------------------------------------------------------------------------
// This callback is invoked on a timer during progressive display.
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
ProgressiveTask::Completion PointCloudProgressiveDisplay::_DoProgressive(Dgn::ProgressiveContext& context, WantShow& wantShow)
    {
    // NEEDS_WORK_CONTINUOUS_RENDER:  Can we do something better than a nextRetryTime?  
    //  ex: if accurate we could use pointsToLoad and PtsLoadedInViewportSinceLastDraw to detect that
    //      we need to redraw.
    if (!m_doLowDensity && !m_lastTentativeStopped && BeTimeUtilities::GetCurrentTimeAsUnixMillis() < m_nextRetryTime)
        {
        //LOG.errorv("Wait %lld until next retry", m_nextRetryTime - BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        return Completion::Aborted;
        }

    SetupPtViewport(context);

    if (m_doLowDensity)
        {
        // Our first draw call must be fast and is considered a dynamic so we draw with a low density 
        // and do the full density update in the progressive display callback.
        static float density = 0.05f;
        static PtQueryDensity densityType = PtQueryDensity::QUERY_DENSITY_VIEW; // Get only points in memory for a view representation. Point still on disk will get loaded at a later time.
        int64_t pointsToLoad = 0;
        Dgn::Render::GraphicBuilderPtr pLowDensityGraphic;

        DrawPointCloud(&pLowDensityGraphic, pointsToLoad, m_firstPassPts, context, densityType, density, false/*checkStop*/);
        m_doLowDensity = false;

        if (pLowDensityGraphic.IsValid())
            {
            const_cast<PointCloudModel&>(m_model).SaveLowDensityGraphic(context.GetViewportR(), pLowDensityGraphic.GetGraphic());
            wantShow = WantShow::Yes;
            }

        return ProgressiveTask::Completion::Aborted;
        }

    ++m_tentativeId;   

    int64_t loadedSinceLastDraw = PointCloudVortex::PtsLoadedInViewportSinceLastDraw(m_model.GetPointCloudSceneP()->GetSceneHandle());  UNUSED_VARIABLE(loadedSinceLastDraw);
    DEBUG_PRINTF("(%d)Begin _DoProgressive loadedSinceDraw                        (%ld)", m_tentativeId, loadedSinceLastDraw);

    float density = m_model.GetViewDensity();
    static PtQueryDensity densityType = PtQueryDensity::QUERY_DENSITY_VIEW; // Get only points in memory for a view representation. Points still on disk will get loaded at a later time.

    uint64_t oldProgressivePts = m_progressivePts;
    m_progressivePts = 0;
    int64_t pointsToLoad = 0;
    if (!DrawPointCloud(nullptr, pointsToLoad, m_progressivePts, context, densityType, density, true/*checkStop*/) || pointsToLoad > 0)
        {
        m_totalProgressivePts += m_progressivePts;
        m_waitTime = (uint64_t)(m_waitTime * 1.33);
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;  

        DEBUG_PRINTF("(%d)Aborted _DoProgressive pointsToLoad=%ld", m_tentativeId, pointsToLoad);
        
        if (m_progressivePts > oldProgressivePts + 20000)
            wantShow = WantShow::Yes;
        return ProgressiveTask::Completion::Aborted;
        }


    wantShow = WantShow::Yes;
    m_totalProgressivePts += m_progressivePts;
    DEBUG_PRINTF("(%d)Finished _DoProgressive", m_tentativeId);
    return ProgressiveTask::Completion::Finished;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
bool PointCloudProgressiveDisplay::ShouldDrawInContext(Dgn::RenderContextR context)
    {
    switch (context.GetDrawPurpose())
        {
        case DrawPurpose::Pick:
        case DrawPurpose::CaptureGeometry:
        case DrawPurpose::FenceAccept:
        case DrawPurpose::RegionFlood:
        case DrawPurpose::FitView:
        case DrawPurpose::ExportVisibleEdges:
        case DrawPurpose::ClashDetection:
        case DrawPurpose::ModelFacet:
            return false;
        }

    return true;
    }

#ifdef __APPLE__
#   pragma clang diagnostic pop
#endif


