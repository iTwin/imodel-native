#pragma once
//__PUBLISH_SECTION_START__
#include "ConstantProfile.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE BuiltUpProfile : ConstantProfile
{
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_BuiltUpProfile, ConstantProfile);

    friend struct BuiltUpProfileHandler;

protected:
    explicit BuiltUpProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PROFILES_QUERYCLASS_METHODS(BuiltUpProfile)
    DECLARE_STRUCTURAL_PROFILES_ELEMENT_BASE_GET_METHODS(BuiltUpProfile)

    STRUCTURAL_DOMAIN_EXPORT static BuiltUpProfilePtr Create(Dgn::PhysicalModelR model);
};

//=======================================================================================
//! The ElementHandler for ConstantProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BuiltUpProfileHandler : ConstantProfileHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_BuiltUpProfile, BuiltUpProfile, BuiltUpProfileHandler, ConstantProfileHandler, STRUCTURAL_DOMAIN_EXPORT)
};

END_BENTLEY_STRUCTURAL_NAMESPACE