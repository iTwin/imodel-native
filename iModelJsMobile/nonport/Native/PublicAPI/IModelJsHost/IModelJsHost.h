/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>


@interface IModelJsHost : NSObject

- (void)loadBackend: (NSURL*)backendUrl :(NSArray<NSString*>*)searchPaths :(NSString*)currentDir;
- (int)getPort;
- (JSContext*)getContext;
- (BOOL)isReady;
- (void)exec: (JSValue*)function arguments: (NSArray*)arguments;
+ (id)sharedInstance;

@end
