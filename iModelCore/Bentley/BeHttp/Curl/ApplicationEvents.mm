/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/ApplicationEvents.mm $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#if !defined (__APPLE__)
    #error This file is only intended for iOS compilands!
#endif
#include <BeHttp/Http.h>
#include <queue>
#include "CurlHttpHandler.h"


USING_NAMESPACE_BENTLEY_HTTP
/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationEventsManager::InitializeApplicationEventsListening ()
    {
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    
    [center addObserverForName:UIApplicationWillEnterForegroundNotification object:nil queue:mainQueue usingBlock:^(NSNotification *note)
        {
        for (auto listener : s_applicationEventsListeners)
            listener->_OnApplicationResume ();
        }];
    }