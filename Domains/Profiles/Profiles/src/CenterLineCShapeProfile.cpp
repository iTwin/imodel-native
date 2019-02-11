/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/CenterLineCShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <Profiles\CenterLineCShapeProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (CenterLineCShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineCShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineCShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double wallThickness, double girth /*= 0.0*/, double filletRadius /*= 0.0*/)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
      , flangeWidth (flangeWidth)
      , depth (depth)
      , wallThickness (wallThickness)
      , girth (girth)
      , filletRadius (filletRadius)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineCShapeProfile::CenterLineCShapeProfile (CreateParams const& params)
     : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetFlangeWidth (params.flangeWidth);
    SetDepth (params.depth);
    SetGirth (params.girth);
    SetFilletRadius (params.filletRadius);
    SetWallThickness (params.wallThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CenterLineCShapeProfile::_Validate() const
    {
    bool isValid (T_Super::_Validate());

    double const flangeWidth = GetFlangeWidth();
    double const filletRadius = GetFilletRadius();
    double const depth = GetDepth();
    double const girth = GetGirth();
    double const wallThickness = GetWallThickness();

    isValid  = isValid  && BeNumerical::BeFinite (flangeWidth) && BeNumerical::IsGreaterThanZero (flangeWidth);
    isValid  = isValid  && BeNumerical::BeFinite (depth) && BeNumerical::IsGreaterThanZero (depth);
    isValid  = isValid  && BeNumerical::BeFinite (girth) && BeNumerical::IsGreaterOrEqualToZero (girth);
    isValid  = isValid  && BeNumerical::BeFinite (wallThickness) && BeNumerical::IsGreaterThanZero (wallThickness);
    isValid  = isValid  && BeNumerical::BeFinite (filletRadius) && BeNumerical::IsGreaterOrEqualToZero (filletRadius);

    if (BeNumerical::IsGreaterThanZero (girth))
        {
        isValid  = isValid  && ProfilesProperty::IsGreaterOrEqual (girth, wallThickness + filletRadius);
        isValid  = isValid  && ProfilesProperty::IsLess (girth, depth / 2.0);
        }

    isValid  = isValid  && (ProfilesProperty::IsLess (wallThickness, flangeWidth / 2.0))
                        && (ProfilesProperty::IsLess (wallThickness, depth / 2.0));

    if (isValid  && BeNumerical::IsGreaterThanZero (filletRadius))
        {
        isValid  = isValid  && ProfilesProperty::IsLessOrEqual (filletRadius, flangeWidth / 2.0 - wallThickness) &&
            ProfilesProperty::IsLessOrEqual (filletRadius, depth / 2.0 - wallThickness);
        }

    return isValid ;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineCShapeProfile::GetFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineCShapeProfile_FlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineCShapeProfile::SetFlangeWidth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineCShapeProfile_FlangeWidth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineCShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineCShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineCShapeProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineCShapeProfile_Depth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineCShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineCShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineCShapeProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineCShapeProfile_FilletRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineCShapeProfile::GetGirth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineCShapeProfile_Girth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineCShapeProfile::SetGirth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineCShapeProfile_Girth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool CenterLineCShapeProfile::_CreateGeometry()
    {
    if (!T_Super::_CreateGeometry())
        return false;

    IGeometryPtr centerLineGeometryPtr = CreateCenterLineGeometry();

    if (centerLineGeometryPtr.IsNull())
        return false;

    SetCenterLine (*centerLineGeometryPtr);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr CenterLineCShapeProfile::CreateCenterLineGeometry()
    {
    return ProfilesGeometry::CreateCenterLineForCShape (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr CenterLineCShapeProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateCenterLineCShape (*this);
    }

END_BENTLEY_PROFILES_NAMESPACE