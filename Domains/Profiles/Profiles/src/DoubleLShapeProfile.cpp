/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/DoubleLShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\DoubleLShapeProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (DoubleLShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleLShapeProfile::CreateParams::CreateParams (DgnModel const& model, Utf8CP pName, double spacing,
                                                 LShapeProfile const& singleProfile, DoubleLShapeProfileType type)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , spacing (spacing)
    , singleProfileId (singleProfile.GetElementId())
    , type (type)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleLShapeProfile::CreateParams::CreateParams (DgnModel const& model, Utf8CP pName, double spacing,
                                                 DgnElementId const& singleProfileId, DoubleLShapeProfileType type)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , spacing (spacing)
    , singleProfileId (singleProfileId)
    , type (type)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleLShapeProfile::DoubleLShapeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetSpacing (params.spacing);
    SetSingleProfile (params.singleProfileId);
    SetType (params.type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DoubleLShapeProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isSpacingValid = ProfilesProperty::IsGreaterOrEqualToZero (GetSpacing());
    bool const isSingleProfileValid = GetSingleProfile().IsValid();
    bool const isTypeValid = ValidateType();

    return isSpacingValid && isSingleProfileValid && isTypeValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr DoubleLShapeProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateDoubleLShape (*this, *GetSingleProfile());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr DoubleLShapeProfile::_UpdateShapeGeometry (Profile const& relatedProfile) const
    {
    if (typeid (relatedProfile) == typeid (LShapeProfile))
        return ProfilesGeometry::CreateDoubleLShape (*this, static_cast<LShapeProfile const&> (relatedProfile));

    BeAssert (false);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DoubleLShapeProfile::ValidateType() const
    {
    switch (GetType())
        {
        case DoubleLShapeProfileType::LLBB:
        case DoubleLShapeProfileType::SLBB:
            return true;
        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double DoubleLShapeProfile::GetSpacing() const
    {
    return GetPropertyValueDouble (PRF_PROP_DoubleLShapeProfile_Spacing);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleLShapeProfile::SetSpacing (double value)
    {
    SetPropertyValue (PRF_PROP_DoubleLShapeProfile_Spacing, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleLShapeProfileType DoubleLShapeProfile::GetType() const
    {
    return static_cast<DoubleLShapeProfileType> (GetPropertyValueInt32 (PRF_PROP_DoubleLShapeProfile_Type));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleLShapeProfile::SetType (DoubleLShapeProfileType value)
    {
    SetPropertyValue (PRF_PROP_DoubleLShapeProfile_Type, ECN::ECValue (static_cast<int> (value)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LShapeProfilePtr DoubleLShapeProfile::GetSingleProfile() const
    {
    DgnElementId singleProfileId = GetPropertyValueId<DgnElementId> (PRF_PROP_DoubleLShapeProfile_SingleProfile);
    return LShapeProfile::GetForEdit (m_dgndb, singleProfileId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleLShapeProfile::SetSingleProfile (LShapeProfile const& singleProfile)
    {
    SetPropertyValue (PRF_PROP_DoubleLShapeProfile_SingleProfile, singleProfile.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleLShapeProfile::SetSingleProfile (DgnElementId const& singleProfileId)
    {
    SetPropertyValue (PRF_PROP_DoubleLShapeProfile_SingleProfile, singleProfileId);
    }

END_BENTLEY_PROFILES_NAMESPACE
