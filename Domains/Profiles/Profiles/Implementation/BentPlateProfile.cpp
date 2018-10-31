/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/BentPlateProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\BentPlateProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(BentPlateProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentPlateProfilePtr BentPlateProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void BentPlateProfile::SetWidth(double val)
    {
    SetPropertyValue(PRF_PROP_BentPlateProfile_Width, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void BentPlateProfile::SetBendAngle(double val)
    {
    SetPropertyValue(PRF_PROP_BentPlateProfile_BendAngle, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void BentPlateProfile::SetBendRadius(double val)
    {
    SetPropertyValue(PRF_PROP_BentPlateProfile_BendRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void BentPlateProfile::SetBendOffset(double val)
    {
    SetPropertyValue(PRF_PROP_BentPlateProfile_BendOffset, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
