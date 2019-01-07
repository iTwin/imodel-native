/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/MaterialProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\MaterialProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialProfile::MaterialProfile (CreateParams const& params) : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetMaterial (params.materialId);
    SetProfile (params.profileId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, DgnElementId const& profileId, DgnElementId const& materialId)
    : T_Super (model.GetDgnDb(), model.GetModelId(), QueryClassId (model.GetDgnDb())), profileId (profileId), materialId (materialId)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialProfile::SetMaterial (DgnElementId const& materialId)
    {
    SetPropertyValue (PRF_PROP_MaterialProfile_Material, materialId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalMaterialCPtr MaterialProfile::GetMaterial() const
    {
    DgnElementId materialId = GetNavigationPropertyInfo (PRF_PROP_MaterialProfile_Material).GetId<DgnElementId>();
    return m_dgndb.Elements().Get<PhysicalMaterial> (materialId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialProfile::SetProfile (DgnElementId const& profileId)
    {
    SetPropertyValue(PRF_PROP_MaterialProfile_Profile, profileId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialProfile::_Validate() const
    {
    bool const isProfileValid = GetProfile().IsValid();
    bool const isMaterialValid = GetMaterial().IsValid();

    return isProfileValid && isMaterialValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ProfileCPtr MaterialProfile::GetProfile() const
    {
    DgnElementId profileId = GetNavigationPropertyInfo (PRF_PROP_MaterialProfile_Profile).GetId<DgnElementId>();
    return m_dgndb.Elements().Get<Profile> (profileId);
    }

HANDLER_DEFINE_MEMBERS(MaterialProfileHandler)

END_BENTLEY_PROFILES_NAMESPACE
