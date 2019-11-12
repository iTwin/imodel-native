/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/EnergyPerformanceAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/EnergyPerformanceAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(EnergyPerformanceAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> EnergyPerformanceAspectHandler::_CreateInstance()
    {
    return new EnergyPerformanceAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE