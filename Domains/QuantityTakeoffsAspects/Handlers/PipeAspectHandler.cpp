/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/PipeAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/PipeAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(PipeAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> PipeAspectHandler::_CreateInstance()
    {
    return new PipeAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE