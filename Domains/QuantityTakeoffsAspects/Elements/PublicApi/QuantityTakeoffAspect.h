/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include <DgnPlatform/DgnPlatformApi.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct QuantityTakeoffAspect : Dgn::DgnElement::UniqueAspect
    {
    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT QuantityTakeoffAspect();
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
