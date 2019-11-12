/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/ThicknessAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/ThicknessAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(ThicknessAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> ThicknessAspectHandler::_CreateInstance()
    {
    return new ThicknessAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE