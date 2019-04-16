/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralMember.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL
BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! Slab
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Slab : StructuralMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Slab, StructuralMember);
    
    friend struct SlabHandler;

protected:
    explicit Slab(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(Slab)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(Slab)

    STRUCTURAL_DOMAIN_EXPORT static SlabPtr Create(Dgn::PhysicalModelCR model);
    };

//=======================================================================================
//! The ElementHandler for SlabHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SlabHandler : StructuralMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Slab, Slab, SlabHandler, StructuralMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
