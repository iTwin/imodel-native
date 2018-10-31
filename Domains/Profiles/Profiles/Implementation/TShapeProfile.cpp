/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/TShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\TShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(TShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TShapeProfilePtr TShapeProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetWidth(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_Width, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetDepth(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_Depth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFlangeThickness(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_FlangeThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetWebThickness(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_WebThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_FilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFlangeEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_FlangeEdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFlangeSlope(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_FlangeSlope, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetWebEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_WebEdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetWebSlope(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_WebSlope, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
