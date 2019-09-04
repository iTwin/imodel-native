/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/EnergyPerformanceAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/EnergyPerformanceAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(EnergyPerformanceAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> EnergyPerformanceAspectHandler::_CreateInstance()
    {
    return new EnergyPerformanceAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE