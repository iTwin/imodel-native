/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudProgressiveDisplay.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/DgnViewport.h>

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

//&&MM remove from publicApi.
//========================================================================================
// @bsiclass                                                        Eric.Paquet     5/2015
//========================================================================================
struct PointCloudProgressiveDisplay : Dgn::ProgressiveTask
    {
    friend struct PointCloudModel;

private:
    uint64_t    m_nextRetryTime;                             //!< When to re-try to query points. unix millis UTC
    uint64_t    m_waitTime;                                  //!< How long to wait before re-trying to query points. millis 

    static bool ShouldDrawInContext (Dgn::RenderContextR context);

protected:
    PointCloudModel const& m_model;

    //! Displays point cloud and schedules downloads. 
    virtual Completion _DoProgressive(Dgn::ProgressiveContext& context, WantShow&) override;

    void DrawView (Dgn::RenderContextR);

    PointCloudProgressiveDisplay (PointCloudModel const& model);
    virtual ~PointCloudProgressiveDisplay();
    };

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE