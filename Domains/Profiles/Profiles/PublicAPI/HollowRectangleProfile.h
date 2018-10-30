/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/HollowRectangleProfile.h $
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
struct HollowRectangleProfile : CenteredProfile, IRectangleShapeProfile, ICenterLineProfile
{
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_HollowRectangleProfile, CenteredProfile);
    friend struct HollowRectangleProfileHandler;

protected:
    explicit HollowRectangleProfile(CreateParams const& params) : T_Super(params) {}

protected:
    virtual Dgn::DgnElementR _IRectangleShapeProfileToDgnElement() override { return *this; }
    virtual Dgn::DgnElementR _ICenterLineProfileToDgnElement() override { return *this; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(HollowRectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(HollowRectangleProfile)

    PROFILES_EXPORT static HollowRectangleProfilePtr Create(/*TODO: args*/);

public:
    double GetFilletRadius() const { return GetPropertyValueDouble(PRF_PROP_HollowRectangleProfile_FilletRadius); }
    PROFILES_EXPORT void SetFilletRadius(double val);

}; // HollowRectangleProfile

END_BENTLEY_PROFILES_NAMESPACE
