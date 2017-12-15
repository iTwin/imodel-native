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

struct GridSplineSurfaceHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_GridSplineSurface, GridSplineSurface, GridSplineSurfaceHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

struct SketchSplineGridSurfaceHandler : GridArcSurfaceHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_SketchSplineGridSurface, SketchSplineGridSurface, SketchSplineGridSurfaceHandler, GridSplineSurfaceHandler, GRIDHANDLERS_EXPORT)
    };

END_GRIDS_NAMESPACE