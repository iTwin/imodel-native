/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Client/Connect/BEUniqueIdManager.m $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#import "BEUniqueIdManager.h"
#import "BEKeychainItem.h"
#import "../libsrc/KeychainItemWrapper/KeychainItemWrapper.h"

@interface BEUniqueIdManager ()
{
__strong KeychainItemWrapper* _keyChainItemWrapper;
}
@end

@implementation BEUniqueIdManager

//---------------------------------------------------------------------------------------
// @bsimethod                               Satyakam.Khadilkar          08/2013
//---------------------------------------------------------------------------------------
+ (NSString*) uniqueId
    {
    CFUUIDRef theUUID = CFUUIDCreate(NULL);
    CFStringRef string = CFUUIDCreateString(NULL, theUUID);
    CFRelease(theUUID);
    return (__bridge_transfer NSString*)(string);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Satyakam.Khadilkar          08/2013
//---------------------------------------------------------------------------------------
- (id) initWithIdentifier:(NSString*)identifier accessGroup:(NSString *)accessGroup
    {
    if (self = [super init])
        {
        _keyChainItemWrapper = [[KeychainItemWrapper alloc] initWithIdentifier:identifier accessGroup:accessGroup];
        [_keyChainItemWrapper setObject:identifier forKey:(__bridge id)(kSecAttrService)];
        }
    return self;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Satyakam.Khadilkar          08/2013
//---------------------------------------------------------------------------------------
- (id) initForDevice
    {
    id accessGroup = [[BEKeychainItem commonAccessGroupPrefix] stringByAppendingString:@"BentleyAppUniqueId"];
    return [self initWithIdentifier:@"BentleyApp-UniqueId" accessGroup:accessGroup];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Satyakam.Khadilkar          08/2013
//---------------------------------------------------------------------------------------
- (NSString*) persistentUniqueId
    {
    NSString* deviceId = [_keyChainItemWrapper objectForKey:(__bridge id)(kSecValueData)];
    if (![deviceId length])
        {
        deviceId = [BEUniqueIdManager uniqueId];
        [_keyChainItemWrapper setObject:deviceId forKey:(__bridge id)(kSecValueData)];
        }
    return deviceId;
    }

@end
