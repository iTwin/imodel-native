/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CapsuleProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CenteredProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CapsuleProfile : CenteredProfile
{
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CapsuleProfile, CenteredProfile);
    friend struct CapsuleProfileHandler;

protected:
    explicit CapsuleProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CapsuleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(CapsuleProfile)

    PROFILES_EXPORT static CapsuleProfilePtr Create(/*TODO: args*/);


}; // CapsuleProfile

END_BENTLEY_PROFILES_NAMESPACE
