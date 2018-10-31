/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/IShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CenteredProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! An I shaped Profile similar to rolled steel I-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct IShapeProfile : CenteredProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_IShapeProfile, CenteredProfile);
    friend struct IShapeProfileHandler;

protected:
    explicit IShapeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(IShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(IShapeProfile)

    PROFILES_EXPORT static IShapeProfilePtr Create(/*TODO: args*/);

public:
    double GetWidth() const { return GetPropertyValueDouble(PRF_PROP_IShapeProfile_Width); }
    PROFILES_EXPORT void SetWidth(double val);
    double GetDepth() const { return GetPropertyValueDouble(PRF_PROP_IShapeProfile_Depth); }
    PROFILES_EXPORT void SetDepth(double val);
    double GetFlangeThickness() const { return GetPropertyValueDouble(PRF_PROP_IShapeProfile_FlangeThickness); }
    PROFILES_EXPORT void SetFlangeThickness(double val);
    double GetWebThickness() const { return GetPropertyValueDouble(PRF_PROP_IShapeProfile_WebThickness); }
    PROFILES_EXPORT void SetWebThickness(double val);
    double GetFilletRadius() const { return GetPropertyValueDouble(PRF_PROP_IShapeProfile_FilletRadius); }
    PROFILES_EXPORT void SetFilletRadius(double val);
    double GetFlangeEdgeRadius() const { return GetPropertyValueDouble(PRF_PROP_IShapeProfile_FlangeEdgeRadius); }
    PROFILES_EXPORT void SetFlangeEdgeRadius(double val);
    double GetFlangeSlope() const { return GetPropertyValueDouble(PRF_PROP_IShapeProfile_FlangeSlope); }
    PROFILES_EXPORT void SetFlangeSlope(double val);

    }; // IShapeProfile

//=======================================================================================
//! Handler for IShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IShapeProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_IShapeProfile, IShapeProfile, IShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // IShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
