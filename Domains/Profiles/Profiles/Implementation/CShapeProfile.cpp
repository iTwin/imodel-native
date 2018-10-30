/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/CShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CShapeProfilePtr CShapeProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetFlangeThickness(double val)
    {
    SetPropertyValue(PRF_PROP_CShapeProfile_FlangeThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetWebThickness(double val)
    {
    SetPropertyValue(PRF_PROP_CShapeProfile_WebThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_CShapeProfile_FilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_CShapeProfile_EdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetFlangeSlope(double val)
    {
    SetPropertyValue(PRF_PROP_CShapeProfile_FlangeSlope, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
