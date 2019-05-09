/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/ElementHandler.h>
#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include <BuildingSpatial/Elements/Space.h>

BEGIN_BUILDINGSPATIAL_NAMESPACE

struct SpaceHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
    ELEMENTHANDLER_DECLARE_MEMBERS(BUILDINGSPATIAL_CLASS_Space, Space, SpaceHandler, Dgn::dgn_ElementHandler::SpatialLocation, BUILDINGSPATIALHANDLERS_EXPORT)
};

END_BUILDINGSPATIAL_NAMESPACE
