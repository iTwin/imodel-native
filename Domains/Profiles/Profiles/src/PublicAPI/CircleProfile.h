/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct CircleProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CircleProfile, ParametricProfile);
    friend struct CircleProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (CircleProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double radius);

    public:
        //! @beginGroup
        double radius = 0.0; //!< The radius of the circle.
        //! @endGroup
        };

protected:
    explicit CircleProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateShapeGeometry() const override; //!< @private

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CircleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CircleProfile)

    //! Creates an instance of CircleProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of CircleProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static CircleProfilePtr Create (CreateParams const& params) { return new CircleProfile (params); }

    //! @beginGroup
    PROFILES_EXPORT double GetRadius() const; //!< Get the value of @ref CreateParams.radius "Radius"
    PROFILES_EXPORT void SetRadius (double value); //!< Set the value for @ref CreateParams.radius "Radius"
    //! @endGroup

    }; // CircleProfile

//=======================================================================================
//! Handler for CircleProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CircleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_CircleProfile, CircleProfile, CircleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // CircleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
