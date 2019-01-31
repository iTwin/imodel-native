/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/CompositeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "Profile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//! @defgroup GROUP_CompositeProfiles Composite Profiles
//! TODO Karolis: Add description

//=======================================================================================
//! A Profile comprised of multiple SinglePerimeterProfiles.
//! @ingroup GROUP_CompositeProfiles
//=======================================================================================
struct CompositeProfile : Profile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CompositeProfile, Profile);
    friend struct CompositeProfileHandler;

protected:
    explicit CompositeProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CompositeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS (CompositeProfile)

    }; // CompositeProfile

//=======================================================================================
//! Handler for CompositeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CompositeProfileHandler : ProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS_ABSTRACT (PRF_CLASS_CompositeProfile, CompositeProfile, CompositeProfileHandler, ProfileHandler, PROFILES_EXPORT)

    }; // CompositeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
