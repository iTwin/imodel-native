/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/Interfaces/IToolSettingsChangedHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ConstraintSystem/Domain/ConstraintModelMacros.h>
#include <DgnClientFx/Messages.h>

BEGIN_BUILDING_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE IToolSettingsChangedHandler {

	BUILDINGUTILS_EXPORT virtual void OnToolSettingsChanged(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response) = 0;
};

END_BUILDING_NAMESPACE