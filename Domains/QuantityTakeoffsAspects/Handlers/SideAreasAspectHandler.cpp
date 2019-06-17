/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/SideAreasAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/SideAreasAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(SideAreasAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> SideAreasAspectHandler::_CreateInstance()
    {
    return new SideAreasAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE