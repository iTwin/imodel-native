/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/IShapeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_IShapeProfile, ParametricProfile);
    friend struct IShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(IShapeProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness,
                                               double webThickness, double filletRadius = 0.0, double flangeEdgeRadius = 0.0,
                                               Angle const& flangeSlope = Angle::FromRadians (0.0));

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
        };

protected:
    explicit IShapeProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

private:
    bool ValidateFlangeThickness() const;
    bool ValidateWebThickness() const;
    bool ValidateFilletRadius() const;
    bool ValidateFlangeEdgeRadius() const;
    bool ValidateFlangeSlope() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (IShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (IShapeProfile)

    PROFILES_EXPORT static IShapeProfilePtr Create (CreateParams const& params) { return new IShapeProfile (params); }

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

public:
    PROFILES_EXPORT double GetFlangeInnerFaceLength() const;
    PROFILES_EXPORT double GetWebFaceLength() const;
    PROFILES_EXPORT double GetFlangeSlopeHeight() const;

    }; // IShapeProfile

//=======================================================================================
//! Handler for IShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_IShapeProfile, IShapeProfile, IShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // IShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
