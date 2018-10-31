/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/AsymmetricIShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CenteredProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! An asymmetric (about x-axis) I shaped Profile similar to rolled steel I-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct AsymmetricIShapeProfile : CenteredProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_AsymmetricIShapeProfile, CenteredProfile);
    friend struct AsymmetricIShapeProfileHandler;

protected:
    explicit AsymmetricIShapeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(AsymmetricIShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(AsymmetricIShapeProfile)

    PROFILES_EXPORT static AsymmetricIShapeProfilePtr Create(/*TODO: args*/);

public:
    double GetTopWidth() const { return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_TopWidth); }
    PROFILES_EXPORT void SetTopWidth(double val);
    double GetBottomWidth() const { return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_BottomWidth); }
    PROFILES_EXPORT void SetBottomWidth(double val);
    double GetDepth() const { return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_Depth); }
    PROFILES_EXPORT void SetDepth(double val);
    double GetTopFlangeThickness() const { return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_TopFlangeThickness); }
    PROFILES_EXPORT void SetTopFlangeThickness(double val);
    double GetBottomFlangeThickness() const { return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_BottomFlangeThickness); }
    PROFILES_EXPORT void SetBottomFlangeThickness(double val);
    double GetWebThickness() const { return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_WebThickness); }
    PROFILES_EXPORT void SetWebThickness(double val);
    double GetTopFlangeFilletRadius() const { return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_TopFlangeFilletRadius); }
    PROFILES_EXPORT void SetTopFlangeFilletRadius(double val);
    double GetTopFlangeEdgeRadius() const { return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_TopFlangeEdgeRadius); }
    PROFILES_EXPORT void SetTopFlangeEdgeRadius(double val);
    double GetTopFlangeSlope() const { return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_TopFlangeSlope); }
    PROFILES_EXPORT void SetTopFlangeSlope(double val);
    double GetBottomFlangeFilletRadius() const { return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_BottomFlangeFilletRadius); }
    PROFILES_EXPORT void SetBottomFlangeFilletRadius(double val);
    double GetBottomFlangeEdgeRadius() const { return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_BottomFlangeEdgeRadius); }
    PROFILES_EXPORT void SetBottomFlangeEdgeRadius(double val);
    double GetBottomFlangeSlope() const { return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_BottomFlangeSlope); }
    PROFILES_EXPORT void SetBottomFlangeSlope(double val);

    }; // AsymmetricIShapeProfile

//=======================================================================================
//! Handler for AsymmetricIShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AsymmetricIShapeProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_AsymmetricIShapeProfile, AsymmetricIShapeProfile, AsymmetricIShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // AsymmetricIShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
