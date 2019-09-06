/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct HollowCircleProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_HollowCircleProfile, ParametricProfile);
    friend struct HollowCircleProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (HollowCircleProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double radius, double wallThickness);

    public:
        //! @beginGroup
        double radius = 0.0; //!< The radius of the circle.
        double wallThickness = 0.0; //!< Constant thickness of profile walls.
        //! @endGroup
        };

protected:
    explicit HollowCircleProfile (CreateParams const& params);

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateShapeGeometry() const override; //!< @private

private:
    bool ValidateWallThickness() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (HollowCircleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (HollowCircleProfile)

    //! Creates an instance of HollowCircleProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of HollowCircleProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static HollowCircleProfilePtr Create (CreateParams const& params) { return new HollowCircleProfile (params); }

    //! @beginGroup
    PROFILES_EXPORT double GetRadius() const; //!< Get the value of @ref CreateParams.radius "Radius"
    PROFILES_EXPORT void SetRadius (double value); //!< Set the value for @ref CreateParams.radius "Radius"
    //! @endGroup

    }; // HollowCircleProfile

//=======================================================================================
//! Handler for HollowCircleProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HollowCircleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_HollowCircleProfile, HollowCircleProfile, HollowCircleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // HollowCircleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
