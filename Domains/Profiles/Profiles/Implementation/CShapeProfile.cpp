/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/CShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CShapeProfile.h>
#include <Profiles\ProfilesGeometry.h>
#include <Profiles\ProfilesProperty.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (CShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness,
                                           double webThickness, double filletRadius, double flangeEdgeRadius, double flangeSlope)
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
CShapeProfile::CShapeProfile (CreateParams const& params)
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
bool CShapeProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isValidFlangeWidth = ProfilesProperty::IsGreaterThanZero (GetFlangeWidth());
    if (!isValidFlangeWidth)
        return false;

    bool const isValidDepth = ProfilesProperty::IsGreaterThanZero (GetDepth());
    if (!isValidDepth)
        return false;

    bool const isValidFlangeThickness = ProfilesProperty::IsGreaterThanZero (GetFlangeThickness()) && GetFlangeThickness() < GetDepth() / 2.0;
    if (!isValidFlangeThickness)
        return false;

    bool const isValidWebThickness = ProfilesProperty::IsGreaterThanZero (GetWebThickness()) && GetWebThickness() < GetFlangeWidth();
    if (!isValidWebThickness)
        return false;

    bool const isValidFilletRadius = ProfilesProperty::IsGreaterOrEqualToZero (GetFilletRadius()) && GetFilletRadius() <= GetInnerWebFaceLength() / 2.0 - GetFlangeSlopeHeight()
                                     && GetFilletRadius() <= GetInnerFlangeFaceLength() / 2.0;
    if (!isValidFilletRadius)
        return false;

    bool const isValidFlangeEdgeRadius = ProfilesProperty::IsGreaterOrEqualToZero (GetFlangeEdgeRadius())
                                         && GetFlangeEdgeRadius() <= std::min (GetInnerFlangeFaceLength() / 2.0, GetFlangeThickness() / 2.0);
    if (!isValidFlangeEdgeRadius)
        return false;

    bool const isFlangeSlopeValid = ProfilesProperty::IsGreaterOrEqualToZero (GetFlangeSlope()) && GetFlangeSlope() < PI / 2.0
                                    && GetInnerWebFaceLength() / 2.0 >= GetFlangeSlopeHeight();
    if (!isFlangeSlopeValid)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr CShapeProfile::_CreateGeometry() const
    {
    return ProfilesGeomApi::CreateCShape (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CShapeProfile_FlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetFlangeWidth (double val)
    {
    SetPropertyValue (PRF_PROP_CShapeProfile_FlangeWidth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_CShapeProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetFlangeThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_CShapeProfile_FlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetFlangeThickness (double val)
    {
    SetPropertyValue (PRF_PROP_CShapeProfile_FlangeThickness, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_CShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetWebThickness (double val)
    {
    SetPropertyValue (PRF_PROP_CShapeProfile_WebThickness, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_CShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_CShapeProfile_FilletRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_CShapeProfile_FlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetFlangeEdgeRadius (double val)
    {
    SetPropertyValue (PRF_PROP_CShapeProfile_FlangeEdgeRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetFlangeSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_CShapeProfile_FlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetFlangeSlope (double val)
    {
    SetPropertyValue (PRF_PROP_CShapeProfile_FlangeSlope, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetInnerFlangeFaceLength() const
    {
    return GetFlangeWidth() - GetWebThickness();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetInnerWebFaceLength() const
    {
    return GetDepth() - GetFlangeThickness() * 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetFlangeSlopeHeight() const
    {
    double const flangeSlopeCos = std::cos (GetFlangeSlope());
    if (flangeSlopeCos <= DBL_EPSILON)
        return 0.0;

    return (GetInnerFlangeFaceLength() / flangeSlopeCos) * std::sin (GetFlangeSlope());
    }

END_BENTLEY_PROFILES_NAMESPACE
