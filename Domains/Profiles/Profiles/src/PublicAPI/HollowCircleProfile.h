/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/HollowCircleProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct HollowCircleProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_HollowCircleProfile, ParametricProfile);
    friend struct HollowCircleProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(HollowCircleProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double radius, double wallThickness);

    public:
        //! Required properties
        double radius = 0.0;
        double wallThickness = 0.0;
        };

protected:
    explicit HollowCircleProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

private:
    bool ValidateWallThickness() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (HollowCircleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (HollowCircleProfile)

    PROFILES_EXPORT static HollowCircleProfilePtr Create (CreateParams const& params) { return new HollowCircleProfile (params); }

    PROFILES_EXPORT double GetRadius() const;
    PROFILES_EXPORT void SetRadius (double val);

    }; // HollowCircleProfile

//=======================================================================================
//! Handler for HollowCircleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HollowCircleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_HollowCircleProfile, HollowCircleProfile, HollowCircleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // HollowCircleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
