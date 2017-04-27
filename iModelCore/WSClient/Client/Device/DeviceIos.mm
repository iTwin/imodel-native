/*--------------------------------------------------------------------------------------+
|     $Source: $
|  $Copyright: $
+--------------------------------------------------------------------------------------*/
#include "../ClientInternal.h"
#include "Device.h"
#import <UIKit/UIDevice.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/14
//---------------------------------------------------------------------------------------
Utf8String Device::GetModelName ()
    {
    return [[[UIDevice currentDevice] model] UTF8String];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/14
//---------------------------------------------------------------------------------------
Utf8String Device::GetOSName ()
    {
    return [[[UIDevice currentDevice] systemName] UTF8String];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/14
//---------------------------------------------------------------------------------------
Utf8String Device::GetOSVersion ()
    {
    return [[[UIDevice currentDevice] systemVersion] UTF8String];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Vincas.Razma                    05/15
//---------------------------------------------------------------------------------------
Utf8String Device::GetDeviceId ()
    {
    return [[[[UIDevice currentDevice] identifierForVendor] UUIDString] UTF8String];
    }
