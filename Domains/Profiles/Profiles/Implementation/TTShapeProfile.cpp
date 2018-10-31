/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/TTShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\TTShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(TTShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TTShapeProfilePtr TTShapeProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetWidth(double val)
    {
    SetPropertyValue(PRF_PROP_TTShapeProfile_Width, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetDepth(double val)
    {
    SetPropertyValue(PRF_PROP_TTShapeProfile_Depth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetSpan(double val)
    {
    SetPropertyValue(PRF_PROP_TTShapeProfile_Span, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFlangeThickness(double val)
    {
    SetPropertyValue(PRF_PROP_TTShapeProfile_FlangeThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetWebThickness(double val)
    {
    SetPropertyValue(PRF_PROP_TTShapeProfile_WebThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_TTShapeProfile_FilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFlangeEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_TTShapeProfile_FlangeEdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetFlangeSlope(double val)
    {
    SetPropertyValue(PRF_PROP_TTShapeProfile_FlangeSlope, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetWebEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_TTShapeProfile_WebEdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTShapeProfile::SetWebSlope(double val)
    {
    SetPropertyValue(PRF_PROP_TTShapeProfile_WebSlope, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
