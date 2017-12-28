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

struct OrthogonalAxisXHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_OrthogonalAxisX, OrthogonalAxisX, OrthogonalAxisXHandler, Dgn::dgn_ElementHandler::Definition, GRIDHANDLERS_EXPORT)
    };

struct OrthogonalAxisYHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_OrthogonalAxisY, OrthogonalAxisY, OrthogonalAxisYHandler, Dgn::dgn_ElementHandler::Definition, GRIDHANDLERS_EXPORT)
    };

struct CircularAxisHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_CircularAxis, CircularAxis, CircularAxisHandler, Dgn::dgn_ElementHandler::Definition, GRIDHANDLERS_EXPORT)
    };

struct RadialAxisHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_RadialAxis, RadialAxis, RadialAxisHandler, Dgn::dgn_ElementHandler::Definition, GRIDHANDLERS_EXPORT)
    };

struct GeneralGridAxisHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_GeneralGridAxis, GeneralGridAxis, GeneralGridAxisHandler, Dgn::dgn_ElementHandler::Definition, GRIDHANDLERS_EXPORT)
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