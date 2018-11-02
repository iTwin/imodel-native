/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/CenterLineLShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CenterLineLShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(CenterLineLShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineLShapeProfilePtr CenterLineLShapeProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineLShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_CenterLineLShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineLShapeProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_CenterLineLShapeProfile_FilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineLShapeProfile::GetLipLength() const
    {
    return GetPropertyValueDouble(PRF_PROP_CenterLineLShapeProfile_LipLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineLShapeProfile::SetLipLength(double val)
    {
    SetPropertyValue(PRF_PROP_CenterLineLShapeProfile_LipLength, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
