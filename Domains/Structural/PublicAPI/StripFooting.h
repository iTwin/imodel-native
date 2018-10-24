/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/StripFooting.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "FoundationMember.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL
BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! StripFooting
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StripFooting : FoundationMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_StripFooting, FoundationMember);
    
    friend struct StripFootingHandler;

protected:
    explicit StripFooting(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(StripFooting)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(StripFooting)

    STRUCTURAL_DOMAIN_EXPORT static StripFootingPtr Create(Dgn::PhysicalModelCR model);
    };

//=======================================================================================
//! The ElementHandler for StripFootingHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StripFootingHandler : FoundationMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_StripFooting, StripFooting, StripFootingHandler, FoundationMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
