/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/BentPlateProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CenteredProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct BentPlateProfile : CenteredProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_BentPlateProfile, CenteredProfile);
    friend struct BentPlateProfileHandler;

protected:
    explicit BentPlateProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(BentPlateProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(BentPlateProfile)

    PROFILES_EXPORT static BentPlateProfilePtr Create(Dgn::DgnModelCR model);

public:
    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth(double val);

    PROFILES_EXPORT double GetBendAngle() const;
    PROFILES_EXPORT void SetBendAngle(double val);

    PROFILES_EXPORT double GetBendRadius() const;
    PROFILES_EXPORT void SetBendRadius(double val);

    PROFILES_EXPORT double GetBendOffset() const;
    PROFILES_EXPORT void SetBendOffset(double val);

    }; // BentPlateProfile

//=======================================================================================
//! Handler for BentPlateProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BentPlateProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_BentPlateProfile, BentPlateProfile, BentPlateProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // BentPlateProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
