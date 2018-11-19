/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/CapsuleProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CapsuleProfile.h>

USING_NAMESPACE_BENTLEY_DGN
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
void CapsuleProfile::SetWidth (double val)
    {
    SetPropertyValue (PRF_PROP_CapsuleProfile_Width, ECN::ECValue (val));
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
void CapsuleProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_CapsuleProfile_Depth, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
