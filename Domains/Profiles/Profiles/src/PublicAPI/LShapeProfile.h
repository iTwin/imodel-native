/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/LShapeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! An L-shaped Profile similar to rolled steel L-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct LShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_LShapeProfile, ParametricProfile);
    friend struct LShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(LShapeProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth, double thickness,
                                               double filletRadius = 0.0, double edgeRadius = 0.0, Angle const& legSlope = Angle::FromRadians (0.0));

    public:
        //! Required properties
        double width = 0.0;
        double depth = 0.0;
        double thickness = 0.0;

        //! Optional properties
        double filletRadius = 0.0;
        double edgeRadius = 0.0;
        Angle legSlope = Angle::FromRadians (0.0);
        };

protected:
    explicit LShapeProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnDelete() const override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElement const& original) override;

private:
    bool ValidateThickness() const;
    bool ValidateFilletRadius() const;
    bool ValidateEdgeRadius() const;
    bool ValidateLegSlope() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (LShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (LShapeProfile)

    PROFILES_EXPORT static LShapeProfilePtr Create (CreateParams const& params) { return new LShapeProfile (params); }

public:
    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth (double value);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double value);

    PROFILES_EXPORT double GetThickness() const;
    PROFILES_EXPORT void SetThickness (double value);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius (double value);

    PROFILES_EXPORT double GetEdgeRadius() const;
    PROFILES_EXPORT void SetEdgeRadius (double value);

    PROFILES_EXPORT Angle GetLegSlope() const;
    PROFILES_EXPORT void SetLegSlope (Angle const& value);

public:
    PROFILES_EXPORT double GetInnerFlangeFaceLength() const;
    PROFILES_EXPORT double GetInnerWebFaceLength() const;
    PROFILES_EXPORT double GetHorizontalLegSlopeHeight() const;
    PROFILES_EXPORT double GetVerticalLegSlopeHeight() const;

    }; // LShapeProfile

//=======================================================================================
//! Handler for LShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_LShapeProfile, LShapeProfile, LShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // LShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
