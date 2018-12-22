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
* @bsiclass                                                                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProfilesProperty
    {
public:
    ProfilesProperty() = delete;

    // TODO Karolis: We should place this header in PrivateAPI so its not visible for users of the domain
    static bool IsGreaterThanZero (double value);
    static bool IsGreaterOrEqualToZero (double value);
    static bool IsEqualToZero (double value);
    };

END_BENTLEY_PROFILES_NAMESPACE
