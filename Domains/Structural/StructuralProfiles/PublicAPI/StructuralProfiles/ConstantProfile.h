#pragma once

//__PUBLISH_SECTION_START__
#include "Profile.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE ConstantProfile : Profile
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_ConstantProfile, Profile);

    friend struct ConstantProfileHandler;

protected:
    explicit ConstantProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PROFILES_QUERYCLASS_METHODS(ConstantProfile)
    DECLARE_STRUCTURAL_PROFILES_ELEMENT_BASE_GET_METHODS(ConstantProfile)

    STRUCTURAL_DOMAIN_EXPORT static ConstantProfilePtr Create(Dgn::PhysicalModelR model);
    };

//=======================================================================================
//! The ElementHandler for ConstantProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ConstantProfileHandler : ProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_ConstantProfile, ConstantProfile, ConstantProfileHandler, ProfileHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
