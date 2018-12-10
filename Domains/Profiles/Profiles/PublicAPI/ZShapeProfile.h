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

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(ZShapeProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness,
                                               double webThickness, double filletRadius = 0.0, double flangeEdgeRadius = 0.0, double flangeSlope = 0.0);

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
        };

protected:
    explicit ZShapeProfile (CreateParams const& params);

    virtual BentleyStatus _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

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

public:
    PROFILES_EXPORT double GetInnerFlangeFaceLength() const;
    PROFILES_EXPORT double GetInnerWebFaceLength() const;
    PROFILES_EXPORT double GetSlopeHeight() const;

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
