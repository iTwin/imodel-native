/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/TTShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
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
double TTShapeProfile::GetInnerFlangeFaceLength() const
    {
    return GetFlangeWidth() - 2.0 * GetWebThickness() - GetWebSpacing();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetInnerWebFaceLength() const
    {
    return GetDepth() - GetFlangeThickness();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetFlangeSlopeHeight() const
    {
    double const flangeSlopeCos = GetFlangeSlope().Cos();
    if (BeNumerical::IsLessOrEqualToZero (flangeSlopeCos))
        return 0.0;

    return (GetInnerFlangeFaceLength() / flangeSlopeCos) * GetFlangeSlope().Sin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetWebSlopeHeight() const
    {
    double const webSlopeCos = GetWebSlope().Cos();
    if (BeNumerical::IsLessOrEqualToZero (webSlopeCos))
        return 0.0;

    return (GetInnerWebFaceLength() / webSlopeCos) * GetWebSlope().Sin();
    }

END_BENTLEY_PROFILES_NAMESPACE
