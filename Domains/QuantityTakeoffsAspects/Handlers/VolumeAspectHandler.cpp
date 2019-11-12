/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/VolumeAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/VolumeAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(VolumeAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> VolumeAspectHandler::_CreateInstance()
    {
    return new VolumeAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE