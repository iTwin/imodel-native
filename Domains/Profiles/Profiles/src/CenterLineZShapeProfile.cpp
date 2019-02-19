/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/CenterLineZShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesProperty.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <Profiles\CenterLineZShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (CenterLineZShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineZShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineZShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth,
                                                     double wallThickness, double filletRadius, double girth)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
      , flangeWidth (flangeWidth)
      , depth (depth)
      , wallThickness (wallThickness)
      , filletRadius (filletRadius)
      , girth (girth)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineZShapeProfile::CenterLineZShapeProfile (CreateParams const& params)
     : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetFlangeWidth (params.flangeWidth);
    SetDepth (params.depth);
    SetWallThickness (params.wallThickness);
    SetFilletRadius (params.filletRadius);
    SetGirth (params.girth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool CenterLineZShapeProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isFlangeWidthValid = ProfilesProperty::IsGreaterThanZero (GetFlangeWidth());
    bool const isDepthValid = ProfilesProperty::IsGreaterThanZero (GetDepth());
    bool const isWallThicknessValid = ValidateWallThickness();
    bool const isFilletRadiusValid = ValidateFilletRadius();
    bool const isGirthValid = ProfilesProperty::IsGreaterOrEqualToZero (GetGirth());

    return isFlangeWidthValid && isDepthValid && isWallThicknessValid && isFilletRadiusValid && isGirthValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool CenterLineZShapeProfile::ValidateWallThickness() const
    {
    double const wallThickness = GetWallThickness();
    double const availableFlangeWidth = ProfilesProperty::IsGreaterThanZero (GetGirth()) ? GetFlangeWidth() / 2.0 : GetFlangeWidth();

    bool const isPositive = ProfilesProperty::IsGreaterThanZero (wallThickness);
    bool const isLessThanHalfDepth = ProfilesProperty::IsLess (wallThickness, GetDepth() / 2.0);
    bool const fitsInFlange = ProfilesProperty::IsLess (wallThickness, availableFlangeWidth);

    return isPositive && isLessThanHalfDepth && fitsInFlange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool CenterLineZShapeProfile::ValidateFilletRadius() const
    {
    double const filletRadius = GetFilletRadius();
    if (ProfilesProperty::IsEqualToZero (filletRadius))
        return true;

    double availableFlangeLength = 0.0;
    if (ProfilesProperty::IsGreaterThanZero (GetGirth()))
        availableFlangeLength = (GetFlangeWidth() - GetWallThickness() * 2.0) / 2.0;
    else
        availableFlangeLength = GetFlangeWidth() - GetWallThickness();

    double const availableWebLength = (GetDepth() - GetWallThickness() * 2.0) / 2.0;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (filletRadius);
    bool const isLessThanAvailableFlangeLength = ProfilesProperty::IsLessOrEqual (filletRadius, availableFlangeLength);
    bool const isLessThanAvailableWebLength = ProfilesProperty::IsLessOrEqual (filletRadius, availableWebLength);

    bool isLessThanInnerGirthLength = true;
    if (ProfilesProperty::IsGreaterThanZero (GetGirth()))
        isLessThanInnerGirthLength = ProfilesProperty::IsLessOrEqual (filletRadius, GetGirth() - GetWallThickness());

    return isPositive && isLessThanAvailableFlangeLength && isLessThanAvailableWebLength && isLessThanInnerGirthLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool CenterLineZShapeProfile::_CreateGeometry()
    {
    IGeometryPtr centerLineGeometryPtr = ProfilesGeometry::CreateCenterLineForZShape (*this);
    if (centerLineGeometryPtr.IsNull())
        return false;
    SetCenterLine (*centerLineGeometryPtr);

    return T_Super::_CreateGeometry();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr CenterLineZShapeProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateCenterLineZShape (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineZShapeProfile::GetFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineZShapeProfile_FlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineZShapeProfile::SetFlangeWidth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineZShapeProfile_FlangeWidth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineZShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineZShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineZShapeProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineZShapeProfile_Depth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineZShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineZShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineZShapeProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineZShapeProfile_FilletRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineZShapeProfile::GetGirth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineZShapeProfile_Girth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineZShapeProfile::SetGirth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineZShapeProfile_Girth, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
