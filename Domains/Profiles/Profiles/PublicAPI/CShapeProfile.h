/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CShapeProfile.h $
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
//! A C shaped Profile similar to rolled steel C-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CShapeProfile : CenteredProfile, ICShapeProfile
{
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CShapeProfile, CenteredProfile);
    friend struct CShapeProfileHandler;

protected:
    explicit CShapeProfile(CreateParams const& params) : T_Super(params) {}

protected:
    virtual Dgn::DgnElementR _ICShapeProfileToDgnElement() override { return *this; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(CShapeProfile)

    PROFILES_EXPORT static CShapeProfilePtr Create(/*TODO: args*/);

public:
    double GetFlangeThickness() const { return GetPropertyValueDouble(PRF_PROP_CShapeProfile_FlangeThickness); }
    PROFILES_EXPORT void SetFlangeThickness(double val);
    double GetWebThickness() const { return GetPropertyValueDouble(PRF_PROP_CShapeProfile_WebThickness); }
    PROFILES_EXPORT void SetWebThickness(double val);
    double GetFilletRadius() const { return GetPropertyValueDouble(PRF_PROP_CShapeProfile_FilletRadius); }
    PROFILES_EXPORT void SetFilletRadius(double val);
    double GetEdgeRadius() const { return GetPropertyValueDouble(PRF_PROP_CShapeProfile_EdgeRadius); }
    PROFILES_EXPORT void SetEdgeRadius(double val);
    double GetFlangeSlope() const { return GetPropertyValueDouble(PRF_PROP_CShapeProfile_FlangeSlope); }
    PROFILES_EXPORT void SetFlangeSlope(double val);

}; // CShapeProfile

END_BENTLEY_PROFILES_NAMESPACE
