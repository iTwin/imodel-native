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

struct GridCurveHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_GridCurve, GridCurve, GridCurveHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

struct GridLineHandler : GridCurveHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_GridLine, GridLine, GridLineHandler, GridCurveHandler, GRIDHANDLERS_EXPORT)
    };

struct GridArcHandler : GridCurveHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_GridArc, GridArc, GridArcHandler, GridCurveHandler, GRIDHANDLERS_EXPORT)
    };

struct GridSplineHandler : GridCurveHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_GridSpline, GridSpline, GridSplineHandler, GridCurveHandler, GRIDHANDLERS_EXPORT)
    };

END_GRIDS_NAMESPACE