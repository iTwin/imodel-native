/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/SlabAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/SlabAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(SlabAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> SlabAspectHandler::_CreateInstance()
    {
    return new SlabAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE