/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/Interfaces/ICreatePressedHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingMacros.h>
#include <DgnClientFx/Messages.h>

BEGIN_BUILDING_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE ICreatePressedHandler
    {

    BUILDINGUTILS_EXPORT virtual void OnCreatePressed(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response) = 0;
    };

END_BUILDING_NAMESPACE