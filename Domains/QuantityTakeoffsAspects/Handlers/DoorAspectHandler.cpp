/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/DoorAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/DoorAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(DoorAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> DoorAspectHandler::_CreateInstance()
    {
    return new DoorAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE