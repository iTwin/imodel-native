/*-------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudProgressiveDisplay.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

#include <Logging/bentleylogging.h>
#define LOG (*NativeLogging::LoggingManager::GetLogger (L"PointCloud"))

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
     m_nextRetryTime(0)
    {
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

    static bool s_frontBias = false;
    PointCloudVortex::SetEnabledState(PtEnable::RGB_SHADER, true);
    PointCloudVortex::SetEnabledState(PtEnable::FRONT_BIAS, s_frontBias);
    PointCloudVortex::SetEnabledState(PtEnable::ADAPTIVE_POINT_SIZE, false);
    PointCloudVortex::SetEnabledState(PtEnable::LIGHTING, false);
    PointCloudVortex::SetEnabledState(PtEnable::INTENSITY_SHADER, false);
    PointCloudVortex::SetEnabledState(PtEnable::PLANE_SHADER, false);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
bool PointCloudProgressiveDisplay::DrawPointCloud(int64_t& pointToLoad, Dgn::RenderContextR context, PtQueryDensity densityType, float density, bool doCheckStop)
    {
    SetupPtViewport(context);

    PointCloudQueryHandlePtr queryHandle(m_model.GetPointCloudSceneP()->GetFrustumQueryHandle());

    PointCloudVortex::StartDrawFrameMetrics();
    PointCloudVortex::ResetQuery(queryHandle->GetHandle());

    // POINTCLOUD_WIP_GR06_PointCloudDisplay - to do change density(or whatever they did in v8i) when DrawPurpose::UpdateDynamic.
    //PtVortex::DynamicFrameRate(displayParams.fps);
    PointCloudVortex::SetQueryDensity(queryHandle->GetHandle(), densityType, density);

    uint32_t channelFlags = (uint32_t) PointCloudChannelId::Xyz;
    if (m_model.GetPointCloudSceneP()->_HasRGBChannel())
        channelFlags |= (uint32_t) PointCloudChannelId::Rgb;

    PointCloudQueryBuffersPtr queryBuffers = PointCloudQueryBuffers::Create(DRAW_QUERYCAPACITY, channelFlags);

    bool queryCompleted = true;

    while (1)
        {
        if (doCheckStop && context.CheckStop())
            {
            queryCompleted = false;
            break;
            }

        uint64_t numQueryPoints = queryBuffers->GetPoints(queryHandle->GetHandle());
        if (numQueryPoints == 0)
            break;

        // NEEDS_WORK_CONTINUOUS_RENDER temporary
        if(queryBuffers->HasXyz())
            {            
            Render::GraphicPtr pGraphic = context.CreateGraphic(Render::Graphic::CreateParams(context.GetViewport(), m_model.GetSceneToWorld()));

            Render::GraphicParams graphicParams;
            graphicParams.SetLineColor(ColorDef::White());
            graphicParams.SetFillColor(ColorDef::White());
            graphicParams.SetWidth(1);
            pGraphic->ActivateGraphicParams(graphicParams);
            pGraphic->AddPointString(queryBuffers->GetNumPoints(), queryBuffers->GetXyzChannel()->GetChannelBuffer());

            context.OutputGraphic(*pGraphic, nullptr);
            }
        }

    pointToLoad = 0;
    if (queryCompleted && PtQueryDensity::QUERY_DENSITY_VIEW == densityType)
        pointToLoad = PointCloudVortex::PtsToLoadInViewport(m_model.GetPointCloudSceneP()->GetSceneHandle(), false/*recompute*/);
        
    PointCloudVortex::EndDrawFrameMetrics();

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

    int64_t pointsToLoad = 0;
    if (!DrawPointCloud(pointsToLoad, context, densityType, density, false/*checkStop*/) || pointsToLoad > 0 || density < 1.0f)
        {
        context.GetViewportR().ScheduleProgressiveTask(*this);
        m_waitTime = 100;
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;
        }
    }

//----------------------------------------------------------------------------------------
// This callback is invoked on a timer during progressive display.
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
ProgressiveTask::Completion PointCloudProgressiveDisplay::_DoProgressive(Dgn::ProgressiveContext& context, WantShow& wantShow)
    {
    if (BeTimeUtilities::GetCurrentTimeAsUnixMillis() < m_nextRetryTime)
        {
        LOG.tracev("Wait %lld until next retry", m_nextRetryTime - BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        return Completion::Aborted;
        }

    wantShow = WantShow::Yes; // Would like to show only when we have a good amount of new pts but we do not have that info.

    static float density = 1.0F;
    static PtQueryDensity densityType = PtQueryDensity::QUERY_DENSITY_VIEW; // Get only points in memory for a view representation. Point still on disk will get loaded at a later time.

    int64_t pointsToLoad = 0;
    if (!DrawPointCloud(pointsToLoad, context, densityType, density, true/*checkStop*/) || pointsToLoad > 0)
        {
        m_waitTime = (uint64_t)(m_waitTime * 1.33);
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;  
        return ProgressiveTask::Completion::Aborted;
        }
    
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