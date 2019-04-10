/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Column.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralMember.h"
#include "StructuralDomainApi.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL
BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! Column
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Column : StructuralMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Column, StructuralMember);
    
    friend struct ColumnHandler;

protected:
    explicit Column(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(Column)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(Column)

    STRUCTURAL_DOMAIN_EXPORT static ColumnPtr Create(Dgn::PhysicalModelCR model);
    };

//=======================================================================================
//! The ElementHandler for ColumnHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ColumnHandler : StructuralMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Column, Column, ColumnHandler, StructuralMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
