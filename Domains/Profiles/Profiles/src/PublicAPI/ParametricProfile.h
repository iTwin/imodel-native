/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/ParametricProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "SinglePerimeterProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A SinglePerimeterProfile that is guaranteed to have the center of the bounding box at (0, 0).
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct ParametricProfile : SinglePerimeterProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_ParametricProfile, SinglePerimeterProfile);
    friend struct ParametricProfileHandler;

protected:
    //! @private
    explicit ParametricProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ParametricProfile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS (ParametricProfile)

    }; // ParametricProfile

//=======================================================================================
//! Handler for ParametricProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ParametricProfileHandler : SinglePerimeterProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS_ABSTRACT (PRF_CLASS_ParametricProfile, ParametricProfile, ParametricProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)

    }; // ParametricProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
