/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/FoundationAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/FoundationAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(FoundationAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> FoundationAspectHandler::_CreateInstance()
    {
    return new FoundationAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE