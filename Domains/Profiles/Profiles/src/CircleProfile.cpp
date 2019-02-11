/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/CircleProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <Profiles\CircleProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (CircleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CircleProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CircleProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double radius)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , radius (radius)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CircleProfile::CircleProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetRadius (params.radius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CircleProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    return ProfilesProperty::IsGreaterThanZero (GetRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr CircleProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateCircle (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CircleProfile::GetRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_CircleProfile_Radius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CircleProfile::SetRadius (double value)
    {
    SetPropertyValue (PRF_PROP_CircleProfile_Radius, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
