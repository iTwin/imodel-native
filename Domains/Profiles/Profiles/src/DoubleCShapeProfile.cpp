/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/DoubleCShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\DoubleCShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (DoubleCShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleCShapeProfilePtr DoubleCShapeProfile::Create (/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double DoubleCShapeProfile::GetSpacing() const
    {
    return GetPropertyValueDouble (PRF_PROP_DoubleCShapeProfile_Spacing);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleCShapeProfile::SetSpacing (double value)
    {
    SetPropertyValue (PRF_PROP_DoubleCShapeProfile_Spacing, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
