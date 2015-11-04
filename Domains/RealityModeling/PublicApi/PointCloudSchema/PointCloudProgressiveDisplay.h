/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudProgressiveDisplay.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/DgnViewport.h>

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

//========================================================================================
// @bsiclass                                                        Eric.Paquet     5/2015
//========================================================================================
struct PointCloudProgressiveDisplay : Dgn::IProgressiveDisplay, NonCopyableClass
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

    friend struct PointCloudModel;

private:
    uint64_t    m_nextRetryTime;                             //!< When to re-try to query points. unix millis UTC
    uint64_t    m_waitTime;                                  //!< How long to wait before re-trying to query points. millis 

    bool        ShouldDrawInContext (ViewContextR context) const;

protected:
    PointCloudModel&         m_model;

    //! Displays point cloud and schedules downloads. 
    virtual Completion _Process(ViewContextR) override;

    // set limit and returns true to cause caller to call EnableStopAfterTimout
    virtual bool _WantTimeoutSet(uint32_t& limit) override {return false;}

    void DrawView (ViewContextR);

    PointCloudProgressiveDisplay (PointCloudModel& model);
    ~PointCloudProgressiveDisplay();
    };

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE