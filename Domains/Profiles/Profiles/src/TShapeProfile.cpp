/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/TShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\TShapeProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (TShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness,
                                           double webThickness, double filletRadius, double flangeEdgeRadius, double flangeSlope,
                                           double webEdgeRadius, double webSlope)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , flangeWidth (flangeWidth)
    , depth (depth)
    , flangeThickness (flangeThickness)
    , webThickness (webThickness)
    , filletRadius (filletRadius)
    , flangeEdgeRadius (flangeEdgeRadius)
    , flangeSlope (flangeSlope)
    , webEdgeRadius (webEdgeRadius)
    , webSlope (webSlope)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TShapeProfile::TShapeProfile (CreateParams const& params)
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
    SetWebEdgeRadius (params.webEdgeRadius);
    SetWebSlope (params.webSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool TShapeProfile::_Validate() const
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
    bool const isWebEdgeRadiusValid = ValidateWebEdgeRadius();
    bool const isWebSlopeValid = ValidateWebSlope();

    return isFlangeWidthValid && isDepthValid && isFlangeThicknessValid && isWebThicknessValid && isFilletRadiusValid
           && isFlangeEdgeRadiusValid && isFlangeSlopeValid && isWebEdgeRadiusValid && isWebSlopeValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr TShapeProfile::_CreateGeometry() const
    {
    return ProfilesGeomApi::CreateTShape (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TShapeProfile::ValidateFlangeThickness() const
    {
    double const flangeThickness = GetFlangeThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (flangeThickness);
    bool const isLessThanDepth = flangeThickness < GetDepth();

    return isPositive && isLessThanDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TShapeProfile::ValidateWebThickness() const
    {
    double const webThickness = GetWebThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (webThickness);
    bool const isLessThanFlangeWidth = webThickness < GetFlangeWidth();

    return isPositive && isLessThanFlangeWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TShapeProfile::ValidateFilletRadius() const
    {
    double const filletRadius = GetFilletRadius();
    if (ProfilesProperty::IsEqualToZero (filletRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (filletRadius);
    bool const fitsInFlange = filletRadius <= GetInnerFlangeFaceLength() / 2.0 - GetWebSlopeHeight();
    bool const fitsInWeb = filletRadius <= GetInnerWebFaceLength() / 2.0 - GetFlangeSlopeHeight();

    return isPositive && fitsInFlange && fitsInWeb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TShapeProfile::ValidateFlangeEdgeRadius() const
    {
    double const flangeEdgeRadius = GetFlangeEdgeRadius();
    if (ProfilesProperty::IsEqualToZero (flangeEdgeRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (flangeEdgeRadius);
    bool const fitsInFlangeLength = flangeEdgeRadius <= GetInnerFlangeFaceLength() / 2.0;
    bool const fitsInFlangeThickness = flangeEdgeRadius <= GetFlangeThickness() / 2.0;

    return isPositive && fitsInFlangeLength && fitsInFlangeThickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TShapeProfile::ValidateFlangeSlope() const
    {
    double const flangeSlope = GetFlangeSlope();
    if (ProfilesProperty::IsEqualToZero (flangeSlope))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (flangeSlope);
    bool const isLessThanHalfPi = flangeSlope < PI / 2.0;
    bool const slopeHeightFitsInWeb = GetFlangeSlopeHeight() <= GetInnerWebFaceLength() / 2.0;

    return isPositive && isLessThanHalfPi && slopeHeightFitsInWeb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TShapeProfile::ValidateWebEdgeRadius() const
    {
    double const webEdgeRadius = GetWebEdgeRadius();
    if (ProfilesProperty::IsEqualToZero (webEdgeRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (webEdgeRadius);
    bool const fitsInWebLength = webEdgeRadius <= GetInnerWebFaceLength() / 2.0;
    bool const fitsInWebThickness = webEdgeRadius <= GetWebThickness() / 2.0;

    return isPositive && fitsInWebLength && fitsInWebThickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool TShapeProfile::ValidateWebSlope() const
    {
    double const webSlope = GetWebSlope();
    if (ProfilesProperty::IsEqualToZero (webSlope))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (webSlope);
    bool const isLessThanHalfPi = webSlope < PI / 2.0;
    bool const slopeHeightFitsInFlange = GetWebSlopeHeight() <= GetInnerFlangeFaceLength() / 2.0;

    return isPositive && isLessThanHalfPi && slopeHeightFitsInFlange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_TShapeProfile_FlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFlangeWidth (double val)
    {
    SetPropertyValue (PRF_PROP_TShapeProfile_FlangeWidth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_TShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_TShapeProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetFlangeThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_TShapeProfile_FlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFlangeThickness (double val)
    {
    SetPropertyValue (PRF_PROP_TShapeProfile_FlangeThickness, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_TShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetWebThickness (double val)
    {
    SetPropertyValue (PRF_PROP_TShapeProfile_WebThickness, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_TShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_TShapeProfile_FilletRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_TShapeProfile_FlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFlangeEdgeRadius (double val)
    {
    SetPropertyValue (PRF_PROP_TShapeProfile_FlangeEdgeRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetFlangeSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_TShapeProfile_FlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFlangeSlope (double val)
    {
    SetPropertyValue (PRF_PROP_TShapeProfile_FlangeSlope, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetWebEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_TShapeProfile_WebEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetWebEdgeRadius (double val)
    {
    SetPropertyValue (PRF_PROP_TShapeProfile_WebEdgeRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetWebSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_TShapeProfile_WebSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetWebSlope (double val)
    {
    SetPropertyValue (PRF_PROP_TShapeProfile_WebSlope, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetInnerFlangeFaceLength() const
    {
    return (GetFlangeWidth() - GetWebThickness()) / 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetInnerWebFaceLength() const
    {
    return GetDepth() - GetFlangeThickness();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetFlangeSlopeHeight() const
    {
    double const flangeSlopeCos = std::cos (GetFlangeSlope());
    if (flangeSlopeCos <= DBL_EPSILON)
        return 0.0;

    return (GetInnerFlangeFaceLength() / flangeSlopeCos) * std::sin (GetFlangeSlope());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetWebSlopeHeight() const
    {
    double const webSlopeCos = std::cos (GetWebSlope());
    if (webSlopeCos <= DBL_EPSILON)
        return 0.0;

    return (GetInnerWebFaceLength() / webSlopeCos) * std::sin (GetWebSlope());
    }

END_BENTLEY_PROFILES_NAMESPACE
