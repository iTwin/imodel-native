/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Client/Connect/BEKeychainItem.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>

//=======================================================================================
// @bsiclass                                              Travis.Cobbs          02/2013
//=======================================================================================
@interface BEKeychainItem : NSObject

@property (nonatomic, retain) NSString *username;
@property (nonatomic, retain) NSString *password;

+ (void)initializeAccessGroupPrefix:(NSString *) prefix;
+ (NSString *)commonAccessGroupPrefix;

+ (BEKeychainItem *)awsKeychainItem;
+ (BEKeychainItem *)wsbKeychainItemWithIdentifier:(NSString *)identifier;
+ (BEKeychainItem *)connectTokenKeychainItemWithIdentifier:(NSString *)identifier;
+ (BEKeychainItem *)connectKeychainItemWithIdentifier:(NSString *)identifier;
+ (BEKeychainItem *)connectEulaTokenKeychainItemWithIdentifier:(NSString *)identifier;

- (id)initWithIdentifier:(NSString *)identifier accessGroup:(NSString *)accessGroup;

@end
