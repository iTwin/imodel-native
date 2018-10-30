/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CenteredProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "SinglePerimeterProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A SinglePerimeterProfile that is guaranteed to have the center of the bounding box at (0, 0).
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CenteredProfile : SinglePerimeterProfile
{
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CenteredProfile, SinglePerimeterProfile);
    friend struct CenteredProfileHandler;

protected:
    explicit CenteredProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CenteredProfile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS(CenteredProfile)


}; // CenteredProfile

END_BENTLEY_PROFILES_NAMESPACE
