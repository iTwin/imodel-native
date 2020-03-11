/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>
#import <WebKit/WebKit.h>

@interface IModelJsHost : NSObject

- (void)loadBackend: (NSURL*)backendUrl :(NSArray<NSString*>*)searchPaths :(NSString*)currentDir;
- (int)getPort;
- (JSContext*)getContext;
- (BOOL)isReady;
- (void)exec: (JSValue*)function arguments: (NSArray*)arguments;
- (void)registerWebView: (WKWebView*)view;
+ (id)sharedInstance;

@end
