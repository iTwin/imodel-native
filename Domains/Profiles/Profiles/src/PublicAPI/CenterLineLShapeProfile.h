/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/CenterLineLShapeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! An L-shaped Profile with rounded corners, similar to cold-formed steel L-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CenterLineLShapeProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CenterLineLShapeProfile, ParametricProfile);
    friend struct CenterLineLShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(CenterLineLShapeProfile::T_Super::CreateParams);
        explicit CreateParams(DgnElement::CreateParams const& params) : T_Super(params) {}

    public:
        PROFILES_EXPORT explicit CreateParams(Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams(Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double girth, double wallThickness, double filletRadius = 0.0);

    public:
        //! Required properties
        double width = 0.0;
        double depth = 0.0;
        double wallThickness = 0.0;

        //! Optional properties
        double girth = 0.0;
        double filletRadius = 0.0;
        };

protected:
    explicit CenterLineLShapeProfile(CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CenterLineLShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CenterLineLShapeProfile)

    PROFILES_EXPORT static CenterLineLShapeProfilePtr Create (CreateParams const& params) { return new CenterLineLShapeProfile (params); }

public:
    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth (double value);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double value);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius (double value);

    PROFILES_EXPORT double GetGirth() const;
    PROFILES_EXPORT void SetGirth (double value);

    }; // CenterLineLShapeProfile

//=======================================================================================
//! Handler for CenterLineLShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenterLineLShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_CenterLineLShapeProfile, CenterLineLShapeProfile, CenterLineLShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)
    }; // CenterLineLShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
