/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/RectangleProfile.h $
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
struct RectangleProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_RectangleProfile, ParametricProfile);
    friend struct RectangleProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(RectangleProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth);

    public:
        //! Required properties
        double width = 0.0;
        double depth = 0.0;
        };

protected:
    explicit RectangleProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (RectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (RectangleProfile)

    PROFILES_EXPORT static RectangleProfilePtr Create (CreateParams const& params) { return new RectangleProfile (params); }

    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth (double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double val);

    }; // RectangleProfile

//=======================================================================================
//! Handler for RectangleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RectangleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_RectangleProfile, RectangleProfile, RectangleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // RectangleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
