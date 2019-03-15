/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/DoubleCShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <Profiles\DoubleCShapeProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (DoubleCShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleCShapeProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName, double spacing, CShapeProfile const& singleProfile)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , spacing (spacing)
    , singleProfileId (singleProfile.GetElementId())
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleCShapeProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName, double spacing, DgnElementId const& singleProfileId)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , spacing (spacing)
    , singleProfileId (singleProfileId)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleCShapeProfile::DoubleCShapeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetSpacing (params.spacing);
    SetSingleProfile (params.singleProfileId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DoubleCShapeProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isSpacingValid = ProfilesProperty::IsGreaterOrEqualToZero (GetSpacing());
    bool const isSingleProfileValid = GetSingleProfile().IsValid();

    return isSpacingValid && isSingleProfileValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr DoubleCShapeProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateDoubleCShape (*this, *GetSingleProfile());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr DoubleCShapeProfile::_UpdateShapeGeometry (Profile const& relatedProfile) const
    {
    if (typeid (relatedProfile) == typeid (CShapeProfile))
        return ProfilesGeometry::CreateDoubleCShape (*this, static_cast<CShapeProfile const&> (relatedProfile));

    BeAssert (false);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double DoubleCShapeProfile::GetSpacing() const
    {
    return GetPropertyValueDouble (PRF_PROP_DoubleCShapeProfile_Spacing);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleCShapeProfile::SetSpacing (double value)
    {
    SetPropertyValue (PRF_PROP_DoubleCShapeProfile_Spacing, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CShapeProfilePtr DoubleCShapeProfile::GetSingleProfile() const
    {
    DgnElementId singleProfileId = GetPropertyValueId<DgnElementId> (PRF_PROP_DoubleCShapeProfile_SingleProfile);
    return CShapeProfile::GetForEdit (m_dgndb, singleProfileId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleCShapeProfile::SetSingleProfile (CShapeProfile const& singleProfile)
    {
    SetPropertyValue (PRF_PROP_DoubleCShapeProfile_SingleProfile, singleProfile.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleCShapeProfile::SetSingleProfile (DgnElementId const& singleProfileId)
    {
    SetPropertyValue (PRF_PROP_DoubleCShapeProfile_SingleProfile, singleProfileId);
    }

END_BENTLEY_PROFILES_NAMESPACE
