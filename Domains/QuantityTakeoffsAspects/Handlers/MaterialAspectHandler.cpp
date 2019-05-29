/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/MaterialAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/MaterialAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(MaterialAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> MaterialAspectHandler::_CreateInstance()
    {
    return new MaterialAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE