/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "FoundationMember.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL
BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! SpreadFooting
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SpreadFooting : FoundationMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_SpreadFooting, FoundationMember);
    
    friend struct SpreadFootingHandler;

protected:
    explicit SpreadFooting(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(SpreadFooting)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(SpreadFooting)

    STRUCTURAL_DOMAIN_EXPORT static SpreadFootingPtr Create(Dgn::PhysicalModelCR model);
    };

//=======================================================================================
//! The ElementHandler for SpreadFootingHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SpreadFootingHandler : FoundationMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_SpreadFooting, SpreadFooting, SpreadFootingHandler, FoundationMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
