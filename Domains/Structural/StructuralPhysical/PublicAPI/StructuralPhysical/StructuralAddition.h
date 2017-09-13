/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/PublicAPI/StructuralPhysical/StructuralAddition.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralElement.h"
#include <StructuralDomain/StructuralDomainApi.h>
#include "StructuralPhysicalDefinitions.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! StructuralMember
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StructuralAddition : StructuralElement
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_StructuralAddition, StructuralElement);
    
    friend struct StructuralAdditionHandler;

protected:
    explicit StructuralAddition(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(StructuralAddition)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(StructuralAddition)

    STRUCTURAL_DOMAIN_EXPORT static StructuralAdditionPtr Create(Structural::StructuralPhysicalModelCPtr model);
    STRUCTURAL_DOMAIN_EXPORT StructuralAdditionCPtr Insert(Dgn::DgnDbStatus* insertStatus = nullptr);
    STRUCTURAL_DOMAIN_EXPORT StructuralAdditionCPtr Update(Dgn::DgnDbStatus* updateStatus = nullptr);
    };

//=======================================================================================
//! The ElementHandler for StructuralMemberHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StructuralAdditionHandler : StructuralElementHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_StructuralAddition, StructuralAddition, StructuralAdditionHandler, StructuralElementHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
