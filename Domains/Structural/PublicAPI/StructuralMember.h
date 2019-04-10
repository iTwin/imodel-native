/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/StructuralMember.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralElement.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! StructuralMember
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StructuralMember : StructuralElement
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_StructuralMember, StructuralElement);
    
    friend struct StructuralMemberHandler;

protected:
    explicit StructuralMember(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(StructuralMember)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(StructuralMember)

    STRUCTURAL_DOMAIN_EXPORT static StructuralMemberPtr Create(Dgn::PhysicalModelCR model);
    STRUCTURAL_DOMAIN_EXPORT StructuralMemberCPtr Insert(Dgn::DgnDbStatus* insertStatus = nullptr);
    STRUCTURAL_DOMAIN_EXPORT StructuralMemberCPtr Update(Dgn::DgnDbStatus* updateStatus = nullptr);
#ifdef _EXCLUDED_FROM_EAP_BUILD_    
    STRUCTURAL_DOMAIN_EXPORT bool IsCurveMember();
    STRUCTURAL_DOMAIN_EXPORT bool IsSurfaceMember();
#endif
    };

//=======================================================================================
//! The ElementHandler for StructuralMemberHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StructuralMemberHandler : StructuralElementHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_StructuralMember, StructuralMember, StructuralMemberHandler, StructuralElementHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
