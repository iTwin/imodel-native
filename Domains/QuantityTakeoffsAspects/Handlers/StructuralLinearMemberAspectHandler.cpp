/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/StructuralLinearMemberAspectHandler.h"
#include <QuantityTakeoffsAspects/Elements/StructuralLinearMemberAspect.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

HANDLER_DEFINE_MEMBERS(StructuralLinearMemberAspectHandler)

RefCountedPtr<Dgn::DgnElement::Aspect> StructuralLinearMemberAspectHandler::_CreateInstance()
    {
    return new StructuralLinearMemberAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE