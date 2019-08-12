/*--------------------------------------------------------------------------------------+
|
|     $Source: Handlers/PublicApi/RegularStoryHandler.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/ElementHandler.h>
#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include <BuildingSpatial/Elements/RegularStory.h>

BEGIN_BUILDINGSPATIAL_NAMESPACE

struct RegularStoryHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
    ELEMENTHANDLER_DECLARE_MEMBERS(BUILDINGSPATIAL_CLASS_RegularStory, RegularStory, RegularStoryHandler, Dgn::dgn_ElementHandler::SpatialLocation, BUILDINGSPATIALHANDLERS_EXPORT)
};

END_BUILDINGSPATIAL_NAMESPACE
