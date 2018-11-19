/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/AsymmetricIShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! An asymmetric (about x-axis) I shaped Profile similar to rolled steel I-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct AsymmetricIShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_AsymmetricIShapeProfile, ParametricProfile);
    friend struct AsymmetricIShapeProfileHandler;

protected:
    explicit AsymmetricIShapeProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (AsymmetricIShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (AsymmetricIShapeProfile)

    PROFILES_EXPORT static AsymmetricIShapeProfilePtr Create (CreateParams const& params) { return new AsymmetricIShapeProfile (params); }

public:
    PROFILES_EXPORT double GetTopFlangeWidth() const;
    PROFILES_EXPORT void SetTopFlangeWidth (double val);

    PROFILES_EXPORT double GetBottomFlangeWidth() const;
    PROFILES_EXPORT void SetBottomFlangeWidth (double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double val);

    PROFILES_EXPORT double GetTopFlangeThickness() const;
    PROFILES_EXPORT void SetTopFlangeThickness (double val);

    PROFILES_EXPORT double GetBottomFlangeThickness() const;
    PROFILES_EXPORT void SetBottomFlangeThickness (double val);

    PROFILES_EXPORT double GetWebThickness() const;
    PROFILES_EXPORT void SetWebThickness (double val);

    PROFILES_EXPORT double GetTopFlangeFilletRadius() const;
    PROFILES_EXPORT void SetTopFlangeFilletRadius (double val);

    PROFILES_EXPORT double GetTopFlangeEdgeRadius() const;
    PROFILES_EXPORT void SetTopFlangeEdgeRadius (double val);

    PROFILES_EXPORT double GetTopFlangeSlope() const;
    PROFILES_EXPORT void SetTopFlangeSlope (double val);

    PROFILES_EXPORT double GetBottomFlangeFilletRadius() const;
    PROFILES_EXPORT void SetBottomFlangeFilletRadius (double val);

    PROFILES_EXPORT double GetBottomFlangeEdgeRadius() const;
    PROFILES_EXPORT void SetBottomFlangeEdgeRadius (double val);

    PROFILES_EXPORT double GetBottomFlangeSlope() const;
    PROFILES_EXPORT void SetBottomFlangeSlope (double val);

    }; // AsymmetricIShapeProfile

//=======================================================================================
//! Handler for AsymmetricIShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AsymmetricIShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_AsymmetricIShapeProfile, AsymmetricIShapeProfile, AsymmetricIShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // AsymmetricIShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
