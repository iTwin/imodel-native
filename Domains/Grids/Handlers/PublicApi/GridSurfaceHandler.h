/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PublicApi/GridSurfaceHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ViewController.h>
#include "DrivingSurfaceHandler.h"
#include <Grids/Elements/GridSurface.h>

BEGIN_GRIDS_NAMESPACE

struct GridSurfaceHandler : DrivingSurfaceHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_GridSurface, GridSurface, GridSurfaceHandler, DrivingSurfaceHandler, GRIDHANDLERS_EXPORT)
};

END_GRIDS_NAMESPACE