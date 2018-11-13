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
struct ArbitraryCompositeProfile : CompositeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_ArbitraryCompositeProfile, CompositeProfile);
    friend struct ArbitraryCompositeProfileHandler;

protected:
    explicit ArbitraryCompositeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(ArbitraryCompositeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(ArbitraryCompositeProfile)

    PROFILES_EXPORT static ArbitraryCompositeProfilePtr Create(/*TODO: args*/);

    }; // ArbitraryCompositeProfile

//=======================================================================================
//! Handler for ArbitraryCompositeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArbitraryCompositeProfileHandler : CompositeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_ArbitraryCompositeProfile, ArbitraryCompositeProfile, ArbitraryCompositeProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)

    }; // ArbitraryCompositeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
