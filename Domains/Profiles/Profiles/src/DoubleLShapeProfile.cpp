/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/DoubleLShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\DoubleLShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (DoubleLShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleLShapeProfilePtr DoubleLShapeProfile::Create (/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double DoubleLShapeProfile::GetSpacing() const
    {
    return GetPropertyValueDouble (PRF_PROP_DoubleLShapeProfile_Spacing);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleLShapeProfile::SetSpacing (double val)
    {
    SetPropertyValue (PRF_PROP_DoubleLShapeProfile_Spacing, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int DoubleLShapeProfile::GetType() const
    {
    return GetPropertyValueInt32 (PRF_PROP_DoubleLShapeProfile_Type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleLShapeProfile::SetType (int val)
    {
    SetPropertyValue (PRF_PROP_DoubleLShapeProfile_Type, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
