/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ZShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\ZShapeProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (ZShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ZShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ZShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness,
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
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ZShapeProfile::ZShapeProfile (CreateParams const& params)
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
bool ZShapeProfile::_Validate() const
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
IGeometryPtr ZShapeProfile::_CreateGeometry() const
    {
    return ProfilesGeomApi::CreateZShape (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ZShapeProfile::ValidateFlangeThickness() const
    {
    double const flangeThickness = GetFlangeThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (flangeThickness);
    bool const isLessThanHalfDepth = flangeThickness < GetDepth() / 2.0;

    return isPositive && isLessThanHalfDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ZShapeProfile::ValidateWebThickness() const
    {
    double const webThickness = GetWebThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (webThickness);
    bool const isLessThanFlangeWidth = webThickness < GetFlangeWidth();

    return isPositive && isLessThanFlangeWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ZShapeProfile::ValidateFilletRadius() const
    {
    double const filletRadius = GetFilletRadius();
    if (ProfilesProperty::IsEqualToZero (filletRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (filletRadius);
    bool const fitsInFlange = filletRadius <= GetInnerFlangeFaceLength() / 2.0;
    bool const fitsInWeb = filletRadius <= GetInnerWebFaceLength() / 2.0 - GetFlangeSlopeHeight();

    return isPositive && fitsInFlange && fitsInWeb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ZShapeProfile::ValidateFlangeEdgeRadius() const
    {
    double const flangeEdgeRadius = GetFlangeEdgeRadius();
    if (ProfilesProperty::IsEqualToZero (flangeEdgeRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (flangeEdgeRadius);
    bool const fitsInFlangeWidth = flangeEdgeRadius <= GetInnerFlangeFaceLength() / 2.0;
    bool const fitsInFlangeThickness = flangeEdgeRadius <= GetFlangeThickness() / 2.0;

    return isPositive && fitsInFlangeWidth && fitsInFlangeThickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ZShapeProfile::ValidateFlangeSlope() const
    {
    double const flangeSlope = GetFlangeSlope().Radians();
    if (ProfilesProperty::IsEqualToZero (flangeSlope))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (flangeSlope);
    bool const isLessThanHalfPi = flangeSlope < PI / 2.0;
    bool const slopeHeightFitsInWeb = GetFlangeSlopeHeight() <= GetInnerWebFaceLength() / 2.0;

    return isPositive && isLessThanHalfPi && slopeHeightFitsInWeb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_ZShapeProfile_FlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetFlangeWidth (double val)
    {
    SetPropertyValue (PRF_PROP_ZShapeProfile_FlangeWidth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_ZShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_ZShapeProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetFlangeThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_ZShapeProfile_FlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetFlangeThickness (double val)
    {
    SetPropertyValue (PRF_PROP_ZShapeProfile_FlangeThickness, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_ZShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetWebThickness (double val)
    {
    SetPropertyValue (PRF_PROP_ZShapeProfile_WebThickness, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_ZShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_ZShapeProfile_FilletRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_ZShapeProfile_FlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetFlangeEdgeRadius (double val)
    {
    SetPropertyValue (PRF_PROP_ZShapeProfile_FlangeEdgeRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Angle ZShapeProfile::GetFlangeSlope() const
    {
    return Angle::FromRadians (GetPropertyValueDouble (PRF_PROP_ZShapeProfile_FlangeSlope));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetFlangeSlope (Angle const& val)
    {
    SetPropertyValue (PRF_PROP_ZShapeProfile_FlangeSlope, ECN::ECValue (val.Radians()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetInnerFlangeFaceLength() const
    {
    return GetFlangeWidth() - GetWebThickness();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetInnerWebFaceLength() const
    {
    return GetDepth() - GetFlangeThickness();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetFlangeSlopeHeight() const
    {
    double const flangeSlopeCos = GetFlangeSlope().Cos();
    if (flangeSlopeCos <= DBL_EPSILON)
        return 0.0;

    return (GetInnerFlangeFaceLength() / flangeSlopeCos) * GetFlangeSlope().Sin();
    }

END_BENTLEY_PROFILES_NAMESPACE
