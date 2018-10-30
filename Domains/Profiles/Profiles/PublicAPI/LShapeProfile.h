/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/LShapeProfile.h $
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
//! An L-shaped Profile similar to rolled steel L-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct LShapeProfile : CenteredProfile, ILShapeProfile
{
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_LShapeProfile, CenteredProfile);
    friend struct LShapeProfileHandler;

protected:
    explicit LShapeProfile(CreateParams const& params) : T_Super(params) {}

protected:
    virtual Dgn::DgnElementR _ILShapeProfileToDgnElement() override { return *this; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(LShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(LShapeProfile)

    PROFILES_EXPORT static LShapeProfilePtr Create(/*TODO: args*/);

public:
    double GetThickness() const { return GetPropertyValueDouble(PRF_PROP_LShapeProfile_Thickness); }
    PROFILES_EXPORT void SetThickness(double val);
    double GetFilletRadius() const { return GetPropertyValueDouble(PRF_PROP_LShapeProfile_FilletRadius); }
    PROFILES_EXPORT void SetFilletRadius(double val);
    double GetEdgeRadius() const { return GetPropertyValueDouble(PRF_PROP_LShapeProfile_EdgeRadius); }
    PROFILES_EXPORT void SetEdgeRadius(double val);
    double GetHorizontalLegSlope() const { return GetPropertyValueDouble(PRF_PROP_LShapeProfile_HorizontalLegSlope); }
    PROFILES_EXPORT void SetHorizontalLegSlope(double val);
    double GetVerticalLegSlope() const { return GetPropertyValueDouble(PRF_PROP_LShapeProfile_VerticalLegSlope); }
    PROFILES_EXPORT void SetVerticalLegSlope(double val);

}; // LShapeProfile

END_BENTLEY_PROFILES_NAMESPACE
