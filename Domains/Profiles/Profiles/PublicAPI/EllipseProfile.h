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

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CircleProfile : CenteredProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CircleProfile, CenteredProfile);
    friend struct CircleProfileHandler;

protected:
    explicit CircleProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CircleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(CircleProfile)

    PROFILES_EXPORT static CircleProfilePtr Create(Dgn::DgnModelCR model);

    PROFILES_EXPORT double GetXRadius() const;
    PROFILES_EXPORT void SetXRadius(double val);

    PROFILES_EXPORT double GetYRadius() const;
    PROFILES_EXPORT void SetYRadius(double val);

    }; // CircleProfile

//=======================================================================================
//! Handler for CircleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CircleProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CircleProfile, CircleProfile, CircleProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // CircleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
