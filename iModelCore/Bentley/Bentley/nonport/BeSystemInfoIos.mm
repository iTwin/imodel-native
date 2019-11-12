/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../BentleyInternal.h"
#include <Bentley/BeSystemInfo.h>
#import <UIKit/UIDevice.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/14
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetOSName ()
    {
    return [[[UIDevice currentDevice] systemName] UTF8String];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/14
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetOSVersion ()
    {
    return [[[UIDevice currentDevice] systemVersion] UTF8String];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Vincas.Razma                    05/15
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetDeviceId ()
    {
    return [[[[UIDevice currentDevice] identifierForVendor] UUIDString] UTF8String];
    }
