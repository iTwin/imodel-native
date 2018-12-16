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

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(AsymmetricIShapeProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double topFlangeWidth, double bottomFlangeWidth, double depth,
                                               double topFlangeThickness, double bottomFlangeThickness, double webThickness, double topFlangeFilletRadius = 0.0,
                                               double topFlangeEdgeRadius = 0.0, double topFlangeSlope = 0.0, double bottomFlangeFilletRadius = 0.0,
                                               double bottomFlangeEdgeRadius = 0.0, double bottomFlangeSlope = 0.0);

    public:
        //! Required properties
        double topFlangeWidth = 0.0;
        double bottomFlangeWidth = 0.0;
        double depth = 0.0;
        double topFlangeThickness = 0.0;
        double bottomFlangeThickness = 0.0;
        double webThickness = 0.0;

        //! Optional properties
        double topFlangeFilletRadius = 0.0;
        double topFlangeEdgeRadius = 0.0;
        double topFlangeSlope = 0.0;
        double bottomFlangeFilletRadius = 0.0;
        double bottomFlangeEdgeRadius = 0.0;
        double bottomFlangeSlope = 0.0;
        };

protected:
    explicit AsymmetricIShapeProfile (CreateParams const& params);

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

public:
    PROFILES_EXPORT double GetInnerTopFlangeFaceLength() const;
    PROFILES_EXPORT double GetInnerBottomFlangeFaceLength() const;
    PROFILES_EXPORT double GetInnerWebFaceLength() const;
    PROFILES_EXPORT double GetTopFlangeSlopeHeight() const;
    PROFILES_EXPORT double GetBottomFlangeSlopeHeight() const;

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
