/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/BentPlateProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! Profile of a plate that has been bent at a given point with a given angle.
//! @ingroup GROUP_ParametricProfiles GROUP_CenterLineProfiles
//=======================================================================================
struct BentPlateProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_BentPlateProfile, ParametricProfile);
    friend struct BentPlateProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (BentPlateProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double width, double wallThickness, Angle const& angle,
                                               double bendOffset, double filletRadius = 0.0);

    public:
        //! @beginGroup
        //! Extent of the plate length, defined parallel to the x axis of the position coordinate system.
        double width = 0.0;
        //! Constant thickness of profile walls.
        double wallThickness = 0.0;
        //! Inner angle of the bend point. @details Value is valid in range (0..180) degrees.
        //! Values near 180 degrees meaning an almost unbent plate and values near 0 are almost fully bent in half plate.
        Angle bendAngle = Angle::FromRadians (0.0);
        //! Offset from the left edge of the plate defining at what point the plate is bent.
        double bendOffset = 0.0;
        //! Inner fillet radius of the bend point.
        double filletRadius = 0.0;
        //! @endGroup
        };

private:
    explicit BentPlateProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual bool _CreateGeometry() override;
    virtual IGeometryPtr _CreateShapeGeometry() const override;

    bool ValidateWallThickness() const;
    bool ValidateBendAngle() const;
    bool ValidateBendOffset() const;
    bool ValidateFilletRadius() const;

    double CalculateMaxWallThickness() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (BentPlateProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (BentPlateProfile)

    //! Creates an instance of BentPlateProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of BentPlateProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static BentPlateProfilePtr Create (CreateParams const& params) { return new BentPlateProfile (params); }

    PROFILES_EXPORT double GetWidth() const; //!< Get the value of @ref CreateParams.width "Width"
    PROFILES_EXPORT void SetWidth (double value); //!< Set the value for @ref CreateParams.width "Width"

    PROFILES_EXPORT Angle GetBendAngle() const; //!< Get the value of @ref CreateParams.bendAngle "BendAngle"
    PROFILES_EXPORT void SetBendAngle (Angle const& value); //!< Set the value for @ref CreateParams.bendAngle "BendAngle"

    PROFILES_EXPORT double GetFilletRadius() const; //!< Get the value of @ref CreateParams.filletRadius "FilletRadius"
    PROFILES_EXPORT void SetFilletRadius (double value); //!< Set the value for @ref CreateParams.filletRadius "FilletRadius"

    PROFILES_EXPORT double GetBendOffset() const; //!< Get the value of @ref CreateParams.bendOffset "BendOffset"
    PROFILES_EXPORT void SetBendOffset (double value); //!< Set the value for @ref CreateParams.bendOffset "BendOffset"

    }; // BentPlateProfile

//=======================================================================================
//! Handler for BentPlateProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BentPlateProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_BentPlateProfile, BentPlateProfile, BentPlateProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // BentPlateProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
