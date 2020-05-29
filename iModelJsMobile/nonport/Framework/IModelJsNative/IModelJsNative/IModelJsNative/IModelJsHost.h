/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>

@interface IModelJsHost : NSObject
- (void)loadBackend: (NSURL*)backendUrl;
- (int)getPort;
- (void)registerWebView: (WKWebView*)view;
+ (id)sharedInstance;
@end
