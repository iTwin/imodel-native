/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFileProgressiveDisplay.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

//&&ep - Can eliminate completely RasterFileProgressiveDisplay ?

#include <RasterSchemaInternal.h>

#include <Logging/bentleylogging.h>
#define LOG (*NativeLogging::LoggingManager::GetLogger (L"RasterFile"))

//&&ep needed here ?
#define QV_RGBA_FORMAT   0
#define QV_BGRA_FORMAT   1
#define QV_RGB_FORMAT    2
#define QV_BGR_FORMAT    3

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_RASTERSCHEMA

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
RasterFileProgressiveDisplay::RasterFileProgressiveDisplay (RasterFileModel& model) 
    :
    m_model(model)
    {
    // Default initialization for ref counted
    DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
RasterFileProgressiveDisplay::~RasterFileProgressiveDisplay() 
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
void RasterFileProgressiveDisplay::DrawView (ViewContextR context)
    {
    // **********************************
    // *** NB: This method must be fast. 
    // **********************************
    // Do not try to read from SQLite or allocate huge amounts of memory in here. 
    // Defer time-consuming tasks to progressive display

    //
    //  First, determine if we can draw map tiles at all.
    //
    if (!ShouldDrawInContext (context) || NULL == context.GetViewport())
        return;


//&&ep test
/*
{
    uint32_t wid = m_model.GetRasterFilePtr()->GetWidth();
    if (wid == 7) return;

    Byte   *bitMap;
    Point2d imageSize;
    BentleyStatus status = m_model.GetRasterFilePtr()->ReadFileToBitmap(&bitMap, &imageSize);
    if (status == ERROR)
        return;

    DPoint3d    rasterPoints[4];
    bool        enableAlpha = true;
    rasterPoints[0].x = 0; rasterPoints[0].y = 0; rasterPoints[0].z = 0; 
    rasterPoints[1].x = 0; rasterPoints[1].y = 10; rasterPoints[1].z = 0; 
    rasterPoints[2].x = 10; rasterPoints[2].y = 0; rasterPoints[2].z = 0; 
    rasterPoints[3].x = 10; rasterPoints[3].y = 15; rasterPoints[3].z = 0; 

    #define WIDTH 100
    #define HEIGHT 100
    int     width = WIDTH;
    int     height = HEIGHT;
    Byte pixels[WIDTH * HEIGHT * 4];

    int pitch = 0;
    for (int i = 0; i < WIDTH * HEIGHT; i++)
        {
        pixels[pitch] = 255;
        pixels[pitch + 1] = 0;
        pixels[pitch + 2] = 0;
        pixels[pitch + 3] = 255;
        pitch += 4;
        }

    context.GetIViewDraw().DrawRaster (rasterPoints, width*4, width, height, enableAlpha, QV_BGRA_FORMAT, pixels, NULL);

}
*/

/* &&ep d

    // **********************************
    // *** NB: This method must be fast. 
    // **********************************
    // Do not try to read from SQLite or allocate huge amounts of memory in here. 
    // Defer time-consuming tasks to progressive display

    //
    //  First, determine if we can draw map tiles at all.
    //
    if (!ShouldDrawInContext (context) || NULL == context.GetViewport())
        return;

    PointCloudScenePtr scenePtr = m_model.GetPointCloudScenePtr();
    IProgressiveDisplay::Completion drawStatus = VisualizationManager::GetInstance().DrawToContext(context, *scenePtr, m_model.GetRangeR());
    if (drawStatus != IProgressiveDisplay::Completion::Finished)
        {
        context.GetViewport()->ScheduleProgressiveDisplay (*this);
        m_waitTime = 100;
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;
        }
*/
    }

//----------------------------------------------------------------------------------------
// This callback is invoked on a timer during progressive display.
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
IProgressiveDisplay::Completion RasterFileProgressiveDisplay::_Process (ViewContextR context)
    {
/* &&ep d
    if (BeTimeUtilities::GetCurrentTimeAsUnixMillis() < m_nextRetryTime)
        {
        LOG.tracev("Wait %lld until next retry", m_nextRetryTime - BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        return Completion::Aborted;
        }

    PointCloudScenePtr scenePtr = m_model.GetPointCloudScenePtr();
    IProgressiveDisplay::Completion drawStatus = VisualizationManager::GetInstance().DrawToContext(context, *scenePtr, m_model.GetRangeR());

    if (drawStatus != IProgressiveDisplay::Completion::Finished)
        {
        m_waitTime = (uint64_t)(m_waitTime * 1.33);
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;  
        }
*/

IProgressiveDisplay::Completion drawStatus = IProgressiveDisplay::Completion::Finished;
    return drawStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFileProgressiveDisplay::ShouldDrawInContext (ViewContextR context) const
    {
    switch (context.GetDrawPurpose())
        {
        case DrawPurpose::Hilite:
        case DrawPurpose::Unhilite:
        case DrawPurpose::ChangedPre:       // Erase, rely on Healing.
        case DrawPurpose::RestoredPre:      // Erase, rely on Healing.
        case DrawPurpose::Pick:
        case DrawPurpose::Flash:
        case DrawPurpose::CaptureGeometry:
        case DrawPurpose::FenceAccept:
        case DrawPurpose::RegionFlood:
        case DrawPurpose::FitView:
        case DrawPurpose::ExportVisibleEdges:
        case DrawPurpose::ModelFacet:
            return false;
        }

    return true;
    }
