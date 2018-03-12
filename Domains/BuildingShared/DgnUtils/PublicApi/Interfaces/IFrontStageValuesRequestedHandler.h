/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnUtils/PublicApi/Interfaces/IFrontStageValuesRequestedHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

struct IFrontStageValuesRequestedHandler
    {
    virtual void OnFrontStageValuesRequested(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response) = 0;
    };

END_BUILDING_SHARED_NAMESPACE