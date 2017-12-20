#pragma once
//__PUBLISH_SECTION_START__
#include "ConstantProfile.h"
#include "ProfilesDomainDefinitions.h"
#include "Profile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE ParametricProfile : ConstantProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PROFILES_CLASS_ParametricProfile, ConstantProfile);

    friend struct ParametricProfileHandler;

protected:
    explicit ParametricProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(ParametricProfile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS(ParametricProfile)

    PROFILES_DOMAIN_EXPORT static ParametricProfilePtr Create(Profiles::ProfileDefinitionModelCPtr model);
    };

//=======================================================================================
//! The ElementHandler for ConstantProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ParametricProfileHandler : ConstantProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PROFILES_CLASS_ParametricProfile, ParametricProfile, ParametricProfileHandler, ConstantProfileHandler, PROFILES_DOMAIN_EXPORT)
    };

END_BENTLEY_PROFILES_NAMESPACE