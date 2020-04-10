//
//  NativeModule.h
//  TestApp
//
//  Created by Affan Khan on 11/21/18.
//  Copyright © 2018 Affan Khan. All rights reserved.
//


#import <JavaScriptCore/JavaScriptCore.h>
NS_ASSUME_NONNULL_BEGIN

@protocol NativeModule <JSExport>
- (bool) nonInternalExists: (NSString*)path;
- (JSValue*) require: (NSString*)str;
- (JSValue*) polyfill: (JSValue*)str;
- (JSValue*) runScript: (NSString*)str;
@end

@interface NativeModule : NSObject<NativeModule>
- (void)extend:(id)jsContext;
- (JSValue*) load:(NSString*) str ;
@end

NS_ASSUME_NONNULL_END

