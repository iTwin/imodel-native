#pragma once

//__PUBLISH_SECTION_START__
#include "ConstantProfile.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE PublishedProfile : ConstantProfile
{
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_PublishedProfile, ConstantProfile);

    friend struct PublishedProfileHandler;

protected:
    explicit PublishedProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PROFILES_QUERYCLASS_METHODS(PublishedProfile)
    DECLARE_STRUCTURAL_PROFILES_ELEMENT_BASE_GET_METHODS(PublishedProfile)

    STRUCTURAL_DOMAIN_EXPORT static PublishedProfilePtr Create(Dgn::PhysicalModelR model);
};

//=======================================================================================
//! The ElementHandler for ConstantProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PublishedProfileHandler : ConstantProfileHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_PublishedProfile, PublishedProfile, PublishedProfileHandler, ConstantProfileHandler, STRUCTURAL_DOMAIN_EXPORT)
};

END_BENTLEY_STRUCTURAL_NAMESPACE