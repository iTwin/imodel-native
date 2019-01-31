/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/CShapeProfile.h $
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
//! A C shaped Profile similar to rolled steel C-shapes.
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct CShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CShapeProfile, ParametricProfile);
    friend struct CShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(CShapeProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth,
                                               double flangeThickness, double webThickness, double filletRadius = 0.0,
                                               double flangeEdgeRadius = 0.0, Angle const& flangeSlope = Angle::FromRadians (0.0));

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
    explicit CShapeProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnDelete() const override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _UpdateInDb() override;

private:
    bool ValidateFlangeThickness() const;
    bool ValidateWebThickness() const;
    bool ValidateFilletRadius() const;
    bool ValidteFlangeEdgeRadius() const;
    bool ValidateFlangeSlope() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CShapeProfile)

    PROFILES_EXPORT static CShapeProfilePtr Create (CreateParams const& params) { return new CShapeProfile (params); }

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
    PROFILES_EXPORT double GetWebInnerFaceLength() const;
    PROFILES_EXPORT double GetFlangeSlopeHeight() const;

    }; // CShapeProfile

//=======================================================================================
//! Handler for CShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_CShapeProfile, CShapeProfile, CShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // CShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
