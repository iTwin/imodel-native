/*-------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudProgressiveDisplay.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

#include <Logging/bentleylogging.h>

//#define POINTCLOUD_TRACE

#if defined (POINTCLOUD_TRACE)
#   define LOG (*NativeLogging::LoggingManager::GetLogger (L"PointCloud"))
#   define DEBUG_PRINTF LOG.errorv
#else
#   define DEBUG_PRINTF
#endif

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD


//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
PointCloudProgressiveDisplay::PointCloudProgressiveDisplay (PointCloudModel const& model, PtViewport& ptViewport)
    :m_ptViewport(&ptViewport),
     m_model(model),
     m_waitTime(0),
     m_nextRetryTime(0),
     m_lastTentativeStopped(false)
    {
    m_tentativeId = 0;
    DRange3d pcRange = m_model.GetSceneRange();
    m_model.GetSceneToWorld().Multiply(m_sceneRangeWorld, pcRange);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
PointCloudProgressiveDisplay::~PointCloudProgressiveDisplay() 
    {
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

    PointCloudVortex::SetEnabledState(PtEnable::RGB_SHADER, context.GetViewport()->GetPointCloudViewSettingsR().GetUseRgb());
    PointCloudVortex::SetEnabledState(PtEnable::FRONT_BIAS, context.GetViewport()->GetPointCloudViewSettingsR().GetUseFrontBias());
    PointCloudVortex::SetEnabledState(PtEnable::ADAPTIVE_POINT_SIZE, false);
    PointCloudVortex::SetEnabledState(PtEnable::LIGHTING, context.GetViewport()->GetPointCloudViewSettingsR().GetUseLightning());
    PointCloudVortex::SetEnabledState(PtEnable::INTENSITY_SHADER, context.GetViewport()->GetPointCloudViewSettingsR().GetUseIntensity());
    PointCloudVortex::SetEnabledState(PtEnable::PLANE_SHADER, context.GetViewport()->GetPointCloudViewSettingsR().GetUsePlane());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
struct MyPointCloudDraw : Render::PointCloudDraw    //NEEDS_WORK_CONTINUOUS_RENDER temporary
    {
    virtual bool _IsThreadBound() { return false; } // I think we can remove that or somehow rework that concept.
    virtual bool _GetRange(DPoint3dP range) { return nullptr; }
    virtual bool _GetOrigin(DPoint3dP origin) { return nullptr; }

    virtual ColorDef const* _GetRgbColors() { return m_pRgb; }

    virtual uint32_t _GetNumPoints() { return m_ptCount; }
    virtual DPoint3dCP _GetDPoints() { return m_ptCP; }
    virtual FPoint3dCP _GetFPoints() { return nullptr; }

    uint32_t m_ptCount;
    DPoint3dCP m_ptCP;
    ColorDef const* m_pRgb;
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
bool PointCloudProgressiveDisplay::DrawPointCloud(int64_t& pointsToLoad, Dgn::RenderContextR context, PtQueryDensity densityType, float density, bool doCheckStop)
    {
    PointCloudQueryHandlePtr queryHandle(m_model.GetPointCloudSceneP()->GetFrustumQueryHandle());

    PointCloudVortex::StartDrawFrameMetrics();
    PointCloudVortex::ResetQuery(queryHandle->GetHandle());

    // POINTCLOUD_WIP_GR06_PointCloudDisplay - to do change density(or whatever they did in v8i) when DrawPurpose::UpdateDynamic.
    //PtVortex::DynamicFrameRate(displayParams.fps);
    PointCloudVortex::SetQueryDensity(queryHandle->GetHandle(), densityType, density);

    uint32_t channelFlags = (uint32_t) PointCloudChannelId::Xyz;
    if (m_model.GetPointCloudSceneP()->_HasRGBChannel())
        channelFlags |= (uint32_t) PointCloudChannelId::Rgb;
    
    RefCountedPtr<PointCloudQueryBuffers> queryBuffers = PointCloudQueryBuffers::Create(DRAW_QUERYCAPACITY, channelFlags);

    bool queryCompleted = true;
    m_lastTentativeStopped = false;

    Render::GraphicPtr pGraphic = context.CreateGraphic(Render::Graphic::CreateParams(context.GetViewport(), m_model.GetSceneToWorld()));
    Render::GraphicParams graphicParams;
    graphicParams.SetLineColor(ColorDef::White());
    graphicParams.SetFillColor(ColorDef::White());
    graphicParams.SetWidth(1);
    pGraphic->ActivateGraphicParams(graphicParams);

    uint32_t buffersCount = 0;

    while (1)
        {
        if (doCheckStop && context.CheckStop())
            {
            DEBUG_PRINTF("***** DrawPointCloud CheckStop Reach");
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
            pointCloudDraw.m_pRgb = (ColorDef const*)queryBuffers->GetRgbChannel()->GetChannelBuffer();
            pointCloudDraw.m_ptCount = queryBuffers->GetNumPoints();

            pGraphic->AddPointCloud(&pointCloudDraw);
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
        
    // Our first draw call must be fast and is considered a dynamic so we draw with a low density 
    // and do the full density update in the progressive display callback.
    static float density = 0.05f;
    static PtQueryDensity densityType = PtQueryDensity::QUERY_DENSITY_VIEW; // Get only points in memory for a view representation. Point still on disk will get loaded at a later time.
    //densityType = PtQueryDensity::QUERY_DENSITY_VIEW_COMPLETE; // Get every points (memory and disk) needed for a view representation. (Display is not progressive).

    SetupPtViewport(context);

    DEBUG_PRINTF("        ");
    DEBUG_PRINTF("Begin PointCloudProgressiveDisplay::DrawView");
    int64_t pointsToLoad = 0;
    if (!DrawPointCloud(pointsToLoad, context, densityType, density, false/*checkStop*/) || pointsToLoad > 0 || density < 1.0f)
        {
        context.GetViewportR().ScheduleTerrainProgressiveTask(*this);
        m_waitTime = 100;
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;
        m_lastTentativeStopped = true;   // do not wait to display full res. do it right away
        }
    DEBUG_PRINTF("End PointCloudProgressiveDisplay::DrawView");
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
    if (!m_lastTentativeStopped && BeTimeUtilities::GetCurrentTimeAsUnixMillis() < m_nextRetryTime)
        {
        //LOG.errorv("Wait %lld until next retry", m_nextRetryTime - BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        return Completion::Aborted;
        }

    ++m_tentativeId;   

    int64_t loadedSinceLastDraw = PointCloudVortex::PtsLoadedInViewportSinceLastDraw(m_model.GetPointCloudSceneP()->GetSceneHandle());
    DEBUG_PRINTF("(%d)Begin _DoProgressive loadedSinceDraw                        (%ld)", m_tentativeId, loadedSinceLastDraw);

    wantShow = WantShow::Yes; // Would like to show only when we have a good amount of new pts but we do not have that info.

    static float density = 1.0F;
    static PtQueryDensity densityType = PtQueryDensity::QUERY_DENSITY_VIEW; // Get only points in memory for a view representation. Points still on disk will get loaded at a later time.

    int64_t pointsToLoad = 0;
    if (!DrawPointCloud(pointsToLoad, context, densityType, density, true/*checkStop*/) || pointsToLoad > 0)
        {
        m_waitTime = (uint64_t)(m_waitTime * 1.33);
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;  

        DEBUG_PRINTF("(%d)Aborted _DoProgressive pointsToLoad=%ld", m_tentativeId, pointsToLoad);
        return ProgressiveTask::Completion::Aborted;
        }
    
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