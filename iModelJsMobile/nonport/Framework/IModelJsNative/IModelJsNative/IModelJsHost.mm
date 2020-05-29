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
#include <vector>

#include <DgnPlatform/DgnPlatformLib.h>
#include <BeSQLite/L10N.h>
#include <Napi/napi.h>

#import "IModelJsNative/IModelJsHost.h"
#include <iModelJs/MobileBackend.h>

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#include <stdlib.h>
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

@interface IModelJsHost()
- (void)reconnectWebView;
@end

@implementation IModelJsHost {
    WKWebView* _webView;
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

- (int)getPort {
    return MobileBackend::GetPort();
}

+ (Napi::Number) getOrientation: (napi_env)env {
    switch (UIDevice.currentDevice.orientation) {
        case UIDeviceOrientationUnknown:
            return Napi::Number::New(env, 0x0);
        case UIDeviceOrientationPortrait:
            return Napi::Number::New(env, 0x1);
        case UIDeviceOrientationPortraitUpsideDown:
            return Napi::Number::New(env, 0x2);
        case UIDeviceOrientationLandscapeLeft:
            return Napi::Number::New(env, 0x4);
        case UIDeviceOrientationLandscapeRight:
            return Napi::Number::New(env, 0x8);
        case UIDeviceOrientationFaceUp:
            return Napi::Number::New(env, 0x10);
        case UIDeviceOrientationFaceDown:
            return Napi::Number::New(env, 0x20);
    }
    return Napi::Number::New(env, 0x0);
}

+ (Napi::Number) getBatteryState: (napi_env)env {
    switch (UIDevice.currentDevice.batteryState) {
        case UIDeviceBatteryStateUnknown:
            return Napi::Number::New(env, 0);
        case UIDeviceBatteryStateUnplugged:
            return Napi::Number::New(env, 1);
        case UIDeviceBatteryStateCharging:
            return Napi::Number::New(env, 2);
        case UIDeviceBatteryStateFull:
            return Napi::Number::New(env, 3);
    }
    return Napi::Number::New(env, 0);
}

- (void)loadBackend: (NSURL*)backendUrl {
    NSString *iModelJsNativePath = [[NSBundle bundleWithIdentifier:@"bentley.IModelJsNative"] resourcePath];
    NSString *iTempFolder = NSTemporaryDirectory();
    imodeljs_addon_setMobileResourcesDir(iModelJsNativePath.UTF8String);
    imodeljs_addon_setMobileTempDir(iTempFolder.UTF8String);

    // sets doc dir
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    setenv(@"NSDocumentDirectory".UTF8String , documentsDirectory.UTF8String, true);

    std::vector<Utf8CP> args { "node", "--inspect=0.0.0.0:9229", backendUrl.path.UTF8String };
    MobileBackend::Start(args.size(), args.data());
    
    // Memory pressure events
    [[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationDidReceiveMemoryWarningNotification
     object:[UIApplication sharedApplication] queue:nil usingBlock:^(NSNotification *notif) {
        MobileBackend::RunOnEventLoop([](napi_env env) {
            auto global = Napi::Env(env).Global();
            auto device = global.Get("device").As<Napi::Object>();
            if (device.IsUndefined())
                return;
            
            auto eventName = Napi::String::New(env, "memoryWarning");
            auto emitFunc = device.Get("emit").As<Napi::Function>();
            emitFunc.Call({eventName});
        });
    }];
    // Orientation changed
    [[NSNotificationCenter defaultCenter] addObserverForName:UIDeviceOrientationDidChangeNotification object:nil queue:nil usingBlock:^(NSNotification *notification) {
        MobileBackend::RunOnEventLoop([](napi_env env) {
            auto global = Napi::Env(env).Global();
            auto device = global.Get("device").As<Napi::Object>();
            if (device.IsUndefined())
                return;
            
            auto eventName = Napi::String::New(env, "orientationChanged");
            auto orientation = [IModelJsHost getOrientation: env];
            auto emitFunc = device.Get("emit").As<Napi::Function>();
            emitFunc.Call({eventName, orientation});
        });
    }];
    // onEnterForground
    [[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationWillEnterForegroundNotification object:nil queue:nil usingBlock:^(NSNotification *notification) {
        MobileBackend::RunOnEventLoop([](napi_env env) {
            auto global = Napi::Env(env).Global();
            auto device = global.Get("device").As<Napi::Object>();
            if (device.IsUndefined())
                return;
            
            auto eventName = Napi::String::New(env, "enterForground");
            auto emitFunc = device.Get("emit").As<Napi::Function>();
            emitFunc.Call({eventName});
        });
    }];
    
    // onEnterBackground
    [[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationDidEnterBackgroundNotification object:nil queue:nil usingBlock:^(NSNotification *notification) {
        MobileBackend::RunOnEventLoop([](napi_env env) {
            auto global = Napi::Env(env).Global();
            auto device = global.Get("device").As<Napi::Object>();
            if (device.IsUndefined())
                return;
            
            auto eventName = Napi::String::New(env, "enterBackground");
            auto emitFunc = device.Get("emit").As<Napi::Function>();
            emitFunc.Call({eventName});
        });
    }];
    
    // onEnterBackground
    [[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationWillTerminateNotification object:nil queue:nil usingBlock:^(NSNotification *notification) {
        MobileBackend::RunOnEventLoop([](napi_env env) {
            auto global = Napi::Env(env).Global();
            auto device = global.Get("device").As<Napi::Object>();
            if (device.IsUndefined())
                return;
            
            auto eventName = Napi::String::New(env, "willTerminate");
            auto emitFunc = device.Get("emit").As<Napi::Function>();
            emitFunc.Call({eventName});
        });
    }];
    
    // Start the notifier, wh
    MobileBackend::RunOnEventLoop([](napi_env env) {
        auto global = Napi::Env(env).Global();
        auto device = global.Get("device").As<Napi::Object>();
        if (device.IsUndefined())
          return;

        device.Set("getOrientation", Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            return [IModelJsHost getOrientation: info.Env()];
        }));
               
        device.Set("getBatteryState", Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            return [IModelJsHost getBatteryState: info.Env()];
        }));
        
        device.Set("downloadFile", Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            auto jsUrl = info[0].As<Napi::String>();
            auto&& jsCbRef = Napi::Persistent(info[1].As<Napi::Function>());
            
            NSURL *requestUrl = [NSURL URLWithString:[NSString stringWithUTF8String:jsUrl.Utf8Value().c_str()]];
            NSURLSessionDownloadTask *downloadTask = [[NSURLSession sharedSession]
             downloadTaskWithURL:requestUrl completionHandler:^(NSURL *location, NSURLResponse *response, NSError *error) {
                MobileBackend::RunOnEventLoop([&](napi_env env) {
                    auto undefined = Napi::Env(env).Undefined();
                     if (error) {
                        auto description = Napi::String::New(env, error.description.UTF8String);
                        jsCbRef.Call({undefined, description});
                    } else {
                        auto filePath = Napi::String::New(env, location.path.UTF8String);
                        jsCbRef.Call({filePath, undefined});
                    }
                });
            }];
            [downloadTask resume];
        }));

        device.Set("reconnect", Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            auto port = info[0].As<Napi::Number>().Uint32Value();
            MobileBackend::SetPort(port);
            
            [[IModelJsHost sharedInstance] reconnectWebView];
        }));
    });
}

- (void)reconnectWebView {
    dispatch_async(dispatch_get_main_queue(), ^() {
        if (_webView == nil) {
            return;
        }
        
        [_webView evaluateJavaScript:[NSString stringWithFormat:@"try { window._imodeljs_rpc_reconnect(%d); } catch (err) { console.log(err); }", [self getPort]] completionHandler:nil];
    });
}

- (void)registerWebView: (WKWebView*)view {
    _webView = view;
}
@end
