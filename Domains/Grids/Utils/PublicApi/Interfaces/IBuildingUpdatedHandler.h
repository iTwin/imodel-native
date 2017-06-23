/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/Interfaces/IBuildingUpdatedHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingMacros.h>
#include <DgnPlatform\DgnDb.h>
#include "IElementChangedHandler.h"


BEGIN_BUILDING_NAMESPACE

struct IBuildingUpdatedHandler : IElementChangedHandler
    {
    };

END_BUILDING_NAMESPACE