/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/TTShapeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A TT-shaped Profile similar to double-tee concrete shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct TTShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_TTShapeProfile, ParametricProfile);
    friend struct TTShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(TTShapeProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness, double webThickness,
                                               double webSpacing, double filletRadius = 0.0, double flangeEdgeRadius = 0.0, Angle const& flangeSlope = Angle::FromRadians (0.0),
                                               double webEdgeRadius = 0.0, Angle const& webSlope = Angle::FromRadians (0.0));

    public:
        //! Required properties
        double flangeWidth = 0.0;
        double depth = 0.0;
        double flangeThickness = 0.0;
        double webThickness = 0.0;
        double webSpacing = 0.0;

        //! Optional properties
        double filletRadius = 0.0;
        double flangeEdgeRadius = 0.0;
        Angle flangeSlope = Angle::FromRadians (0.0);
        double webEdgeRadius = 0.0;
        Angle webSlope = Angle::FromRadians (0.0);
        };

protected:
    explicit TTShapeProfile (CreateParams const& params);

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (TTShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (TTShapeProfile)

    PROFILES_EXPORT static TTShapeProfilePtr Create (CreateParams const& params) { return new TTShapeProfile (params); }

public:
    PROFILES_EXPORT double GetFlangeWidth() const;
    PROFILES_EXPORT void SetFlangeWidth (double value);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double value);

    PROFILES_EXPORT double GetFlangeThickness() const;
    PROFILES_EXPORT void SetFlangeThickness (double value);

    PROFILES_EXPORT double GetWebThickness() const;
    PROFILES_EXPORT void SetWebThickness (double value);

    PROFILES_EXPORT double GetWebSpacing() const;
    PROFILES_EXPORT void SetWebSpacing (double value);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius (double value);

    PROFILES_EXPORT double GetFlangeEdgeRadius() const;
    PROFILES_EXPORT void SetFlangeEdgeRadius (double value);

    PROFILES_EXPORT Angle GetFlangeSlope() const;
    PROFILES_EXPORT void SetFlangeSlope (Angle const& value);

    PROFILES_EXPORT double GetWebEdgeRadius() const;
    PROFILES_EXPORT void SetWebEdgeRadius (double value);

    PROFILES_EXPORT Angle GetWebSlope() const;
    PROFILES_EXPORT void SetWebSlope (Angle const& value);

    }; // TTShapeProfile

//=======================================================================================
//! Handler for TTShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TTShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_TTShapeProfile, TTShapeProfile, TTShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // TTShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
