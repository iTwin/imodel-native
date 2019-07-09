/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/WallAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/WallAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(WallAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> WallAspectHandler::_CreateInstance()
    {
    return new WallAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE