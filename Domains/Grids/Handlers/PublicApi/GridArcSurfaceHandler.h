/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PublicApi/GridArcSurfaceHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ViewController.h>
#include <Grids/gridsApi.h>

BEGIN_GRIDS_NAMESPACE

struct SketchArcGridSurfaceHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_SketchArcGridSurface, SketchArcGridSurface, SketchArcGridSurfaceHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

struct PlanCircumferentialGridSurfaceHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_PlanCircumferentialGridSurface, PlanCircumferentialGridSurface, PlanCircumferentialGridSurfaceHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

END_GRIDS_NAMESPACE