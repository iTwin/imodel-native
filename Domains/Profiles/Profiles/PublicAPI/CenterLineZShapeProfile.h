/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CenterLineZShapeProfile.h $
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
//! A Z-shaped Profile with rounded corners, similar to cold-formed steel Z-shapes
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CenterLineZShapeProfile : CenteredProfile, ICenterLineProfile
{
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CenterLineZShapeProfile, CenteredProfile);
    friend struct CenterLineZShapeProfileHandler;

protected:
    explicit CenterLineZShapeProfile(CreateParams const& params) : T_Super(params) {}

protected:
    virtual Dgn::DgnElementR _ICenterLineProfileToDgnElement() override { return *this; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CenterLineZShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(CenterLineZShapeProfile)

    PROFILES_EXPORT static CenterLineZShapeProfilePtr Create(/*TODO: args*/);

public:
    double GetFlangeWidth() const { return GetPropertyValueDouble(PRF_PROP_CenterLineZShapeProfile_FlangeWidth); }
    PROFILES_EXPORT void SetFlangeWidth(double val);
    double GetDepth() const { return GetPropertyValueDouble(PRF_PROP_CenterLineZShapeProfile_Depth); }
    PROFILES_EXPORT void SetDepth(double val);
    double GetFilletRadius() const { return GetPropertyValueDouble(PRF_PROP_CenterLineZShapeProfile_FilletRadius); }
    PROFILES_EXPORT void SetFilletRadius(double val);
    double GetLipLength() const { return GetPropertyValueDouble(PRF_PROP_CenterLineZShapeProfile_LipLength); }
    PROFILES_EXPORT void SetLipLength(double val);

}; // CenterLineZShapeProfile

END_BENTLEY_PROFILES_NAMESPACE
