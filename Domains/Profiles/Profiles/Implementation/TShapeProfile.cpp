/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/TShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\TShapeProfile.h>
#include <Profiles\ProfilesGeometry.h>

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
BentleyStatus TShapeProfile::_Validate() const
    {
    BentleyStatus status = T_Super::_Validate();
    if (status != BSISUCCESS)
        return status;

    bool const isValidFlangeWidth = std::isfinite (GetFlangeWidth()) && GetFlangeWidth() > 0.0;
    if (!isValidFlangeWidth)
        return BSIERROR;

    bool const isValidDepth = std::isfinite (GetDepth()) && GetDepth() > 0.0;
    if (!isValidDepth)
        return BSIERROR;

    bool const isValidFlangeThickness = std::isfinite (GetFlangeThickness()) && GetFlangeThickness() > 0.0 && GetFlangeThickness() < GetDepth();
    if (!isValidFlangeThickness)
        return BSIERROR;

    bool const isValidWebThickness = std::isfinite (GetWebThickness()) && GetWebThickness() > 0.0 && GetWebThickness() < GetFlangeWidth();
    if (!isValidWebThickness)
        return BSIERROR;

    bool const isValidFilletRadius = std::isfinite (GetFilletRadius()) && GetFilletRadius() >= 0.0
                                     && GetFilletRadius() <= GetInnerWebFaceLength() / 2.0 - GetFlangeSlopeHeight()
                                     && GetFilletRadius() <= GetInnerFlangeFaceLength() / 2.0 - GetWebSlopeHeight();
    if (!isValidFilletRadius)
        return BSIERROR;

    bool const isValidFlangeEdgeRadius = std::isfinite (GetFlangeEdgeRadius()) && GetFlangeEdgeRadius() >= 0.0
                                         && GetFlangeEdgeRadius() <= std::min (GetInnerFlangeFaceLength() / 2.0, GetFlangeThickness() / 2.0);
    if (!isValidFlangeEdgeRadius)
        return BSIERROR;

    bool const isFlangeSlopeValid = std::isfinite (GetFlangeSlope()) && GetFlangeSlope() >= 0.0 && GetFlangeSlope() < PI / 2.0
                                    && GetInnerWebFaceLength() / 2.0 >= GetFlangeSlopeHeight();
    if (!isFlangeSlopeValid)
        return BSIERROR;

    bool const isValidWebEdgeRadius = std::isfinite (GetWebEdgeRadius()) && GetWebEdgeRadius() >= 0.0
                                      && GetWebEdgeRadius() <= std::min (GetInnerWebFaceLength() / 2.0, GetWebThickness() / 2.0);
    if (!isValidWebEdgeRadius)
        return BSIERROR;

    bool const isWebSlopeValid = std::isfinite (GetWebSlope()) && GetWebSlope() >= 0.0 && GetWebSlope() < PI / 2.0
                                 && GetInnerFlangeFaceLength() / 2.0 >= GetWebSlopeHeight();
    if (!isWebSlopeValid)
        return BSIERROR;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr TShapeProfile::_CreateGeometry() const
    {
    return ProfilesGeomApi::CreateTShape (this);
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
