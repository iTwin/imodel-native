/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/ParametricProfile.h $
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
struct ParametricProfile : SinglePerimeterProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_ParametricProfile, SinglePerimeterProfile);
    friend struct ParametricProfileHandler;

protected:
    explicit ParametricProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ParametricProfile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS (ParametricProfile)

    }; // ParametricProfile

//=======================================================================================
//! Handler for ParametricProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ParametricProfileHandler : SinglePerimeterProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS_ABSTRACT (PRF_CLASS_ParametricProfile, ParametricProfile, ParametricProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)

    }; // ParametricProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
