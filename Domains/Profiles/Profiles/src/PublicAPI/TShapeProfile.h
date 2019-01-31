/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/TShapeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A T-shaped Profile similar to rolled steel T-shapes.
//! @ingroup GROUP_ParametricProfiles
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
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness, double webThickness,
                                               double filletRadius = 0.0, double flangeEdgeRadius = 0.0, Angle const& flangeSlope = Angle::FromRadians (0.0),
                                               double webEdgeRadius = 0.0, Angle const& webSlope = Angle::FromRadians (0.0));

    public:
        //! Required properties
        double flangeWidth = 0.0;
        double depth = 0.0;
        double flangeThickness = 0.0;
        double webThickness = 0.0;

        //! Optional properties
        double filletRadius = 0.0;
        double flangeEdgeRadius = 0.0;
        Angle flangeSlope = Angle::FromRadians (0.0);
        double webEdgeRadius = 0.0;
        Angle webSlope = Angle::FromRadians (0.0);
        };

protected:
    explicit TShapeProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

private:
    bool ValidateFlangeThickness() const;
    bool ValidateWebThickness() const;
    bool ValidateFilletRadius() const;
    bool ValidateFlangeEdgeRadius() const;
    bool ValidateFlangeSlope() const;
    bool ValidateWebEdgeRadius() const;
    bool ValidateWebSlope() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (TShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (TShapeProfile)

    PROFILES_EXPORT static TShapeProfilePtr Create (CreateParams const& params) { return new TShapeProfile (params); }

public:
    PROFILES_EXPORT double GetFlangeWidth() const;
    PROFILES_EXPORT void SetFlangeWidth (double value);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double value);

    PROFILES_EXPORT double GetFlangeThickness() const;
    PROFILES_EXPORT void SetFlangeThickness (double value);

    PROFILES_EXPORT double GetWebThickness() const;
    PROFILES_EXPORT void SetWebThickness (double value);

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

public:
    PROFILES_EXPORT double GetFlangeInnerFaceLength() const;
    PROFILES_EXPORT double GetWebFaceLength() const;
    PROFILES_EXPORT double GetFlangeSlopeHeight() const;
    PROFILES_EXPORT double GetWebSlopeHeight() const;

    }; // TShapeProfile

//=======================================================================================
//! Handler for TShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_TShapeProfile, TShapeProfile, TShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // TShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
