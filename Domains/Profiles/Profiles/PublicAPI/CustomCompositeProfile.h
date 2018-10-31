/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CustomCompositeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CompositeProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A Profile comprised of multiple SinglePerimeterProfiles.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CustomCompositeProfile : CompositeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CustomCompositeProfile, CompositeProfile);
    friend struct CustomCompositeProfileHandler;

protected:
    explicit CustomCompositeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CustomCompositeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(CustomCompositeProfile)

    PROFILES_EXPORT static CustomCompositeProfilePtr Create(/*TODO: args*/);

    }; // CustomCompositeProfile

//=======================================================================================
//! Handler for CustomCompositeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CustomCompositeProfileHandler : CompositeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CustomCompositeProfile, CustomCompositeProfile, CustomCompositeProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)

    }; // CustomCompositeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
