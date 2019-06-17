/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/WindowAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/WindowAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(WindowAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> WindowAspectHandler::_CreateInstance()
    {
    return new WindowAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE