/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <Profiles\IShapeProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (IShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IShapeProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IShapeProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness,
                                           double webThickness, double filletRadius, double flangeEdgeRadius, Angle const& flangeSlope)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , flangeWidth (flangeWidth)
    , depth (depth)
    , flangeThickness (flangeThickness)
    , webThickness (webThickness)
    , filletRadius (filletRadius)
    , flangeEdgeRadius (flangeEdgeRadius)
    , flangeSlope (flangeSlope)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IShapeProfile::IShapeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetFlangeWidth (params.flangeWidth);
    SetDepth (params.depth);
    SetFlangeThickness (params.flangeThickness);
    SetWebThickness (params.webThickness);
    SetFilletRadius (params.filletRadius);
    SetFlangeEdgeRadius (params.flangeEdgeRadius);
    SetFlangeSlope (params.flangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool IShapeProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isFlangeWidthValid = ProfilesProperty::IsGreaterThanZero (GetFlangeWidth());
    bool const isDepthValid = ProfilesProperty::IsGreaterThanZero (GetDepth());
    bool const isFlangeThicknessValid = ValidateFlangeThickness();
    bool const isWebThicknessValid = ValidateWebThickness();
    bool const isFilletRadiusValid = ValidateFilletRadius();
    bool const isFlangeEdgeRadiusValid = ValidateFlangeEdgeRadius();
    bool const isFlangeSlopeValid = ValidateFlangeSlope();

    return isFlangeWidthValid && isDepthValid && isFlangeThicknessValid && isWebThicknessValid
           && isFilletRadiusValid && isFlangeEdgeRadiusValid && isFlangeSlopeValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr IShapeProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateIShape (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool IShapeProfile::ValidateFlangeThickness() const
    {
    double const flangeThickness = GetFlangeThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (flangeThickness);
    bool const isLessThanHalfDepth = ProfilesProperty::IsLess (flangeThickness, GetDepth() / 2.0);

    return isPositive && isLessThanHalfDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool IShapeProfile::ValidateWebThickness() const
    {
    double const webThickness = GetWebThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (webThickness);
    bool const isLessThanFlangeWidth = ProfilesProperty::IsLess (webThickness, GetFlangeWidth());

    return isPositive && isLessThanFlangeWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool IShapeProfile::ValidateFilletRadius() const
    {
    double const filletRadius = GetFilletRadius();
    if (ProfilesProperty::IsEqualToZero (filletRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (filletRadius);
    bool const fitsinFlange = ProfilesProperty::IsLessOrEqual (filletRadius, GetFlangeInnerFaceLength() / 2.0);
    bool const fitsInWeb = ProfilesProperty::IsLessOrEqual (filletRadius, GetWebFaceLength() / 2.0 - GetFlangeSlopeHeight());

    return isPositive && fitsinFlange && fitsInWeb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool IShapeProfile::ValidateFlangeEdgeRadius() const
    {
    double const flangeEdgeRadius = GetFlangeEdgeRadius();
    if (ProfilesProperty::IsEqualToZero (flangeEdgeRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (flangeEdgeRadius);
    bool const fitsInFlangeWidth = ProfilesProperty::IsLessOrEqual (flangeEdgeRadius, GetFlangeInnerFaceLength() / 2.0);
    bool const fitsInFlangeThickness = ProfilesProperty::IsLessOrEqual (flangeEdgeRadius, GetFlangeThickness());

    return isPositive && fitsInFlangeWidth && fitsInFlangeThickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool IShapeProfile::ValidateFlangeSlope() const
    {
    double const flangeSlope = GetFlangeSlope().Radians();
    if (ProfilesProperty::IsEqualToZero (flangeSlope))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (flangeSlope);
    bool const isLessThanHalfPi = ProfilesProperty::IsLess (flangeSlope, PI / 2.0);
    bool const slopeHeightFitsInWeb = ProfilesProperty::IsLessOrEqual (GetFlangeSlopeHeight(), GetWebFaceLength() / 2.0);

    return isPositive && isLessThanHalfPi && slopeHeightFitsInWeb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_IShapeProfile_FlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFlangeWidth (double value)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_FlangeWidth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_IShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_Depth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetFlangeThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_IShapeProfile_FlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFlangeThickness (double value)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_FlangeThickness, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_IShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetWebThickness (double value)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_WebThickness, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_IShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_FilletRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_IShapeProfile_FlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFlangeEdgeRadius (double value)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_FlangeEdgeRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Angle IShapeProfile::GetFlangeSlope() const
    {
    return Angle::FromRadians (GetPropertyValueDouble (PRF_PROP_IShapeProfile_FlangeSlope));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFlangeSlope (Angle const& value)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_FlangeSlope, ECN::ECValue (value.Radians()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetFlangeInnerFaceLength() const
    {
    return (GetFlangeWidth() - GetWebThickness()) / 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetWebFaceLength() const
    {
    return GetDepth() - GetFlangeThickness() * 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetFlangeSlopeHeight() const
    {
    double const flangeSlopeCos = GetFlangeSlope().Cos();
    if (BeNumerical::IsLessOrEqualToZero (flangeSlopeCos))
        return 0.0;

    return (GetFlangeInnerFaceLength() / flangeSlopeCos) * GetFlangeSlope().Sin();
    }

END_BENTLEY_PROFILES_NAMESPACE
