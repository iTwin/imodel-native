/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/HollowCircleProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <Profiles\HollowCircleProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (HollowCircleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowCircleProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowCircleProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName, double radius, double wallThickness)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , radius (radius)
    , wallThickness (wallThickness)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowCircleProfile::HollowCircleProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetRadius (params.radius);
    SetWallThickness (params.wallThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool HollowCircleProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isRadiusValid = ProfilesProperty::IsGreaterThanZero (GetRadius());
    bool const isWallThicknessValid = ValidateWallThickness();

    return isRadiusValid && isWallThicknessValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr HollowCircleProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateHollowCircle (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool HollowCircleProfile::ValidateWallThickness() const
    {
    double const wallThickness = GetWallThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (wallThickness);
    bool const isLessThanRadius = ProfilesProperty::IsLess (wallThickness, GetRadius());

    return isPositive && isLessThanRadius;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowCircleProfile::GetRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_HollowCircleProfile_Radius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowCircleProfile::SetRadius (double value)
    {
    SetPropertyValue (PRF_PROP_HollowCircleProfile_Radius, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
