/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/PublicAPI/StructuralPhysical/CurveMember.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralMember.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL_PHYSICAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! CurveMember
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CurveMember : StructuralMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_CurveMember, StructuralMember);
    
    friend struct CurveMemberHandler;

protected:
    explicit CurveMember(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(CurveMember)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(CurveMember)

    STRUCTURAL_DOMAIN_EXPORT static CurveMemberPtr Create(Dgn::PhysicalModelR model);
    };

//=======================================================================================
//! The ElementHandler for CurveMemberHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CurveMemberHandler : StructuralMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_CurveMember, CurveMember, CurveMemberHandler, StructuralMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
