/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/DoubleLProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CompositeProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A CompositeProfile comprised of back-to-back Ls with the horizontal legs at the top of the Profile.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct DoubleLProfile : CompositeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_DoubleLProfile, CompositeProfile);
    friend struct DoubleLProfileHandler;

protected:
    explicit DoubleLProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(DoubleLProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(DoubleLProfile)

    PROFILES_EXPORT static DoubleLProfilePtr Create(/*TODO: args*/);

public:
    double GetSpacing() const { return GetPropertyValueDouble(PRF_PROP_DoubleLProfile_Spacing); }
    PROFILES_EXPORT void SetSpacing(double val);
    int GetEnum() const { return GetPropertyValueInt32(PRF_PROP_DoubleLProfile_Enum); }
    PROFILES_EXPORT void SetEnum(int val);

    }; // DoubleLProfile

//=======================================================================================
//! Handler for DoubleLProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoubleLProfileHandler : CompositeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_DoubleLProfile, DoubleLProfile, DoubleLProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)

    }; // DoubleLProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
