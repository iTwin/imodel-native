/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/ZShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A Z-shaped Profile similar to rolled steel Z-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct ZShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_ZShapeProfile, ParametricProfile);
    friend struct ZShapeProfileHandler;

protected:
    explicit ZShapeProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ZShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (ZShapeProfile)

    PROFILES_EXPORT static ZShapeProfilePtr Create (CreateParams const& params) { return new ZShapeProfile (params); }

public:
    PROFILES_EXPORT double GetFlangeWidth() const;
    PROFILES_EXPORT void SetFlangeWidth (double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double val);

    PROFILES_EXPORT double GetFlangeThickness() const;
    PROFILES_EXPORT void SetFlangeThickness (double val);

    PROFILES_EXPORT double GetWebThickness() const;
    PROFILES_EXPORT void SetWebThickness (double val);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius (double val);

    PROFILES_EXPORT double GetFlangeEdgeRadius() const;
    PROFILES_EXPORT void SetFlangeEdgeRadius (double val);

    PROFILES_EXPORT double GetFlangeSlope() const;
    PROFILES_EXPORT void SetFlangeSlope (double val);

    }; // ZShapeProfile

//=======================================================================================
//! Handler for ZShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ZShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_ZShapeProfile, ZShapeProfile, ZShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // ZShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
