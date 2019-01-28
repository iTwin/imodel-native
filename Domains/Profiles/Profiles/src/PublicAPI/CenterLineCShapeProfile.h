/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/CenterLineCShapeProfile.h $
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
//! A C shaped Profile with rounded corners, similar to cold-formed steel C-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CenterLineCShapeProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CenterLineCShapeProfile, ParametricProfile);
    friend struct CenterLineCShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(CenterLineCShapeProfile::T_Super::CreateParams);
        explicit CreateParams(DgnElement::CreateParams const& params) : T_Super(params) {}

    public:
        PROFILES_EXPORT explicit CreateParams(Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams(Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double wallThickness, double girth = 0.0, double filletRadius = 0.0);
    public:
        //! Required properties
        double flangeWidth = 0.0;
        double depth = 0.0;
        double wallThickness = 0.0;

        //! Optional properties
        double girth = 0.0;
        double filletRadius = 0.0;
        };

protected:
    explicit CenterLineCShapeProfile(CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

    bool CreateCenterLineGeometry();

    virtual IGeometryPtr _CreateCenterLineGeometry() const;
    virtual IGeometryPtr _UpdateCenterLineGeometry(Profile const& relatedProfile) const;


public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CenterLineCShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CenterLineCShapeProfile)

    PROFILES_EXPORT static CenterLineCShapeProfilePtr Create (CreateParams const& params) { return new CenterLineCShapeProfile (params); }

protected:
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnInsert() override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnUpdate(Dgn::DgnElement const& original) override;
public:
    PROFILES_EXPORT double GetFlangeWidth() const;
    PROFILES_EXPORT void SetFlangeWidth (double value);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double value);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius (double value);

    PROFILES_EXPORT double GetGirth() const;
    PROFILES_EXPORT void SetGirth (double value);
    }; // CenterLineCShapeProfile

//=======================================================================================
//! Handler for CenterLineCShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenterLineCShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_CenterLineCShapeProfile, CenterLineCShapeProfile, CenterLineCShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)
    }; // CenterLineCShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
