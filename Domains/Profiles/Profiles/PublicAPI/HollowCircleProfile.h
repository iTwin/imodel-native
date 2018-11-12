/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/HollowCircleProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CenteredProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct HollowCircleProfile : CenteredProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_HollowCircleProfile, CenteredProfile);
    friend struct HollowCircleProfileHandler;

protected:
    explicit HollowCircleProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(HollowCircleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(HollowCircleProfile)

    PROFILES_EXPORT static HollowCircleProfilePtr Create(Dgn::DgnModelCR model);

    PROFILES_EXPORT double GetRadius() const;
    PROFILES_EXPORT void SetRadius(double val);

    }; // HollowCircleProfile

//=======================================================================================
//! Handler for HollowCircleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HollowCircleProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_HollowCircleProfile, HollowCircleProfile, HollowCircleProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // HollowCircleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
