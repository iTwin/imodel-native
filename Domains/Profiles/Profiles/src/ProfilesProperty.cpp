/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesProperty.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsEqual (double leftValue, double rightValue)
    {
    return BeNumerical::BeFinite (leftValue) && BeNumerical::BeFinite (rightValue) && BeNumerical::IsEqual (leftValue, rightValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsGreater (double leftValue, double rightValue)
    {
    return BeNumerical::BeFinite (leftValue) && BeNumerical::BeFinite (rightValue) && BeNumerical::IsGreater (leftValue, rightValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsGreaterOrEqual (double leftValue, double rightValue)
    {
    return BeNumerical::BeFinite (leftValue) && BeNumerical::BeFinite (rightValue) && BeNumerical::IsGreaterOrEqual (leftValue, rightValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsLess (double leftValue, double rightValue)
    {
    return BeNumerical::BeFinite (leftValue) && BeNumerical::BeFinite (rightValue) && BeNumerical::IsLess (leftValue, rightValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProfilesProperty::IsLessOrEqual (double leftValue, double rightValue)
    {
    return BeNumerical::BeFinite (leftValue) && BeNumerical::BeFinite (rightValue) && BeNumerical::IsLessOrEqual (leftValue, rightValue);
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
