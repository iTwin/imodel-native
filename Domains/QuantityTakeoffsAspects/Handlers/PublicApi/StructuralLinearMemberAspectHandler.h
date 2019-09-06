/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/DgnDomain.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct StructuralLinearMemberAspectHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_CLASS_StructuralLinearMemberAspect, StructuralLinearMemberAspectHandler, Dgn::dgn_AspectHandler::Aspect, QUANTITYTAKEOFFSASPECTS_EXPORT)
    
    public:
        virtual RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override;
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE