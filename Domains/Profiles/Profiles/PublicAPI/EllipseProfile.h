/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/EllipseProfile.h $
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
struct EllipseProfile : CenteredProfile, IEllipseProfile
{
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_EllipseProfile, CenteredProfile);
    friend struct EllipseProfileHandler;

protected:
    explicit EllipseProfile(CreateParams const& params) : T_Super(params) {}

protected:
    virtual Dgn::DgnElementR _IEllipseProfileToDgnElement() override { return *this; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(EllipseProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(EllipseProfile)

    PROFILES_EXPORT static EllipseProfilePtr Create(/*TODO: args*/);


}; // EllipseProfile

END_BENTLEY_PROFILES_NAMESPACE
