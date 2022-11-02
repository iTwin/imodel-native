/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../BentleyInternal.h"
#include <Bentley/BeSystemInfo.h>
#import <UIKit/UIDevice.h>

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetOSName ()
    {
    return [[[UIDevice currentDevice] systemName] UTF8String];
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetOSVersion ()
    {
    return [[[UIDevice currentDevice] systemVersion] UTF8String];
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetDeviceId ()
    {
    return [[[[UIDevice currentDevice] identifierForVendor] UUIDString] UTF8String];
    }
