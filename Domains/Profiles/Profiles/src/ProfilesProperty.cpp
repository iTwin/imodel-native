/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ProfilesProperty.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <ProfilesInternal\ProfilesProperty.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsGreaterThanZero (double value)
    {
    return BeNumerical::BeFinite (value) && BeNumerical::IsGreaterThanZero (value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsGreaterOrEqualToZero (double value)
    {
    return BeNumerical::BeFinite (value) && BeNumerical::IsGreaterOrEqualToZero (value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsEqualToZero (double value)
    {
    return BeNumerical::BeFinite (value) && BeNumerical::IsEqualToZero (value);
    }

END_BENTLEY_PROFILES_NAMESPACE
