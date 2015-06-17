/*--------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFileProgressiveDisplay.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

//========================================================================================
// @bsiclass                                                        Eric.Paquet     5/2015
//========================================================================================
struct RasterFileProgressiveDisplay : DgnPlatform::IProgressiveDisplay, NonCopyableClass
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

    friend struct RasterFileModel;

private:
    bool        ShouldDrawInContext (ViewContextR context) const;

protected:
    RasterFileModel&         m_model;

    //! Displays point cloud and schedules downloads. 
    virtual Completion _Process(ViewContextR) override;

    // set limit and returns true to cause caller to call EnableStopAfterTimout
    virtual bool _WantTimeoutSet(uint32_t& limit) override {return false;}

    void DrawView (ViewContextR);

    RasterFileProgressiveDisplay (RasterFileModel& model);
    ~RasterFileProgressiveDisplay();
    };

END_BENTLEY_RASTERSCHEMA_NAMESPACE