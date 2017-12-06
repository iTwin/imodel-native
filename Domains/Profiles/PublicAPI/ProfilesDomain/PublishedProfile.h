#pragma once
//__PUBLISH_SECTION_START__
#include "ConstantProfile.h"
#include "ProfilesDomainDefinitions.h"
#include "Profile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE PublishedProfile : ConstantProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PROFILES_CLASS_PublishedProfile, ConstantProfile);

    friend struct PublishedProfileHandler;

protected:
    explicit PublishedProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(PublishedProfile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS(PublishedProfile)

    PROFILES_DOMAIN_EXPORT static PublishedProfilePtr Create(Profiles::ProfilesModelCPtr model);
    };

//=======================================================================================
//! The ElementHandler for ConstantProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PublishedProfileHandler : ConstantProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PROFILES_CLASS_PublishedProfile, PublishedProfile, PublishedProfileHandler, ConstantProfileHandler, PROFILES_DOMAIN_EXPORT)
    };

END_BENTLEY_PROFILES_NAMESPACE