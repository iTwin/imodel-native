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
#include <Grids/gridsApi.h>

BEGIN_GRIDS_NAMESPACE

struct GridPlanarSurfaceHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_GridPlanarSurface, GridPlanarSurface, GridPlanarSurfaceHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
};

struct PlanCartesianGridSurfaceHandler : GridPlanarSurfaceHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_PlanCartesianGridSurface, PlanCartesianGridSurface, PlanCartesianGridSurfaceHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

END_GRIDS_NAMESPACE