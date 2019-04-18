/*--------------------------------------------------------------------------------------+
|
|     $Source: Handlers/PublicApi/ElevationStoryHandler.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/ElementHandler.h>
#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include <BuildingSpatial/Elements/ElevationStory.h>

BEGIN_BUILDINGSPATIAL_NAMESPACE

struct ElevationStoryHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
    ELEMENTHANDLER_DECLARE_MEMBERS(BUILDINGSPATIAL_CLASS_ElevationStory, ElevationStory, ElevationStoryHandler, Dgn::dgn_ElementHandler::SpatialLocation, BUILDINGSPATIALHANDLERS_EXPORT)
};

END_BUILDINGSPATIAL_NAMESPACE
