/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/TTShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CenteredProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A TT-shaped Profile similar to double-tee concrete shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct TTShapeProfile : CenteredProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_TTShapeProfile, CenteredProfile);
    friend struct TTShapeProfileHandler;

protected:
    explicit TTShapeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(TTShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(TTShapeProfile)

    PROFILES_EXPORT static TTShapeProfilePtr Create(/*TODO: args*/);

public:
    double GetWidth() const { return GetPropertyValueDouble(PRF_PROP_TTShapeProfile_Width); }
    PROFILES_EXPORT void SetWidth(double val);
    double GetDepth() const { return GetPropertyValueDouble(PRF_PROP_TTShapeProfile_Depth); }
    PROFILES_EXPORT void SetDepth(double val);
    double GetSpan() const { return GetPropertyValueDouble(PRF_PROP_TTShapeProfile_Span); }
    PROFILES_EXPORT void SetSpan(double val);
    double GetFlangeThickness() const { return GetPropertyValueDouble(PRF_PROP_TTShapeProfile_FlangeThickness); }
    PROFILES_EXPORT void SetFlangeThickness(double val);
    double GetWebThickness() const { return GetPropertyValueDouble(PRF_PROP_TTShapeProfile_WebThickness); }
    PROFILES_EXPORT void SetWebThickness(double val);
    double GetFilletRadius() const { return GetPropertyValueDouble(PRF_PROP_TTShapeProfile_FilletRadius); }
    PROFILES_EXPORT void SetFilletRadius(double val);
    double GetFlangeEdgeRadius() const { return GetPropertyValueDouble(PRF_PROP_TTShapeProfile_FlangeEdgeRadius); }
    PROFILES_EXPORT void SetFlangeEdgeRadius(double val);
    double GetFlangeSlope() const { return GetPropertyValueDouble(PRF_PROP_TTShapeProfile_FlangeSlope); }
    PROFILES_EXPORT void SetFlangeSlope(double val);
    double GetWebEdgeRadius() const { return GetPropertyValueDouble(PRF_PROP_TTShapeProfile_WebEdgeRadius); }
    PROFILES_EXPORT void SetWebEdgeRadius(double val);
    double GetWebSlope() const { return GetPropertyValueDouble(PRF_PROP_TTShapeProfile_WebSlope); }
    PROFILES_EXPORT void SetWebSlope(double val);

    }; // TTShapeProfile

//=======================================================================================
//! Handler for TTShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TTShapeProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_TTShapeProfile, TTShapeProfile, TTShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // TTShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
