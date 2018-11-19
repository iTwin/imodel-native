/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CircleProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CircleProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CircleProfile, ParametricProfile);
    friend struct CircleProfileHandler;

protected:
    explicit CircleProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CircleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CircleProfile)

    PROFILES_EXPORT static CircleProfilePtr Create (Dgn::DgnModelCR model);

    PROFILES_EXPORT double GetRadius() const;
    PROFILES_EXPORT void SetRadius (double val);

    }; // CircleProfile

//=======================================================================================
//! Handler for CircleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CircleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_CircleProfile, CircleProfile, CircleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // CircleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
