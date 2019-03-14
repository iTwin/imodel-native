/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BEKeychainItem.m $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#import "BEKeychainItem.h"
#import "libsrc/KeychainItemWrapper/KeychainItemWrapper.h"

@interface BEKeychainItem ()

@property(nonatomic, retain) KeychainItemWrapper *keychainItemWrapper;

@end

@implementation BEKeychainItem

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
- (id)initWithIdentifier:(NSString*)identifier accessGroup:(NSString*)accessGroup
    {
    if ((self = [super init]) != nil)
        {
        self.keychainItemWrapper = [[[KeychainItemWrapper alloc] initWithIdentifier:identifier accessGroup:accessGroup] autorelease];
        [self.keychainItemWrapper setObject:identifier forKey:(id)kSecAttrService];
        }
    return self;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
- (NSString*)username
    {
    return [self.keychainItemWrapper objectForKey:(id)kSecAttrAccount];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
- (void)setUsername:(NSString*)username
    {
    [self.keychainItemWrapper setObject:username forKey:(id)kSecAttrAccount];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
- (NSString*)password
    {
    return [self.keychainItemWrapper objectForKey:(id)kSecValueData];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
- (void)setPassword:(NSString*)password
    {
    [self.keychainItemWrapper setObject:password forKey:(id)kSecValueData];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          04/2015
//---------------------------------------------------------------------------------------
- (void)deleteItem
    {
    [self.keychainItemWrapper resetKeychainItem];
    }

@end
