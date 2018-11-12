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
struct EllipseProfile : CenteredProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_EllipseProfile, CenteredProfile);
    friend struct EllipseProfileHandler;

protected:
    explicit EllipseProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(EllipseProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(EllipseProfile)

    PROFILES_EXPORT static EllipseProfilePtr Create(/*TODO: args*/);

public:
    PROFILES_EXPORT double GetXRadius() const;
    PROFILES_EXPORT void SetXRadius(double val);

    PROFILES_EXPORT double GetYRadius() const;
    PROFILES_EXPORT void SetYRadius(double val);

    }; // EllipseProfile

//=======================================================================================
//! Handler for EllipseProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EllipseProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_EllipseProfile, EllipseProfile, EllipseProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // EllipseProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
