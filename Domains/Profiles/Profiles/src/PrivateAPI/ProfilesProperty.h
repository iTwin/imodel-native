/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Profiles\ProfilesDefinitions.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//*
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProfilesProperty
    {
public:
    ProfilesProperty() = delete;

    static bool IsEqual (double leftValue, double rightValue);
    static bool IsGreater (double leftValue, double rightValue);
    static bool IsGreaterOrEqual (double leftValue, double rightValue);
    static bool IsLess (double leftValue, double rightValue);
    static bool IsLessOrEqual (double leftValue, double rightValue);

    static bool IsEqualToZero (double value);
    static bool IsGreaterThanZero (double value);
    static bool IsGreaterOrEqualToZero (double value);
    };

END_BENTLEY_PROFILES_NAMESPACE
