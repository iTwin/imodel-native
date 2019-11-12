/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//  AppRegistry.h
//  
//
//  Created by Affan Khan on 1/23/19.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN
@class AppInfo;

@interface AppRegistry : NSObject
@property (nonatomic, strong) NSString *autoRun;
@property (nonatomic, strong) NSArray<AppInfo*>* apps;
- (AppInfo*) find: (NSString*)appId;
+ (id)sharedInstance;
@end

@interface  AppInfo : NSObject
@property (nonatomic, strong) NSString *appId;
@property (nonatomic, strong) NSString *title;
@property (nonatomic, strong) NSString *backend;
@property (nonatomic, strong) NSString *frontend;
@property (nonatomic, strong) NSDictionary<NSString*, NSString*>* env;
@end
NS_ASSUME_NONNULL_END
