/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PublicApi/IntersectionCurveHandlers.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Grids/gridsApi.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE

struct GridLineHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_GridLine, GridLine, GridLineHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

struct GridArcHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_GridArc, GridArc, GridArcHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

struct GridSplineHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_GridSpline, GridSpline, GridSplineHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

END_GRIDS_NAMESPACE