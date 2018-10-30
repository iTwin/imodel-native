/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/HollowEllipseProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CenteredProfile.h"
#include "ProfileMixins.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct HollowEllipseProfile : CenteredProfile, ICenterLineProfile, IEllipseProfile
{
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_HollowEllipseProfile, CenteredProfile);
    friend struct HollowEllipseProfileHandler;

protected:
    explicit HollowEllipseProfile(CreateParams const& params) : T_Super(params) {}

protected:
    virtual Dgn::DgnElementR _ICenterLineProfileToDgnElement() override { return *this; }
    virtual Dgn::DgnElementR _IEllipseProfileToDgnElement() override { return *this; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(HollowEllipseProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(HollowEllipseProfile)

    PROFILES_EXPORT static HollowEllipseProfilePtr Create(/*TODO: args*/);


}; // HollowEllipseProfile

END_BENTLEY_PROFILES_NAMESPACE
