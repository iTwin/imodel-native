/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/AsymmetricIShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\AsymmetricIShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(AsymmetricIShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsymmetricIShapeProfilePtr AsymmetricIShapeProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopWidth() const
    {
    return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_TopWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopWidth(double val)
    {
    SetPropertyValue(PRF_PROP_AsymmetricIShapeProfile_TopWidth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomWidth() const
    {
    return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_BottomWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomWidth(double val)
    {
    SetPropertyValue(PRF_PROP_AsymmetricIShapeProfile_BottomWidth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetDepth(double val)
    {
    SetPropertyValue(PRF_PROP_AsymmetricIShapeProfile_Depth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeThickness() const
    {
    return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_TopFlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeThickness(double val)
    {
    SetPropertyValue(PRF_PROP_AsymmetricIShapeProfile_TopFlangeThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeThickness() const
    {
    return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_BottomFlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeThickness(double val)
    {
    SetPropertyValue(PRF_PROP_AsymmetricIShapeProfile_BottomFlangeThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetWebThickness(double val)
    {
    SetPropertyValue(PRF_PROP_AsymmetricIShapeProfile_WebThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeFilletRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_TopFlangeFilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_AsymmetricIShapeProfile_TopFlangeFilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_TopFlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_AsymmetricIShapeProfile_TopFlangeEdgeRadius, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetTopFlangeSlope() const
    {
    return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_TopFlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetTopFlangeSlope(double val)
    {
    SetPropertyValue(PRF_PROP_AsymmetricIShapeProfile_TopFlangeSlope, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeFilletRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_BottomFlangeFilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_AsymmetricIShapeProfile_BottomFlangeFilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_BottomFlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_AsymmetricIShapeProfile_BottomFlangeEdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double AsymmetricIShapeProfile::GetBottomFlangeSlope() const
    {
    return GetPropertyValueDouble(PRF_PROP_AsymmetricIShapeProfile_BottomFlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AsymmetricIShapeProfile::SetBottomFlangeSlope(double val)
    {
    SetPropertyValue(PRF_PROP_AsymmetricIShapeProfile_BottomFlangeSlope, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
