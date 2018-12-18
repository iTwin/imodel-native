/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/RoundedRectangleProfile.h $
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
struct RoundedRectangleProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_RoundedRectangleProfile, ParametricProfile);
    friend struct RoundedRectangleProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(RoundedRectangleProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth, double roundingRadius);

    public:
        //! Required properties
        double width = 0.0;
        double depth = 0.0;
        double roundingRadius = 0.0;
        };

protected:
    explicit RoundedRectangleProfile (CreateParams const& params);

    virtual bool _Validate() const override;

private:
    bool ValidateWidth() const;
    bool ValidateDepth() const;
    bool validateRoundingRadius() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (RoundedRectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (RoundedRectangleProfile)

    PROFILES_EXPORT static RoundedRectangleProfilePtr Create (CreateParams const& params) { return new RoundedRectangleProfile (params); }

public:
    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth (double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double val);

    PROFILES_EXPORT double GetRoundingRadius() const;
    PROFILES_EXPORT void SetRoundingRadius (double val);

    }; // RoundedRectangleProfile

//=======================================================================================
//! Handler for RoundedRectangleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoundedRectangleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_RoundedRectangleProfile, RoundedRectangleProfile, RoundedRectangleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // RoundedRectangleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
