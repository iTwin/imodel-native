/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "MaterialProfileDefinition.h"
#include "Profile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A resource defining Material and Profile pair.
//! @ingroup GROUP_MaterialProfiles
//=======================================================================================
struct MaterialProfile : MaterialProfileDefinition
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_MaterialProfile, MaterialProfileDefinition);
    friend struct MaterialProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (MaterialProfile)

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model,
            Dgn::DgnElementId const& profileId = Dgn::DgnElementId(), Dgn::DgnElementId const& materialId = Dgn::DgnElementId());

    public:
        Dgn::DgnElementId profileId;
        Dgn::DgnElementId materialId;
        };

protected:
    explicit MaterialProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const;

    PROFILES_EXPORT virtual void _OnUpdateFinished() const override; //!< @private

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (MaterialProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (MaterialProfile)

    //! Creates an instance of MaterialProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of MaterialProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static MaterialProfilePtr Create (CreateParams const& params) { return new MaterialProfile (params); }

    PROFILES_EXPORT ProfileCPtr GetProfile() const; //!< Get the value of @ref CreateParams.profile "Profile"
    PROFILES_EXPORT void SetProfile (Dgn::DgnElementId const& profileId); //!< Set the value for @ref CreateParams.profile "Profile"

    PROFILES_EXPORT Dgn::PhysicalMaterialCPtr GetMaterial() const; //!< Get the value of @ref CreateParams.material "Material"
    PROFILES_EXPORT void SetMaterial (Dgn::DgnElementId const& materialId); //!< Set the value for @ref CreateParams.material "Material"
    }; // MaterialProfile

//=======================================================================================
//! Handler for MaterialProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE MaterialProfileHandler : MaterialProfileDefinitionHandler, DependencyUpdateNotifier
    {
    friend struct MaterialProfile;

    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_MaterialProfile, MaterialProfile, MaterialProfileHandler, MaterialProfileDefinitionHandler, PROFILES_EXPORT)
    }; // MaterialProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
