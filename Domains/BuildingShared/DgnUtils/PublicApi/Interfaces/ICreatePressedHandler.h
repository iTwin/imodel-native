/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnUtils/PublicApi/Interfaces/ICreatePressedHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

struct ICreatePressedHandler
    {
    virtual void OnCreatePressed(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response) = 0;
    };

END_BUILDING_SHARED_NAMESPACE