/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CircleProfile.h $
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
struct CircleProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CircleProfile, ParametricProfile);
    friend struct CircleProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(CircleProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double radius);

    public:
        //! Required properties
        double radius = 0.0;
        };

protected:
    explicit CircleProfile (CreateParams const& params);

    virtual BentleyStatus _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

private:
    bool ValidateRadius() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CircleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CircleProfile)

    PROFILES_EXPORT static CircleProfilePtr Create (CreateParams const& params) { return new CircleProfile (params); }

    PROFILES_EXPORT double GetRadius() const;
    PROFILES_EXPORT void SetRadius (double val);

    }; // CircleProfile

//=======================================================================================
//! Handler for CircleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CircleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_CircleProfile, CircleProfile, CircleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // CircleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
