/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/ZShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\ZShapeProfile.h>
#include <Profiles\ProfilesGeometry.h>

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
BentleyStatus ZShapeProfile::_Validate() const
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

    bool const isValidFilletRadius = std::isfinite (GetFilletRadius()) && GetFilletRadius() >= 0.0 && GetFilletRadius() <= GetInnerWebFaceLength() / 2.0 - GetSlopeHeight()
                                     && GetFilletRadius() <= GetInnerFlangeFaceLength() / 2.0;
    if (!isValidFilletRadius)
        return BSIERROR;

    bool const isValidFlangeEdgeRadius = std::isfinite (GetFlangeEdgeRadius()) && GetFlangeEdgeRadius() >= 0.0
                                         && GetFlangeEdgeRadius() <= std::min (GetInnerFlangeFaceLength() / 2.0, GetFlangeThickness() / 2.0);
    if (!isValidFlangeEdgeRadius)
        return BSIERROR;

    bool const isFlangeSlopeValid = std::isfinite (GetFlangeSlope()) && GetFlangeSlope() >= 0.0 && GetFlangeSlope() < PI / 2.0
                                    && GetInnerWebFaceLength() / 2.0 >= GetSlopeHeight();
    if (!isFlangeSlopeValid)
        return BSIERROR;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ZShapeProfile::_CreateGeometry() const
    {
    return ProfilesGeomApi::CreateZShape (this);
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
double ZShapeProfile::GetFlangeSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_ZShapeProfile_FlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetFlangeSlope (double val)
    {
    SetPropertyValue (PRF_PROP_ZShapeProfile_FlangeSlope, ECN::ECValue (val));
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
double ZShapeProfile::GetSlopeHeight() const
    {
    double const flangeSlopeCos = std::cos (GetFlangeSlope());
    if (flangeSlopeCos <= DBL_EPSILON)
        return 0.0;

    return (GetInnerFlangeFaceLength() / flangeSlopeCos) * std::sin (GetFlangeSlope());
    }

END_BENTLEY_PROFILES_NAMESPACE
