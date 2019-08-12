/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/PileAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/PileAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(PileAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> PileAspectHandler::_CreateInstance()
    {
    return new PileAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE