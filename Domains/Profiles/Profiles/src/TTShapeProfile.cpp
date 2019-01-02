/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/TTShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\TTShapeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (TTShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetWidth (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_Width, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_Depth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetSpan() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_Span);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetSpan (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_Span, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetFlangeThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_FlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFlangeThickness (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_FlangeThickness, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetWebThickness (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_WebThickness, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_FilletRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_FlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFlangeEdgeRadius (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_FlangeEdgeRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetFlangeSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_FlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFlangeSlope (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_FlangeSlope, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetWebEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_WebEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetWebEdgeRadius (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_WebEdgeRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TTShapeProfile::GetWebSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_TTShapeProfile_WebSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetWebSlope (double value)
    {
    SetPropertyValue (PRF_PROP_TTShapeProfile_WebSlope, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
