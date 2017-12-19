/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/Interfaces/ISiteUpdatedHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingShared/BuildingSharedMacros.h>
#include <DgnPlatform\DgnDb.h>
#include "IElementChangedHandler.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct ISiteUpdatedHandler : IElementChangedHandler
    {
    };

END_BUILDING_SHARED_NAMESPACE