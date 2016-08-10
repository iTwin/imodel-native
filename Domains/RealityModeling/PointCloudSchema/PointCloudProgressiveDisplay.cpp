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
    auto spatial = context.GetViewport()->GetViewController()._ToSpatialView();
    if (nullptr==spatial)
        {
        BeAssert(false);
        return;
        }

    PointCloudVortex::SetViewport(m_ptViewport->GetId());
    VisualizationManager::SetViewportInfo(context, m_model.GetSceneToWorld(), m_sceneRangeWorld);

    // Use same values than Vancouver implementation - maybe some tuning would be needed here.
    PointCloudVortex::DynamicFrameRate (15.0f);
    PointCloudVortex::StaticOptimizer(0.5);

    // Changing the global density has no effect once points have been loaded. Instead, alter the display query density 
    PointCloudVortex::GlobalDensity(1.0f);

    auto const& settings = PointCloudViewSettings::FromView(*spatial);

    PointCloudVortex::SetEnabledState(PtEnable::RGB_SHADER, settings.GetUseRgb());
    PointCloudVortex::SetEnabledState(PtEnable::FRONT_BIAS, settings.GetUseFrontBias());
    PointCloudVortex::SetEnabledState(PtEnable::ADAPTIVE_POINT_SIZE, false);
    PointCloudVortex::SetEnabledState(PtEnable::LIGHTING, false);

    PointCloudViewSettings::DisplayStyle displayStyle = settings.GetDisplayStyle();
    if (displayStyle == PointCloudViewSettings::DisplayStyle::Custom)
        {
        PointCloudVortex::SetEnabledState (PtEnable::INTENSITY_SHADER, settings.GetUseIntensity());
        PointCloudVortex::SetEnabledState (PtEnable::PLANE_SHADER, settings.GetUsePlane());
        if (settings.GetUseIntensity()) //if intensity set to true
            {
            PointCloudVortex::ShaderOptionf( PtShaderOptions::INTENSITY_SHADER_BRIGHTNESS, settings.GetBrightness () );
            PointCloudVortex::ShaderOptionf( PtShaderOptions::INTENSITY_SHADER_CONTRAST, settings.GetContrast () );
            PointCloudVortex::ShaderOptioni( PtShaderOptions::INTENSITY_SHADER_RAMP, settings.GetIntensityRampIdx ());
            BeAssert (settings.GetIntensityRampIdx() != INVALID_RAMP_INDEX);
            }

        if (settings.GetUsePlane()) //if elevation is set to true
            {
            PointCloudVortex::ShaderOptioni( PtShaderOptions::PLANE_SHADER_RAMP, settings.GetPlaneRampIdx ());
            BeAssert (settings.GetPlaneRampIdx() != INVALID_RAMP_INDEX);
            PointCloudVortex::ShaderOptionf (PtShaderOptions::PLANE_SHADER_DISTANCE, settings.GetDistance ());
            PointCloudVortex::ShaderOptionf (PtShaderOptions::PLANE_SHADER_OFFSET, settings.GetOffset ());

            //Clamp elevation value
            if (settings.GetClampIntensity())
                PointCloudVortex::ShaderOptioni (PtShaderOptions::PLANE_SHADER_EDGE, 0x01);
            else
                PointCloudVortex::ShaderOptioni (PtShaderOptions::PLANE_SHADER_EDGE, 0x00);

            float axis [] = {0,0,0};
            if (settings.GetUseACSAsPlaneAxis () && IACSManager::GetManager().GetActive (*context.GetViewport()))
                {
                RotMatrix rot;
                DVec3d direction;

                IACSManager::GetManager().GetActive (*context.GetViewport())->GetRotation (rot);
                rot.GetRow(direction, 2);
                direction.Normalize();
                axis[0]= (float)direction.x;
                axis[1]= (float)direction.y;
                axis[2]= (float)direction.z;
                }
            else
                axis [std::min<uint16_t>(2, settings.GetPlaneAxis ())] = 1.0f;

            PointCloudVortex::ShaderOptionfv( PtShaderOptions::PLANE_SHADER_VECTOR, axis ); 
            }
        }
    else if (displayStyle == PointCloudViewSettings::DisplayStyle::Intensity)
        {
        PointCloudVortex::SetEnabledState (PtEnable::INTENSITY_SHADER, true);
        PointCloudVortex::SetEnabledState (PtEnable::PLANE_SHADER, false);
        PointCloudVortex::ShaderOptionf( PtShaderOptions::INTENSITY_SHADER_BRIGHTNESS, settings.GetBrightness () );
        PointCloudVortex::ShaderOptionf( PtShaderOptions::INTENSITY_SHADER_CONTRAST, settings.GetContrast () );
        PointCloudVortex::ShaderOptioni( PtShaderOptions::INTENSITY_SHADER_RAMP, settings.GetIntensityRampIdx ());
        BeAssert (settings.GetIntensityRampIdx() != INVALID_RAMP_INDEX);
        }
    else if (displayStyle == PointCloudViewSettings::DisplayStyle::Location)
        {
        PointCloudVortex::SetEnabledState (PtEnable::INTENSITY_SHADER, false);
        PointCloudVortex::SetEnabledState (PtEnable::PLANE_SHADER, true);
        PointCloudVortex::ShaderOptioni( PtShaderOptions::PLANE_SHADER_RAMP, settings.GetPlaneRampIdx ());

        BeAssert (settings.GetPlaneRampIdx() != INVALID_RAMP_INDEX);

        //Clamp elevation value
        if (settings.GetClampIntensity())
            PointCloudVortex::ShaderOptioni (PtShaderOptions::PLANE_SHADER_EDGE, 0x01);
        else
            PointCloudVortex::ShaderOptioni (PtShaderOptions::PLANE_SHADER_EDGE, 0x00);

        PointCloudVortex::ShaderOptionf (PtShaderOptions::PLANE_SHADER_DISTANCE, settings.GetDistance ());
        PointCloudVortex::ShaderOptionf (PtShaderOptions::PLANE_SHADER_OFFSET, settings.GetOffset ());

        float axis [] = {0,0,0};
        if (settings.GetUseACSAsPlaneAxis () && IACSManager::GetManager().GetActive (*context.GetViewport()))
            {
            RotMatrix rot;
            DVec3d direction;
            IACSManager::GetManager().GetActive (*context.GetViewport())->GetRotation (rot);
            rot.GetRow(direction,  2);
            direction.Normalize();
            axis[0]= (float)direction.x;
            axis[1]= (float)direction.y;
            axis[2]= (float)direction.z;
            }
        else
            axis [std::min<uint16_t>(2, settings.GetPlaneAxis ())] = 1.0f;
        PointCloudVortex::ShaderOptionfv( PtShaderOptions::PLANE_SHADER_VECTOR, axis ); 
        }
    else
        {
        BeAssert (displayStyle == PointCloudViewSettings::DisplayStyle::None || 
                    displayStyle == PointCloudViewSettings::DisplayStyle::Classification);
        PointCloudVortex::SetEnabledState (PtEnable::INTENSITY_SHADER, false);
        PointCloudVortex::SetEnabledState (PtEnable::PLANE_SHADER, false);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
bool PointCloudProgressiveDisplay::DrawPointCloud(Render::GraphicPtr* pGraphicsPtr, int64_t& pointsToLoad, uint64_t& pointsDrawn, Dgn::RenderContextR context, PtQueryDensity densityType, float density, bool doCheckStop)
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

    uint32_t buffersCount = 0;

    Dgn::Render::GraphicBranch graphicArray;

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

        if(queryBuffers->HasXyz())
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

            //&&MM Pointools query optimization
            //      - A query that doesn't use global var. and that we can resume. ex: ptViewport
            //      - A query that will return all points without having to restart over over.
            //      - Query that return points in floats with an origin. local to the query and not a global setting.
            //      - Can it supports multiple query? Will it affect ptsTolLoad and loaded voxel?
            //          ex: full density is started
            //              full density pause
            //              low density start and complete
            //              resume full density.
            //          
            //          Same scenario may occurs with multiple viewports. i.e. alternating between 2 queries of a different region.
            //  

            //&&MM Need query in floats NEEDS_WORK_POINT_CLOUD
            static std::vector<FPoint3d> s_floatsPts;
            if (s_floatsPts.size() < queryBuffers->GetNumPoints())
                s_floatsPts.resize(queryBuffers->GetNumPoints());

            ByteCP     pRgb = queryBuffers->HasRgb() ? (ByteCP)queryBuffers->GetRgbChannel()->GetChannelBuffer() : nullptr;
            DPoint3dCP pXyz = queryBuffers->GetXyzChannel()->GetChannelBuffer();
            DPoint3d   origin = pXyz[0];
            uint32_t   numPoints = queryBuffers->GetNumPoints();
          
            for (size_t i = 0; i < numPoints; ++i)
                {
                s_floatsPts[i].x = (float)(pXyz[i].x - origin.x);
                s_floatsPts[i].y = (float)(pXyz[i].y - origin.y);
                s_floatsPts[i].z = (float)(pXyz[i].z - origin.z);
                }            

            auto graphic = context.CreateGraphic(Render::Graphic::CreateParams(context.GetViewport()));
            Render::GraphicParams graphicParams;
            graphicParams.SetLineColor(m_model.GetColor());
            graphicParams.SetFillColor(m_model.GetColor());
            graphicParams.SetWidth(context.GetViewport()->GetIndexedLineWidth(m_model.GetWeight()));
            graphic->ActivateGraphicParams(graphicParams);
            graphic->AddPointCloud(queryBuffers->GetNumPoints(), origin, s_floatsPts.data(), pRgb);
            graphicArray.Add(*graphic);

            pointsDrawn += queryBuffers->GetNumPoints();
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

    if (!graphicArray.m_entries.empty())
        {
        auto graphicGroup = context.CreateBranch(Render::Graphic::CreateParams(context.GetViewport(), m_model.GetSceneToWorld()), graphicArray);
        context.OutputGraphic(*graphicGroup, nullptr);

        if (pGraphicsPtr != nullptr)
            *pGraphicsPtr = graphicGroup;
        }

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
        static float density = 0.10f; //&&MM make this an option? used to be 0.05 in Microstation but low density is now defer to progressive display so we can be a bit slower.
        static PtQueryDensity densityType = PtQueryDensity::QUERY_DENSITY_VIEW; // Get only points in memory for a view representation. Point still on disk will get loaded at a later time.
        int64_t pointsToLoad = 0;
        Dgn::Render::GraphicPtr pLowDensityGraphic;

        DrawPointCloud(&pLowDensityGraphic, pointsToLoad, m_firstPassPts, context, densityType, density, false/*checkStop*/);
        
        if (pLowDensityGraphic.IsValid())
            {
            const_cast<PointCloudModel&>(m_model).SaveLowDensityGraphic(context.GetViewportR(), pLowDensityGraphic.get());
            wantShow = WantShow::Yes;
            m_doLowDensity = false;
            }
        else if (0 == pointsToLoad)
            {
            // no graphic no points to load: nothing to see in this view.
            m_doLowDensity = false;
            return ProgressiveTask::Completion::Finished;
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

