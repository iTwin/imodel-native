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
struct HollowEllipseProfile : CenteredProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_HollowEllipseProfile, CenteredProfile);
    friend struct HollowEllipseProfileHandler;

protected:
    explicit HollowEllipseProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(HollowEllipseProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(HollowEllipseProfile)

    PROFILES_EXPORT static HollowEllipseProfilePtr Create(Dgn::DgnModelCR model);

    PROFILES_EXPORT double GetXRadius() const;
    PROFILES_EXPORT void SetXRadius(double val);

    PROFILES_EXPORT double GetYRadius() const;
    PROFILES_EXPORT void SetYRadius(double val);

    }; // HollowEllipseProfile

//=======================================================================================
//! Handler for HollowEllipseProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HollowEllipseProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_HollowEllipseProfile, HollowEllipseProfile, HollowEllipseProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // HollowEllipseProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
