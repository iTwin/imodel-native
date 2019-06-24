/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <BeHttp/Http.h>
#include <functional>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass Utility functionality for tests
+---------------+---------------+---------------+---------------+---------------+------*/
struct Backdoor
    {
    typedef std::function<void(Utf8CP name, std::function<void()> task, std::function<void()> onExpired)> StartBackgroundTask;
    BEHTTP_EXPORT static void InitStartBackgroundTask(StartBackgroundTask callback);
    BEHTTP_EXPORT static void CallOnApplicationSentToBackground();
    BEHTTP_EXPORT static void CallOnApplicationSentToForeground();
    BEHTTP_EXPORT static void UninitializeCancelAllRequests();
    };

END_BENTLEY_HTTP_NAMESPACE
