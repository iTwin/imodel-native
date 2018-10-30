/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/TShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CenteredProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A T-shaped Profile similar to rolled steel T-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct TShapeProfile : CenteredProfile
{
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_TShapeProfile, CenteredProfile);
    friend struct TShapeProfileHandler;

protected:
    explicit TShapeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(TShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(TShapeProfile)

    PROFILES_EXPORT static TShapeProfilePtr Create(/*TODO: args*/);

public:
    double GetWidth() const { return GetPropertyValueDouble(PRF_PROP_TShapeProfile_Width); }
    PROFILES_EXPORT void SetWidth(double val);
    double GetDepth() const { return GetPropertyValueDouble(PRF_PROP_TShapeProfile_Depth); }
    PROFILES_EXPORT void SetDepth(double val);
    double GetFlangeThickness() const { return GetPropertyValueDouble(PRF_PROP_TShapeProfile_FlangeThickness); }
    PROFILES_EXPORT void SetFlangeThickness(double val);
    double GetWebThickness() const { return GetPropertyValueDouble(PRF_PROP_TShapeProfile_WebThickness); }
    PROFILES_EXPORT void SetWebThickness(double val);
    double GetFilletRadius() const { return GetPropertyValueDouble(PRF_PROP_TShapeProfile_FilletRadius); }
    PROFILES_EXPORT void SetFilletRadius(double val);
    double GetFlangeEdgeRadius() const { return GetPropertyValueDouble(PRF_PROP_TShapeProfile_FlangeEdgeRadius); }
    PROFILES_EXPORT void SetFlangeEdgeRadius(double val);
    double GetFlangeSlope() const { return GetPropertyValueDouble(PRF_PROP_TShapeProfile_FlangeSlope); }
    PROFILES_EXPORT void SetFlangeSlope(double val);
    double GetWebEdgeRadius() const { return GetPropertyValueDouble(PRF_PROP_TShapeProfile_WebEdgeRadius); }
    PROFILES_EXPORT void SetWebEdgeRadius(double val);
    double GetWebSlope() const { return GetPropertyValueDouble(PRF_PROP_TShapeProfile_WebSlope); }
    PROFILES_EXPORT void SetWebSlope(double val);

}; // TShapeProfile

END_BENTLEY_PROFILES_NAMESPACE
