/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/HollowRectangleProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\HollowRectangleProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(HollowRectangleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowRectangleProfilePtr HollowRectangleProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowRectangleProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_HollowRectangleProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowRectangleProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_HollowRectangleProfile_FilletRadius, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
