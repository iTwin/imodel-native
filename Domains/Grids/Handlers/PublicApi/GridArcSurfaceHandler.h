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
#include <Grids/gridsApi.h>

BEGIN_GRIDS_NAMESPACE

struct GridArcSurfaceHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_GridArcSurface, GridArcSurface, GridArcSurfaceHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
};

struct SketchArcGridSurfaceHandler : GridArcSurfaceHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_SketchArcGridSurface, SketchArcGridSurface, SketchArcGridSurfaceHandler, GridArcSurfaceHandler, GRIDHANDLERS_EXPORT)
    };

END_GRIDS_NAMESPACE