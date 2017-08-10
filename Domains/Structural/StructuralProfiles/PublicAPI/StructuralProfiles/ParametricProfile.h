#pragma once
//__PUBLISH_SECTION_START__
#include "ConstantProfile.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE ParametricProfile : ConstantProfile
{
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_ParametricProfile, ConstantProfile);

    friend struct ParametricProfileHandler;

protected:
    explicit ParametricProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PROFILES_QUERYCLASS_METHODS(ParametricProfile)
    DECLARE_STRUCTURAL_PROFILES_ELEMENT_BASE_GET_METHODS(ParametricProfile)

    STRUCTURAL_DOMAIN_EXPORT static ParametricProfilePtr Create(Dgn::PhysicalModelR model);
};

//=======================================================================================
//! The ElementHandler for ConstantProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ParametricProfileHandler : ConstantProfileHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_ParametricProfile, ParametricProfile, ParametricProfileHandler, ConstantProfileHandler, STRUCTURAL_DOMAIN_EXPORT)
};

END_BENTLEY_STRUCTURAL_NAMESPACE