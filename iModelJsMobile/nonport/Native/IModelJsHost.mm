//
//  main.m
//  TestApp
//
//  Created by Affan Khan on 11/6/18.
//  Copyright © 2018 Affan Khan. All rights reserved.
//

// Note: the following MUST be above any iOS includes, since there is a B0 macro
// buried in the bowels of the iOS includes that causes a compile error in some file
// included in DgnPlatformLib with a function definition that uses B0 as a parameter.
#include <DgnPlatform/DgnPlatformLib.h>
#include <BeSQLite/L10N.h>

#import "PublicAPI/IModelJsHost/IModelJsHost.h"
#import <UIKit/UIKit.h>

#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>
#include <iModelJs/iModelJs.h>
#include <iModelJs/iModelJsServicesTier.h>

#import "Bindings/XMLHttpRequest.h"
#import "Bindings/WindowTimers.h"
#import "Bindings/FileSystem.h"
#import "Bindings/VM.h"
#import "Bindings/NativeModule.h"
#import "Bindings/WindowTimers.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

@implementation IModelJsHost {
    JSContext* _jsContext;
    BentleyApi::iModelJs::ServicesTier::UvHostPtr _host;
}
extern "C" {
    void imodeljs_addon_setMobileResourcesDir(Utf8CP d);
    void imodeljs_addon_setMobileTempDir(Utf8CP d);
}
+ (id)sharedInstance {
    static IModelJsHost *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
     });

    return sharedInstance;
}
- (id)init {
    if ((self = [super init]) != nil) {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive) name:UIApplicationWillResignActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive) name:UIApplicationDidBecomeActiveNotification object:nil];
    }
    return self;
}
- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}
- (void)applicationWillResignActive {
    DgnPlatformLib::GetHost().GetFontAdmin().Suspend();
    L10N::Suspend();
}
- (void)applicationDidBecomeActive {
    // Note: the order probably doesn't matter, but these calls are in the reverse order
    // of the calls in applicationWillResignActive, just to be safe.
    L10N::Resume();
    DgnPlatformLib::GetHost().GetFontAdmin().Resume();
}
- (BOOL)isReady {
    if (_host.IsValid() && _host->IsReady())
        return YES;
    return NO;
}
- (int)getPort {
    if (_host.IsNull()) {
        return 0;
    }

   return BentleyApi::iModelJs::ServicesTier::MobileGateway::GetInstance().GetPort();
}
- (void)loadBackend: (NSURL*)backendUrl :(NSArray<NSString*>*)searchPaths :(NSString*)currentDir {
    if (_host.IsValid()) {
        @throw [NSException exceptionWithName:@"BackendAlreadyLoaded" reason:@"Backend is already" userInfo:nil];
    }

    using namespace BentleyApi::iModelJs;
    NSString *appFolderPath = [[NSBundle mainBundle] resourcePath];
    NSString *iModelJsNativePath = [appFolderPath stringByAppendingPathComponent:@"iModelJsNative"];
    NSString *iTempFolder = NSTemporaryDirectory();
    imodeljs_addon_setMobileResourcesDir(iModelJsNativePath.UTF8String);
    imodeljs_addon_setMobileTempDir(iTempFolder.UTF8String);
    _host = new ServicesTier::UvHost;
    while (!_host->IsReady()) { ; }
    JSGlobalContextRef jsGlobalContext = JSContextGetGlobalContext(_host->GetContext());
    _jsContext =[JSContext contextWithJSGlobalContextRef:jsGlobalContext];
    _jsContext.exceptionHandler = ^(JSContext *context, JSValue *exception) {
        NSLog(@"[JSC] %@: [#%d:%d] - %@",
              [exception objectForKeyedSubscript:@"name"].toString,
              [exception objectForKeyedSubscript:@"line"].toInt32,
              [exception objectForKeyedSubscript:@"column"].toInt32,
              [exception objectForKeyedSubscript:@"message"].toString);
        
        NSLog(@"Stack: %@",
              [exception objectForKeyedSubscript:@"stack"].toString);
    };
    
    _jsContext[@"internalBinding"] = ^(JSValue* val){
        NSString* name = val.toString;
        NSLog(@"internalBinding('%@')",  name);
    };
    // change current directory
    if(currentDir) {
        NSString* resourcePath = [[NSBundle mainBundle] bundlePath];
        NSString* newFolder = [resourcePath stringByAppendingPathComponent:currentDir];
        NSFileManager *filemgr = [NSFileManager defaultManager];
        if([filemgr changeCurrentDirectoryPath:newFolder] == NO) {
            NSLog(@"Cannot change current directory from '%@' to '%@'", [filemgr currentDirectoryPath], newFolder);
        }
    }
    BootstrapBackend(_jsContext, backendUrl, searchPaths);
    
    _host->PostToEventLoop([]() {
                               auto& runtime = ServicesTier::Host::GetInstance().GetJsRuntime();
                               auto evaluated = runtime.EvaluateScript ("native_module.runScript('bootstrap');");
                               BeAssert (evaluated.status == Js::EvaluateStatus::Success);
                           });
    
}
- (JSContext*)getContext {
    return _jsContext;
}

- (void)exec: (JSValue*)function arguments: (NSArray*)arguments {
    _host->PostToEventLoop([=]() {
        // Unfortunately, JavaScript allows it that the first argument is a string which will be
        // evaluated as callback.
        if ([function isString]) {
            [function.context evaluateScript:[function toString]];
            // In all other cases, execute the callback. TODO explain the arguments.
        } else {
            [function callWithArguments:arguments];
        }
    });
}

void BootstrapBackend(JSContext* ctx, NSURL* backendUrl, NSArray<NSString*>* searchPaths) {
    NSString* resourcePath = [[NSBundle mainBundle] bundlePath];
    NSProcessInfo* currentProcess = [NSProcessInfo processInfo];
    NSMutableArray<NSString*>* paths = [NSMutableArray<NSString*> new];
    [paths addObject: [resourcePath stringByAppendingPathComponent:@"iModelJsNative/Assets/system"]];
    [paths addObject: [resourcePath stringByAppendingPathComponent:@"iModelJsNative/Assets/system/polyfill"]];
    [paths addObject: [resourcePath stringByAppendingPathComponent:@"iModelJsNative/Assets/system/polyfill/node_modules"]];
    if (searchPaths != nil) {
        for (NSString* searchPath in searchPaths) {
            [paths addObject: [resourcePath stringByAppendingPathComponent:searchPath]];
        }
    }
    ctx[@"process"] = [JSValue valueWithNewObjectInContext:ctx];
    ctx[@"process"][@"env"] = [[NSProcessInfo processInfo] environment];
    ctx[@"process"][@"env"][@"DOCS"] = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    ctx[@"process"][@"env"][@"NODE_PATH"] = [paths componentsJoinedByString:@":"];
    ctx[@"process"][@"env"][@"HOME"] = NSHomeDirectory();
    ctx[@"process"][@"env"][@"TEMP"] = NSTemporaryDirectory();
    ctx[@"process"][@"env"][@"USERNAME"] = NSUserName();
    ctx[@"process"][@"env"][@"FULLUSERNAME"] = NSFullUserName();
    ctx[@"process"][@"env"][@"HOSTNAME"] = [[NSProcessInfo processInfo] hostName];
    ctx[@"process"][@"env"][@"OS_VERSION"] = currentProcess.operatingSystemVersionString;
    ctx[@"process"][@"env"][@"PROCESSOR_COUNT"] = [JSValue valueWithDouble:currentProcess.processorCount inContext:ctx];
    ctx[@"process"][@"env"][@"ACTIVE_PROCESSOR_COUNT"] = [JSValue valueWithDouble:currentProcess.activeProcessorCount inContext:ctx];
    ctx[@"process"][@"env"][@"PHYSICAL_MEMORY"] =[JSValue valueWithDouble:currentProcess.physicalMemory inContext:ctx];
    ctx[@"process"][@"env"][@"PROCESS_NAME"] = [[NSProcessInfo processInfo] processName];
    ctx[@"process"][@"env"][@"PROCESS_ID"] = [JSValue valueWithDouble:currentProcess.processIdentifier inContext:ctx];
  
    ctx[@"process"][@"execPath"] = NSBundle.mainBundle.bundlePath;
    ctx[@"process"][@"argv"] = NSProcessInfo.processInfo.arguments;
    ctx[@"process"][@"title"] = NSProcessInfo.processInfo.processName;
    ctx[@"process"][@"platform"] =  @"iOS";
    ctx[@"process"][@"browser"] =  @(YES);
    ctx[@"process"][@"versions"] =  [NSArray new];
    ctx[@"process"][@"version"] =  @"1.0.0";
    ctx[@"process"][@"cwd"] = ^(){ return [NSFileManager.defaultManager currentDirectoryPath]; };
    ctx[@"process"][@"exit"] = ^() { exit(0); };
    ctx[@"process"][@"ext"] = [JSValue valueWithNewObjectInContext:ctx];
    ctx[@"process"][@"chdir"] = ^(NSString* newFolder){
        NSFileManager *filemgr = [NSFileManager defaultManager];
        if([filemgr changeCurrentDirectoryPath:newFolder] == NO) {
            NSLog(@"Cannot change current directory from %@ to %@", [filemgr currentDirectoryPath], newFolder);
        }
    };
    ctx[@"process"][@"log"] = ^( NSString* prefix, NSString* msg) {
        NSLog(@"[%@] %@", prefix, msg);
    };
    
    [[WTWindowTimers new] extend: ctx];
    [[XMLHttpRequest new] extend: ctx];
    [[NativeModule new] extend: ctx];
    [[FileSystem new] extend: ctx];
    [[VM new] extend: ctx];
    
    //assert
    ctx[@"assert"] = [JSValue valueWithNewObjectInContext:ctx];
    ctx[@"assert"][@"ok"] = ^(JSValue* val){
        assert(val.toBool == true);
    };
    
    //util
    ctx[@"util_ex"] = [JSValue valueWithNewObjectInContext:ctx];
    ctx[@"util_ex"][@"isString"] = ^(JSValue* val){
        return [JSValue valueWithBool:val.isString inContext:val.context];
    };
    ctx[@"util_ex"][@"debuglog"] = ^(JSValue* val){
        __block NSString* logCat = val.toString;
        return ^(NSString* str){
            NSLog(@"[%@] %@", logCat, str);
        };
    };
    

    // load this module when finish setting up the loader
    ctx[@"process"][@"env"][@"BACKEND_URL"] = backendUrl.absoluteString;
    [ctx evaluateScript:@"function require(str) { return native_module.require(str);}"];
    ctx[@"module"] = [JSValue valueWithNewObjectInContext:ctx];
}
@end
