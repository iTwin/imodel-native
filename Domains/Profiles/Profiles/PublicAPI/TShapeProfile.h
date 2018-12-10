/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/TShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A T-shaped Profile similar to rolled steel T-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct TShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_TShapeProfile, ParametricProfile);
    friend struct TShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(TShapeProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness,
                                               double webThickness, double filletRadius = 0.0, double flangeEdgeRadius = 0.0, double flangeSlope = 0.0,
                                               double webEdgeRadius = 0.0, double webSlope = 0.0);

    public:
        //! Required properties
        double flangeWidth = 0.0;
        double depth = 0.0;
        double flangeThickness = 0.0;
        double webThickness = 0.0;

        //! Optional properties
        double filletRadius = 0.0;
        double flangeEdgeRadius = 0.0;
        double flangeSlope = 0.0;
        double webEdgeRadius = 0.0;
        double webSlope = 0.0;
        };

protected:
    explicit TShapeProfile (CreateParams const& params);

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (TShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (TShapeProfile)

    PROFILES_EXPORT static TShapeProfilePtr Create (CreateParams const& params) { return new TShapeProfile (params); }

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

    PROFILES_EXPORT double GetWebEdgeRadius() const;
    PROFILES_EXPORT void SetWebEdgeRadius (double val);

    PROFILES_EXPORT double GetWebSlope() const;
    PROFILES_EXPORT void SetWebSlope (double val);

    }; // TShapeProfile

//=======================================================================================
//! Handler for TShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_TShapeProfile, TShapeProfile, TShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // TShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
