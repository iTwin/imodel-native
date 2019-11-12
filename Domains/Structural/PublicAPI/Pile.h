/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "FoundationMember.h"
#include "StructuralDomainApi.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! Pile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Pile : FoundationMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Pile, FoundationMember);
    
    friend struct PileHandler;

protected:
    explicit Pile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(Pile)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(Pile)

    STRUCTURAL_DOMAIN_EXPORT static PilePtr Create(Dgn::PhysicalModelCR model);
    };

//=======================================================================================
//! The ElementHandler for PileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PileHandler : FoundationMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Pile, Pile, PileHandler, FoundationMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
