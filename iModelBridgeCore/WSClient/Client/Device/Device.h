/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/Device/Device.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

struct Device : NonCopyableClass
{
private:
    Device() {}

public:
    static void CacheAndroidDeviceId(Utf8StringCR deviceId);
    static Utf8String GetModelName();
    static Utf8String GetOSName();
    static Utf8String GetOSVersion();
    static Utf8String GetDeviceId();
};

END_BENTLEY_WEBSERVICES_NAMESPACE