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
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @details Geometry must be of type IGeometry::GeometryType::CurvePrimitive or IGeometry::GeometryType::CurveVector.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        //! @param[in] geometryPtr CenterLine geometry used to generate the shape for this profile.
        //! @param[in] wallThickness Constant thickness of profile walls.
        //! @param[in] arcAngle (If this is positive) turns larger than this become arcs.
        //! @param[in] chamferAngle (If this is positive) "outer chamfers" are created with this max angle.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, IGeometryPtr const& geometryPtr, double wallThickness, Angle const& arcAngle = Angle::FromRadians(-1.0), Angle const& chamferAngle = Angle::AnglePiOver2());

    public:
        //! Constant thickness of profile walls.
        double wallThickness = 0.0;
        //! (If this is positive) turns larger than this become arcs.
        Angle arcAngle = Angle::FromRadians (-1.0);
        //! (If this is positive) "outer chamfers" are created with this max angle.
        Angle chamferAngle = Angle::AnglePiOver2();
        };

private:
    explicit ArbitraryCenterLineProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual bool _CreateGeometry() override;
    virtual IGeometryPtr _CreateShapeGeometry() const override;

    virtual void _CopyFrom (Dgn::DgnElement const& source, CopyFromOptions const& opts) override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ArbitraryCenterLineProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (ArbitraryCenterLineProfile)

    PROFILES_EXPORT static ArbitraryCenterLineProfilePtr Create (CreateParams const& params) { return new ArbitraryCenterLineProfile (params); }

private:
    Angle m_arcAngle;
    Angle m_chamferAngle;
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
