/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/EllipseProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <Profiles\EllipseProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (EllipseProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
EllipseProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
EllipseProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName, double xRadius, double yRadius)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , xRadius (xRadius)
    , yRadius (yRadius)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
EllipseProfile::EllipseProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetXRadius (params.xRadius);
    SetYRadius (params.yRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool EllipseProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isXRadiusValid = ProfilesProperty::IsGreaterThanZero (GetXRadius());
    bool const isYRadiusValid = ProfilesProperty::IsGreaterThanZero (GetYRadius());

    return isXRadiusValid && isYRadiusValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr EllipseProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateEllipse (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double EllipseProfile::GetXRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_EllipseProfile_XRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void EllipseProfile::SetXRadius (double value)
    {
    SetPropertyValue (PRF_PROP_EllipseProfile_XRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double EllipseProfile::GetYRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_EllipseProfile_YRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void EllipseProfile::SetYRadius (double value)
    {
    SetPropertyValue (PRF_PROP_EllipseProfile_YRadius, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
