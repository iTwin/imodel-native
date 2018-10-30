/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/IShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\IShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IShapeProfilePtr IShapeProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetWidth(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_Width, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetDepth(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_Depth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFlangeThickness(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_FlangeThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetWebThickness(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_WebThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_FilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFlangeEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_FlangeEdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFlangeSlope(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_FlangeSlope, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
