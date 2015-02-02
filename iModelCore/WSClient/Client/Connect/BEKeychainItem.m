/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Client/Connect/BEKeychainItem.m $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#import "BEKeychainItem.h"
#import "../libsrc/KeychainItemWrapper/KeychainItemWrapper.h"

static NSDictionary *g_keychainInfos = nil;
static NSString *g_accessGroupPrefix = nil;
static NSString *const kAWSIdentifier = @"AWS";

//=======================================================================================
// @bsiclass                                              Travis.Cobbs          02/2013
//=======================================================================================
@interface BEKeychainInfo : NSObject

@property(nonatomic, copy) NSString *identifier;
@property(nonatomic, copy) NSString *accessGroup;

+ (id)keychainInfoWithIdentifier:(NSString *)identifier accessGroup:(NSString *)accessGroup;
- (id)initWithIdentifier:(NSString *)identifier accessGroup:(NSString *)accessGroup;

@end

//=======================================================================================
// @bsiclass                                              Travis.Cobbs          02/2013
//=======================================================================================
@implementation BEKeychainInfo

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
- (id)initWithIdentifier:(NSString *)identifier accessGroup:(NSString *)accessGroup
    {
    if ((self = [super init]) != nil)
        {
        self.identifier = identifier;
        self.accessGroup = accessGroup;
        }
    return self;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
+ (id)keychainInfoWithIdentifier:(NSString *)identifier accessGroup:(NSString *)accessGroup
    {
    return [[[self alloc] initWithIdentifier:identifier accessGroup:accessGroup] autorelease];
    }

@end

//=======================================================================================
// @bsiclass                                              Travis.Cobbs          02/2013
//=======================================================================================
@interface BEKeychainItem ()

@property(nonatomic, retain) KeychainItemWrapper *keychainItemWrapper;

+ (void)initializeAccessGroupPrefix:(NSString *) prefix;

@end

//=======================================================================================
// To add new items, four things need to be done:
//   * Add a new identifier constant (next to kAWSIdentifier above).
//   * Update +initialize to add a new entry to g_keychainInfos for the new item.
//   * Create a new +someKeychainItem method modeled after +awsKeychainItem, but using
//     the new identifier constant.
//   * Add the new access group to Entitlements.plist and Entitlements-debug.plist.
// @bsiclass                                              Travis.Cobbs          02/2013
//=======================================================================================
@implementation BEKeychainItem

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
+ (void)initializeAccessGroupPrefix:(NSString *) prefix
    {
    g_accessGroupPrefix = prefix;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Vincas.Razma          01/2014
//---------------------------------------------------------------------------------------
+ (NSString *)commonAccessGroupPrefix
    {
    return g_accessGroupPrefix;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
+ (void)initialize
    {
    g_keychainInfos =
        @{
        kAWSIdentifier: [BEKeychainInfo keychainInfoWithIdentifier:@"AWS Credentials" accessGroup:@"AWS"],
        };
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
+ (BEKeychainItem *)awsKeychainItem
    {
    return [self itemWithIdentifier:kAWSIdentifier];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          04/2013
//---------------------------------------------------------------------------------------
+ (BEKeychainItem *)connectTokenKeychainItemWithIdentifier:(NSString *)identifier;
    {
    return [[[BEKeychainItem alloc] initWithIdentifier:identifier accessGroup:[g_accessGroupPrefix stringByAppendingString:@"ConnectToken"]] autorelease];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          08/2013
//---------------------------------------------------------------------------------------
+ (BEKeychainItem *)connectEulaTokenKeychainItemWithIdentifier:(NSString *)identifier;
    {
    return [[[BEKeychainItem alloc] initWithIdentifier:identifier accessGroup:[g_accessGroupPrefix stringByAppendingString:@"ConnectEulaToken"]] autorelease];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          04/2013
//---------------------------------------------------------------------------------------
+ (BEKeychainItem *)connectKeychainItemWithIdentifier:(NSString *)identifier;
    {
    return [[[self alloc] initWithIdentifier:identifier accessGroup:[g_accessGroupPrefix stringByAppendingString:@"Connect"]] autorelease];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Pavan.Emani           02/2013
//---------------------------------------------------------------------------------------
+ (BEKeychainItem *)wsbKeychainItemWithIdentifier:(NSString *)identifier
    {
    return [[[self alloc] initWithIdentifier:identifier accessGroup:[g_accessGroupPrefix stringByAppendingString:@"WSB"]] autorelease];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
+ (BEKeychainItem *)itemWithIdentifier:(NSString *)identifier
    {
    BEKeychainInfo *info = g_keychainInfos[identifier];
    if (info != nil)
        {
        return [[[self alloc] initWithIdentifier:info.identifier accessGroup:[g_accessGroupPrefix stringByAppendingString:info.accessGroup]] autorelease];
        }
    else
        {
        return nil;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
- (id)initWithIdentifier:(NSString *)identifier accessGroup:(NSString *)accessGroup
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
- (NSString *)username
    {
    return [self.keychainItemWrapper objectForKey:(id)kSecAttrAccount];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
- (void)setUsername:(NSString *)username
    {
    [self.keychainItemWrapper setObject:username forKey:(id)kSecAttrAccount];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
- (NSString *)password
    {
    return [self.keychainItemWrapper objectForKey:(id)kSecValueData];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Travis.Cobbs          02/2013
//---------------------------------------------------------------------------------------
- (void)setPassword:(NSString *)password
    {
    [self.keychainItemWrapper setObject:password forKey:(id)kSecValueData];
    }

@end
