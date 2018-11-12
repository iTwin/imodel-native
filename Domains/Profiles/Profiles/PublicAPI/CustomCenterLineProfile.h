/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CustomCenterLineProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "SinglePerimeterProfile.h"
#include "ProfileMixins.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CustomCenterLineProfile : SinglePerimeterProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CustomCenterLineProfile, SinglePerimeterProfile);
    friend struct CustomCenterLineProfileHandler;

protected:
    explicit CustomCenterLineProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CustomCenterLineProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(CustomCenterLineProfile)

    PROFILES_EXPORT static CustomCenterLineProfilePtr Create(/*TODO: args*/);

    }; // CustomCenterLineProfile

//=======================================================================================
//! Handler for CustomCenterLineProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CustomCenterLineProfileHandler : SinglePerimeterProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CustomCenterLineProfile, CustomCenterLineProfile, CustomCenterLineProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)

    }; // CustomCenterLineProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
