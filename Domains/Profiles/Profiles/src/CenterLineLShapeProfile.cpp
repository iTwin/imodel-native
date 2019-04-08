/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/CenterLineLShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <Profiles\CenterLineLShapeProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (CenterLineLShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineLShapeProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineLShapeProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName, double width, double depth,
                                                     double wallThickness, double girth, double filletRadius)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , width (width)
    , depth (depth)
    , wallThickness (wallThickness)
    , girth (girth)
    , filletRadius (filletRadius)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineLShapeProfile::CenterLineLShapeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetWidth (params.width);
    SetDepth (params.depth);
    SetGirth (params.girth);
    SetFilletRadius (params.filletRadius);
    SetWallThickness (params.wallThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CenterLineLShapeProfile::_Validate() const
    {
    bool isValid (T_Super::_Validate());

    double const width = GetWidth();
    double const depth = GetDepth();
    double const wallThickness = GetWallThickness();
    double const girth = GetGirth();
    double const filletRadius = GetFilletRadius();

    isValid = isValid && BeNumerical::BeFinite (width) && BeNumerical::IsGreaterThanZero (width);
    isValid = isValid && BeNumerical::BeFinite (depth) && BeNumerical::IsGreaterThanZero (depth);
    isValid = isValid && BeNumerical::BeFinite (girth) && BeNumerical::IsGreaterOrEqualToZero (girth);
    isValid = isValid && BeNumerical::BeFinite (wallThickness) && BeNumerical::IsGreaterThanZero (wallThickness);
    isValid = isValid && BeNumerical::BeFinite (filletRadius) && BeNumerical::IsGreaterOrEqualToZero (filletRadius);

    if (isValid && BeNumerical::IsGreaterThanZero (girth))
        {
        isValid = isValid && ProfilesProperty::IsGreater (girth, wallThickness);
        isValid = isValid && ProfilesProperty::IsLess (girth, depth - wallThickness);
        }

    isValid = isValid && ProfilesProperty::IsLess (wallThickness, width / 2.0) && ProfilesProperty::IsLess (wallThickness, depth / 2.0);

    if (isValid  && BeNumerical::IsGreaterThanZero (filletRadius))
        {
        isValid = isValid && ProfilesProperty::IsLessOrEqual (filletRadius, width / 2.0 - wallThickness)
                && ProfilesProperty::IsLessOrEqual (filletRadius, depth / 2.0 - wallThickness);
        }

    return isValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineLShapeProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineLShapeProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineLShapeProfile::SetWidth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineLShapeProfile_Width, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineLShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineLShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineLShapeProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineLShapeProfile_Depth, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineLShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineLShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineLShapeProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineLShapeProfile_FilletRadius, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineLShapeProfile::GetGirth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineLShapeProfile_Girth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineLShapeProfile::SetGirth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineLShapeProfile_Girth, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool CenterLineLShapeProfile::_CreateGeometry()
    {
    if (!T_Super::_CreateGeometry())
        return false;

    IGeometryPtr centerLineGeometryPtr = ProfilesGeometry::CreateCenterLineForLShape (*this);
    if (centerLineGeometryPtr.IsNull())
        return false;

    SetCenterLine (*centerLineGeometryPtr);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr CenterLineLShapeProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateCenterLineLShape (*this);
    }

END_BENTLEY_PROFILES_NAMESPACE
