/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PublicApi/GridHandlers.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

struct GridCurvesSetHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_GridCurvesSet, GridCurvesSet, GridCurvesSetHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

struct OrthogonalAxisXHandler : Dgn::dgn_ElementHandler::GroupInformation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_OrthogonalAxisX, OrthogonalAxisX, OrthogonalAxisXHandler, Dgn::dgn_ElementHandler::GroupInformation, GRIDHANDLERS_EXPORT)
    };

struct OrthogonalAxisYHandler : Dgn::dgn_ElementHandler::GroupInformation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_OrthogonalAxisY, OrthogonalAxisY, OrthogonalAxisYHandler, Dgn::dgn_ElementHandler::GroupInformation, GRIDHANDLERS_EXPORT)
    };

struct CircularAxisHandler : Dgn::dgn_ElementHandler::GroupInformation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_CircularAxis, CircularAxis, CircularAxisHandler, Dgn::dgn_ElementHandler::GroupInformation, GRIDHANDLERS_EXPORT)
    };

struct RadialAxisHandler : Dgn::dgn_ElementHandler::GroupInformation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_RadialAxis, RadialAxis, RadialAxisHandler, Dgn::dgn_ElementHandler::GroupInformation, GRIDHANDLERS_EXPORT)
    };

struct GeneralGridAxisHandler : Dgn::dgn_ElementHandler::GroupInformation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_GeneralGridAxis, GeneralGridAxis, GeneralGridAxisHandler, Dgn::dgn_ElementHandler::GroupInformation, GRIDHANDLERS_EXPORT)
    };

struct SketchSplineGridSurfaceHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_SketchSplineGridSurface, SketchSplineGridSurface, SketchSplineGridSurfaceHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
    };

END_GRIDS_NAMESPACE