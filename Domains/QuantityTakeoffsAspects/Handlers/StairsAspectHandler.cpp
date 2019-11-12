/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/StairsAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/StairsAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(StairsAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> StairsAspectHandler::_CreateInstance()
    {
    return new StairsAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE