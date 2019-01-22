/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/DerivedProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\DerivedProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (DerivedProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DerivedProfile::CreateParams::CreateParams (DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , offset (DPoint2d::From (0.0, 0.0))
    , scale (DPoint2d::From (1.0, 1.0))
    , rotation (Angle::FromRadians (0.0))
    , mirrorAboutYAxis (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DerivedProfile::CreateParams::CreateParams (DgnModel const& model, Utf8CP pName, DPoint2d const& offset, DPoint2d const& scale,
                                            Angle const& rotation, bool mirrorAboutYAxis)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , offset (offset)
    , scale (scale)
    , rotation (rotation)
    , mirrorAboutYAxis (mirrorAboutYAxis)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DerivedProfile::DerivedProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetOffset (params.offset);
    SetScale (params.scale);
    SetRotation (params.rotation);
    SetMirrorAboutYAxis (params.mirrorAboutYAxis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DerivedProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isBaseProfileValid = GetBaseProfile().IsValid();
    bool const isOffsetXValid = BeNumerical::BeFinite (GetOffset().x);
    bool const isOffsetYValid = BeNumerical::BeFinite (GetOffset().y);
    bool const isScaleXValid = BeNumerical::BeFinite (GetScale().x);
    bool const isScaleYValid = BeNumerical::BeFinite (GetScale().y);
    bool const isRotationValid = BeNumerical::BeFinite (GetRotation().Radians());

    return isBaseProfileValid && isOffsetXValid && isOffsetYValid && isScaleXValid && isScaleYValid && isRotationValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr DerivedProfile::_CreateGeometry() const
    {
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SinglePerimeterProfilePtr DerivedProfile::GetBaseProfile() const
    {
    DgnElementId baseProfileId = GetPropertyValueId<DgnElementId> (PRF_PROP_DerivedProfile_BaseProfile);
    return SinglePerimeterProfile::GetForEdit (m_dgndb, baseProfileId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DerivedProfile::GetBaseProfileId() const
    {
    return GetPropertyValueId<DgnElementId> (PRF_PROP_DerivedProfile_BaseProfile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DerivedProfile::SetBaseProfile (SinglePerimeterProfile const& baseProfile)
    {
    SetPropertyValue (PRF_PROP_DerivedProfile_BaseProfile, baseProfile.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DerivedProfile::SetBaseProfile (Dgn::DgnElementId const& baseProfileId)
    {
    SetPropertyValue (PRF_PROP_DerivedProfile_BaseProfile, baseProfileId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d DerivedProfile::GetOffset() const
    {
    return GetPropertyValueDPoint2d (PRF_PROP_DerivedProfile_Offset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DerivedProfile::SetOffset (DPoint2d const& value)
    {
    SetPropertyValue (PRF_PROP_DerivedProfile_Offset, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Angle DerivedProfile::GetRotation() const
    {
    return Angle::FromRadians (GetPropertyValueDouble (PRF_PROP_DerivedProfile_Rotation));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DerivedProfile::SetRotation (Angle const& value)
    {
    SetPropertyValue (PRF_PROP_DerivedProfile_Rotation, ECValue (value.Radians()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d DerivedProfile::GetScale() const
    {
    return GetPropertyValueDPoint2d (PRF_PROP_DerivedProfile_Scale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DerivedProfile::SetScale (DPoint2d const& value)
    {
    SetPropertyValue (PRF_PROP_DerivedProfile_Scale, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool DerivedProfile::GetMirrorAboutYAxis() const
    {
    return GetPropertyValueBoolean (PRF_PROP_DerivedProfile_MirrorAboutYAxis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DerivedProfile::SetMirrorAboutYAxis (bool value)
    {
    SetPropertyValue (PRF_PROP_DerivedProfile_MirrorAboutYAxis, ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
