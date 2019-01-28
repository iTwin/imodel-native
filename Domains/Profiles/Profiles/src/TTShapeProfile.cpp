/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/TTShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <ProfilesInternal\ProfilesProperty.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <Profiles\TTShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (TTShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TTShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TTShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness,
                                           double webThickness, double webSpacing, double filletRadius, double flangeEdgeRadius,
                                           Angle const& flangeSlope, double webEdgeRadius, Angle const& webSlope)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , flangeWidth (flangeWidth)
    , depth (depth)
    , flangeThickness (flangeThickness)
    , webThickness (webThickness)
    , webSpacing (webSpacing)
    , filletRadius (filletRadius)
    , flangeEdgeRadius (flangeEdgeRadius)
    , flangeSlope (flangeSlope)
    , webEdgeRadius (webEdgeRadius)
    , webSlope (webSlope)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TTShapeProfile::TTShapeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetFlangeWidth (params.flangeWidth);
    SetDepth (params.depth);
    SetFlangeThickness (params.flangeThickness);
    SetWebThickness (params.webThickness);
    SetWebSpacing (params.webSpacing);
    SetFilletRadius (params.filletRadius);
    SetFlangeEdgeRadius (params.flangeEdgeRadius);
    SetFlangeSlope (params.flangeSlope);
    SetWebEdgeRadius (params.webEdgeRadius);
    SetWebSlope (params.webSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TTShapeProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isFlangeWidthValid = ProfilesProperty::IsGreaterThanZero (GetFlangeWidth());
    bool const isDepthValid = ProfilesProperty::IsGreaterThanZero (GetDepth());
    bool const isFlangeThicknessValid = ValidateFlangeThickness();
    bool const isWebThicknessValid = ValidateWebThickness();
    bool const isWebSpacingValid = ValidateWebSpacing();
    bool const isFilletRadiusValid = ValidateFilletRadius();
    bool const isFlangeEdgeRadiusValid = ValidateFlangeEdgeRadius();
    bool const isFlangeSlopeValid = ValidateFlangeSlope();
    bool const isWebEdgeRadiusValid = ValidateWebEdgeRadius();
    bool const isWebSlopeValid = ValidateWebSlope();

    return isFlangeWidthValid && isDepthValid && isFlangeThicknessValid && isWebThicknessValid && isWebSpacingValid
           && isFilletRadiusValid && isFlangeEdgeRadiusValid && isFlangeSlopeValid && isWebEdgeRadiusValid && isWebSlopeValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr TTShapeProfile::_CreateGeometry() const
    {
    return ProfilesGeometry::CreateTTShape (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TTShapeProfile::ValidateFlangeThickness() const
    {
    double const flangeThickness = GetFlangeThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (flangeThickness);
    bool const isLessThanDepth = ProfilesProperty::IsLess (flangeThickness, GetDepth());

    return isPositive && isLessThanDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TTShapeProfile::ValidateWebThickness() const
    {
    double const webThickness = GetWebThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (webThickness);
    bool const fitsInFlange = ProfilesProperty::IsLess (webThickness * 2.0 + GetWebSpacing(), GetFlangeWidth());

    return isPositive && fitsInFlange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TTShapeProfile::ValidateWebSpacing() const
    {
    double const webSpacing = GetWebSpacing();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (webSpacing);
    bool const fitsInFlange = ProfilesProperty::IsLess (webSpacing, GetFlangeWidth() - GetWebThickness() * 2.0);

    return isPositive && fitsInFlange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TTShapeProfile::ValidateFilletRadius() const
    {
    double const filletRadius = GetFilletRadius();
    if (ProfilesProperty::IsEqualToZero (filletRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (filletRadius);
    bool const fitsInFlange = ProfilesProperty::IsLessOrEqual (filletRadius, GetFlangeInnerFaceLength() / 2.0 - GetWebOuterSlopeHeight());
    bool const fitsInWeb = ProfilesProperty::IsLessOrEqual (filletRadius, GetWebOuterFaceLength() / 2.0 - GetFlangeSlopeHeight());
    // FilletRadius is not being validated against WebSpacing, because fillet radius used between the two webs
    // is adjusted/cliped if it is too big (i.e. doesn't fit in the spacing)

    return isPositive && fitsInFlange && fitsInWeb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TTShapeProfile::ValidateFlangeEdgeRadius() const
    {
    double const flangeEdgeRadius = GetFlangeEdgeRadius();
    if (ProfilesProperty::IsEqualToZero (flangeEdgeRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (flangeEdgeRadius);
    bool const fitsInFlangeLength = ProfilesProperty::IsLessOrEqual (flangeEdgeRadius, GetFlangeInnerFaceLength() / 2.0);
    bool const fitsInFlangeThickness = ProfilesProperty::IsLessOrEqual (flangeEdgeRadius, GetFlangeThickness() / 2.0);

    return isPositive && fitsInFlangeLength && fitsInFlangeThickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TTShapeProfile::ValidateFlangeSlope() const
    {
    double const flangeSlope = GetFlangeSlope().Radians();
    if (ProfilesProperty::IsEqualToZero (flangeSlope))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (flangeSlope);
    bool const isLessThanHalfPi = ProfilesProperty::IsLess (flangeSlope, PI / 2.0);
    bool const slopeHeightFitsInWeb = ProfilesProperty::IsLessOrEqual (GetFlangeSlopeHeight(), GetWebOuterFaceLength() / 2.0);

    return isPositive && isLessThanHalfPi && slopeHeightFitsInWeb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TTShapeProfile::ValidateWebEdgeRadius() const
    {
    double const webEdgeRadius = GetWebEdgeRadius();
    if (ProfilesProperty::IsEqualToZero (webEdgeRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (webEdgeRadius);
    bool const fitsInWebLength = ProfilesProperty::IsLessOrEqual (webEdgeRadius, GetWebOuterFaceLength() / 2.0);
    bool const fitsInWebThickness = ProfilesProperty::IsLessOrEqual (webEdgeRadius, GetWebThickness() / 2.0);

    return isPositive && fitsInWebLength && fitsInWebThickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TTShapeProfile::ValidateWebSlope() const
    {
    double const webSlope = GetWebSlope().Radians();
    if (ProfilesProperty::IsEqualToZero (webSlope))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (webSlope);
    bool const isLessThanHalfPi = ProfilesProperty::IsLess (webSlope, PI / 2.0);
    bool const slopeHeightFitsInFlange = ProfilesProperty::IsLessOrEqual (GetWebOuterSlopeHeight(), GetFlangeInnerFaceLength() / 2.0);
    bool const slopeHeightFitsInWebSpacing = ProfilesProperty::IsLessOrEqual (GetWebInnerSlopeHeight(), GetWebSpacing() / 2.0);

    return isPositive && isLessThanHalfPi && slopeHeightFitsInFlange && slopeHeightFitsInWebSpacing;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_FlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFlangeWidth (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_FlangeWidth, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_Depth, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetFlangeThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_FlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFlangeThickness (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_FlangeThickness, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetWebThickness (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_WebThickness, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetWebSpacing() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_WebSpacing);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetWebSpacing (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_WebSpacing, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_FilletRadius, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_FlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFlangeEdgeRadius (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_FlangeEdgeRadius, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Angle TTShapeProfile::GetFlangeSlope() const
    {
    return Angle::FromRadians (GetPropertyValueDouble (PRF_PROP_TTShapeProfile_FlangeSlope));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFlangeSlope (Angle const& value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_FlangeSlope, ECValue (value.Radians()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetWebEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_WebEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetWebEdgeRadius (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_WebEdgeRadius, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Angle TTShapeProfile::GetWebSlope() const
    {
    return Angle::FromRadians (GetPropertyValueDouble (PRF_PROP_TTShapeProfile_WebSlope));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetWebSlope (Angle const& value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_WebSlope, ECValue (value.Radians()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetFlangeInnerFaceLength() const
    {
    return (GetFlangeWidth() - 2.0 * GetWebThickness() - GetWebSpacing()) / 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetFlangeSlopeHeight() const
    {
    double const flangeSlopeCos = GetFlangeSlope().Cos();
    if (BeNumerical::IsLessOrEqualToZero (flangeSlopeCos))
        return 0.0;

    return (GetFlangeInnerFaceLength() / flangeSlopeCos) * GetFlangeSlope().Sin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetWebInnerFaceLength() const
    {
    return GetWebOuterFaceLength() - GetFlangeSlopeHeight();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetWebOuterFaceLength() const
    {
    return GetDepth() - GetFlangeThickness();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetWebInnerSlopeHeight() const
    {
    double const webSlopeCos = GetWebSlope().Cos();
    if (BeNumerical::IsLessOrEqualToZero (webSlopeCos))
        return 0.0;

    return (GetWebInnerFaceLength() / webSlopeCos) * GetWebSlope().Sin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetWebOuterSlopeHeight() const
    {
    double const webSlopeCos = GetWebSlope().Cos();
    if (BeNumerical::IsLessOrEqualToZero (webSlopeCos))
        return 0.0;

    return (GetWebOuterFaceLength() / webSlopeCos) * GetWebSlope().Sin();
    }

END_BENTLEY_PROFILES_NAMESPACE
