/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PrivateAPI/ProfilesProperty.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

    static bool IsGreaterThanZero (double value);
    static bool IsGreaterOrEqualToZero (double value);
    static bool IsEqualToZero (double value);
    };

END_BENTLEY_PROFILES_NAMESPACE
