/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PublicApi/PortionHandlers.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <SurfaceSet.h>
#include <GridPortion.h>
#include <OrthogonalGridPortion.h>
#include <RadialGridPortion.h>
#include <SketchGridPortion.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ViewController.h>
#include <GridSurface.h>

BEGIN_GRIDS_NAMESPACE

struct SurfaceSetHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_SurfaceSet, SurfaceSet, SurfaceSetHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

struct GridPortionHandler : SurfaceSetHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_GridPortion, GridPortion, GridPortionHandler, SurfaceSetHandler, GRIDHANDLERS_EXPORT)

    GRIDHANDLERS_EXPORT static  BentleyStatus   IntersectGridSurface (GridPortionCPtr thisPortion, GridSurfaceCPtr surface, Dgn::DgnModelCR targetModel);
    };

struct OrthogonalGridPortionHandler : GridPortionHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_OrthogonalGridPortion, OrthogonalGridPortion, OrthogonalGridPortionHandler, GridPortionHandler, GRIDHANDLERS_EXPORT)
    };

struct RadialGridPortionHandler : GridPortionHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_RadialGridPortion, RadialGridPortion, RadialGridPortionHandler, GridPortionHandler, GRIDHANDLERS_EXPORT)
    };

struct SketchGridPortionHandler : GridPortionHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_SketchGridPortion, SketchGridPortion, SketchGridPortionHandler, GridPortionHandler, GRIDHANDLERS_EXPORT)
    };

END_GRIDS_NAMESPACE