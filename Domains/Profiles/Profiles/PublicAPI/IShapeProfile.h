/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/IShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! An I shaped Profile similar to rolled steel I-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct IShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_IShapeProfile, ParametricProfile);
    friend struct IShapeProfileHandler;

protected:
    explicit IShapeProfile(T_Super::CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(IShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(IShapeProfile)

    PROFILES_EXPORT static IShapeProfilePtr Create(Dgn::DgnModelCR model);

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

    }; // IShapeProfile

//=======================================================================================
//! Handler for IShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_IShapeProfile, IShapeProfile, IShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // IShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
