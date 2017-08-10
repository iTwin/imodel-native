/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/PublicAPI/StructuralPhysical/SurfaceMember.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralMember.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! SurfaceMember
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SurfaceMember : StructuralMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_SurfaceMember, StructuralMember);
    
    friend struct SurfaceMemberHandler;

protected:
    explicit SurfaceMember(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(SurfaceMember)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(SurfaceMember)

    STRUCTURAL_DOMAIN_EXPORT static SurfaceMemberPtr Create(Dgn::PhysicalModelR model);
    };

//=======================================================================================
//! The ElementHandler for SurfaceMemberHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SurfaceMemberHandler : StructuralMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_SurfaceMember, SurfaceMember, SurfaceMemberHandler, StructuralMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
