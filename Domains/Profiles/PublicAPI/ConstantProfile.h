#pragma once

//__PUBLISH_SECTION_START__
#include "ConstantProfile.h"
#include "ProfilesDomainDefinitions.h"
#include "Profile.h"
#include "ProfilesModel.h"


BEGIN_BENTLEY_PROFILES_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE ConstantProfile : Profile
    {
    DGNELEMENT_DECLARE_MEMBERS(PROFILES_CLASS_ConstantProfile, Profile);

    friend struct ConstantProfileHandler;

protected:
    explicit ConstantProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(ConstantProfile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS(ConstantProfile)
    PROFILES_DOMAIN_EXPORT static ConstantProfilePtr Create(ProfilesModelCPtr model);
    };

//=======================================================================================
//! The ElementHandler for ConstantProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ConstantProfileHandler : ProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PROFILES_CLASS_ConstantProfile, ConstantProfile, ConstantProfileHandler, ProfileHandler, PROFILES_DOMAIN_EXPORT)
    };

END_BENTLEY_PROFILES_NAMESPACE
