/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#define _structuralelement_included_
//__PUBLISH_SECTION_START__
#include "StructuralPhysicalDefinitions.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! StructuralElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StructuralElement : Dgn::PhysicalElement
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_StructuralElement, Dgn::PhysicalElement);
    
    friend struct StructuralElementHandler;

protected:
    explicit StructuralElement(CreateParams const& params) : T_Super(params) {}

    Dgn::DgnDbStatus _OnDelete() const override { return Dgn::DgnDbStatus::Success; }

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(StructuralElement)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(StructuralElement)
    };

//=======================================================================================
//! The ElementHandler for StructuralElementHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StructuralElementHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_StructuralElement, StructuralElement, StructuralElementHandler, Dgn::dgn_ElementHandler::Physical, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
