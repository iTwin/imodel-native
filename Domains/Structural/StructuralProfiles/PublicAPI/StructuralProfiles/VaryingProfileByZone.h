#pragma once

//__PUBLISH_SECTION_START__
#include "VaryingProfile.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE VaryingProfileByZone : VaryingProfile
{
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_VaryingProfileByZone, VaryingProfile);

    friend struct VaryingProfileByZoneHandler;

protected:
    explicit VaryingProfileByZone(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PROFILES_QUERYCLASS_METHODS(VaryingProfileByZone)
    DECLARE_STRUCTURAL_PROFILES_ELEMENT_BASE_GET_METHODS(VaryingProfileByZone)

    STRUCTURAL_DOMAIN_EXPORT static VaryingProfileByZonePtr Create(Dgn::PhysicalModelR model);
};

//=======================================================================================
//! The ElementHandler for ConstantProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE VaryingProfileByZoneHandler : VaryingProfileHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_VaryingProfileByZone, VaryingProfileByZone, VaryingProfileByZoneHandler, VaryingProfileHandler, STRUCTURAL_DOMAIN_EXPORT)
};

END_BENTLEY_STRUCTURAL_NAMESPACE
