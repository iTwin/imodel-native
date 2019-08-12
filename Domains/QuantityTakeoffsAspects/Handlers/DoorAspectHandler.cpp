/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/DoorAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/DoorAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(DoorAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> DoorAspectHandler::_CreateInstance()
    {
    return new DoorAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE