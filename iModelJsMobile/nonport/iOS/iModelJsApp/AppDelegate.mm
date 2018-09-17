//
//  AppDelegate.m
//  iModelJsApp
//
//  Created by Satyakam Khadilkar on 4/5/18.
//  Copyright © 2018 Bentley Systems. All rights reserved.
//

#import "AppDelegate.h"
#include <iModelJs/iModelJs.h>
#include <iModelJs/iModelJsServicesTier.h>
#import "XMLHTTPRequest.h"
@interface AppDelegate ()

@end

@implementation AppDelegate

BentleyApi::iModelJs::ServicesTier::UvHostPtr m_host;
extern void imodeljs_addon_setMobileResourcesDir(Utf8CP d);
extern void imodeljs_addon_setMobileTempDir(Utf8CP d);
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {

    // Start BackEnd libUv Thread
    using namespace BentleyApi::iModelJs;
    
    NSString *appFolderPath = [[NSBundle mainBundle] resourcePath];
    NSString *iModelJsNativePath = [appFolderPath stringByAppendingPathComponent:@"iModelJsNative"];
    NSString *iTempFolder = NSTemporaryDirectory();
    imodeljs_addon_setMobileResourcesDir(iModelJsNativePath.UTF8String);
    imodeljs_addon_setMobileTempDir(iTempFolder.UTF8String);
    m_host = new ServicesTier::UvHost;
    // Copy document folders
    NSString *documentsDirectory = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents/sample_documents"];
    if (![[NSFileManager defaultManager] fileExistsAtPath:documentsDirectory isDirectory:NULL])
        {
            NSString *sampleDocuments = [appFolderPath stringByAppendingPathComponent:@"Assets/assets/sample_documents"];
        NSError *copyError = nil;
        if (![[NSFileManager defaultManager] copyItemAtPath:sampleDocuments toPath:documentsDirectory error:&copyError])
            {
            NSLog(@"Error copying files: %@", [copyError localizedDescription]);
            }
        }
    
    while (!m_host->IsReady()) { ; }
    
    JSGlobalContextRef jsGlobalContext = JSContextGetGlobalContext(m_host->GetContext());
    JSContext* ctx =[JSContext contextWithJSGlobalContextRef:jsGlobalContext];
    // Setup xmlhttprequest for jscore
    _xmlHttpRequest = [XMLHttpRequest new];
    [_xmlHttpRequest extend:ctx];
    _windowTimers = [WTWindowTimers new];
    [_windowTimers extend:ctx];
    

    //[ctx evaluateScript:jsTemp];
    // [ctx :@"setTimeout(function(){ alert(\"Hello\"); }, 3000);"];
    m_host->PostToEventLoop([]()
        {
        NSString *jsTemp = [NSString stringWithFormat:@"imodeljsMobile.knownLocations.tempDir='%@'", NSTemporaryDirectory()];
        auto& runtime = ServicesTier::Host::GetInstance().GetJsRuntime();
        runtime.EvaluateScript (jsTemp.UTF8String);
            
            
        NSString* backendJsPath = [[ [NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Assets/MobileMain.js"];
        NSString* backendJs = [NSString stringWithContentsOfFile:backendJsPath encoding:NSUTF8StringEncoding error:NULL];
        auto evaluated = runtime.EvaluateScript (backendJs.UTF8String,
                        [NSURL fileURLWithPath:backendJsPath].absoluteString.UTF8String);
        BeAssert (evaluated.status == Js::EvaluateStatus::Success);
        });
    
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


@end
