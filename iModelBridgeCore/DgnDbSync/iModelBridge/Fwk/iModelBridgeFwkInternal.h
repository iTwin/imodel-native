/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatform/DesktopTools/envvutil.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
bool SetValueIfEmptyFromEnv(WCharCP envValue, T& arg)
    {
    if (!arg.empty())
        return false;

    WString value;
    if (SUCCESS != util_getSysEnv(&value, envValue))
        return false;
    
    arg = T(value);
    return true;
    }

template <typename T>
bool SetValueFromEnv(WCharCP envValue, T& arg)
    {
    WString value;
    if (SUCCESS != util_getSysEnv(&value, envValue))
        return false;
    
    arg = T(BeStringUtilities::Wtoi(value.c_str()));
    return true;
    }