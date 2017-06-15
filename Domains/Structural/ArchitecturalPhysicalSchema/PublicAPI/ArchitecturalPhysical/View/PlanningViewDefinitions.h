/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/View/PlanningViewDefinitions.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Planning/PlanningApi.h>
#include <DgnView/ViewManager.h>

#ifdef __PLANNING_VIEW_BUILD__
    #define PLANNING_VIEW_EXPORT EXPORT_ATTRIBUTE
#else
    #define PLANNING_VIEW_EXPORT IMPORT_ATTRIBUTE
#endif

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
PLANNING_TYPEDEFS(SchedulePlayer)
PLANNING_TYPEDEFS(ScheduleChannel)
PLANNING_TYPEDEFS(ScheduleViewController)
PLANNING_REFCOUNTED_TYPEDEFS(ScheduleViewController)
PLANNING_REFCOUNTED_TYPEDEFS(SchedulePlayer)
