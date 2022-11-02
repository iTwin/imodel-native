/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../BentleyInternal.h"
#include <Bentley/BeSystemInfo.h>

// #import <Foundation/Foundation.h>
// #import <IOKit/IOKitLib.h>
// #import <IOKit/network/IONetworkLib.h>

//---------------------------------------------------------------------------------------
// Gets an identifier for a Mac that is analagous to how Apple uniquely identifies a machine for Apple store receipt purposes.
// This essentially gets the MAC address of the first ethernet device.
// Updated link: https://developer.apple.com/documentation/appstorereceipts/validating_receipts_on_the_device?language=objc
// https://developer.apple.com/library/content/releasenotes/General/ValidateAppStoreReceipt/Chapters/ValidateLocally.html#//apple_ref/doc/uid/TP40010573-CH1-SW14
// @bsimethod
//---------------------------------------------------------------------------------------
// static CFDataRef getMachineID()
    // {
    // mach_port_t master_port;
    // kern_return_t kernResult = IOMasterPort(MACH_PORT_NULL, &master_port);
    // if (KERN_SUCCESS != kernResult)
    //     {
    //     // printf("IOMasterPort returned %d\n", kernResult);
    //     return nil;
    //     }
 
    // CFMutableDictionaryRef matchingDict = IOBSDNameMatching(master_port, 0, "en0");
    // if (nil == matchingDict)
    //     {
    //     // printf("IOBSDNameMatching returned empty dictionary\n");
    //     return nil;
    //     }
 
    // io_iterator_t iterator;
    // kernResult = IOServiceGetMatchingServices(master_port, matchingDict, &iterator);
    // if (KERN_SUCCESS != kernResult)
    //     {
    //     // printf("IOServiceGetMatchingServices returned %d\n", kernResult);
    //     return nil;
    //     }
 
    // CFDataRef macAddress = nil;
    // io_object_t service;
    // while(0 != (service = IOIteratorNext(iterator)))
    //     {
    //     io_object_t parentService;
 
    //     kernResult = IORegistryEntryGetParentEntry(service, kIOServicePlane, &parentService);
    //     if (KERN_SUCCESS == kernResult)
    //         {
    //         if (nil != macAddress)
    //             CFRelease(macAddress);
 
    //         macAddress = (CFDataRef) IORegistryEntryCreateCFProperty(parentService, CFSTR("IOMACAddress"), kCFAllocatorDefault, 0);
    //         IOObjectRelease(parentService);
    //         }
    //     else
    //         {
    //         // printf("IORegistryEntryGetParentEntry returned %d\n", kernResult);
    //         }
 
    //     IOObjectRelease(service);
    //     }
    
    // IOObjectRelease(iterator);
 
    // return macAddress;
    //   }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetOSName ()
    {
    BeAssert (false);
    return "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetOSVersion ()
    {
    BeAssert (false);
    return "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetDeviceId ()
    {
    // FIXME: [Caleb] The old and current way recommended to get the macAddress both use a deprecated portion of the
    // IOKit API. The deprecation is not set to go into effect until macOS 12.0, however we have experienced issues with
    // linking those specific deprecated APIs in newer versions of Node.
    return "";
    }
