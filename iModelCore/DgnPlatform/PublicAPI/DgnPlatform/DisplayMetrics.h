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

#if defined(TODO_ELEMENT_TILE)
//=======================================================================================
// @bsiclass                                                      John.Gooding    01/17
//=======================================================================================
struct DisplayMetricsRecorder
{
    DGNPLATFORM_EXPORT static void RecordQuerySceneComplete(double seconds, ViewController::QueryResults const& queryResults);
    DGNPLATFORM_EXPORT static void RecordCreateSceneComplete(double seconds, ViewController::Scene const & scene, bool aborted, bool complete);
};

//=======================================================================================
// @bsiclass                                                      John.Gooding    01/17
//=======================================================================================
struct IDisplayMetricsRecorder :  DgnHost::IHostObject
{
    DGNPLATFORM_EXPORT static bool IsRecorderActive();
    DGNPLATFORM_EXPORT static IDisplayMetricsRecorder* GetRecorder();
    DGNPLATFORM_EXPORT static void SetRecorder(IDisplayMetricsRecorder*logger);
    virtual bool _IsActive() const = 0;
    virtual void _RecordMeasurement(Utf8CP measurementType, JsonValueCR measurement) = 0;
};
#endif

END_BENTLEY_DGN_NAMESPACE

