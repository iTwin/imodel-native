/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/PipeAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/PipeAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(PipeAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> PipeAspectHandler::_CreateInstance()
    {
    return new PipeAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE