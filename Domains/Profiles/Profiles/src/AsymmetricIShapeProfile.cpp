/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <Profiles\AsymmetricIShapeProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (AsymmetricIShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsymmetricIShapeProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsymmetricIShapeProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName, double topFlangeWidth, double bottomFlangeWidth, double depth,
                                                     double topFlangeThickness, double bottomFlangeThickness, double webThickness, double topFlangeFilletRadius,
                                                     double topFlangeEdgeRadius, Angle const& topFlangeSlope, double bottomFlangeFilletRadius,
                                                     double bottomFlangeEdgeRadius, Angle const& bottomFlangeSlope)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , topFlangeWidth (topFlangeWidth)
    , bottomFlangeWidth (bottomFlangeWidth)
    , depth (depth)
    , topFlangeThickness (topFlangeThickness)
    , bottomFlangeThickness (bottomFlangeThickness)
    , webThickness (webThickness)
    , topFlangeFilletRadius (topFlangeFilletRadius)
    , topFlangeEdgeRadius (topFlangeEdgeRadius)
    , topFlangeSlope (topFlangeSlope)
    , bottomFlangeFilletRadius (bottomFlangeFilletRadius)
    , bottomFlangeEdgeRadius (bottomFlangeEdgeRadius)
    , bottomFlangeSlope (bottomFlangeSlope)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsymmetricIShapeProfile::AsymmetricIShapeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetTopFlangeWidth (params.topFlangeWidth);
    SetBottomFlangeWidth (params.bottomFlangeWidth);
    SetDepth (params.depth);
    SetTopFlangeThickness (params.topFlangeThickness);
    SetBottomFlangeThickness (params.bottomFlangeThickness);
    SetWebThickness (params.webThickness);
    SetTopFlangeFilletRadius (params.topFlangeFilletRadius);
    SetTopFlangeEdgeRadius (params.topFlangeEdgeRadius);
    SetTopFlangeSlope (params.topFlangeSlope);
    SetBottomFlangeFilletRadius (params.bottomFlangeFilletRadius);
    SetBottomFlangeEdgeRadius (params.bottomFlangeEdgeRadius);
    SetBottomFlangeSlope (params.bottomFlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsymmetricIShapeProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isTopFlangeWidthValid = ProfilesProperty::IsGreaterThanZero (GetTopFlangeWidth());
    bool const isBottomFlangeWidthValid = ProfilesProperty::IsGreaterThanZero (GetBottomFlangeWidth());
    bool const isDepthValid = ProfilesProperty::IsGreaterThanZero (GetDepth());
    bool const isTopFlangeThicknessValid = ValidateTopFlangeThickness();
    bool const isBottomFlangeThicknessValid = ValidateBottomFlangeThickness();
    bool const isWebThicknessValid = ValidateWebThickness();
    bool const isTopFlangeFilletRadiusValid = ValidateTopFlangeFilletRadius();
    bool const isTopFlangeEdgeRadiusValid = ValidateTopFlangeEdgeRadius();
    bool const isTopFlangeSlopeValid = ValidateTopFlangeSlope();
    bool const isBottomFlangeFilletRadiusValid = ValidateBottomFlangeFilletRadius();
    bool const isBottomFlangeEdgeRadiusValid = ValidateBottomFlangeEdgeRadius();
    bool const isBottomFlangeSlopeValid = ValidateBottomFlangeSlope();

    return isTopFlangeWidthValid && isBottomFlangeWidthValid && isDepthValid && isTopFlangeThicknessValid && isBottomFlangeThicknessValid
           && isWebThicknessValid && isTopFlangeFilletRadiusValid && isTopFlangeEdgeRadiusValid && isTopFlangeSlopeValid
           && isBottomFlangeFilletRadiusValid && isBottomFlangeEdgeRadiusValid && isBottomFlangeSlopeValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr AsymmetricIShapeProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateAsymmetricIShape (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsymmetricIShapeProfile::ValidateTopFlangeThickness() const
    {
    double const topFlangeThickness = GetTopFlangeThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (topFlangeThickness);
    bool const isLessThanDepth = ProfilesProperty::IsLess (topFlangeThickness + GetBottomFlangeThickness(), GetDepth());

    return isPositive && isLessThanDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsymmetricIShapeProfile::ValidateBottomFlangeThickness() const
    {
    double const bottomFlangeThickness = GetBottomFlangeThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (bottomFlangeThickness);
    bool const isLessThanDepth = ProfilesProperty::IsLess (bottomFlangeThickness + GetTopFlangeThickness(), GetDepth());

    return isPositive && isLessThanDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsymmetricIShapeProfile::ValidateWebThickness() const
    {
    double const webThickness = GetWebThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (webThickness);
    bool const isLessThanTopFlangeWidth = ProfilesProperty::IsLess (webThickness, GetTopFlangeWidth());
    bool const isLessThanBottomFlangeWidth = ProfilesProperty::IsLess (webThickness, GetBottomFlangeWidth());

    return isPositive && isLessThanTopFlangeWidth && isLessThanBottomFlangeWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateFlangeFilletRadius (double filletRadius, double innerFlangeFaceLength, double flangeSlopeHeight, double innerWebFaceLength)
    {
    if (ProfilesProperty::IsEqualToZero (filletRadius))
        return true;

    double const availableWebLength = innerWebFaceLength / 2.0 - flangeSlopeHeight;
    double const availableFlangeLength = innerFlangeFaceLength / 2.0;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (filletRadius);
    bool const isLessThanAvailableWebLength = ProfilesProperty::IsLessOrEqual (filletRadius, availableWebLength);
    bool const isLessThanAvailableFlangeLength = ProfilesProperty::IsLessOrEqual (filletRadius, availableFlangeLength);

    return isPositive && isLessThanAvailableWebLength && isLessThanAvailableFlangeLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateFlangeEdgeRadius (double edgeRadius, double innerFlangeFaceLength, double flangeThickness)
    {
    if (ProfilesProperty::IsEqualToZero (edgeRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (edgeRadius);
    bool const isLessThanFlangeThickness = ProfilesProperty::IsLessOrEqual (edgeRadius, flangeThickness);
    bool const isLessThanAvailableFlangeLength = ProfilesProperty::IsLessOrEqual (edgeRadius, innerFlangeFaceLength / 2.0);

    return isPositive && isLessThanFlangeThickness && isLessThanAvailableFlangeLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateFlangeSlope (Angle const& flangeSlope, double flangeSlopeHeight, double innerWebFaceLength)
    {
    double const angle = flangeSlope.Radians();
    if (ProfilesProperty::IsEqualToZero (angle))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (angle);
    bool const isLessThan90Degrees = ProfilesProperty::IsLess (angle, PI / 2.0);
    bool const slopeHeightIsLessThanAvailableWebLength = ProfilesProperty::IsLessOrEqual (flangeSlopeHeight, innerWebFaceLength / 2.0);

    return isPositive && isLessThan90Degrees && slopeHeightIsLessThanAvailableWebLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsymmetricIShapeProfile::ValidateTopFlangeFilletRadius() const
    {
    return validateFlangeFilletRadius (GetTopFlangeFilletRadius(), GetTopFlangeInnerFaceLength(), GetTopFlangeSlopeHeight(), GetWebInnerFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsymmetricIShapeProfile::ValidateTopFlangeEdgeRadius() const
    {
    return validateFlangeEdgeRadius (GetTopFlangeEdgeRadius(), GetTopFlangeInnerFaceLength(), GetTopFlangeThickness());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsymmetricIShapeProfile::ValidateTopFlangeSlope() const
    {
    return validateFlangeSlope (GetTopFlangeSlope(), GetTopFlangeSlopeHeight(), GetWebInnerFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsymmetricIShapeProfile::ValidateBottomFlangeFilletRadius() const
    {
    return validateFlangeFilletRadius (GetBottomFlangeFilletRadius(), GetBottomFlangeInnerFaceLength(), GetBottomFlangeSlopeHeight(), GetWebInnerFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsymmetricIShapeProfile::ValidateBottomFlangeEdgeRadius() const
    {
    return validateFlangeEdgeRadius (GetBottomFlangeEdgeRadius(), GetBottomFlangeInnerFaceLength(), GetBottomFlangeThickness());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsymmetricIShapeProfile::ValidateBottomFlangeSlope() const
    {
    return validateFlangeSlope (GetBottomFlangeSlope(), GetBottomFlangeSlopeHeight(), GetWebInnerFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_TopFlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeWidth (double value)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_TopFlangeWidth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeWidth (double value)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeWidth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_Depth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_TopFlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeThickness (double value)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_TopFlangeThickness, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeThickness (double value)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeThickness, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetWebThickness (double value)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_WebThickness, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_TopFlangeFilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_TopFlangeFilletRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_TopFlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeEdgeRadius (double value)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_TopFlangeEdgeRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Angle AsymmetricIShapeProfile::GetTopFlangeSlope() const
    {
    return Angle::FromRadians (GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_TopFlangeSlope));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeSlope (Angle const& value)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_TopFlangeSlope, ECN::ECValue (value.Radians()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeFilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeFilletRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeEdgeRadius (double value)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeEdgeRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Angle AsymmetricIShapeProfile::GetBottomFlangeSlope() const
    {
    return Angle::FromRadians (GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeSlope));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeSlope (Angle const& value)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeSlope, ECN::ECValue (value.Radians()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeInnerFaceLength() const
    {
    return (GetTopFlangeWidth() - GetWebThickness()) / 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeInnerFaceLength() const
    {
    return (GetBottomFlangeWidth() - GetWebThickness()) / 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetWebInnerFaceLength() const
    {
    return GetDepth() - GetTopFlangeThickness() - GetBottomFlangeThickness();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeSlopeHeight() const
    {
    double const topFlangeSlopeCos = GetTopFlangeSlope().Cos();
    if (BeNumerical::IsLessOrEqualToZero (topFlangeSlopeCos))
        return 0.0;

    return (GetTopFlangeInnerFaceLength() / topFlangeSlopeCos) * GetTopFlangeSlope().Sin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeSlopeHeight() const
    {
    double const bottomFlangeSlopeCos = GetBottomFlangeSlope().Cos();
    if (BeNumerical::IsLessOrEqualToZero (bottomFlangeSlopeCos))
        return 0.0;

    return (GetBottomFlangeInnerFaceLength() / bottomFlangeSlopeCos) * GetBottomFlangeSlope().Sin();
    }

END_BENTLEY_PROFILES_NAMESPACE
