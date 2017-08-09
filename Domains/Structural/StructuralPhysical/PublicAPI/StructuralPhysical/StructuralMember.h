/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/PublicAPI/StructuralPhysical/StructuralMember.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralElement.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL_PHYSICAL

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
    };

//=======================================================================================
//! The ElementHandler for StructuralMemberHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StructuralMemberHandler : StructuralElementHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_StructuralMember, StructuralMember, StructuralMemberHandler, StructuralElementHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
