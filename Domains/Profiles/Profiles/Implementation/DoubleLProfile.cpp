/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/DoubleLProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\DoubleLProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(DoubleLProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleLProfilePtr DoubleLProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double DoubleLProfile::GetSpacing() const
    {
    return GetPropertyValueDouble(PRF_PROP_DoubleLProfile_Spacing);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleLProfile::SetSpacing(double val)
    {
    SetPropertyValue(PRF_PROP_DoubleLProfile_Spacing, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int DoubleLProfile::GetEnum() const
    {
    return GetPropertyValueInt32(PRF_PROP_DoubleLProfile_Enum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleLProfile::SetEnum(int val)
    {
    SetPropertyValue(PRF_PROP_DoubleLProfile_Enum, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
