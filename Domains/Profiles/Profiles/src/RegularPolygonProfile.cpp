/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/RegularPolygonProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <ProfilesInternal\ProfilesProperty.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <Profiles\RegularPolygonProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (RegularPolygonProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
RegularPolygonProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
RegularPolygonProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, uint64_t sideCount, double sideLength)
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
IGeometryPtr RegularPolygonProfile::_CreateGeometry() const
    {
    return ProfilesGeometry::CreateRegularPolygon (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t RegularPolygonProfile::GetSideCount() const
    {
    return GetPropertyValueUInt64 (PRF_PROP_RegularPolygonProfile_SideCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RegularPolygonProfile::SetSideCount (uint64_t value)
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
