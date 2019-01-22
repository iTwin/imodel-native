/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/CapsuleProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CapsuleProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (CapsuleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CapsuleProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CapsuleProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CapsuleProfile::SetWidth (double value)
    {
    SetPropertyValue (PRF_PROP_CapsuleProfile_Width, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CapsuleProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CapsuleProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CapsuleProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_CapsuleProfile_Depth, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
