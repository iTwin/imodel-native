/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnViewUtils/PublicApi/interfaces/IResetPressedHandler.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

struct IResetPressedHandler
    {
    virtual void OnResetPressed(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response) = 0;
    };

END_BUILDING_SHARED_NAMESPACE