/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/PublicAPI/StructuralPhysical/Column.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "CurveMember.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! Column
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Column : CurveMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Column, CurveMember);
    
    friend struct ColumnHandler;

protected:
    explicit Column(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(Column)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(Column)

    STRUCTURAL_DOMAIN_EXPORT static ColumnPtr Create(Dgn::PhysicalModelR model);
    };

//=======================================================================================
//! The ElementHandler for ColumnHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ColumnHandler : CurveMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Column, Column, ColumnHandler, CurveMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
