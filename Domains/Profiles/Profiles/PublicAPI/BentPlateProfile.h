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
#include "ProfileMixins.h"

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

protected:
    virtual Dgn::DgnElementR _ICenterLineProfileToDgnElement() override { return *this; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(BentPlateProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(BentPlateProfile)

    PROFILES_EXPORT static BentPlateProfilePtr Create(/*TODO: args*/);

public:
    double GetWidth() const { return GetPropertyValueDouble(PRF_PROP_BentPlateProfile_Width); }
    PROFILES_EXPORT void SetWidth(double val);
    double GetBendAngle() const { return GetPropertyValueDouble(PRF_PROP_BentPlateProfile_BendAngle); }
    PROFILES_EXPORT void SetBendAngle(double val);
    double GetBendRadius() const { return GetPropertyValueDouble(PRF_PROP_BentPlateProfile_BendRadius); }
    PROFILES_EXPORT void SetBendRadius(double val);
    double GetBendOffset() const { return GetPropertyValueDouble(PRF_PROP_BentPlateProfile_BendOffset); }
    PROFILES_EXPORT void SetBendOffset(double val);

}; // BentPlateProfile

END_BENTLEY_PROFILES_NAMESPACE
