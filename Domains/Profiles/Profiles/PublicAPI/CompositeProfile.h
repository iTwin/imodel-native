/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CompositeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "Profile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A Profile comprised of multiple SinglePerimeterProfiles.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CompositeProfile : Profile
{
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CompositeProfile, Profile);
    friend struct CompositeProfileHandler;

protected:
    explicit CompositeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CompositeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS(CompositeProfile)


}; // CompositeProfile

END_BENTLEY_PROFILES_NAMESPACE
