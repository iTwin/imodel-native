/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PublicApi/GridHandlers.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ViewController.h>
#include <Grids/gridsApi.h>

BEGIN_GRIDS_NAMESPACE

struct GridAxisHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_GridAxis, GridAxis, GridAxisHandler, Dgn::dgn_ElementHandler::Definition, GRIDHANDLERS_EXPORT)
    };

struct GridSplineSurfaceHandler : GridSurfaceHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_GridSplineSurface, GridSplineSurface, GridSplineSurfaceHandler, GridSurfaceHandler, GRIDHANDLERS_EXPORT)
    };


END_GRIDS_NAMESPACE