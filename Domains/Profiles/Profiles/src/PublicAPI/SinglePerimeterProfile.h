/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/SinglePerimeterProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "Profile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A Profile with a single outer perimiter.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct SinglePerimeterProfile : Profile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_SinglePerimeterProfile, Profile);
    friend struct SinglePerimeterProfileHandler;

protected:
    explicit SinglePerimeterProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (SinglePerimeterProfile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS (SinglePerimeterProfile)

    }; // SinglePerimeterProfile

//=======================================================================================
//! Handler for SinglePerimeterProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SinglePerimeterProfileHandler : ProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS_ABSTRACT (PRF_CLASS_SinglePerimeterProfile, SinglePerimeterProfile, SinglePerimeterProfileHandler, ProfileHandler, PROFILES_EXPORT)

    }; // SinglePerimeterProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
