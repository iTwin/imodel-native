/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PublicApi/GridPlaneSurfaceHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ViewController.h>
#include "GridSurfaceHandler.h"
#include <GridPlaneSurface.h>

BEGIN_GRIDS_NAMESPACE

struct GridPlaneSurfaceHandler : GridSurfaceHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_GridPlaneSurface, GridPlaneSurface, GridPlaneSurfaceHandler, GridSurfaceHandler, GRIDHANDLERS_EXPORT)
};

END_GRIDS_NAMESPACE