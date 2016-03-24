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
PointCloudProgressiveDisplay::PointCloudProgressiveDisplay (PointCloudModel const& model) 
    :
    m_model(model),
    m_waitTime(0),
    m_nextRetryTime(0)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
PointCloudProgressiveDisplay::~PointCloudProgressiveDisplay() 
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
void PointCloudProgressiveDisplay::DrawView (Dgn::RenderContextR context)
    {
    BeAssert(nullptr != context.GetViewport()); // a scene should always have a viewport.

    // **********************************
    // *** NB: This method must be fast. 
    // **********************************
    // Do not try to read from SQLite or allocate huge amounts of memory in here. 
    // Defer time-consuming tasks to progressive display

    //
    //  First, determine if we can draw map tiles at all.
    //
    if (!ShouldDrawInContext (context) || m_model.GetPointCloudSceneP() == nullptr)     //&&MM do that prior to allocate PointCloudProgressiveDisplay.
        return;

     PointCloudScenePtr scenePtr = m_model.GetPointCloudSceneP();
     ProgressiveTask::Completion drawStatus = VisualizationManager::GetInstance().DrawToContext(context, *scenePtr, m_model.GetRange());
     if (drawStatus != ProgressiveTask::Completion::Finished)
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

    PointCloudScenePtr scenePtr = m_model.GetPointCloudSceneP();
    ProgressiveTask::Completion drawStatus = VisualizationManager::GetInstance().DrawToContext(context, *scenePtr, m_model.GetRange());

    if (drawStatus != ProgressiveTask::Completion::Finished)
        {
        m_waitTime = (uint64_t)(m_waitTime * 1.33);
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;  
        }

    //&&MM todo wantShow only if have a good amount of pts.
    wantShow = WantShow::Yes;

    return drawStatus;
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