/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/Interfaces/IFrontStageValuesRequestedHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingShared/BuildingSharedMacros.h>
#include <DgnClientFx/Messages.h>

BEGIN_BUILDING_SHARED_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE IFrontStageValuesRequestedHandler
    {
    BUILDINGSHAREDUNITS_EXPORT virtual void OnFrontStageValuesRequested(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response) = 0;
    };

END_BUILDING_SHARED_NAMESPACE