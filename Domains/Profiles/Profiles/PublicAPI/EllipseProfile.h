/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/EllipseProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct EllipseProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_EllipseProfile, ParametricProfile);
    friend struct EllipseProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(EllipseProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double xRadius, double yRadius);

    public:
        //! Required properties
        double xRadius = 0.0;
        double yRadius = 0.0;
        };

protected:
    explicit EllipseProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

private:
    bool ValidateXRadius() const;
    bool ValidateYRadius() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (EllipseProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (EllipseProfile)

    PROFILES_EXPORT static EllipseProfilePtr Create (CreateParams const& params) { return new EllipseProfile (params); }

public:
    PROFILES_EXPORT double GetXRadius() const;
    PROFILES_EXPORT void SetXRadius (double val);

    PROFILES_EXPORT double GetYRadius() const;
    PROFILES_EXPORT void SetYRadius (double val);

    }; // EllipseProfile

//=======================================================================================
//! Handler for EllipseProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EllipseProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_EllipseProfile, EllipseProfile, EllipseProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // EllipseProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
