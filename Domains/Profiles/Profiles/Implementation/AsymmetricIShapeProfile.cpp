/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/AsymmetricIShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\AsymmetricIShapeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (AsymmetricIShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsymmetricIShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsymmetricIShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double topFlangeWidth, double bottomFlangeWidth, double depth,
                                                     double topFlangeThickness, double bottomFlangeThickness, double webThickness, double topFlangeFilletRadius,
                                                     double topFlangeEdgeRadius, double topFlangeSlope, double bottomFlangeFilletRadius,
                                                     double bottomFlangeEdgeRadius, double bottomFlangeSlope)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , topFlangeWidth (topFlangeWidth)
    , bottomFlangeWidth (bottomFlangeWidth)
    , depth (depth)
    , topFlangeThickness (topFlangeThickness)
    , bottomFlangeThickness (bottomFlangeThickness)
    , webThickness (webThickness)
    , topFlangeFilletRadius (topFlangeFilletRadius)
    , topFlangeEdgeRadius (topFlangeEdgeRadius)
    , topFlangeSlope (topFlangeSlope)
    , bottomFlangeFilletRadius (bottomFlangeFilletRadius)
    , bottomFlangeEdgeRadius (bottomFlangeEdgeRadius)
    , bottomFlangeSlope (bottomFlangeSlope)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsymmetricIShapeProfile::AsymmetricIShapeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetTopFlangeWidth (params.topFlangeWidth);
    SetBottomFlangeWidth (params.bottomFlangeWidth);
    SetDepth (params.depth);
    SetTopFlangeThickness (params.topFlangeThickness);
    SetBottomFlangeThickness (params.bottomFlangeThickness);
    SetWebThickness (params.webThickness);
    SetTopFlangeFilletRadius (params.topFlangeFilletRadius);
    SetTopFlangeEdgeRadius (params.topFlangeEdgeRadius);
    SetTopFlangeSlope (params.topFlangeSlope);
    SetBottomFlangeFilletRadius (params.bottomFlangeFilletRadius);
    SetBottomFlangeEdgeRadius (params.bottomFlangeEdgeRadius);
    SetBottomFlangeSlope (params.bottomFlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_TopFlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeWidth (double val)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_TopFlangeWidth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeWidth (double val)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeWidth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_TopFlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeThickness (double val)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_TopFlangeThickness, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeThickness (double val)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeThickness, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetWebThickness (double val)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_WebThickness, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_TopFlangeFilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_TopFlangeFilletRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_TopFlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeEdgeRadius (double val)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_TopFlangeEdgeRadius, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_TopFlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeSlope (double val)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_TopFlangeSlope, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeFilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeFilletRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeEdgeRadius (double val)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeEdgeRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeSlope (double val)
    {
    SetPropertyValue (PRF_PROP_AsymmetricIShapeProfile_BottomFlangeSlope, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
