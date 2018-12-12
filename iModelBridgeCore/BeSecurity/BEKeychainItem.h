/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BEKeychainItem.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>

//=======================================================================================
// @bsiclass                                              Travis.Cobbs          02/2013
//=======================================================================================
@interface BEKeychainItem : NSObject

@property (nonatomic, retain) NSString* username;
@property (nonatomic, retain) NSString* password;

//! Create item for accessing keychain
- (id)initWithIdentifier:(NSString*)identifier accessGroup:(NSString*)accessGroup;

//! Delete item from keychain
- (void)deleteItem;

@end
