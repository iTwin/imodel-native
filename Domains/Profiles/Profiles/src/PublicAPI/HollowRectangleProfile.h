/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/HollowRectangleProfile.h $
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
struct HollowRectangleProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_HollowRectangleProfile, ParametricProfile);
    friend struct HollowRectangleProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(HollowRectangleProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth, double wallThickness,
                                               double innerFilletRadius = 0.0, double outerFilletRadisu = 0.0);

    public:
        //! Required properties
        double width = 0.0;
        double depth = 0.0;
        double wallThickness = 0.0;

        //! Optional properties
        double innerFilletRadius = 0.0;
        double outerFilletRadius = 0.0;
        };

protected:
    explicit HollowRectangleProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

private:
    bool ValidateWallThickness() const;
    bool ValidateInnerFilletRadius() const;
    bool ValidateOuterFilletRadius() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (HollowRectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (HollowRectangleProfile)

    PROFILES_EXPORT static HollowRectangleProfilePtr Create (CreateParams const& params) { return new HollowRectangleProfile (params); }

public:
    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth (double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double val);

    PROFILES_EXPORT double GetInnerFilletRadius() const;
    PROFILES_EXPORT void SetInnerFilletRadius (double val);

    PROFILES_EXPORT double GetOuterFilletRadius() const;
    PROFILES_EXPORT void SetOuterFilletRadius (double val);

    }; // HollowRectangleProfile

//=======================================================================================
//! Handler for HollowRectangleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HollowRectangleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_HollowRectangleProfile, HollowRectangleProfile, HollowRectangleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // HollowRectangleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
