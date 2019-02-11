/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/BentPlateProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <Profiles\BentPlateProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (BentPlateProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double BentPlateProfile::GetWidth() const 
    {
    return GetPropertyValueDouble (PRF_PROP_BentPlateProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void BentPlateProfile::SetWidth (double value)
    {
    SetPropertyValue (PRF_PROP_BentPlateProfile_Width, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double BentPlateProfile::GetBendAngle() const 
    {
    return GetPropertyValueDouble (PRF_PROP_BentPlateProfile_BendAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void BentPlateProfile::SetBendAngle (double value)
    {
    SetPropertyValue (PRF_PROP_BentPlateProfile_BendAngle, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double BentPlateProfile::GetBendRadius() const 
    {
    return GetPropertyValueDouble (PRF_PROP_BentPlateProfile_BendRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void BentPlateProfile::SetBendRadius (double value)
    {
    SetPropertyValue (PRF_PROP_BentPlateProfile_BendRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double BentPlateProfile::GetBendOffset() const 
    {
    return GetPropertyValueDouble (PRF_PROP_BentPlateProfile_BendOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void BentPlateProfile::SetBendOffset (double value)
    {
    SetPropertyValue (PRF_PROP_BentPlateProfile_BendOffset, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
