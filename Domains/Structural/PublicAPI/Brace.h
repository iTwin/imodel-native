/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Brace.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralMember.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL
BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! Brace
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Brace : StructuralMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Brace, StructuralMember);
    
    friend struct BraceHandler;

protected:
    explicit Brace(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(Brace)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(Brace)

    STRUCTURAL_DOMAIN_EXPORT static BracePtr Create(Dgn::PhysicalModelCR model);
    };

//=======================================================================================
//! The ElementHandler for BraceHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BraceHandler : StructuralMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Brace, Brace, BraceHandler, StructuralMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
