/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/CShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CShapeProfile.h>

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
BentleyStatus CShapeProfile::_Validate() const
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

    bool const isValidFlangeThickness = std::isfinite (GetFlangeThickness()) && GetFlangeThickness() > 0.0 && GetFlangeThickness() < GetDepth() / 2.0;
    if (!isValidFlangeThickness)
        return BSIERROR;

    bool const isValidWebThickness = std::isfinite (GetWebThickness()) && GetWebThickness() > 0.0 && GetWebThickness() < GetFlangeWidth();
    if (!isValidWebThickness)
        return BSIERROR;

    bool const isFlangeSlopeValid = std::isfinite (GetFlangeSlope()) && GetFlangeSlope() >= 0.0 && GetFlangeSlope() < PI / 2.0
                                    && (GetInnerWebFaceLength() / 2.0) - GetSlopeHeight() >= 0.0;
    if (!isFlangeSlopeValid)
        return BSIERROR;

    bool const isValidFilletRadius = std::isfinite (GetFilletRadius()) && GetFilletRadius() >= 0.0 && GetFilletRadius() <= GetInnerWebFaceLength() / 2.0 - GetSlopeHeight()
                                     && GetFilletRadius() <= GetInnerFlangeFaceLength() / 2.0;
    if (!isValidFilletRadius)
        return BSIERROR;

    bool const isValidFlangeEdgeRadius = std::isfinite (GetFlangeEdgeRadius()) && GetFlangeEdgeRadius() >= 0.0
                                         && GetFlangeEdgeRadius() <= std::min (GetInnerFlangeFaceLength() / 2.0, GetFlangeThickness() / 2.0);
    if (!isValidFlangeEdgeRadius)
        return BSIERROR;

    return BSISUCCESS;
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
double CShapeProfile::GetSlopeHeight() const
    {
    double const flangeSlopeCos = std::cos (GetFlangeSlope());
    if (flangeSlopeCos == 0.0)
        return 0.0;

    return (GetInnerFlangeFaceLength() / flangeSlopeCos) * std::sin (GetFlangeSlope());
    }

END_BENTLEY_PROFILES_NAMESPACE
