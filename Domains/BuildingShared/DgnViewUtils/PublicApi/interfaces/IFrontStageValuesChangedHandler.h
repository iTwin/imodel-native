/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnViewUtils/PublicApi/interfaces/IFrontStageValuesChangedHandler.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

struct IFrontStageValuesChangedHandler
    {
    virtual void OnFrontStageValuesChanged(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response) = 0;
    };

END_BUILDING_SHARED_NAMESPACE