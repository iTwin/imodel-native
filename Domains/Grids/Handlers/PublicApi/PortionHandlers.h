/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PublicApi/PortionHandlers.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Grids/gridsApi.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE

struct OrthogonalGridPortionHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_OrthogonalGridPortion, OrthogonalGridPortion, OrthogonalGridPortionHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

struct RadialGridPortionHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_RadialGridPortion, RadialGridPortion, RadialGridPortionHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

struct SketchGridPortionHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_SketchGridPortion, SketchGridPortion, SketchGridPortionHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

END_GRIDS_NAMESPACE