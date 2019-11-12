/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesProperty.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <Profiles\RegularPolygonProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (RegularPolygonProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
RegularPolygonProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
RegularPolygonProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName, int32_t sideCount, double sideLength)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , sideCount (sideCount)
    , sideLength (sideLength)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
RegularPolygonProfile::RegularPolygonProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetSideCount (params.sideCount);
    SetSideLength (params.sideLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegularPolygonProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isSideCountValid = ProfilesProperty::IsGreaterOrEqual (GetSideCount(), 3) && ProfilesProperty::IsLessOrEqual (GetSideCount(), 32);
    bool const isSideLengthValid = ProfilesProperty::IsGreaterThanZero (GetSideLength());

    return isSideCountValid && isSideLengthValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr RegularPolygonProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateRegularPolygon (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t RegularPolygonProfile::GetSideCount() const
    {
    return GetPropertyValueInt32 (PRF_PROP_RegularPolygonProfile_SideCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RegularPolygonProfile::SetSideCount (int32_t value)
    {
    SetPropertyValue (PRF_PROP_RegularPolygonProfile_SideCount, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double RegularPolygonProfile::GetSideLength() const
    {
    return GetPropertyValueDouble (PRF_PROP_RegularPolygonProfile_SideLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RegularPolygonProfile::SetSideLength (double value)
    {
    SetPropertyValue (PRF_PROP_RegularPolygonProfile_SideLength, ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
