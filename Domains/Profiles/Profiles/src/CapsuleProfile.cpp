/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/CapsuleProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>
#include <Profiles\CapsuleProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (CapsuleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CapsuleProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CapsuleProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName, double width, double depth)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , width (width)
    , depth (depth)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CapsuleProfile::CapsuleProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetWidth (params.width);
    SetDepth (params.depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool CapsuleProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isWidthValid = ProfilesProperty::IsGreaterThanZero (GetWidth());
    bool const isDepthValid = ProfilesProperty::IsGreaterThanZero (GetDepth());
    bool const isOfCapsuleShape = !BeNumerical::IsEqual (GetWidth(), GetDepth());

    return isWidthValid && isDepthValid && isOfCapsuleShape;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr CapsuleProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateCapsule (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CapsuleProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CapsuleProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CapsuleProfile::SetWidth (double value)
    {
    SetPropertyValue (PRF_PROP_CapsuleProfile_Width, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CapsuleProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CapsuleProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CapsuleProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_CapsuleProfile_Depth, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
