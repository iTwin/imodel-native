/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/ArbitraryCenterLineProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ArbitraryShapeProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! Arbitrary center-line profile whose 2d geometry is constructed from a single curve
//! or vector of curves defining a center-line.
//! @ingroup GROUP_SinglePerimeterProfiles GROUP_CenterLineProfiles
//=======================================================================================
struct ArbitraryCenterLineProfile : ArbitraryShapeProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_ArbitraryCenterLineProfile, ArbitraryShapeProfile);
    friend struct ArbitraryCenterLineProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (ArbitraryCenterLineProfile)

    public:
        //! Constructor to initialize members.
        //! @details Geometry must be of type IGeometry::GeometryType::CurvePrimitive or IGeometry::GeometryType::CurveVector.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        //! @param[in] geometryPtr CenterLine geometry used to generate the shape for this profile.
        //! @param[in] wallThickness Constant thickness of profile walls.
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, IGeometryPtr const& geometryPtr, double wallThickness);

    public:
        //! Constant thickness of profile walls.
        double wallThickness;
        };

private:
    explicit ArbitraryCenterLineProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual bool _CreateGeometry() override;
    virtual IGeometryPtr _CreateShapeGeometry() const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ArbitraryCenterLineProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (ArbitraryCenterLineProfile)

    PROFILES_EXPORT static ArbitraryCenterLineProfilePtr Create (CreateParams const& params) { return new ArbitraryCenterLineProfile (params); }

    }; // ArbitraryCenterLineProfile

//=======================================================================================
//! Handler for ArbitraryCenterLineProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArbitraryCenterLineProfileHandler : ArbitraryShapeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_ArbitraryCenterLineProfile, ArbitraryCenterLineProfile, ArbitraryCenterLineProfileHandler, ArbitraryShapeProfileHandler, PROFILES_EXPORT)

    }; // ArbitraryCenterLineProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
