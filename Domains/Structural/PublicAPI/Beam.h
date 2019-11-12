/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralMember.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL
BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! Beam
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Beam : StructuralMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Beam, StructuralMember);
    
    friend struct BeamHandler;

protected:
    explicit Beam(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(Beam)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(Beam)

    STRUCTURAL_DOMAIN_EXPORT static BeamPtr Create(Dgn::PhysicalModelCR model);
    };

//=======================================================================================
//! The ElementHandler for BeamHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BeamHandler : StructuralMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Beam, Beam, BeamHandler, StructuralMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
