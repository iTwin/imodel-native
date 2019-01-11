/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/DoubleCShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\DoubleCShapeProfile.h>
#include <ProfilesInternal\ProfilesProperty.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (DoubleCShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleCShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double spacing, DgnElementId const& singleProfileId)
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
void DoubleCShapeProfile::SetSingleProfile (DgnElementId const& singleProfileId)
    {
    SetPropertyValue (PRF_PROP_DoubleCShapeProfile_SingleProfile, singleProfileId);
    }

END_BENTLEY_PROFILES_NAMESPACE
