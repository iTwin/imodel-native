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
CShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model)
    : T_Super (model, QueryClassId (model.GetDgnDb()))
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model,  double flangeWidth, double depth, double flangeThickness,
    double webThickness, double filletRadius, double edgeRadius, double flangeSlope)
    : T_Super (model, QueryClassId (model.GetDgnDb()))
    , flangeWidth (flangeWidth)
    , depth (depth)
    , flangeThickness (flangeThickness)
    , webThickness (webThickness)
    , filletRadius (filletRadius)
    , edgeRadius (edgeRadius)
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
    SetEdgeRadius (params.edgeRadius);
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

    bool const isValidFlangeWidth = GetFlangeWidth() > 0.0;
    if (!isValidFlangeWidth)
        return BSIERROR;

    bool const isValidDepth = GetDepth() > 0.0;
    if (!isValidDepth)
        return BSIERROR;

    bool const isValidFlangeThickness = GetFlangeThickness() > 0.0 && GetFlangeThickness() < GetDepth() / 2.0;
    if (!isValidFlangeThickness)
        return BSIERROR;

    bool const isValidWebThickness = GetWebThickness() > 0.0 && GetWebThickness() < GetFlangeWidth();
    if (!isValidWebThickness)
        return BSIERROR;

    double const innerFlangeFaceWidth = GetFlangeWidth() - GetWebThickness();
    double const innerWebFaceDepth = GetDepth() - GetFlangeThickness() * 2.0;
    double const slopeDepth = (innerFlangeFaceWidth / std::cos (GetFlangeSlope())) * std::sin (GetFlangeSlope());
    double const halfFlangeThickness = GetFlangeThickness() / 2.0;

    bool const isFlangeSlopeValid = GetFlangeSlope() >= 0.0 && GetFlangeSlope() < PI && (innerWebFaceDepth / 2.0) - slopeDepth > 0.0;
    if (!isFlangeSlopeValid)
        return BSIERROR;

    bool const isValidFilletRadius = GetFilletRadius() >= 0.0 && GetFilletRadius() <= (innerWebFaceDepth / 2.0) - slopeDepth &&
                                     GetFilletRadius() <= innerFlangeFaceWidth / 2.0;
    if (!isValidFilletRadius)
        return BSIERROR;

    bool const isValidEdgeRadius = GetEdgeRadius() >= 0.0 && GetEdgeRadius() <= std::min (innerFlangeFaceWidth, halfFlangeThickness);
    if (!isValidEdgeRadius)
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
double CShapeProfile::GetEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_CShapeProfile_EdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetEdgeRadius (double val)
    {
    SetPropertyValue (PRF_PROP_CShapeProfile_EdgeRadius, ECN::ECValue (val));
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

END_BENTLEY_PROFILES_NAMESPACE
