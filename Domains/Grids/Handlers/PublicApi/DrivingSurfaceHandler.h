/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PublicApi/DrivingSurfaceHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

struct DrivingSurfaceHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_DrivingSurface, DrivingSurface, DrivingSurfaceHandler, Dgn::dgn_ElementHandler::SpatialLocation, GRIDHANDLERS_EXPORT)
};

END_GRIDS_NAMESPACE