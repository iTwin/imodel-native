/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/IShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\IShapeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (IShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness,
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
BentleyStatus IShapeProfile::_Validate() const
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

    double const innerFlangeFaceLength = (GetFlangeWidth() - GetWebThickness()) / 2.0;
    double const innerWebFaceLength = GetDepth() - GetFlangeThickness() * 2.0;
    double const slopeDepth = (innerFlangeFaceLength / std::cos (GetFlangeSlope())) * std::sin (GetFlangeSlope());

    bool const isFlangeSlopeValid = std::isfinite (GetFlangeSlope()) && GetFlangeSlope() >= 0.0 && GetFlangeSlope() < PI / 2.0
                                    && (innerWebFaceLength / 2.0) - slopeDepth >= 0.0;
    if (!isFlangeSlopeValid)
        return BSIERROR;

    bool const isValidFilletRadius = std::isfinite (GetFilletRadius()) && GetFilletRadius() >= 0.0 && GetFilletRadius() <= innerWebFaceLength / 2.0 - slopeDepth
                                     && GetFilletRadius() <= innerFlangeFaceLength / 2.0;
    if (!isValidFilletRadius)
        return BSIERROR;

    bool const isValidFlangeEdgeRadius = std::isfinite (GetFlangeEdgeRadius()) && GetFlangeEdgeRadius() >= 0.0
                                         && GetFlangeEdgeRadius() <= std::min (innerFlangeFaceLength / 2.0, GetFlangeThickness() / 2.0);
    if (!isValidFlangeEdgeRadius)
        return BSIERROR;

    return BSISUCCESS;
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
void IShapeProfile::SetFlangeWidth (double val)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_FlangeWidth, ECN::ECValue (val));
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
void IShapeProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_Depth, ECN::ECValue (val));
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
void IShapeProfile::SetFlangeThickness (double val)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_FlangeThickness, ECN::ECValue (val));
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
void IShapeProfile::SetWebThickness (double val)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_WebThickness, ECN::ECValue (val));
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
void IShapeProfile::SetFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_FilletRadius, ECN::ECValue (val));
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
void IShapeProfile::SetFlangeEdgeRadius (double val)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_FlangeEdgeRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetFlangeSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_IShapeProfile_FlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFlangeSlope (double val)
    {
    SetPropertyValue (PRF_PROP_IShapeProfile_FlangeSlope, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetInnerFlangeFaceLength() const
    {
    return (GetFlangeWidth() - GetWebThickness()) / 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetInnerWebFaceLength() const
    {
    return GetDepth() - GetFlangeThickness() * 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetSlopeHeight() const
    {
    double const flangeSlopeCos = std::cos (GetFlangeSlope());
    if (flangeSlopeCos == 0.0)
        return 0.0;

    return (GetInnerFlangeFaceLength() / flangeSlopeCos) * std::sin (GetFlangeSlope());
    }

END_BENTLEY_PROFILES_NAMESPACE
