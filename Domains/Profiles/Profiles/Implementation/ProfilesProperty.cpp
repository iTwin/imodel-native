/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/ProfilesProperty.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\ProfilesProperty.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsGreaterThanZero (double value)
    {
    return BeNumerical::BeFinite (value) && BeNumerical::Compare (value, 0.0) >= 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsGreaterOrEqualToZero (double value)
    {
    return BeNumerical::BeFinite (value) && BeNumerical::Compare (value, 0.0) >= 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsEqualToZero (double value)
    {
    return BeNumerical::BeFinite (value) && BeNumerical::Compare (value, 0.0) == 0;
    }

END_BENTLEY_PROFILES_NAMESPACE
