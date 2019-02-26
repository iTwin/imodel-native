/*--------------------------------------------------------------------------------------+
|
|     $Source: Handlers/PublicApi/BuildingHandler.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDINGSPATIAL_NAMESPACE

struct BuildingHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
    ELEMENTHANDLER_DECLARE_MEMBERS(BUILDINGSPATIAL_CLASS_Building, Building, BuildingHandler, Dgn::dgn_ElementHandler::SpatialLocation, BUILDINGSPATIALHANDLERS_EXPORT)
};

END_BUILDINGSPATIAL_NAMESPACE
