#pragma once

//__PUBLISH_SECTION_START__
#include "Profile.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE VaryingProfile : Profile
{
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_VaryingProfile, Profile);

    friend struct VaryingProfileHandler;

protected:
    explicit VaryingProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PROFILES_QUERYCLASS_METHODS(VaryingProfile)
    DECLARE_STRUCTURAL_PROFILES_ELEMENT_BASE_GET_METHODS(VaryingProfile)
};

//=======================================================================================
//! The ElementHandler for ConstantProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE VaryingProfileHandler : ProfileHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_VaryingProfile, VaryingProfile, VaryingProfileHandler, ProfileHandler, STRUCTURAL_DOMAIN_EXPORT)
};

END_BENTLEY_STRUCTURAL_NAMESPACE
