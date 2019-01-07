/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/MaterialProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

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
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_MaterialProfile, MaterialProfileDefinition);
    friend struct MaterialProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(MaterialProfileDefinition::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, 
            Dgn::DgnElementId const& profileId = Dgn::DgnElementId(), Dgn::DgnElementId const& materialId = Dgn::DgnElementId());

    public:
        Dgn::DgnElementId profileId;
        Dgn::DgnElementId materialId;
        };

protected:
    explicit MaterialProfile (CreateParams const& params);

    virtual bool _Validate() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(MaterialProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(MaterialProfile)

    PROFILES_EXPORT static MaterialProfilePtr Create (CreateParams const& params) { return new MaterialProfile (params); }

    PROFILES_EXPORT ProfileCPtr GetProfile() const;
    PROFILES_EXPORT void SetProfile (Dgn::DgnElementId const& profileId);

    PROFILES_EXPORT Dgn::PhysicalMaterialCPtr GetMaterial() const;
    PROFILES_EXPORT void SetMaterial (Dgn::DgnElementId const& materialId);
    }; // MaterialProfile

   //=======================================================================================
   //! Handler for MaterialProfile class
   //! @ingroup GROUP_Profiles
   //! @private
   //=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE MaterialProfileHandler : MaterialProfileDefinitionHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_MaterialProfile, MaterialProfile, MaterialProfileHandler, MaterialProfileDefinitionHandler, PROFILES_EXPORT)
    }; // MaterialProfileHandler

END_BENTLEY_PROFILES_NAMESPACE