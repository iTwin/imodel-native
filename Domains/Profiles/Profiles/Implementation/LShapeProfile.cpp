/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/LShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\LShapeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (LShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth, double thickness,
                                           double filletRadius, double edgeRadius, double legSlope)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , width (width)
    , depth (depth)
    , thickness (thickness)
    , filletRadius (filletRadius)
    , edgeRadius (edgeRadius)
    , legSlope (legSlope)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LShapeProfile::LShapeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetWidth (params.width);
    SetDepth (params.depth);
    SetThickness (params.thickness);
    SetFilletRadius (params.filletRadius);
    SetEdgeRadius (params.edgeRadius);
    SetLegSlope (params.legSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetWidth (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_Width, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_Thickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetThickness (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_Thickness, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_FilletRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_EdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetEdgeRadius (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_EdgeRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetLegSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_LegSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetLegSlope (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_LegSlope, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetInnerFlangeFaceLength() const
    {
    return GetWidth() - GetThickness();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetInnerWebFaceLength() const
    {
    return GetDepth() - GetThickness();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetFlangeSlopeHeight() const
    {
    double const flangeSlopeCos = std::cos (GetLegSlope());
    if (flangeSlopeCos <= DBL_EPSILON)
        return 0.0;

    return (GetInnerFlangeFaceLength() / flangeSlopeCos) * std::sin (GetLegSlope());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetWebSlopeHeight() const
    {
    double const webSlopeCos = std::cos (GetLegSlope());
    if (webSlopeCos <= DBL_EPSILON)
        return 0.0;

    return (GetInnerWebFaceLength() / webSlopeCos) * std::sin (GetLegSlope());
    }

END_BENTLEY_PROFILES_NAMESPACE
