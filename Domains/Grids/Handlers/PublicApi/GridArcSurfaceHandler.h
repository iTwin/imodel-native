/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PublicApi/GridArcSurfaceHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ViewController.h>
#include "GridSurfaceHandler.h"
#include <Grids/Elements/GridArcSurface.h>

BEGIN_GRIDS_NAMESPACE

struct GridArcSurfaceHandler : GridSurfaceHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_GridArcSurface, GridArcSurface, GridArcSurfaceHandler, GridSurfaceHandler, GRIDHANDLERS_EXPORT)
};

END_GRIDS_NAMESPACE