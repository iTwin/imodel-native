/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/WallAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/WallAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(WallAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> WallAspectHandler::_CreateInstance()
    {
    return new WallAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE