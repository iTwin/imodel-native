/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/SlabAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/SlabAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(SlabAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> SlabAspectHandler::_CreateInstance()
    {
    return new SlabAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE