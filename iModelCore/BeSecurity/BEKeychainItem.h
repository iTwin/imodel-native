/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
