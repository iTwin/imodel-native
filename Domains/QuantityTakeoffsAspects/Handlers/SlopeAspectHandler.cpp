/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/SlopeAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/SlopeAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(SlopeAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> SlopeAspectHandler::_CreateInstance()
    {
    return new SlopeAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE