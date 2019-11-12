/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/DgnDomain.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct SurfaceAreaAspectHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_CLASS_SurfaceAreaAspect, SurfaceAreaAspectHandler, Dgn::dgn_AspectHandler::Aspect, QUANTITYTAKEOFFSASPECTS_EXPORT)
    
    public:
        virtual RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override;
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE