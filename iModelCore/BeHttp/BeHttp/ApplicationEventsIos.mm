/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#if !defined (BENTLEYCONFIG_OS_APPLE_IOS)
    #error This file is only intended for iOS compilands!
#endif

#include "ApplicationEvents.h"
#include <queue>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationEventsManager::InitializeApplicationEventsListening()
    {
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];

    [center addObserverForName:UIApplicationDidEnterBackgroundNotification object:nil queue:mainQueue usingBlock:^(NSNotification *note)
        {
        for (auto listener : m_applicationEventsListeners)
            listener->_OnApplicationSentToBackground();
        }];

    [center addObserverForName:UIApplicationWillEnterForegroundNotification object:nil queue:mainQueue usingBlock:^(NSNotification *note)
        {
        for (auto listener : m_applicationEventsListeners)
            listener->_OnApplicationSentToForeground();
        }];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Vincas.Razma            07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationEventsManager::StartBackgroundTask(Utf8CP name, std::function<void()> task, std::function<void()> onExpired)
    {
    BeAssert(nullptr != name);
    BeAssert(nullptr != task);
    BeAssert(nullptr != onExpired);

    UIApplication* application = [UIApplication sharedApplication];
    NSString* nameStr = [NSString stringWithUTF8String:name];

    // Create task
    auto bgTaskPtr = std::make_shared<UIBackgroundTaskIdentifier>();
    *bgTaskPtr = [application beginBackgroundTaskWithName:nameStr expirationHandler:
        ^{
        onExpired();
        [application endBackgroundTask:*bgTaskPtr];
        *bgTaskPtr = UIBackgroundTaskInvalid;
        }];

    BeAssert(*bgTaskPtr != UIBackgroundTaskInvalid);

    // Run task code
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
        ^{
        task();
        [application endBackgroundTask:*bgTaskPtr];
        *bgTaskPtr = UIBackgroundTaskInvalid;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationEventsManager::SetNetworkActivityIndicatorVisible(bool visible)
    {
    dispatch_async(dispatch_get_main_queue(), ^
        {
        [[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:visible];
        });
    }