/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/WString.h>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
//! Static methods for retrieving system information.
// @bsiclass
//=======================================================================================
struct BeSystemInfo
{
private:
    BeSystemInfo () {};

public:
    //! If sucessful will return non-zero value for device physical memory, otherwise - 0.
    BENTLEYDLL_EXPORT static uint64_t GetAmountOfPhysicalMemory ();

    //! Caches the DeviceId for Android platform.
    //! To get DeviceId us the following Java code:
    //! String deviceId = Secure.getString(context.getContentResolver(), Secure.ANDROID_ID);
    //! The context here is an intance of class derived from Context, usually instance of the Activity class.
    BENTLEYDLL_EXPORT static void CacheAndroidDeviceId(Utf8StringCR deviceId);

    //! Returns detailed device model name, currently only implemented for iOS
    BENTLEYDLL_EXPORT static Utf8String GetModelName();

    //! Returns device name, like "Nexus 7", "iPhone9,4", etc..
    BENTLEYDLL_EXPORT static Utf8String GetMachineName();

    //! Returns the number of processors currently available.
    BENTLEYDLL_EXPORT static uint32_t GetNumberOfCpus();

    //! Returns operating system name
    BENTLEYDLL_EXPORT static Utf8String GetOSName();

    //! Returns operating system version
    BENTLEYDLL_EXPORT static Utf8String GetOSVersion();

    //! Returns unique device id.
    //! For Android: if used without DgnClientFx library - please refer to @ref CacheAndroidDeviceId method
    BENTLEYDLL_EXPORT static Utf8String GetDeviceId();
};

END_BENTLEY_NAMESPACE

