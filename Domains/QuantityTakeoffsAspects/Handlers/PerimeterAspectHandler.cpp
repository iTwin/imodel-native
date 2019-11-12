/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/PerimeterAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/PerimeterAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(PerimeterAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> PerimeterAspectHandler::_CreateInstance()
    {
    return new PerimeterAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE