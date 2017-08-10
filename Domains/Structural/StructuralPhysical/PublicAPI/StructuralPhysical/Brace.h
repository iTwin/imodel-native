/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/PublicAPI/StructuralPhysical/Brace.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "CurveMember.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL_PHYSICAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! Brace
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Brace : CurveMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Brace, CurveMember);
    
    friend struct BraceHandler;

protected:
    explicit Brace(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(Brace)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(Brace)

    STRUCTURAL_DOMAIN_EXPORT static BracePtr Create(Dgn::PhysicalModelR model);
    };

//=======================================================================================
//! The ElementHandler for BraceHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BraceHandler : CurveMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Brace, Brace, BraceHandler, CurveMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
