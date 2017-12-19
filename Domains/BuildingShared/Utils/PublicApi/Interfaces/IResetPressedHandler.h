/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/Interfaces/IResetPressedHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingShared/BuildingSharedMacros.h>
#include <DgnClientFx/Messages.h>

BEGIN_BUILDING_SHARED_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE IResetPressedHandler
    {
    BUILDINGSHAREDUNITS_EXPORT virtual void OnResetPressed(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response) = 0;
    };

END_BUILDING_SHARED_NAMESPACE