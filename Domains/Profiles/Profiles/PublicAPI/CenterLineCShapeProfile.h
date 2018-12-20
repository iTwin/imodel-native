/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CenterLineCShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"
#include "ICenterLineProfile.h"
#include <Geom/GeomApi.h>

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
        PROFILES_EXPORT explicit CreateParams(Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double girth, double wallThickness, double filletRadius = 0.0);

    public:
        //! Required properties
        double flangeWidth = 0.0;
        double depth = 0.0;
        double girth = 0.0;
        double wallThickness = 0.0;

        //! Optional properties
        double filletRadius = 0.0;
        };


protected:
    explicit CenterLineCShapeProfile(CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CenterLineCShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CenterLineCShapeProfile)

    PROFILES_EXPORT static CenterLineCShapeProfilePtr Create (CreateParams const& params) { return new CenterLineCShapeProfile (params); }

public:
    PROFILES_EXPORT double GetFlangeWidth() const;
    PROFILES_EXPORT void SetFlangeWidth (double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double val);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius (double val);

    PROFILES_EXPORT double GetGirth() const;
    PROFILES_EXPORT void SetGirth (double val);
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
