/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/CenterLineCShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CenterLineCShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(CenterLineCShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineCShapeProfilePtr CenterLineCShapeProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineCShapeProfile::GetFilletRadius() const 
    {
    return GetPropertyValueDouble(PRF_PROP_CenterLineCShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineCShapeProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_CenterLineCShapeProfile_FilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineCShapeProfile::GetLipLength() const 
    {
    return GetPropertyValueDouble(PRF_PROP_CenterLineCShapeProfile_LipLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineCShapeProfile::SetLipLength(double val)
    {
    SetPropertyValue(PRF_PROP_CenterLineCShapeProfile_LipLength, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
