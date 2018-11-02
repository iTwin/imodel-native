/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/ZShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CenteredProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A Z-shaped Profile similar to rolled steel Z-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct ZShapeProfile : CenteredProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_ZShapeProfile, CenteredProfile);
    friend struct ZShapeProfileHandler;

protected:
    explicit ZShapeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(ZShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(ZShapeProfile)

    PROFILES_EXPORT static ZShapeProfilePtr Create(Dgn::DgnModelCR model);

public:
    PROFILES_EXPORT double GetFlangeWidth() const;
    PROFILES_EXPORT void SetFlangeWidth(double val);

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

    }; // ZShapeProfile

//=======================================================================================
//! Handler for ZShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ZShapeProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_ZShapeProfile, ZShapeProfile, ZShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // ZShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
