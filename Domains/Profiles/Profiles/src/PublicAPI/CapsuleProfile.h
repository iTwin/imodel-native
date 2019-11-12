/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct CapsuleProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CapsuleProfile, ParametricProfile);
    friend struct CapsuleProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (CapsuleProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double width, double depth);

    public:
        //! @beginGroup
        double width = 0.0; //!< Extent of the capsule in the direction of the x-axis.
        double depth = 0.0; //!< Extent of the capsule in the direction of the y-axis.
        //! @endGroup
        };

protected:
    explicit CapsuleProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateShapeGeometry() const override; //!< @private

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CapsuleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CapsuleProfile)

    //! Creates an instance of CapsuleProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of CapsuleProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static CapsuleProfilePtr Create (CreateParams const& params) { return new CapsuleProfile (params); }

    //! @beginGroup
    PROFILES_EXPORT double GetWidth() const; //!< Get the value of @ref CreateParams.width "Width"
    PROFILES_EXPORT void SetWidth (double value); //!< Set the value for @ref CreateParams.width "Width"

    PROFILES_EXPORT double GetDepth() const; //!< Get the value of @ref CreateParams.depth "Depth"
    PROFILES_EXPORT void SetDepth (double value); //!< Set the value for @ref CreateParams.depth "Depth"
    //! @endGroup

    }; // CapsuleProfile

//=======================================================================================
//! Handler for CapsuleProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CapsuleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_CapsuleProfile, CapsuleProfile, CapsuleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // CapsuleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
