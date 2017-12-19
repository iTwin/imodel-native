/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/Interfaces/IFrontStageValuesChangedHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ConstraintSystem/Domain/ConstraintModelMacros.h>
#include <DgnClientFx/Messages.h>

BEGIN_BUILDING_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE IFrontStageValuesChangedHandler
    {
    BUILDINGUTILS_EXPORT virtual void OnFrontStageValuesChanged(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response) = 0;
    };

END_BUILDING_NAMESPACE