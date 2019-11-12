/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include "StructuralMember.h"
#include "StructuralDomainApi.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL
BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! Wall
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Wall : StructuralMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Wall, StructuralMember);
    
    friend struct WallHandler;

protected:
    explicit Wall(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(Wall)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(Wall)

    STRUCTURAL_DOMAIN_EXPORT static WallPtr Create(Dgn::PhysicalModelCR model);

    STRUCTURAL_DOMAIN_EXPORT WallCPtr Insert(Dgn::DgnDbStatus* insertStatus = nullptr);
    STRUCTURAL_DOMAIN_EXPORT WallCPtr Update(Dgn::DgnDbStatus* updateStatus = nullptr);
    };

//=======================================================================================
//! The ElementHandler for WallHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WallHandler : StructuralMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Wall, Wall, WallHandler, StructuralMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
