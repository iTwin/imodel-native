/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/DimensionsAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/DimensionsAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(DimensionsAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> DimensionsAspectHandler::_CreateInstance()
    {
    return new DimensionsAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE