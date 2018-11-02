/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/DoubleCProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\DoubleCProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(DoubleCProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleCProfilePtr DoubleCProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double DoubleCProfile::GetSpacing() const
    {
    return GetPropertyValueDouble(PRF_PROP_DoubleCProfile_Spacing);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleCProfile::SetSpacing(double val)
    {
    SetPropertyValue(PRF_PROP_DoubleCProfile_Spacing, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
