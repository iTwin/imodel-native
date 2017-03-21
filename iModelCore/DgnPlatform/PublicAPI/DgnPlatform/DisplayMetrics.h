/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DisplayMetrics.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "DgnCategory.h"
#include "DgnElement.h"
#include "Render.h"
#include "ViewController.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                      John.Gooding    01/17
//=======================================================================================
struct DisplayMetricsHandler
{
    DGNPLATFORM_EXPORT static void RecordQuerySceneComplete(double seconds, ViewController::QueryResults const& queryResults);
    DGNPLATFORM_EXPORT static void RecordCreateSceneComplete(double seconds, ViewController::Scene const & scene, bool aborted, bool complete);
    DGNPLATFORM_EXPORT static bool HandleForceHealImmediate(DgnViewportP vp, UpdatePlan& plan);
};

//=======================================================================================
// @bsiclass                                                      John.Gooding    01/17
//=======================================================================================
struct IDisplayMetricsHandler :  DgnHost::IHostObject
{
    DGNPLATFORM_EXPORT static bool IsRecorderActive();
    DGNPLATFORM_EXPORT static IDisplayMetricsHandler* GetHandler();
    DGNPLATFORM_EXPORT static void SetHandler(IDisplayMetricsHandler*logger);
    virtual bool _IsRecorderActive() const = 0;
    virtual void _RecordMeasurement(Utf8CP measurementType, JsonValueCR measurement) = 0;
    virtual bool _HandleForceHealImmediate(DgnViewportP vp, UpdatePlan& plan) { return false; }
};

END_BENTLEY_DGN_NAMESPACE

