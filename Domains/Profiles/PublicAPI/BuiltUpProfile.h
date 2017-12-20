#pragma once
//__PUBLISH_SECTION_START__
#include "ConstantProfile.h"
#include "ProfilesDomainDefinitions.h"
#include "Profile.h"
#include "ProfilesModel.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE BuiltUpProfile : ConstantProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PROFILES_CLASS_BuiltUpProfile, ConstantProfile);

    friend struct BuiltUpProfileHandler;

protected:
    explicit BuiltUpProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(BuiltUpProfile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS(BuiltUpProfile)
    PROFILES_DOMAIN_EXPORT static BuiltUpProfilePtr Create(Profiles::ProfileDefinitionModelCPtr model);
    };

//=======================================================================================
//! The ElementHandler for ConstantProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BuiltUpProfileHandler : ConstantProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PROFILES_CLASS_BuiltUpProfile, BuiltUpProfile, BuiltUpProfileHandler, ConstantProfileHandler, PROFILES_DOMAIN_EXPORT)
    };

END_BENTLEY_PROFILES_NAMESPACE