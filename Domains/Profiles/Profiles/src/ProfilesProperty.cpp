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
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsEqual (double leftValue, double rightValue)
    {
    return BeNumerical::BeFinite (leftValue) && BeNumerical::BeFinite (rightValue) && BeNumerical::Compare (leftValue, rightValue) == 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsGreater (double leftValue, double rightValue)
    {
    return BeNumerical::BeFinite (leftValue) && BeNumerical::BeFinite (rightValue) && BeNumerical::Compare (leftValue, rightValue) == 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsGreaterOrEqual (double leftValue, double rightValue)
    {
    return BeNumerical::BeFinite (leftValue) && BeNumerical::BeFinite (rightValue) && BeNumerical::Compare (leftValue, rightValue) >= 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsLess (double leftValue, double rightValue)
    {
    return BeNumerical::BeFinite (leftValue) && BeNumerical::BeFinite (rightValue) && BeNumerical::Compare (leftValue, rightValue) == -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsLessOrEqual (double leftValue, double rightValue)
    {
    return BeNumerical::BeFinite (leftValue) && BeNumerical::BeFinite (rightValue) && BeNumerical::Compare (leftValue, rightValue) <= 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsEqualToZero (double value)
    {
    return BeNumerical::BeFinite (value) && BeNumerical::IsEqualToZero (value);
    }

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

END_BENTLEY_PROFILES_NAMESPACE
