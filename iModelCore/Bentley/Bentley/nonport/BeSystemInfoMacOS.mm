/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../BentleyInternal.h"
#include <Bentley/BeSystemInfo.h>

#import <IOKit/IOKitLib.h>
#import <Foundation/Foundation.h>

//---------------------------------------------------------------------------------------
// Gets an identifier for a Mac that is analagous to how Apple uniquely identifies a machine for Apple store receipt purposes.
// This essentially gets the MAC address of the first ethernet device.
// https://developer.apple.com/library/content/releasenotes/General/ValidateAppStoreReceipt/Chapters/ValidateLocally.html#//apple_ref/doc/uid/TP40010573-CH1-SW14
// @bsimethod                                                   Jeff.Marker     03/2018
//---------------------------------------------------------------------------------------
static CFDataRef getMachineID()
    {
    mach_port_t master_port;
    kern_return_t kernResult = IOMasterPort(MACH_PORT_NULL, &master_port);
    if (KERN_SUCCESS != kernResult)
        {
        // printf("IOMasterPort returned %d\n", kernResult);
        return nil;
        }
 
    CFMutableDictionaryRef matchingDict = IOBSDNameMatching(master_port, 0, "en0");
    if (nil == matchingDict)
        {
        // printf("IOBSDNameMatching returned empty dictionary\n");
        return nil;
        }
 
    io_iterator_t iterator;
    kernResult = IOServiceGetMatchingServices(master_port, matchingDict, &iterator);
    if (KERN_SUCCESS != kernResult)
        {
        // printf("IOServiceGetMatchingServices returned %d\n", kernResult);
        return nil;
        }
 
    CFDataRef macAddress = nil;
    io_object_t service;
    while(0 != (service = IOIteratorNext(iterator)))
        {
        io_object_t parentService;
 
        kernResult = IORegistryEntryGetParentEntry(service, kIOServicePlane, &parentService);
        if (KERN_SUCCESS == kernResult)
            {
            if (nil != macAddress)
                CFRelease(macAddress);
 
            macAddress = (CFDataRef) IORegistryEntryCreateCFProperty(parentService, CFSTR("IOMACAddress"), kCFAllocatorDefault, 0);
            IOObjectRelease(parentService);
            }
        else
            {
            // printf("IORegistryEntryGetParentEntry returned %d\n", kernResult);
            }
 
        IOObjectRelease(service);
        }
    
    IOObjectRelease(iterator);
 
    return macAddress;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/14
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetOSName ()
    {
    BeAssert (false);
    return "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/14
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetOSVersion ()
    {
    BeAssert (false);
    return "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jeff.Marker                     03/18
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetDeviceId ()
    {
    NSData* macData = (__bridge_transfer NSData*)getMachineID();
    if (nil == macData)
        {
        BeAssert (false);
        return "";
        }

    NSString* macString = [macData base64EncodedStringWithOptions:0];
    if (nil == macString)
        {
        BeAssert (false);
        return "";
        }

    Utf8String deviceId(macString.UTF8String);
    return deviceId;
    }
