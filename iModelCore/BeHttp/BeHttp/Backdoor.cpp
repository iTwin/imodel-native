/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/
#include "Backdoor.h"

#include "Curl/ThreadCurlHttpHandler.h"
#include <BeHttp/DefaultHttpHandler.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadCurlHttpHandler* GetCurlHttpHandler()
    {
    IHttpHandlerPtr handler = DefaultHttpHandler::GetInstance()->GetInternalHandler();
    ThreadCurlHttpHandler* curlHandler = dynamic_cast<ThreadCurlHttpHandler*>(handler.get());
    BeAssert(nullptr != curlHandler);
    return curlHandler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Backdoor::InitStartBackgroundTask(StartBackgroundTask callback)
    {
    GetCurlHttpHandler()->InitStartBackgroundTask(callback);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Backdoor::CallOnApplicationSentToBackground()
    {
    GetCurlHttpHandler()->_OnApplicationSentToBackground();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Backdoor::CallOnApplicationSentToForeground()
    {
    GetCurlHttpHandler()->_OnApplicationSentToForeground();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Backdoor::UninitializeCancelAllRequests()
    {
    GetCurlHttpHandler()->CancelAllRequests();
    }