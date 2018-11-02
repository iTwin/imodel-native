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
    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth(double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth(double val);

    PROFILES_EXPORT double GetFlangeThickness() const;
    PROFILES_EXPORT void SetFlangeThickness(double val);

    PROFILES_EXPORT double GetWebThickness() const;
    PROFILES_EXPORT void SetWebThickness(double val);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius(double val);

    PROFILES_EXPORT double GetFlangeEdgeRadius() const;
    PROFILES_EXPORT void SetFlangeEdgeRadius(double val);

    PROFILES_EXPORT double GetFlangeSlope() const;
    PROFILES_EXPORT void SetFlangeSlope(double val);

    PROFILES_EXPORT double GetWebEdgeRadius() const;
    PROFILES_EXPORT void SetWebEdgeRadius(double val);

    PROFILES_EXPORT double GetWebSlope() const;
    PROFILES_EXPORT void SetWebSlope(double val);

    }; // TShapeProfile

//=======================================================================================
//! Handler for TShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TShapeProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_TShapeProfile, TShapeProfile, TShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // TShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
