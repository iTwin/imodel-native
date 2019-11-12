/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/SurfaceAreaAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/SurfaceAreaAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(SurfaceAreaAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> SurfaceAreaAspectHandler::_CreateInstance()
    {
    return new SurfaceAreaAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE